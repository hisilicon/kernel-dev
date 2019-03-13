// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.
// Based on arch/x86/kernel/cpu/resctrl/internal.h

#ifndef __LINUX_RESCTRL_H
#define __LINUX_RESCTRL_H

#include <linux/list.h>
#include <linux/kernel.h>


typedef struct { u32 val; } hw_closid_t;
#define as_hwclosid_t(x)	((hw_closid_t){(x)})
#define hwclosid_val(x)		(x.val)

/*
 * The longest name we expect in the schemata file:
 */
#define RESCTRL_NAME_LEN	7

enum resctrl_conf_type {
	CDP_BOTH = 0,
	CDP_CODE,
	CDP_DATA,
};
#define NUM_CDP_TYPES	CDP_DATA + 1


/**
 * struct resctrl_staged_config - parsed configuration to be applied
 * @hw_closid:		raw closid for this configuration, regardless of CDP
 * @new_ctrl:		new ctrl value to be loaded
 * @have_new_ctrl:	did user provide new_ctrl for this domain
 * @new_ctrl_type:	CDP property of the new ctrl
 */
struct resctrl_staged_config {
	hw_closid_t		hw_closid;
	u32			new_ctrl;
	bool			have_new_ctrl;
	enum resctrl_conf_type  new_ctrl_type;
};

/**
 * struct rdt_domain - group of cpus sharing an RDT resource
 * @list:		all instances of this resource
 * @id:			unique id for this instance
 * @cpu_mask:		which cpus share this resource
 * @rmid_busy_llc:	bitmap of which limbo RMIDs are above threshold
 * @mbm_total:		saved state for MBM total bandwidth
 * @mbm_local:		saved state for MBM local bandwidth
 * @mbm_over:		worker to periodically read MBM h/w counters
 * @cqm_limbo:		worker to periodically read CQM h/w counters
 * @mbm_work_cpu:	worker cpu for MBM h/w counters
 * @cqm_work_cpu:	worker cpu for CQM h/w counters
 * @plr:		pseudo-locked region (if any) associated with domain
 * @staged_config:	parsed configuration to be applied
 */
struct rdt_domain {
	struct list_head		list;
	int				id;
	struct cpumask			cpu_mask;

	unsigned long			*rmid_busy_llc;
	struct mbm_state		*mbm_total;
	struct mbm_state		*mbm_local;
	struct delayed_work		mbm_over;
	struct delayed_work		cqm_limbo;
	int				mbm_work_cpu;
	int				cqm_work_cpu;

	struct pseudo_lock_region	*plr;
	struct resctrl_staged_config	staged_config[NUM_CDP_TYPES];
};

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
struct resctrl_schema;

/**
 * @rid:		The index of the resource
 * @mon_enabled:	Is monitoring enabled for this feature
 * @cdp_enabled		Is CDP enabled for this resource
 * @alloc_capable:	Is allocation available on this machine
 * @mon_capable:	Is monitor feature available on this machine
 * @cdp_capable:	Is CDP feature available on this resource
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
	bool			mon_enabled;
	bool			cdp_enabled;
	bool			alloc_capable;
	bool			mon_capable;
	bool			cdp_capable;

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
						 struct resctrl_schema *s,
						 struct rdt_domain *d);
	struct list_head	evt_list;
	unsigned long		fflags;
};

int resctrl_arch_update_domains(struct rdt_resource *r);
void resctrl_arch_get_config(struct rdt_resource *r, struct rdt_domain *d,
			     hw_closid_t hw_closid, u32 *value);

/* Enable/Disable CDP on all applicable resources */
int resctrl_arch_set_cdp_enabled(bool enable);

/**
 * @list:	Member of resctrl's schema list
 * @name:	Name visible in the schemata file
 * @conf_type:	Type of configuration, e.g. code/data/both
 * @res:	The rdt_resource for this entry
 * @cdp_peer:	The partner schema for the same resource when using CDP
 */
struct resctrl_schema {
	struct list_head		list;
	char				name[RESCTRL_NAME_LEN];
	enum resctrl_conf_type		conf_type;
	struct rdt_resource		*res;
	struct resctrl_schema		*cdp_peer;
};

#endif /* __LINUX_RESCTRL_H */
