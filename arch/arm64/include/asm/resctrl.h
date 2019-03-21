/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

#include <linux/arm_mpam.h>

#include <asm/barrier.h>
#include <asm/mpam.h>

/* This is the MPAM->resctrl<-arch glue. */

typedef struct { u16 val; } hw_closid_t;
#define as_hwclosid_t(x)	((hw_closid_t){(x)})
#define hwclosid_val(x)		(x.val)

#define resctrl_arch_get_resource(l)	mpam_resctrl_get_resource(l)

#define resctrl_arch_alloc_capable()	mpam_resctrl_alloc_capable()
#define resctrl_arch_mon_capable()	mpam_resctrl_mon_capable()
#define resctrl_arch_is_mbm_total_enabled()	mpam_resctrl_mbm_total_enabled()
#define resctrl_arch_is_mbm_local_enabled()	false
#define resctrl_arch_is_llc_occupancy_enabled()	mpam_resctrl_llc_occupancy_enabled()

static inline void resctrl_arch_enable_alloc(void) { }
static inline void resctrl_arch_disable_alloc(void) { }
static inline void resctrl_arch_enable_mon(void) { }
static inline void resctrl_arch_disable_mon(void) { }

static inline u32 resctrl_arch_system_num_rmid(void)
{
	return mpam_resctrl_num_rmid();
}

static inline u32 resctrl_arch_system_num_closid(void)
{
	return mpam_resctrl_num_closid();
}

static inline void resctrl_sched_in(void)
{
	_mpam_thread_switch(current);

	/*
	 * We depend on the dsb() in __switch_to() to finish any writes
	 * that had the old mpam partid. This allows us to re-allocate that
	 * partid after the last task has finished using it.
	 */
	dsb(ishst);
}

static inline u32 resctrl_arch_max_rmid_threshold(void)
{
	return mpam_resctrl_llc_cache_size();
}

static inline void resctrl_arch_set_closid(struct task_struct *tsk,
					   hw_closid_t closid_code,
					   hw_closid_t closid_data)
{
	mpam_set_task_partid(tsk, hwclosid_val(closid_data),
			     hwclosid_val(closid_code));
}

#ifdef CONFIG_ARCH_HAS_CPU_RESCTRL
static inline bool resctrl_arch_match_closid(struct task_struct *tsk,
					     hw_closid_t closid_code,
					     hw_closid_t closid_data)
{
	u32 partid = hwclosid_val(closid_data)<<MPAM_SYSREG_PARTID_D_SHIFT |
		     hwclosid_val(closid_code)<<MPAM_SYSREG_PARTID_I_SHIFT;

	return READ_ONCE(tsk->closid) == partid;
}

static inline bool resctrl_arch_match_rmid(struct task_struct *tsk, u32 rmid)
{
	u32 val;

	val = ((u64)rmid << (MPAM_SYSREG_PMG_D_SHIFT - 32)) |
	      ((u64)rmid << (MPAM_SYSREG_PMG_I_SHIFT - 32));

	return READ_ONCE(tsk->rmid) == val;
}
#else
static inline bool resctrl_arch_match_closid(struct task_struct *tsk,
					     hw_closid_t closid_code,
					     hw_closid_t closid_data)
{
	return false;
}

static inline bool resctrl_arch_match_rmid(struct task_struct *tsk, u32 rmid)
{
	return false;
}
#endif /* CONFIG_ARCH_HAS_CPU_RESCTRL */

static inline void resctrl_arch_set_rmid(struct task_struct *tsk, u32 rmid)
{
	mpam_set_task_pmg(tsk, rmid, rmid);
}

static inline void resctrl_arch_set_cpu_default_closid(int cpu,
						       hw_closid_t closid_code,
						       hw_closid_t closid_data)
{
	mpam_set_default_partid(cpu, hwclosid_val(closid_data),
				hwclosid_val(closid_code));
}

static inline void resctrl_arch_set_cpu_default_rmid(int cpu, u32 rmid)
{
	mpam_set_default_pmg(cpu, rmid, rmid);
}

static inline int resctrl_arch_rmid_read(hw_closid_t hw_closid, u32 rmid,
					 enum resctrl_event_id eventid,
					 u64 *res)
{
	return mpam_resctrl_rmid_read(hwclosid_val(hw_closid), rmid, eventid,
				      res);
}

static inline rmid_idx_t resctrl_arch_rmid_idx_encode(hw_closid_t closid,
						      u32 rmid)
{
	u8 shift = mpam_pmg_bits();

	return hwclosid_val(closid)<<shift | rmid;
}

static inline void resctrl_arch_rmid_idx_decode(rmid_idx_t idx,
						hw_closid_t *closid, u32 *rmid)
{
	u8 shift = mpam_pmg_bits();

	*closid = as_hwclosid_t(idx >> shift);
	*rmid = (idx & ~(shift - 1));
}

static inline rmid_idx_t resctrl_arch_num_rmid_idx(void)
{
	u8 shift = mpam_pmg_bits();

	return mpam_resctrl_num_closid()<<shift;
}

/* cache lockdown works differently on arm */
static inline u64 resctrl_arch_get_prefetch_disable_bits(void) { return 0; }
static inline int resctrl_arch_pseudo_lock_fn(void *p) { return -EOPNOTSUPP; }
static inline int resctrl_arch_measure_cycles_lat_fn(void *p) { return -EOPNOTSUPP; }
static inline int resctrl_arch_measure_l2_residency(void *p) { return -EOPNOTSUPP; }
static inline int resctrl_arch_measure_l3_residency(void *p) { return -EOPNOTSUPP; }
#endif /* __ASM_RESCTRL_H__ */
