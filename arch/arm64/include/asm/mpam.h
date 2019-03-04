/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Arm Ltd. */

#ifndef __ASM__MPAM_H
#define __ASM__MPAM_H

#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/jump_label.h>
#include <linux/percpu.h>
#include <linux/sched.h>

#include <asm/cpucaps.h>
#include <asm/cpufeature.h>
#include <asm/mpam_hwdef.h>
#include <asm/sysreg.h>

DECLARE_STATIC_KEY_FALSE(arm64_system_has_mpam);
DECLARE_STATIC_KEY_FALSE(arm64_mpam_has_hcr);

struct mpam_cpu_state {
	u64	current_regval;
	u32	default_partid;
	u32	default_pmg;
};

static inline bool system_supports_mpam(void)
{
	if (IS_ENABLED(CONFIG_ARM64_MPAM))
		return static_branch_unlikely(&arm64_system_has_mpam);

	return false;
}


/* Enable system wide support */
static inline void __enable_system_mpam_cpuslocked(void)
{
	lockdep_assert_cpus_held();

	if (IS_ENABLED(CONFIG_ARM64_MPAM))
		static_branch_enable_cpuslocked(&arm64_system_has_mpam);
}

/* check whether all CPUs have MPAM support */
static inline bool mpam_cpus_have_feature(void)
{
	if (IS_ENABLED(CONFIG_ARM64_MPAM))
		return cpus_have_const_cap(ARM64_MPAM);
	return false;
}

/* check whether all CPUs have MPAM virtualisation support */
static inline bool mpam_cpus_have_mpam_hcr(void)
{
	if (IS_ENABLED(CONFIG_ARM64_MPAM))
		return static_branch_unlikely(&arm64_mpam_has_hcr);
	return false;
}

/* enable MPAM virtualisation support */
static inline void __init __enable_mpam_hcr(void)
{
	if (IS_ENABLED(CONFIG_ARM64_MPAM))
		static_branch_enable(&arm64_mpam_has_hcr);
}

/*
 * The MPAMy_ELx registers have two 16bit PARTIDs, and two 8bit PMGs.
 * We store these pairs in the hardware register format, in task_struct's
 * closid and rmid fields.
 */
static inline u32 mpam_partid_to_task(u16 partid_d, u16 partid_i)
{
	return ((u64)partid_d << MPAM_SYSREG_PARTID_D_SHIFT) |
	       ((u64)partid_i << MPAM_SYSREG_PARTID_I_SHIFT);
}

static inline u32 mpam_pmg_to_task(u8 pmg_d, u8 pmg_i)
{
	return ((u64)pmg_d << (MPAM_SYSREG_PMG_D_SHIFT - 32)) |
	       ((u64)pmg_i << (MPAM_SYSREG_PMG_I_SHIFT - 32));
}

#ifdef CONFIG_ARCH_HAS_CPU_RESCTRL
static inline void mpam_set_task_partid(struct task_struct *tsk, u16 partid_d,
					u16 partid_i)
{
	u32 val = mpam_partid_to_task(partid_d, partid_i);

	WRITE_ONCE(tsk->closid, val);
}

static inline void mpam_set_task_pmg(struct task_struct *tsk, u8 pmg_d,
				     u8 pmg_i)
{
	u32 val = mpam_pmg_to_task(pmg_d, pmg_i);

	WRITE_ONCE(tsk->rmid, val);
}

DECLARE_PER_CPU(struct mpam_cpu_state, mpam_cpu_state);

/* Update the CPU's MPAM config for user space, called from context switch */
static inline void _mpam_thread_switch(struct task_struct *next)
{
	u64 new_regval;
	u32 pmg = READ_ONCE(next->rmid);
	u32 partid = READ_ONCE(next->closid);
	struct mpam_cpu_state *local_state = this_cpu_ptr(&mpam_cpu_state);

	if (!partid)
		partid = local_state->default_partid;
	if (!pmg)
		pmg = local_state->default_pmg;

	new_regval = partid | ((u64)pmg << 32);

	if (local_state->current_regval != new_regval) {
		local_state->current_regval = new_regval;
		write_sysreg_s(new_regval, SYS_MPAM0_EL1);
	}
}

static inline void mpam_set_default_partid(int cpu, u16 partid_d, u16 partid_i)
{
	u32 val = mpam_partid_to_task(partid_d, partid_i);

	per_cpu(mpam_cpu_state, cpu).default_partid = val;
}

static inline void mpam_set_default_pmg(int cpu, u8 pmg_d, u8 pmg_i)
{
	u32 val = mpam_pmg_to_task(pmg_d, pmg_i);

	per_cpu(mpam_cpu_state, cpu).default_pmg = val;
}

static inline u64 _mpam_get_current_regval(void)
{
	struct mpam_cpu_state *local_state = this_cpu_ptr(&mpam_cpu_state);

	return local_state->current_regval;
}

#else
static inline void mpam_set_task_partid(struct task_struct *t, u16 d, u16 i) { }
static inline void mpam_set_task_pmg(struct task_struct *t, u8 d, u8 i) { }
static inline void mpam_set_default_partid(int c, u16 d, u16 i) { }
static inline void mpam_set_default_pmg(int c, u8 d, u8 i) { }
static inline void _mpam_thread_switch(struct task_struct *next) { }
static inline u64 _mpam_get_current_regval(void) { return 0; }
#endif /* CONFIG_ARCH_HAS_CPU_RESCTRL */

u16 mpam_cpu_max_partids(void);
u16 mpam_cpu_max_pmgs(void);

#endif /* __ASM__MPAM_H */
