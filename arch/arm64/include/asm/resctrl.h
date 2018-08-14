/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

/* This is the MPAM->resctrl<-arch glue. */

typedef struct { u16 val; } hw_closid_t;

#define resctrl_arch_is_mbm_total_enabled()	false
#define resctrl_arch_is_mbm_local_enabled()	false

#endif /* __ASM_RESCTRL_H__ */
