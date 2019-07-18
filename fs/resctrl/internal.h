// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.
// Based on arch/x86/kernel/cpu/resctrl/internal.h

#ifndef __FS_RESCTRL_INTERNAL_H
#define __FS_RESCTRL_INTERNAL_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/resctrl.h>

extern struct mutex rdtgroup_mutex;
extern struct list_head resctrl_all_schema;
extern bool resctrl_mounted;
extern u32 resctrl_rmid_realloc_threshold;
extern struct rdtgroup rdtgroup_default;
extern struct dentry *debugfs_resctrl;

static inline bool is_mba_sc(struct rdt_resource *r)
{
	if (!r)
		r = resctrl_arch_get_resource(RDT_RESOURCE_MBA);

	return r->membw.mba_sc;
}

void rdt_last_cmd_clear(void);
void rdt_last_cmd_puts(const char *s);
void rdt_last_cmd_printf(const char *fmt, ...);

void mbm_setup_overflow_handler(struct rdt_domain *dom,
				unsigned long delay_ms,
				int exclude_cpu);
void mbm_handle_overflow(struct work_struct *work);
void setup_default_ctrlval(struct rdt_resource *r, u32 *dc);
void cqm_setup_limbo_handler(struct rdt_domain *dom, unsigned long delay_ms,
			     int exclude_cpu);
void cqm_handle_limbo(struct work_struct *work);
bool has_busy_rmid(struct rdt_resource *r, struct rdt_domain *d);
void __check_limbo(struct rdt_domain *d, bool force_free);
hw_closid_t resctrl_closid_cdp_map(u32 closid, enum resctrl_conf_type t);

struct rdtgroup *rdtgroup_kn_lock_live(struct kernfs_node *kn);
void rdtgroup_kn_unlock(struct kernfs_node *kn);
int rdtgroup_kn_mode_restrict(struct rdtgroup *r, const char *name);
int rdtgroup_kn_mode_restore(struct rdtgroup *r, const char *name,
			     umode_t mask);
ssize_t rdtgroup_schemata_write(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off);
int rdtgroup_schemata_show(struct kernfs_open_file *of,
			   struct seq_file *s, void *v);
bool rdtgroup_cbm_overlaps(struct resctrl_schema *s, struct rdt_domain *d,
			   unsigned long cbm, int closid, bool exclusive);
unsigned int rdtgroup_cbm_to_size(struct rdt_resource *r, struct rdt_domain *d,
				  unsigned long cbm);
enum rdtgrp_mode rdtgroup_mode_by_closid(int closid);
int rdtgroup_tasks_assigned(struct rdtgroup *r);
int rdtgroup_locksetup_enter(struct rdtgroup *rdtgrp);
int rdtgroup_locksetup_exit(struct rdtgroup *rdtgrp);
bool rdtgroup_cbm_overlaps_pseudo_locked(struct rdt_domain *d, unsigned long cbm);
bool rdtgroup_pseudo_locked_in_hierarchy(struct rdt_domain *d);
int rdt_pseudo_lock_init(void);
void rdt_pseudo_lock_release(void);
int rdtgroup_pseudo_lock_create(struct rdtgroup *rdtgrp);
void rdtgroup_pseudo_lock_remove(struct rdtgroup *rdtgrp);
int update_domains(struct rdt_resource *r);
u32 closids_supported(void);
void closid_free(int closid);
int alloc_rmid(hw_closid_t closid);
void free_rmid(hw_closid_t closid, u32 rmid);
void mon_event_count(void *info);
int rdtgroup_mondata_show(struct seq_file *m, void *arg);
void mon_event_read(struct rmid_read *rr, struct rdt_domain *d,
		    struct rdtgroup *rdtgrp, int evtid, int first);
int resctrl_mon_resource_init(void);

#endif /* __FS_RESCTRL_INTERNAL_H */
