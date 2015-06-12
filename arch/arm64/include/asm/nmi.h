/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _ASM_ARCH64_NMI_H
#define _ASM_ARCH64_NMI_H

#ifdef CONFIG_HISI_AARCH64_NMI
int register_nmi_handler(int cpuid, int (*func)(void));
void nmi_set_active_state(int cpuid, bool state);
void nmi_set_timeout(int cpuid, int time);
#else
static inline int register_nmi_handler(int cpuid, int (*func)(void))
{
}

static inline void nmi_set_active_state(int cpuid, bool state)
{
}

static inline void nmi_set_timeout(int cpuid, int time)
{
}
#endif

struct hisi_nmi_watchdog {
	int id;
	int timeout;
	int (*nmi_handler)(void);
	bool state;
};

#define NMI_WATCHDOG_ON	1
#define NMI_WATCHDOG_OFF 0

extern struct hisi_nmi_watchdog cpu_nmi[NR_CPUS];

#endif /* _ASM_ARCH64_NMI_H */

