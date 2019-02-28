/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

#include <linux/arm_mpam.h>

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

static inline u32 resctrl_arch_system_num_rmid(void)
{
	return mpam_resctrl_num_rmid();
}

static inline u32 resctrl_arch_system_num_closid(void)
{
	return mpam_resctrl_num_closid();
}

static inline u32 resctrl_arch_max_rmid_threshold(void)
{
	return mpam_resctrl_llc_cache_size();
}
#endif /* __ASM_RESCTRL_H__ */
