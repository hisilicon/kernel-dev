/*
 * kexec for arm64
 *
 * Copyright (C) Linaro.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#if !defined(_ARM64_KEXEC_H)
#define _ARM64_KEXEC_H

/* Maximum physical address we can use pages from */

#define KEXEC_SOURCE_MEMORY_LIMIT (-1UL)

/* Maximum address we can reach in physical address mode */

#define KEXEC_DESTINATION_MEMORY_LIMIT (-1UL)

/* Maximum address we can use for the control code buffer */

#define KEXEC_CONTROL_MEMORY_LIMIT (-1UL)

#define KEXEC_CONTROL_PAGE_SIZE	4096

#define KEXEC_ARCH KEXEC_ARCH_ARM64

#if !defined(__ASSEMBLY__)

extern bool in_crash_kexec;

/**
 * crash_setup_regs() - save registers for the panic kernel
 *
 * @newregs: registers are saved here
 * @oldregs: registers to be saved (may be %NULL)
 */

static inline void crash_setup_regs(struct pt_regs *newregs,
				    struct pt_regs *oldregs)
{
	if (oldregs) {
		memcpy(newregs, oldregs, sizeof(*newregs));
	} else {
		__asm__ __volatile__ (
			"stp	 x0,   x1, [%3]\n\t"
			"stp	 x2,   x3, [%3, 0x10]\n\t"
			"stp	 x4,   x5, [%3, 0x20]\n\t"
			"stp	 x6,   x7, [%3, 0x30]\n\t"
			"stp	 x8,   x9, [%3, 0x40]\n\t"
			"stp	x10,  x11, [%3, 0x50]\n\t"
			"stp	x12,  x13, [%3, 0x60]\n\t"
			"stp	x14,  x15, [%3, 0x70]\n\t"
			"stp	x16,  x17, [%3, 0x80]\n\t"
			"stp	x18,  x19, [%3, 0x90]\n\t"
			"stp	x20,  x21, [%3, 0xa0]\n\t"
			"stp	x22,  x23, [%3, 0xb0]\n\t"
			"stp	x24,  x25, [%3, 0xc0]\n\t"
			"stp	x26,  x27, [%3, 0xd0]\n\t"
			"stp	x28,  x29, [%3, 0xe0]\n\t"
			"str	x30,	   [%3, 0xf0]\n\t"
			"mov	%0, sp\n\t"
			"adr	%1, 1f\n\t"
			"mrs	%2, spsr_el1\n\t"
		"1:"
			: "=r" (newregs->sp),
			  "=r" (newregs->pc),
			  "=r" (newregs->pstate)
			: "r"  (&newregs->regs)
			: "memory"
		);
	}
}

#endif /* !defined(__ASSEMBLY__) */

#endif
