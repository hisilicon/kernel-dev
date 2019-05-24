/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

#include <linux/bug.h>
#include <linux/jump_label.h>
#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/resctrl_types.h>

#include <asm/processor.h>

typedef struct { u32 val; } hw_closid_t;
#define as_hwclosid_t(x)	((hw_closid_t){(x)})
#define hwclosid_val(x)		(x.val)

#define IA32_PQR_ASSOC	0x0c8f

#define RMID_VAL_ERROR			BIT_ULL(63)
#define RMID_VAL_UNAVAIL		BIT_ULL(62)
#define MBM_CNTR_WIDTH			24

extern bool rdt_alloc_capable;
extern bool rdt_mon_capable;

DECLARE_STATIC_KEY_FALSE(rdt_enable_key);
DECLARE_STATIC_KEY_FALSE(rdt_alloc_enable_key);
DECLARE_STATIC_KEY_FALSE(rdt_mon_enable_key);

/*
 * Get the system wide closid/rmid limits. The number of closids should not be
 * affected by CDP.
 */
u32 resctrl_arch_system_num_closid(void);
u32 resctrl_arch_system_num_rmid(void);

void resctrl_arch_reset_resources(void);

/**
 * struct resctrl_pqr_state - State cache for the PQR MSR
 * @cur_rmid:		The cached Resource Monitoring ID
 * @cur_closid:		The cached Class Of Service ID
 * @default_rmid:	The user assigned Resource Monitoring ID
 * @default_closid:	The user assigned cached Class Of Service ID
 *
 * The upper 32 bits of IA32_PQR_ASSOC contain closid and the
 * lower 10 bits rmid. The update to IA32_PQR_ASSOC always
 * contains both parts, so we need to cache them. This also
 * stores the user configured per cpu CLOSID and RMID.
 *
 * The cache also helps to avoid pointless updates if the value does
 * not change.
 */
struct resctrl_pqr_state {
	u32			cur_rmid;
	u32			cur_closid;
	u32			default_rmid;
	u32			default_closid;
};

DECLARE_PER_CPU(struct resctrl_pqr_state, pqr_state);

static inline bool resctrl_arch_alloc_capable(void)
{
	return rdt_alloc_capable;
}

static inline void resctrl_arch_enable_alloc(void)
{
	static_branch_enable_cpuslocked(&rdt_alloc_enable_key);
	static_branch_inc_cpuslocked(&rdt_enable_key);
}

static inline void resctrl_arch_disable_alloc(void)
{
	static_branch_disable_cpuslocked(&rdt_alloc_enable_key);
	static_branch_dec_cpuslocked(&rdt_enable_key);
}

static inline bool resctrl_arch_mon_capable(void)
{
	return rdt_mon_capable;
}

static inline void resctrl_arch_enable_mon(void)
{
	static_branch_enable_cpuslocked(&rdt_mon_enable_key);
	static_branch_inc_cpuslocked(&rdt_enable_key);
}

static inline void resctrl_arch_disable_mon(void)
{
	static_branch_disable_cpuslocked(&rdt_mon_enable_key);
	static_branch_dec_cpuslocked(&rdt_enable_key);
}

static inline u32 resctrl_arch_max_rmid_threshold(void)
{
	return (boot_cpu_data.x86_cache_size * 1024);
}

static inline void resctrl_arch_set_closid(struct task_struct *tsk,
					   hw_closid_t closid_code,
					   hw_closid_t closid_data)
{
	u32 c = hwclosid_val(closid_code);
	u32 d = hwclosid_val(closid_data);

	/* For CDP the code/data closid must be adjacent even/odd pairs */
	if (c != d) {
		WARN_ON_ONCE(d + 1 != c);
		tsk->closid = d >> 1;
	} else {
		tsk->closid = d;
	}
}

static inline bool resctrl_arch_match_closid(struct task_struct *tsk,
					     hw_closid_t closid_code,
					     hw_closid_t closid_data)
{
	u32 c = hwclosid_val(closid_code);
	u32 d = hwclosid_val(closid_data);

	/* For CDP the code/data closid must be adjacent even/odd pairs */
	if (c != d) {
		WARN_ON_ONCE(d + 1 != c);
		return tsk->closid == d >> 1;
	} else {
		return tsk->closid == d;
	}
}

static inline void resctrl_arch_set_rmid(struct task_struct *tsk, u32 rmid)
{
	tsk->rmid = rmid;
}

static inline bool resctrl_arch_match_rmid(struct task_struct *tsk, u32 rmid)
{
	return tsk->rmid == rmid;
}

static inline void resctrl_arch_set_cpu_default_closid(int cpu,
						       hw_closid_t closid_code,
						       hw_closid_t closid_data)
{
	u32 c = hwclosid_val(closid_code);
	u32 d = hwclosid_val(closid_data);

	/* For CDP the code/data closid must be adjacent even/odd pairs */
	if (c != d) {
		WARN_ON_ONCE(d + 1 != c);
		per_cpu(pqr_state.default_closid, cpu) = d >> 1;
	} else {
		per_cpu(pqr_state.default_closid, cpu) = d;
	}
}

static inline void resctrl_arch_set_cpu_default_rmid(int cpu, u32 rmid)
{
	per_cpu(pqr_state.default_rmid, cpu) = rmid;
}

static inline rmid_idx_t resctrl_arch_rmid_idx_encode(hw_closid_t closid,
						      u32 rmid)
{
	/* On x86, the closid is not relevant to the rmid space */
	return rmid;
}

static inline void resctrl_arch_rmid_idx_decode(rmid_idx_t idx,
						hw_closid_t *closid,
						u32 *rmid)
{
	*closid = as_hwclosid_t(0);
	*rmid = idx;
}

static inline rmid_idx_t resctrl_arch_num_rmid_idx(void)
{
	/* On x86, rmid is the rmid_idx */
	return resctrl_arch_system_num_rmid();
}

/*
 * __resctrl_sched_in() - Writes the task's CLOSid/RMID to IA32_PQR_MSR
 *
 * Following considerations are made so that this has minimal impact
 * on scheduler hot path:
 * - This will stay as no-op unless we are running on an Intel SKU
 *   which supports resource control or monitoring and we enable by
 *   mounting the resctrl file system.
 * - Caches the per cpu CLOSid/RMID values and does the MSR write only
 *   when a task with a different CLOSid/RMID is scheduled in.
 * - We allocate RMIDs/CLOSids globally in order to keep this as
 *   simple as possible.
 * Must be called with preemption disabled.
 */
static inline void __resctrl_sched_in(void)
{
	struct resctrl_pqr_state *state = this_cpu_ptr(&pqr_state);
	u32 closid = state->default_closid;
	u32 rmid = state->default_rmid;

	/*
	 * If this task has a closid/rmid assigned, use it.
	 * Else use the closid/rmid assigned to this cpu.
	 */
	if (static_branch_likely(&rdt_alloc_enable_key)) {
		if (current->closid)
			closid = current->closid;
	}

	if (static_branch_likely(&rdt_mon_enable_key)) {
		if (current->rmid)
			rmid = current->rmid;
	}

	if (closid != state->cur_closid || rmid != state->cur_rmid) {
		state->cur_closid = closid;
		state->cur_rmid = rmid;
		wrmsr(IA32_PQR_ASSOC, rmid, closid);
	}
}

static inline void resctrl_sched_in(void)
{
	if (IS_ENABLED(CONFIG_X86_CPU_RESCTRL) &&
	    static_branch_likely(&rdt_enable_key))
		__resctrl_sched_in();
}

bool resctrl_arch_is_llc_occupancy_enabled(void);
bool resctrl_arch_is_mbm_total_enabled(void);
bool resctrl_arch_is_mbm_local_enabled(void);

u64 resctrl_arch_get_prefetch_disable_bits(void);
int resctrl_arch_pseudo_lock_fn(void *_plr);
int resctrl_arch_measure_cycles_lat_fn(void *_plr);
int resctrl_arch_measure_l2_residency(void *_plr);
int resctrl_arch_measure_l3_residency(void *_plr);

#endif /* _ASM_RESCTRL_H_ */
