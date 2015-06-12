/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * This file contains NUMA specific prototypes and definitions.
 *
 * 2014/07/28 Zhen Lei<thunder.leizhen@huawei.com>
 *
 */
#ifndef _ASM_ARM64_NUMA_H
#define _ASM_ARM64_NUMA_H


#ifdef CONFIG_NUMA

#include <linux/cache.h>
#include <linux/cpumask.h>
#include <linux/numa.h>
#include <linux/smp.h>
#include <linux/threads.h>

#include <asm/mmzone.h>

extern cpumask_t node_to_cpu_mask[MAX_NUMNODES] __cacheline_aligned;

#define node_distance(from, to)		(((from) == (to)) ? 10 : (10 + ((((0x3 & ((from) ^ (to))) + 2) >> 1) * 5)))
#define numa_clear_node(cpu)		numa_set_node(cpu, NUMA_NO_NODE)

#else /* !CONFIG_NUMA */
#define numa_clear_node(cpu)		do { } while (0)
#endif /* CONFIG_NUMA */

#endif /* _ASM_ARM64_NUMA_H */
