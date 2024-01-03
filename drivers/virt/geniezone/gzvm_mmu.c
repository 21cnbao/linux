// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/gzvm_drv.h>

/**
 * hva_to_pa_fast() - converts hva to pa in generic fast way
 * @hva: Host virtual address.
 *
 * Return: GZVM_PA_ERR_BAD for translation error
 */
u64 hva_to_pa_fast(u64 hva)
{
	struct page *page[1];
	u64 pfn;

	if (get_user_page_fast_only(hva, 0, page)) {
		pfn = page_to_phys(page[0]);
		put_page(page[0]);
		return pfn;
	}
	return GZVM_PA_ERR_BAD;
}

/**
 * hva_to_pa_slow() - converts hva to pa in a slow way
 * @hva: Host virtual address
 *
 * This function converts HVA to PA in a slow way because the target hva is not
 * yet allocated and mapped in the host stage1 page table, we cannot find it
 * directly from current page table.
 * Thus, we have to allocate it and this operation is much slower than directly
 * find via current page table.
 *
 * Context: This function may sleep
 * Return: PA or GZVM_PA_ERR_BAD for translation error
 */
u64 hva_to_pa_slow(u64 hva)
{
	struct page *page = NULL;
	u64 pfn = 0;
	int npages;

	npages = get_user_pages_unlocked(hva, 1, &page, 0);
	if (npages != 1)
		return GZVM_PA_ERR_BAD;

	if (page) {
		pfn = page_to_phys(page);
		put_page(page);
		return pfn;
	}

	return GZVM_PA_ERR_BAD;
}

static u64 __gzvm_gfn_to_pfn_memslot(struct gzvm_memslot *memslot, u64 gfn)
{
	u64 hva, pa;

	hva = gzvm_gfn_to_hva_memslot(memslot, gfn);

	pa = gzvm_hva_to_pa_arch(hva);
	if (pa != GZVM_PA_ERR_BAD)
		return PHYS_PFN(pa);

	pa = hva_to_pa_fast(hva);
	if (pa != GZVM_PA_ERR_BAD)
		return PHYS_PFN(pa);

	pa = hva_to_pa_slow(hva);
	if (pa != GZVM_PA_ERR_BAD)
		return PHYS_PFN(pa);

	return GZVM_PA_ERR_BAD;
}

/**
 * gzvm_gfn_to_pfn_memslot() - Translate gfn (guest ipa) to pfn (host pa),
 *			       result is in @pfn
 * @memslot: Pointer to struct gzvm_memslot.
 * @gfn: Guest frame number.
 * @pfn: Host page frame number.
 *
 * Return:
 * * 0			- Succeed
 * * -EFAULT		- Failed to convert
 */
int gzvm_gfn_to_pfn_memslot(struct gzvm_memslot *memslot, u64 gfn,
			    u64 *pfn)
{
	u64 __pfn;

	if (!memslot)
		return -EFAULT;

	__pfn = __gzvm_gfn_to_pfn_memslot(memslot, gfn);
	if (__pfn == GZVM_PA_ERR_BAD) {
		*pfn = 0;
		return -EFAULT;
	}

	*pfn = __pfn;

	return 0;
}

static int handle_single_demand_page(struct gzvm *vm, int memslot_id, u64 gfn)
{
	int ret;
	u64 pfn;

	ret = gzvm_gfn_to_pfn_memslot(&vm->memslot[memslot_id], gfn, &pfn);
	if (unlikely(ret))
		return -EFAULT;

	ret = gzvm_arch_map_guest(vm->vm_id, memslot_id, pfn, gfn, 1);
	if (unlikely(ret))
		return -EFAULT;

	return 0;
}

/**
 * gzvm_handle_page_fault() - Handle guest page fault, find corresponding page
 *                            for the faulting gpa
 * @vcpu: Pointer to struct gzvm_vcpu_run of the faulting vcpu
 *
 * Return:
 * * 0		- Success to handle guest page fault
 * * -EFAULT	- Failed to map phys addr to guest's GPA
 */
int gzvm_handle_page_fault(struct gzvm_vcpu *vcpu)
{
	struct gzvm *vm = vcpu->gzvm;
	int memslot_id;
	u64 gfn;

	gfn = PHYS_PFN(vcpu->run->exception.fault_gpa);
	memslot_id = gzvm_find_memslot(vm, gfn);
	if (unlikely(memslot_id < 0))
		return -EFAULT;

	return handle_single_demand_page(vm, memslot_id, gfn);
}
