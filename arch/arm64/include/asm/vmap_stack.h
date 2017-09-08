/*
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
#ifndef __ASM_VMAP_STACK_H
#define __ASM_VMAP_STACK_H

#include <linux/vmalloc.h>
#include <asm/memory.h>
#include <asm/pgtable.h>
#include <asm/thread_info.h>

/*
 * To ensure that VMAP'd stack overflow detection works correctly, all VMAP'd
 * stacks need to have the same alignment.
 */
static inline unsigned long *arch_alloc_vmap_stack(size_t stack_size, int node)
{
	BUILD_BUG_ON(!IS_ENABLED(CONFIG_VMAP_STACK));

	return __vmalloc_node_range(stack_size, THREAD_ALIGN,
				    VMALLOC_START, VMALLOC_END,
				    THREADINFO_GFP, PAGE_KERNEL, 0, node,
				    __builtin_return_address(0));
}

#endif /* __ASM_VMAP_STACK_H */
