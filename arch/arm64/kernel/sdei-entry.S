/*
 * Software Delegated Exception entry point from firmware to the kernel.
 *
 * Copyright (C) 2017 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/linkage.h>

#include <asm/alternative.h>
#include <asm/asm-offsets.h>
#include <asm/assembler.h>
#include <asm/memory.h>
#include <asm/ptrace.h>
#include <asm/sdei.h>
#include <uapi/linux/arm_sdei.h>

/*
 * Software Delegated Exception entry point.
 *
 * x0: Event number
 * x1: struct sdei_registered_event argument from registration time.
 * x2: interrupted PC
 * x3: interrupted PSTATE
 *
 * Firmware has preserved x0->x17 for us, we must save/restore the rest to
 * follow SMC-CC. We save (or retrieve) all the registers as the handler may
 * want them.
 */
ENTRY(__sdei_asm_handler)
	stp     x2, x3, [x1, #SDEI_EVENT_INTREGS + S_PC]
	stp     x4, x5, [x1, #SDEI_EVENT_INTREGS + 16 * 2]
	stp     x6, x7, [x1, #SDEI_EVENT_INTREGS + 16 * 3]
	stp     x8, x9, [x1, #SDEI_EVENT_INTREGS + 16 * 4]
	stp     x10, x11, [x1, #SDEI_EVENT_INTREGS + 16 * 5]
	stp     x12, x13, [x1, #SDEI_EVENT_INTREGS + 16 * 6]
	stp     x14, x15, [x1, #SDEI_EVENT_INTREGS + 16 * 7]
	stp     x16, x17, [x1, #SDEI_EVENT_INTREGS + 16 * 8]
	stp     x18, x19, [x1, #SDEI_EVENT_INTREGS + 16 * 9]
	stp     x20, x21, [x1, #SDEI_EVENT_INTREGS + 16 * 10]
	stp     x22, x23, [x1, #SDEI_EVENT_INTREGS + 16 * 11]
	stp     x24, x25, [x1, #SDEI_EVENT_INTREGS + 16 * 12]
	stp     x26, x27, [x1, #SDEI_EVENT_INTREGS + 16 * 13]
	stp     x28, x29, [x1, #SDEI_EVENT_INTREGS + 16 * 14]
	mov	x4, sp
	stp     lr, x4, [x1, #SDEI_EVENT_INTREGS + S_LR]

	mov	x19, x1

#ifdef CONFIG_VMAP_STACK
	/*
	 * entry.S may have been using sp as a scratch register, find whether
	 * this is a normal or critical event and switch to the appropriate
	 * stack for this CPU.
	 */
	ldrb	w4, [x19, #SDEI_EVENT_PRIORITY]
	cbnz	w4, 1f
	ldr_this_cpu dst=x5, sym=sdei_stack_normal_ptr, tmp=x6
	b	2f
1:	ldr_this_cpu dst=x5, sym=sdei_stack_critical_ptr, tmp=x6
2:	mov	x6, #SDEI_STACK_SIZE
	add	x5, x5, x6
	mov	sp, x5
#endif

	/*
	 * We may have interrupted userspace, or a guest, or exit-from or
	 * return-to either of these. We can't trust sp_el0, restore it.
	 */
	mrs	x28, sp_el0
	ldr_this_cpu	dst=x0, sym=__entry_task, tmp=x1
	msr	sp_el0, x0

	/* If we interrupted the kernel point to the previous stack/frame. */
	and     x0, x3, #0xc
	mrs     x1, CurrentEL
	cmp     x0, x1
	csel	x29, x29, xzr, eq	// fp, or zero
	csel	x4, x2, xzr, eq		// elr, or zero

	stp	x29, x4, [sp, #-16]!
	mov	x29, sp

	add	x0, x19, #SDEI_EVENT_INTREGS
	mov	x1, x19
	bl	__sdei_handler

	msr	sp_el0, x28
	/* restore regs >x17 that we clobbered */
	ldp     x28, x29, [x19, #SDEI_EVENT_INTREGS + 16 * 14]
	ldp     lr, x4, [x19, #SDEI_EVENT_INTREGS + S_LR]
	mov	sp, x4
	ldp     x18, x19, [x19, #SDEI_EVENT_INTREGS + 16 * 9]

	mov	x1, x0			// address to complete_and_resume
	/* x0 = (x0 <= 1) ? EVENT_COMPLETE:EVENT_COMPLETE_AND_RESUME */
	cmp	x0, #1
	mov_q	x2, SDEI_1_0_FN_SDEI_EVENT_COMPLETE
	mov_q	x3, SDEI_1_0_FN_SDEI_EVENT_COMPLETE_AND_RESUME
	csel	x0, x2, x3, ls

	/* On success, this call never returns... */
	ldr_l	x2, sdei_exit_mode
	cmp	x2, #SDEI_EXIT_SMC
	b.ne	1f
	smc	#0
	b	.
1:	hvc	#0
	b	.
ENDPROC(__sdei_asm_handler)
NOKPROBE(__sdei_asm_handler)
