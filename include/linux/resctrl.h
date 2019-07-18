// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.
// Based on arch/x86/kernel/cpu/resctrl/internal.h

#ifndef __LINUX_RESCTRL_H
#define __LINUX_RESCTRL_H

#include <linux/bug.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/kernfs.h>
#include <linux/fs_context.h>
#include <linux/resctrl_types.h>

#include <asm/resctrl.h>

#define MBM_OVERFLOW_INTERVAL		1000
#define CQM_LIMBOCHECK_INTERVAL		1000
#define MAX_MBA_BW			100u
#define MAX_MBA_BW_AMD			0x800

/* Closids are stored as a bitmap in a u32 */
#define RESCTRL_MAX_CLOSID 32

/* CBMs are passed around and stored in a u32 */
#define RESCTRL_MAX_CBM 32

/* max value for struct resctrl_mba_sc's mbps_val */
#define MBA_MAX_MBPS   U32_MAX

/*
 * The longest name we expect in the schemata file:
 */
#define RESCTRL_NAME_LEN	7

static inline bool resctrl_is_mbm_event(int e)
{
	return (e >= QOS_L3_MBM_TOTAL_EVENT_ID &&
		e <= QOS_L3_MBM_LOCAL_EVENT_ID);
}

/**
 * struct pseudo_lock_region - pseudo-lock region information
 * @s:			Resctrl schema for the resource to which this
 *			pseudo-locked region belongs
 * @hw_closid:		The closid that this pseudo-locked region uses
 * @d:			RDT domain to which this pseudo-locked region
 *			belongs
 * @cbm:		bitmask of the pseudo-locked region
 * @lock_thread_wq:	waitqueue used to wait on the pseudo-locking thread
 *			completion
 * @thread_done:	variable used by waitqueue to test if pseudo-locking
 *			thread completed
 * @cpu:		core associated with the cache on which the setup code
 *			will be run
 * @line_size:		size of the cache lines
 * @size:		size of pseudo-locked region in bytes
 * @kmem:		the kernel memory associated with pseudo-locked region
 * @minor:		minor number of character device associated with this
 *			region
 * @debugfs_dir:	pointer to this region's directory in the debugfs
 *			filesystem
 * @pm_reqs:		Power management QoS requests related to this region
 */
struct pseudo_lock_region {
	struct resctrl_schema	*s;
	hw_closid_t		hw_closid;
	struct rdt_domain	*d;
	u32			cbm;
	wait_queue_head_t	lock_thread_wq;
	int			thread_done;
	int			cpu;
	unsigned int		line_size;
	unsigned int		size;
	void			*kmem;
	unsigned int		minor;
	struct dentry		*debugfs_dir;
	struct list_head	pm_reqs;
};

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
 * struct resctrl_mba_sc - per-closid values for the control loop
 * @mbps_val:		The user's specified control value
 */
struct resctrl_mba_sc {
	u32		mbps_val;
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
 * @mba_sc:	the mba software controller properties, indexed by closid
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
	struct resctrl_mba_sc		*mba_sc;
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

/**
 * @rid:		The index of the resource
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
 * @domains:		All domains for this resource
 *
 * @name:		Name to use in "schemata" file.
 * @data_width:		Character width of data when displaying.
 * @default_ctrl:	Specifies default cache cbm or memory B/W percent.
 * @format_str:		Per resource format string to show domain value
 *
 * @evt_list:		List of monitoring events
 * @fflags:		flags to choose base and info files
 */
struct rdt_resource {
	int			rid;
	bool			cdp_enabled;
	bool			alloc_capable;
	bool			mon_capable;
	bool			cdp_capable;

	int			cache_level;

	struct resctrl_cache	cache;
	struct resctrl_membw	membw;

	struct list_head	domains;

	char			*name;
	int			data_width;
	u32			default_ctrl;
	const char		*format_str;

	struct list_head	evt_list;
	unsigned long		fflags;
};


struct rdt_fs_context {
	struct kernfs_fs_context	kfc;
	bool				enable_cdpl2;
	bool				enable_cdpl3;
	bool				enable_mba_mbps;
};

static inline struct rdt_fs_context *rdt_fc2context(struct fs_context *fc)
{
	struct kernfs_fs_context *kfc = fc->fs_private;

	return container_of(kfc, struct rdt_fs_context, kfc);
}

/**
 * struct mon_evt - Entry in the event list of a resource
 * @evtid:		event id
 * @name:		name of the event
 */
struct mon_evt {
	enum resctrl_event_id	evtid;
	char			*name;
	struct list_head	list;
};

/**
 * struct mon_data_bits - Monitoring details for each event file
 * @rid:               Resource id associated with the event file.
 * @evtid:             Event id associated with the event file
 * @domid:             The domain to which the event file belongs
 */
union mon_data_bits {
	void *priv;
	struct {
		unsigned int rid	: 10;
		unsigned int evtid	: 8;
		unsigned int domid	: 14;
	} u;
};

struct rmid_read {
	struct rdtgroup		*rgrp;
	struct rdt_domain	*d;
	int			evtid;
	bool			first;
	int			err;
	u64			val;
};

enum rdt_group_type {
	RDTCTRL_GROUP = 0,
	RDTMON_GROUP,
	RDT_NUM_GROUP,
};

/**
 * enum rdtgrp_mode - Mode of a RDT resource group
 * @RDT_MODE_SHAREABLE: This resource group allows sharing of its allocations
 * @RDT_MODE_EXCLUSIVE: No sharing of this resource group's allocations allowed
 * @RDT_MODE_PSEUDO_LOCKSETUP: Resource group will be used for Pseudo-Locking
 * @RDT_MODE_PSEUDO_LOCKED: No sharing of this resource group's allocations
 *                          allowed AND the allocations are Cache Pseudo-Locked
 *
 * The mode of a resource group enables control over the allowed overlap
 * between allocations associated with different resource groups (classes
 * of service). User is able to modify the mode of a resource group by
 * writing to the "mode" resctrl file associated with the resource group.
 *
 * The "shareable", "exclusive", and "pseudo-locksetup" modes are set by
 * writing the appropriate text to the "mode" file. A resource group enters
 * "pseudo-locked" mode after the schemata is written while the resource
 * group is in "pseudo-locksetup" mode.
 */
enum rdtgrp_mode {
	RDT_MODE_SHAREABLE = 0,
	RDT_MODE_EXCLUSIVE,
	RDT_MODE_PSEUDO_LOCKSETUP,
	RDT_MODE_PSEUDO_LOCKED,

	/* Must be last */
	RDT_NUM_MODES,
};

/**
 * struct mongroup - store mon group's data in resctrl fs.
 * @mon_data_kn		kernlfs node for the mon_data directory
 * @parent:			parent rdtgrp
 * @crdtgrp_list:		child rdtgroup node list
 * @rmid:			rmid for this rdtgroup
 */
struct mongroup {
	struct kernfs_node	*mon_data_kn;
	struct rdtgroup		*parent;
	struct list_head	crdtgrp_list;
	u32			rmid;
};

/**
 * struct rdtgroup - store rdtgroup's data in resctrl file system.
 * @kn:				kernfs node
 * @rdtgroup_list:		linked list for all rdtgroups
 * @closid:			closid for this rdtgroup
 * @cpu_mask:			CPUs assigned to this rdtgroup
 * @flags:			status bits
 * @waitcount:			how many cpus expect to find this
 *				group when they acquire rdtgroup_mutex
 * @type:			indicates type of this rdtgroup - either
 *				monitor only or ctrl_mon group
 * @mon:			mongroup related data
 * @mode:			mode of resource group
 * @plr:			pseudo-locked region
 */
struct rdtgroup {
	struct kernfs_node		*kn;
	struct list_head		rdtgroup_list;
	u32				closid;
	struct cpumask			cpu_mask;
	int				flags;
	atomic_t			waitcount;
	enum rdt_group_type		type;
	struct mongroup			mon;
	enum rdtgrp_mode		mode;
	struct pseudo_lock_region	*plr;
};


/* rdtgroup.flags */
#define	RDT_DELETED		1

/* rftype.flags */
#define RFTYPE_FLAGS_CPUS_LIST	1

/*
 * Define the file type flags for base and info directories.
 */
#define RFTYPE_INFO			BIT(0)
#define RFTYPE_BASE			BIT(1)
#define RF_CTRLSHIFT			4
#define RF_MONSHIFT			5
#define RF_TOPSHIFT			6
#define RFTYPE_CTRL			BIT(RF_CTRLSHIFT)
#define RFTYPE_MON			BIT(RF_MONSHIFT)
#define RFTYPE_TOP			BIT(RF_TOPSHIFT)
#define RFTYPE_RES_CACHE		BIT(8)
#define RFTYPE_RES_MB			BIT(9)
#define RF_CTRL_INFO			(RFTYPE_INFO | RFTYPE_CTRL)
#define RF_MON_INFO			(RFTYPE_INFO | RFTYPE_MON)
#define RF_TOP_INFO			(RFTYPE_INFO | RFTYPE_TOP)
#define RF_CTRL_BASE			(RFTYPE_BASE | RFTYPE_CTRL)

/* List of all resource groups */
extern struct list_head rdt_all_groups;

extern int max_data_width;

/**
 * struct rftype - describe each file in the resctrl file system
 * @name:	File name
 * @mode:	Access mode
 * @kf_ops:	File operations
 * @flags:	File specific RFTYPE_FLAGS_* flags
 * @fflags:	File specific RF_* or RFTYPE_* flags
 * @seq_show:	Show content of the file
 * @write:	Write to the file
 */
struct rftype {
	char			*name;
	umode_t			mode;
	struct kernfs_ops	*kf_ops;
	unsigned long		flags;
	unsigned long		fflags;

	int (*seq_show)(struct kernfs_open_file *of,
			struct seq_file *sf, void *v);
	/*
	 * write() is the generic write callback which maps directly to
	 * kernfs write operation and overrides all other operations.
	 * Maximum write size is determined by ->max_write_len.
	 */
	ssize_t (*write)(struct kernfs_open_file *of,
			 char *buf, size_t nbytes, loff_t off);
};

/**
 * struct mbm_state - status for each MBM counter in each domain
 * @chunks:	Total data moved in bytes
 * @prev_msr	Value of IA32_QM_CTR for this RMID last time we read it
 * @prev_bw_msr:Value of previous IA32_QM_CTR for bandwidth counting
 * @prev_bw	The most recent bandwidth in MBps
 * @delta_bw	Difference between the current and previous bandwidth
 * @delta_comp	Indicates whether to compute the delta_bw
 */
struct mbm_state {
	u64	chunks;
	u64	prev_msr;
	u64	prev_bw_msr;
	u32	prev_bw;
	u32	delta_bw;
	bool	delta_comp;
};

/*
 * Get the resource that exists at this level. If the level is not supproted
 * a dummy/not-capable resource can be returned. Levels >= RDT_NUM_RESOURCES
 * will return NULL.
 */
struct rdt_resource *resctrl_arch_get_resource(enum resctrl_resource_level l);

struct rdt_domain *resctrl_arch_find_domain(struct rdt_resource *r, int id);
int resctrl_arch_update_domains(struct rdt_resource *r);

static inline struct rdt_domain *
resctrl_get_domain_from_cpu(int cpu, struct rdt_resource *r)
{
	struct rdt_domain *d;

	list_for_each_entry(d, &r->domains, list) {
		/* Find the domain that contains this CPU */
		if (cpumask_test_cpu(cpu, &d->cpu_mask))
			return d;
	}

	return NULL;
}

/*
 * Update the ctrl_val and apply this config right now.
 * Must be called one one of the domains cpus.
 */
int resctrl_arch_update_one(struct rdt_resource *r, struct rdt_domain *d,
			    hw_closid_t hw_closid, u32 cfg_val);

void resctrl_arch_get_config(struct rdt_resource *r, struct rdt_domain *d,
			     hw_closid_t hw_closid, u32 *value);

int resctrl_arch_rmid_read(hw_closid_t hw_closid, u32 rmid,
			   enum resctrl_event_id eventid, u64 *res);

/* Enable/Disable CDP on all applicable resources */
int resctrl_arch_set_cdp_enabled(bool enable);


struct resctrl_cpu_sync
{
	hw_closid_t closid_code, closid_data;
	u32 rmid;
};

void resctrl_arch_update_cpu_defaults(void *info);

static inline bool resctrl_is_mbm_enabled(void)
{
	return (resctrl_arch_is_mbm_total_enabled() ||
		resctrl_arch_is_mbm_local_enabled());
}

/**
 * @list:	Member of resctrl's schema list
 * @name:	Name visible in the schemata file
 * @conf_type:	Type of configuration, e.g. code/data/both
 * @res:	The rdt_resource for this entry
 */
struct resctrl_schema {
	struct list_head		list;
	char				name[RESCTRL_NAME_LEN];
	enum resctrl_conf_type		conf_type;
	struct rdt_resource		*res;
};


int resctrl_online_domain(struct rdt_resource *r, struct rdt_domain *d);
int resctrl_online_cpu(unsigned int cpu);
void resctrl_offline_cpu(unsigned int cpu);
void resctrl_offline_domain(struct rdt_resource *r, struct rdt_domain *d);

int resctrl_init(void);
void resctrl_exit(void);

#endif /* __LINUX_RESCTRL_H */
