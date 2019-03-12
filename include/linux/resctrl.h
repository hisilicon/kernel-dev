// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.
// Based on arch/x86/kernel/cpu/resctrl/internal.h

#ifndef __LINUX_RESCTRL_H
#define __LINUX_RESCTRL_H

#include <linux/list.h>
#include <linux/kernel.h>

struct rdt_domain;

/**
 * struct resctrl_cache - Cache allocation related data
 * @cbm_len:		Length of the cache bit mask
 * @min_cbm_bits:	Minimum number of consecutive bits to be set
 * @cbm_idx_mult:	Multiplier of CBM index
 * @cbm_idx_offset:	Offset of CBM index. CBM index is computed by:
 *			closid * cbm_idx_multi + cbm_idx_offset
 *			in a cache bit mask
 * @shareable_bits:	Bitmask of shareable resource with other
 *			executing entities
 * @arch_has_sparse_bitmaps:   True if a bitmap like f00f is valid.
 */
struct resctrl_cache {
	u32		cbm_len;
	u32		min_cbm_bits;
	unsigned int	cbm_idx_mult;	// TODO remove this
	unsigned int	cbm_idx_offset; // TODO remove this
	u32		shareable_bits;
	bool		arch_has_sparse_bitmaps;
};

/**
 * struct resctrl_membw - Memory bandwidth allocation related data
 * @min_bw:		Minimum memory bandwidth percentage user can request
 * @bw_gran:		Granularity at which the memory bandwidth is allocated
 * @delay_linear:	True if memory B/W delay is in linear scale
 * @arch_needs_linear:  True if we can't configure non-linear resources
 * @mba_sc:		True if MBA software controller(mba_sc) is enabled
 * @mb_map:		Mapping of memory B/W percentage to memory B/W delay
 */
struct resctrl_membw {
	u32		min_bw;
	u32		bw_gran;
	u32		delay_linear;
	bool		arch_needs_linear;
	bool		mba_sc;
	u32		*mb_map;
};

struct rdt_parse_data;

/**
 * @rid:		The index of the resource
 * @alloc_enabled:	Is allocation enabled on this machine
 * @mon_enabled:	Is monitoring enabled for this feature
 * @alloc_capable:	Is allocation available on this machine
 * @mon_capable:	Is monitor feature available on this machine
 *
 * @cache_level:	Which cache level defines scope of this resource
 *
 * @cache:		If the component has cache controls, their properties.
 * @membw:		If the component has bandwidth controls, their properties.
 *
 * @num_closid:		Number of CLOSIDs available.
 * @num_rmid:		Number of RMIDs available.
 *
 * @domains:		All domains for this resource
 *
 * @name:		Name to use in "schemata" file.
 * @data_width:		Character width of data when displaying.
 * @default_ctrl:	Specifies default cache cbm or memory B/W percent.
 * @format_str:		Per resource format string to show domain value
 * @parse_ctrlval:	Per resource function pointer to parse control values
 *
 * @evt_list:		List of monitoring events
 * @fflags:		flags to choose base and info files
 */
struct rdt_resource {
	int			rid;
	bool			alloc_enabled;
	bool			mon_enabled;
	bool			alloc_capable;
	bool			mon_capable;

	int			cache_level;

	struct resctrl_cache	cache;
	struct resctrl_membw	membw;

	int			num_closid;
	int			num_rmid;

	struct list_head	domains;

	char			*name;
	int			data_width;
	u32			default_ctrl;
	const char		*format_str;
	int			(*parse_ctrlval)(struct rdt_parse_data *data,
						 struct rdt_resource *r,
						 struct rdt_domain *d);
	struct list_head	evt_list;
	unsigned long		fflags;

};

#endif /* __LINUX_RESCTRL_H */
