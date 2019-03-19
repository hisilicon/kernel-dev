/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Arm Ltd. */

#ifndef __ASM_RESCTRL_H__
#define __ASM_RESCTRL_H__

#include <linux/bug.h>
#include <linux/sched.h>

typedef struct { u32 val; } hw_closid_t;
#define as_hwclosid_t(x)	((hw_closid_t){(x)})
#define hwclosid_val(x)		(x.val)

/*
 * Get the system wide closid/rmid limits. The number of closids should not be
 * affected by CDP.
 */
u32 resctrl_arch_system_num_closid(void);
u32 resctrl_arch_system_num_rmid(void);

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

#endif /* _ASM_RESCTRL_H_ */
