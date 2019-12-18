/*
 * Defines machines for Linuxer DEMOSOC
 *
 * Copyright (c) 2017 Baohua Song <21cnbao@gmail.com>
 *
 * Licensed under GPLv2.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <asm/sizes.h>
#include <asm/mach/arch.h>

static void __init demosoc_init_late(void)
{
	if(of_machine_is_compatible("linuxer,demosoc-evb"))
		abc;
	if(of_machine_is_compatible("huawei,boardabc"))
		efg;

	if(of_device_is_compatible....)
		....
}

#ifdef CONFIG_ARCH_DEMOSOC
static const char *const demosoc_dt_match[] __initconst = {
	"linuxer,demosoc",
	NULL
};

DT_MACHINE_START(DEMOSOC_DT, "Linuxer DEMOSOC (Flattened Device Tree)")
	/* Maintainer: Barry Song <baohua@kernel.org> */
	.init_late	= demosoc_init_late,
	.dt_compat      = demosoc_dt_match,
MACHINE_END
#endif

