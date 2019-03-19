/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

/*
 * Get the system wide closid/rmid limits. The number of closids should not be
 * affected by CDP.
 */
u32 resctrl_arch_system_num_closid(void);
u32 resctrl_arch_system_num_rmid(void);

#endif /* __ASM_RESCTRL_H__ */
