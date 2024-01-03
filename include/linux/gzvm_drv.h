/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __GZVM_DRV_H__
#define __GZVM_DRV_H__

#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/gzvm.h>

/*
 * For the normal physical address, the highest 12 bits should be zero, so we
 * can mask bit 62 ~ bit 52 to indicate the error physical address
 */
#define GZVM_PA_ERR_BAD (0x7ffULL << 52)

#define INVALID_VM_ID   0xffff

/*
 * These are the definitions of APIs between GenieZone hypervisor and driver,
 * there's no need to be visible to uapi. Furthermore, we need GenieZone
 * specific error code in order to map to Linux errno
 */
#define NO_ERROR                (0)
#define ERR_NO_MEMORY           (-5)
#define ERR_NOT_SUPPORTED       (-24)
#define ERR_NOT_IMPLEMENTED     (-27)
#define ERR_FAULT               (-40)

/*
 * The following data structures are for data transferring between driver and
 * hypervisor, and they're aligned with hypervisor definitions
 */
#define GZVM_MAX_MEM_REGION	10

/* struct mem_region_addr_range - Identical to ffa memory constituent */
struct mem_region_addr_range {
	/* the base IPA of the constituent memory region, aligned to 4 kiB */
	__u64 address;
	/* the number of 4 kiB pages in the constituent memory region. */
	__u32 pg_cnt;
	__u32 reserved;
};

struct gzvm_memory_region_ranges {
	__u32 slot;
	__u32 constituent_cnt;
	__u64 total_pages;
	__u64 gpa;
	struct mem_region_addr_range constituents[];
};

/* struct gzvm_memslot - VM's memory slot descriptor */
struct gzvm_memslot {
	u64 base_gfn;			/* begin of guest page frame */
	unsigned long npages;		/* number of pages this slot covers */
	unsigned long userspace_addr;	/* corresponding userspace va */
	struct vm_area_struct *vma;	/* vma related to this userspace addr */
	u32 flags;
	u32 slot_id;
};

struct gzvm {
	/* userspace tied to this vm */
	struct mm_struct *mm;
	struct gzvm_memslot memslot[GZVM_MAX_MEM_REGION];
	/* lock for list_add*/
	struct mutex lock;
	struct list_head vm_list;
	u16 vm_id;
};

int gzvm_dev_ioctl_create_vm(unsigned long vm_type);

int gzvm_err_to_errno(unsigned long err);

void gzvm_destroy_all_vms(void);

/* arch-dependant functions */
int gzvm_arch_probe(void);
int gzvm_arch_set_memregion(u16 vm_id, size_t buf_size,
			    phys_addr_t region);
int gzvm_arch_create_vm(unsigned long vm_type);
int gzvm_arch_destroy_vm(u16 vm_id);

u64 gzvm_hva_to_pa_arch(u64 hva);
u64 hva_to_pa_fast(u64 hva);
u64 hva_to_pa_slow(u64 hva);
int gzvm_gfn_to_pfn_memslot(struct gzvm_memslot *memslot, u64 gfn, u64 *pfn);
u64 gzvm_gfn_to_hva_memslot(struct gzvm_memslot *memslot, u64 gfn);

#endif /* __GZVM_DRV_H__ */
