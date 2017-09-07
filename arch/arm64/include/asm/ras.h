/*
 * ARM64 SEA error recoery support
 *
 * Copyright 2017 Huawei Technologies Co., Ltd.
 *   Author: Xie XiuQi <xiexiuqi@huawei.com>
 *   Author: Wang Xiongfeng <wangxiongfeng2@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _ASM_RAS_H
#define _ASM_RAS_H

#include <linux/cper.h>
#include <linux/ras.h>
#include <acpi/ghes.h>

extern void sea_notify_process(void);

#ifdef CONFIG_ARM64_ERR_RECOV
extern void arm_process_error(struct ghes *ghes, struct cper_sec_proc_arm *err);
#else
static inline void arm_process_error(struct ghes *ghes, struct cper_sec_proc_arm *err)
{
	log_arm_hw_error(err);
}
#endif /* CONFIG_ARM64_ERR_RECOV */

#endif /*_ASM_RAS_H*/
