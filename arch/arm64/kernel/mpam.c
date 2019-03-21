/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Arm Ltd. */

#include <asm/mpam.h>

#include <linux/jump_label.h>
#include <linux/percpu.h>

DEFINE_STATIC_KEY_FALSE(arm64_system_has_mpam);
DEFINE_STATIC_KEY_FALSE(arm64_mpam_has_hcr);
DEFINE_PER_CPU(struct mpam_cpu_state, mpam_cpu_state);

u16 mpam_cpu_max_partids(void)
{
	u64 idr;

	idr = read_sanitised_ftr_reg(SYS_MPAMIDR_EL1);
	return idr & MPAMIDR_PARTID_MASK;
}

u16 mpam_cpu_max_pmgs(void)
{
	u64 idr;

	idr = read_sanitised_ftr_reg(SYS_MPAMIDR_EL1);
	return (idr & MPAMIDR_PMG_MASK) >> MPAMIDR_PMG_SHIFT;
}
