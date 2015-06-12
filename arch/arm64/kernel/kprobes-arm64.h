/*
 * arch/arm64/kernel/kprobes-arm64.h
 *
 * Copyright (C) 2013 Linaro Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _ARM_KERNEL_KPROBES_ARM64_H
#define _ARM_KERNEL_KPROBES_ARM64_H

enum kprobe_insn {
	INSN_REJECTED,
	INSN_GOOD_NO_SLOT,
	INSN_GOOD,
};

extern kprobes_pstate_check_t * const kprobe_condition_checks[16];

enum kprobe_insn __kprobes
arm_kprobe_decode_insn(kprobe_opcode_t insn, struct arch_specific_insn *asi);

#define SAVE_REGS_STRING\
	"	stp x0, x1, [sp, #16 * 0]\n"	\
	"	stp x2, x3, [sp, #16 * 1]\n"	\
	"	stp x4, x5, [sp, #16 * 2]\n"	\
	"	stp x6, x7, [sp, #16 * 3]\n"	\
	"	stp x8, x9, [sp, #16 * 4]\n"	\
	"	stp x10, x11, [sp, #16 * 5]\n"	\
	"	stp x12, x13, [sp, #16 * 6]\n"	\
	"	stp x14, x15, [sp, #16 * 7]\n"	\
	"	stp x16, x17, [sp, #16 * 8]\n"	\
	"	stp x18, x19, [sp, #16 * 9]\n"	\
	"	stp x20, x21, [sp, #16 * 10]\n"	\
	"	stp x22, x23, [sp, #16 * 11]\n"	\
	"	stp x24, x25, [sp, #16 * 12]\n"	\
	"	stp x26, x27, [sp, #16 * 13]\n"	\
	"	stp x28, x29, [sp, #16 * 14]\n"	\
	"	str x30,   [sp, #16 * 15]\n"    \
	"	mrs x0, nzcv\n"			\
	"	str x0, [sp, #8 * 33]\n"


#define RESTORE_REGS_STRING\
	"	ldr x0, [sp, #8 * 33]\n"	\
	"	msr nzcv, x0\n"			\
	"	ldp x0, x1, [sp, #16 * 0]\n"	\
	"	ldp x2, x3, [sp, #16 * 1]\n"	\
	"	ldp x4, x5, [sp, #16 * 2]\n"	\
	"	ldp x6, x7, [sp, #16 * 3]\n"	\
	"	ldp x8, x9, [sp, #16 * 4]\n"	\
	"	ldp x10, x11, [sp, #16 * 5]\n"	\
	"	ldp x12, x13, [sp, #16 * 6]\n"	\
	"	ldp x14, x15, [sp, #16 * 7]\n"	\
	"	ldp x16, x17, [sp, #16 * 8]\n"	\
	"	ldp x18, x19, [sp, #16 * 9]\n"	\
	"	ldp x20, x21, [sp, #16 * 10]\n"	\
	"	ldp x22, x23, [sp, #16 * 11]\n"	\
	"	ldp x24, x25, [sp, #16 * 12]\n"	\
	"	ldp x26, x27, [sp, #16 * 13]\n"	\
	"	ldp x28, x29, [sp, #16 * 14]\n"	\
	"	ldr x30,   [sp, #16 * 15]\n"

#endif /* _ARM_KERNEL_KPROBES_ARM64_H */
