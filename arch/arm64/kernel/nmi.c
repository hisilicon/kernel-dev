/*
 * Huawei driver
 *
 * Copyright (C)
 * Author: Huawei majun258@huawei.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/kernel.h>
#include <asm/smc_call.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/nmi.h>
#include <asm/smp_plat.h>

/* This value defined by the SMC Call manual*/
#define SMC_FUNCTION_REGISTE_ID 0x83000000
#define SMC_CALLBACK_ID 0x83000001
#define SMC_NMI_STATE_ID 0x83000002
#define SMC_NMI_TIMEOUT_ID 0x83000003

struct hisi_nmi_watchdog cpu_nmi[NR_CPUS];

static void smc_callback_func(void)
{
	struct smc_param64 param = { 0 };

	param.a0 = SMC_CALLBACK_ID;
	param.a1 = 0;
	param.a2 = 0;

	pr_debug("%s--start[%llx][%llx][%llx]:cpu: %d\n", __func__,
		 param.a0, param.a1, param.a2, smp_processor_id());

	smc_call(param.a0, param.a1, param.a2);
}

static void hisi_nmi_handler(void)
{
	int cpu, ret;
	int (*nmi_handler)(void);

	pr_debug("%s: cpu %d\n", __func__, smp_processor_id());

	cpu = smp_processor_id();
	nmi_handler = cpu_nmi[cpu].nmi_handler;
	if (nmi_handler) {
		ret = nmi_handler();
		if (ret)
			pr_err("nmi handler return error\n");
	}
	smc_callback_func();
}

static void hisi_register_nmi_handler(void *func)
{
	struct smc_param64 param = { 0 };

	param.a0 = SMC_FUNCTION_REGISTE_ID;
	param.a1 = (uint64_t)func;
	param.a2 = 0;

	pr_debug("%s--start[%llx][%llx][%llx]: cpu: %d\n", __func__,
		 param.a0, param.a1, param.a2, smp_processor_id());

	smc_call(param.a0, param.a1, param.a2);
}

int register_nmi_handler(int cpuid, int (*func)(void))
{
	static int register_handler = 0;

	if (cpuid >= NR_CPUS) {
		pr_err("the cpuid %d is not correctly\n", cpuid);
		return -1;
	}

	cpu_nmi[cpuid].id = cpuid;

	if (cpu_nmi[cpuid].nmi_handler)
		pr_debug("cpu: %d cover the old handler\n", cpuid);

	cpu_nmi[cpuid].nmi_handler = func;

	if (register_handler == 0) {
		hisi_register_nmi_handler(hisi_nmi_handler);
		register_handler = 1;
	}
	
	return 0;
}
EXPORT_SYMBOL(register_nmi_handler);

void nmi_set_active_state(int cpuid, bool state)
{
	struct smc_param64 param = { 0 };
	u64 cpu_hwid = cpu_logical_map(cpuid);

	param.a0 = SMC_NMI_STATE_ID;
	param.a1 = cpu_hwid;
	param.a2 = state;

	pr_debug("%s--start[%llx][%llx][%llx]:cpu: %d\n", __func__,
		 param.a0, param.a1, param.a2, smp_processor_id());

	cpu_nmi[cpuid].state = state;
	smc_call(param.a0, param.a1, param.a2);
}
EXPORT_SYMBOL(nmi_set_active_state);

void nmi_set_timeout(int cpuid, int time)
{
	struct smc_param64 param = { 0 };
	u64 cpu_hwid = cpu_logical_map(cpuid);

	param.a0 = SMC_NMI_TIMEOUT_ID;
	param.a1 = cpu_hwid;
	param.a2 = time;

	pr_debug("%s--start[%llx][%llx][%llx]:cpu: %d\n", __func__,
		 param.a0, param.a1, param.a2, smp_processor_id());

	cpu_nmi[cpuid].timeout = time;
	smc_call(param.a0, param.a1, param.a2);
}
EXPORT_SYMBOL(nmi_set_timeout);

void touch_nmi_watchdog(void)
{
	/* add your function here to feed watchdog */
	touch_softlockup_watchdog();
}
EXPORT_SYMBOL(touch_nmi_watchdog);

