/*
 * L1/L2 cache error detect driver for hip05
 * ARM doc: DDI0488D, Cortex-A57 Technical Reference Manual
 *
 * Copyright (c) 2015, Hisilicon Ltd. All rights reserved.
 *	Author: Hanjun Guo <hanjun.guo@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kdebug.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/spinlock.h>

#include "edac_core.h"

/* For specific meaning, please refer to A57 Technical Reference Manual */
#define A57_CPMUERRSR_FATAL(a)	((a) & (1UL << 63))
#define A57_CPUMERRSR_OTHER(a)	(((a) >> 40) & 0xff)
#define A57_CPUMERRSR_REPT(a)	(((a) >> 32) & 0xff)
#define A57_CPUMERRSR_VALID(a)	((a) & (1 << 31))
#define A57_CPUMERRSR_RAMID(a)	(((a) >> 24) & 0x7f)
#define A57_CPUMERRSR_BANK(a)	(((a) >> 18) & 0x1f)
#define A57_CPUMERRSR_INDEX(a)	((a) & 0x1ffff)

#define A57_L2MERRSR_FATAL(a)	((a) & (1UL << 63))
#define A57_L2MERRSR_OTHER(a)	(((a) >> 40) & 0xff)
#define A57_L2MERRSR_REPT(a)	(((a) >> 32) & 0xff)
#define A57_L2MERRSR_VALID(a)	((a) & (1 << 31))
#define A57_L2MERRSR_RAMID(a)	(((a) >> 24) & 0x7f)
#define A57_L2MERRSR_CPUID(a)	(((a) >> 18) & 0x0f)
#define A57_L2MERRSR_INDEX(a)	((a) & 0x1ffff)

#define L2ECTLR_INT_ERR		(1 << 30)
#define L2ECTLR_EXT_ERR		(1 << 29)

#define L1_CACHE		0
#define L2_CACHE		1

#define A57_L1_CE			0
#define A57_L1_UE			1
#define A57_L2_CE			2
#define A57_L2_UE			3
#define L2_EXT_UE			4

#define EDAC_CPU	"hip05"

enum error_type {
	SBE, /* Single-bit error */
	DBE, /* Double-bit error */
};

const char *err_name[] = {
	"Single-bit",
	"Double-bit",
};

struct erp_drvdata {
	struct edac_device_ctl_info *edev_ctl;

	/* notify called when SEI happened in bad_mode() */
	struct notifier_block nb_die;
};

static struct erp_drvdata *ecc_handler_drvdata;

struct erp_local_data {
	struct erp_drvdata *drv;
	enum error_type err;
};

struct errors_edac {
	const char * const msg;
	void (*func)(struct edac_device_ctl_info *edac_dev,
			int inst_nr, int block_nr, const char *msg);
};

static const struct errors_edac errors[] = {
	{"A57 L1 Correctable Error", edac_device_handle_ce },
	{"A57 L1 Uncorrectable Error", edac_device_handle_ue },
	{"A57 L2 Correctable Error", edac_device_handle_ce },
	{"A57 L2 Uncorrectable Error", edac_device_handle_ue },
	{"L2 External Error", edac_device_handle_ue },
};

static inline u64 read_l2merrsr_el1(void)
{
	u64 val;
	asm("mrs %0, s3_1_c15_c2_3" : "=r" (val));
	return val;
}

static inline u32 read_l2ectlr_el1(void)
{
	u32 val;
	asm("mrs %0, s3_1_c11_c0_3" : "=r" (val));
	return val;
}

static inline u64 read_cpumerrsr_el1(void)
{
	u64 val;
	asm("mrs %0, s3_1_c15_c2_2" : "=r" (val));
	return val;
}

static inline void write_l2merrsr_el1(u64 val)
{
	asm("msr s3_1_c15_c2_3, %0" : : "r" (val));
}

static inline void write_l2ectlr_el1(u32 val)
{
	asm("msr s3_1_c11_c0_3, %0" : : "r" (val));
}

static inline void write_cpumerrsr_el1(u64 val)
{
	asm("msr s3_1_c15_c2_2, %0" : : "r" (val));
}

/* check and parse L1 cache ECC */
static void ca57_parse_cpumerrsr(struct erp_local_data *ed)
{
	u64 cpumerrsr;
	int bank;

	cpumerrsr = read_cpumerrsr_el1();

	if (!A57_CPUMERRSR_VALID(cpumerrsr))
		return;

	ed->err = SBE;
	/* TODO: need to confirm with Hisilicon people that how to identify double-bit ECC */
	if (A57_CPMUERRSR_FATAL(cpumerrsr))
		ed->err = DBE;

	edac_printk(KERN_CRIT, EDAC_CPU, "Cortex A57 CPU%d L1 %s Error detected\n",
					 smp_processor_id(), err_name[ed->err]);
	edac_printk(KERN_CRIT, EDAC_CPU, "CPUMERRSR value = %#llx\n", cpumerrsr);

	bank = A57_CPUMERRSR_BANK(cpumerrsr);

	switch (A57_CPUMERRSR_RAMID(cpumerrsr)) {
	case 0x0:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L1 Instruction tag RAM bank %d\n", bank);
		break;
	case 0x1:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L1 Instruction data RAM bank %d\n", bank);
		break;
	case 0x8:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L1 Data tag RAM bank %d\n", bank);
		break;
	case 0x9:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L1 Data data RAM bank %d\n", bank);
		break;
	case 0x18:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"TLB RAM bank %d\n", bank);
		break;
	default:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"Error in unknown RAM ID: %d\n",
				(int) A57_CPUMERRSR_RAMID(cpumerrsr));
		break;
	}

	edac_printk(KERN_CRIT, EDAC_CPU, "Repeated error count: %d\n",
					 (int) A57_CPUMERRSR_REPT(cpumerrsr));
	edac_printk(KERN_CRIT, EDAC_CPU, "Other error count: %d\n",
					 (int) A57_CPUMERRSR_OTHER(cpumerrsr));

	if (ed->err == SBE)
		errors[A57_L1_CE].func(ed->drv->edev_ctl, smp_processor_id(),
					L1_CACHE, errors[A57_L1_CE].msg);
	else if (ed->err == DBE)
		errors[A57_L1_UE].func(ed->drv->edev_ctl, smp_processor_id(),
					L1_CACHE, errors[A57_L1_UE].msg);
	write_cpumerrsr_el1(0);
}

/* check and parse L2 cache ECC */
static void ca57_parse_l2merrsr(struct erp_local_data *ed)
{
	u64 l2merrsr;
	int cpuid;

	l2merrsr = read_l2merrsr_el1();

	if (!A57_L2MERRSR_VALID(l2merrsr))
		return;

	ed->err = SBE;
	if (A57_L2MERRSR_FATAL(l2merrsr))
		ed->err = DBE;

	edac_printk(KERN_CRIT, EDAC_CPU, "CortexA57 L2 %s Error detected\n",
							err_name[ed->err]);
	edac_printk(KERN_CRIT, EDAC_CPU, "L2MERRSR value = %#llx\n", l2merrsr);

	/* cpuid inside a cluster */
	cpuid = A57_L2MERRSR_CPUID(l2merrsr);

	switch (A57_L2MERRSR_RAMID(l2merrsr)) {
	case 0x10:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L2 tag RAM cpu %d way is %d\n",
				cpuid / 2, cpuid % 2);
		break;
	case 0x11:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L2 data RAM cpu %d bank is %d\n",
				cpuid / 2, cpuid % 2);
		break;
	case 0x12:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"SCU snoop tag RAM bank is %d\n", cpuid);
		break;
	case 0x14:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L2 dirty RAM cpu %d bank is %d\n",
				cpuid / 2, cpuid % 2);
		break;
	case 0x18:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"L2 inclusion PF RAM bank is %d\n", cpuid);
		break;
	default:
		edac_printk(KERN_CRIT, EDAC_CPU,
				"Error in unknown RAM ID: %d\n",
				(int) A57_L2MERRSR_RAMID(l2merrsr));
		break;
	}

	edac_printk(KERN_CRIT, EDAC_CPU, "Repeated error count: %d\n",
					 (int) A57_L2MERRSR_REPT(l2merrsr));
	edac_printk(KERN_CRIT, EDAC_CPU, "Other error count: %d\n",
					 (int) A57_L2MERRSR_OTHER(l2merrsr));

	if (ed->err == SBE) {
		errors[A57_L2_CE].func(ed->drv->edev_ctl, smp_processor_id(),
					L2_CACHE, errors[A57_L2_CE].msg);
	} else if (ed->err == DBE) {
		errors[A57_L2_UE].func(ed->drv->edev_ctl, smp_processor_id(),
					L2_CACHE, errors[A57_L2_UE].msg);
	}
	write_l2merrsr_el1(0);
}

/* check and parse L2 extend control register */
static void hip05_check_L2_extended_ctl_reg(void *info)
{
	struct erp_drvdata *drv = info;
	u32 l2ectlr;

	l2ectlr = read_l2ectlr_el1();

	/* Acklowledge internal error in L2ECTLR */
	if (l2ectlr & L2ECTLR_INT_ERR) {
		l2ectlr &= ~L2ECTLR_INT_ERR;
		write_l2ectlr_el1(l2ectlr);
	}

	if (l2ectlr & L2ECTLR_EXT_ERR) {
		edac_printk(KERN_CRIT, EDAC_CPU,
		    "L2 external error detected by CPU%d\n",
		    smp_processor_id());

		errors[L2_EXT_UE].func(drv->edev_ctl, smp_processor_id(),
				       L2_CACHE, errors[L2_EXT_UE].msg);

		l2ectlr &= ~L2ECTLR_EXT_ERR;
		write_l2ectlr_el1(l2ectlr);
	}
}

static DEFINE_SPINLOCK(local_handler_lock);

/* Check L1/L2 cache error on single CPU */
static void hip05_check_cache_error(void *info)
{
	struct erp_local_data *errdata = info;
	unsigned long flags;

	spin_lock_irqsave(&local_handler_lock, flags);
	ca57_parse_cpumerrsr(errdata);
	ca57_parse_l2merrsr(errdata);
	hip05_check_L2_extended_ctl_reg(errdata->drv);
	spin_unlock_irqrestore(&local_handler_lock, flags);
}

/* Check all the L1/L2 cache ECC errors in the system */
static void hip05_check_system_cache_ecc(void)
{
	struct erp_local_data errdata;

	if (ecc_handler_drvdata) {
		errdata.drv = ecc_handler_drvdata;
		on_each_cpu(hip05_check_cache_error, &errdata, 1);
	}
}

static int hip05_cache_errors_die_notify(struct notifier_block *this,
			      unsigned long event, void *ptr)
{
	hip05_check_system_cache_ecc();

	return NOTIFY_OK;
}

static void hip05_monitor_cache_errors(struct edac_device_ctl_info *edev)
{
	hip05_check_system_cache_ecc();
}

static int hip05_cpu_erp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv;
	u32 poll_msec;
	int rc;

	drv = devm_kzalloc(dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;

	drv->edev_ctl = edac_device_alloc_ctl_info(0, "cpu",
					num_possible_cpus(), "L", 3, 1, NULL, 0,
					edac_device_alloc_index());
	if (!drv->edev_ctl) {
		kfree(drv);
		return -ENOMEM;
	}

	/*
	 * hip05 has no interrupt triggered when single bit of ECC error
	 * happened, so we must use the poll mechanism to get the single
	 * bit error and count it.
	 */
	rc = of_property_read_u32(pdev->dev.of_node, "poll-delay-ms",
							&poll_msec);
	if (!rc)
		drv->edev_ctl->poll_msec = poll_msec;
	else
		drv->edev_ctl->poll_msec = 10000; /* default 10s */

	drv->edev_ctl->edac_check = hip05_monitor_cache_errors;
	drv->edev_ctl->dev = dev;
	drv->edev_ctl->mod_name = dev_name(dev);
	drv->edev_ctl->dev_name = dev_name(dev);
	drv->edev_ctl->ctl_name = "cache";

	/*
	 * if UE happened, just panic the system, but this may
	 * useless, because SEI will take over it
	 */
	drv->edev_ctl->panic_on_ue = 1;

	rc = edac_device_add_device(drv->edev_ctl);
	if (rc)
		goto out_mem;

	/*
	 * hip05 triggers SEI when Double bits ECC error happens,
	 * so that will enter bad_mode() which will call the die
	 * notify, just hook the edac handler here to let people
	 * know the detail causes of death
	 */
	drv->nb_die.notifier_call = hip05_cache_errors_die_notify;
	register_die_notifier(&drv->nb_die);

	ecc_handler_drvdata = drv;
	return 0;

out_mem:
	edac_device_free_ctl_info(drv->edev_ctl);
	kfree(drv);
	return rc;
}

static const struct of_device_id hip05_cpu_erp_match_table[] = {
	{ .compatible = "hisi,hip05-edac-cache" },
	{ }
};

static struct platform_driver hip05_cpu_erp_driver = {
	.probe = hip05_cpu_erp_probe,
	.driver = {
		.name = "hip05_cpu_cache_erp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hip05_cpu_erp_match_table),
	},
};

static int __init hip05_cpu_erp_init(void)
{
	return platform_driver_register(&hip05_cpu_erp_driver);
}
device_initcall_sync(hip05_cpu_erp_init);
