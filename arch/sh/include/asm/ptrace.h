/*
 * Copyright (C) 1999, 2000  Niibe Yutaka
 */
#ifndef __ASM_SH_PTRACE_H
#define __ASM_SH_PTRACE_H


#include <linux/stringify.h>
#include <linux/stddef.h>
#include <linux/thread_info.h>
#include <asm/addrspace.h>
#include <asm/page.h>
#include <uapi/asm/ptrace.h>

#define user_mode(regs)			(((regs)->sr & 0x40000000)==0)
#define kernel_stack_pointer(_regs)	((unsigned long)(_regs)->regs[15])

#define GET_FP(regs)	((regs)->regs[14])
#define GET_USP(regs)	((regs)->regs[15])

#define arch_has_single_step()	(1)

/*
 * kprobe-based event tracer support
 */
#define REG_OFFSET_NAME(r) {.name = #r, .offset = offsetof(struct pt_regs, r)}
#define REGS_OFFSET_NAME(num)	\
	{.name = __stringify(r##num), .offset = offsetof(struct pt_regs, regs[num])}
#define TREGS_OFFSET_NAME(num)	\
	{.name = __stringify(tr##num), .offset = offsetof(struct pt_regs, tregs[num])}
#define REG_OFFSET_END {.name = NULL, .offset = 0}

/* Query offset/name of register from its name/offset */

/**
 * regs_get_register() - get register value from its offset
 * @regs:	pt_regs from which register value is gotten.
 * @offset:	offset number of the register.
 *
 * regs_get_register returns the value of a register. The @offset is the
 * offset of the register in struct pt_regs address which specified by @regs.
 * If @offset is bigger than MAX_REG_OFFSET, this returns 0.
 */
static inline unsigned long regs_get_register(struct pt_regs *regs,
					      unsigned int offset)
{
	if (unlikely(offset > MAX_REG_OFFSET))
		return 0;
	return *(unsigned long *)((unsigned long)regs + offset);
}

struct perf_event;
struct perf_sample_data;

extern void ptrace_triggered(struct perf_event *bp,
		      struct perf_sample_data *data, struct pt_regs *regs);

#define task_pt_regs(task) \
	((struct pt_regs *) (task_stack_page(task) + THREAD_SIZE) - 1)

static inline unsigned long profile_pc(struct pt_regs *regs)
{
	unsigned long pc = regs->pc;

	if (virt_addr_uncached(pc))
		return CAC_ADDR(pc);

	return pc;
}
#define profile_pc profile_pc

#include <asm-generic/ptrace.h>
#endif /* __ASM_SH_PTRACE_H */
