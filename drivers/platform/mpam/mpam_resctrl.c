// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Arm Ltd.

#define pr_fmt(fmt) "mpam: " fmt

#include <linux/arm_mpam.h>
#include <linux/cacheinfo.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/rculist.h>
#include <linux/resctrl.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/mpam.h>

#include "mpam_internal.h"

/* The classes we've picked to map to resctrl resources */
static struct mpam_class *mpam_resctrl_exports[RDT_NUM_RESOURCES];
static struct mpam_class *mpam_resctrl_events[RESCTRL_NUM_EVENT_IDS];
static struct mpam_class *mpam_llc_class;

static bool exposed_alloc_capable;
static bool exposed_mon_capable;

bool mpam_resctrl_alloc_capable(void)
{
	return exposed_alloc_capable;
}

bool mpam_resctrl_mon_capable(void)
{
	return exposed_mon_capable;
}

u32 mpam_resctrl_num_closid(void)
{
	return min((u32)mpam_sysprops.max_partid, (u32)RESCTRL_MAX_CLOSID);
}

u32 mpam_resctrl_num_rmid(void)
{
	/*
	 * num_rmid is not equivalent between RDT and MPAM systems.
	 * With RDT, rmid is an independent number, each closid is allocate one
	 * or more. With MPAM, pmg is effectively an extention to the partid
	 * space. Each partid has a the platforms number of pmg, whether it uses
	 * them or not.
	 *
	 * The value we pick here is exposed to user-space.
	 * max_partid*max_pmg is the size of the rmid space, but you can't use
	 * them all for a specific task. Export max_pmg, you will be using
	 * more than this, but you can use at least this many for your specific
	 * task.
	 */
	return mpam_sysprops.max_pmg;
}

u32 mpam_resctrl_llc_cache_size(void)
{
	return mpam_sysprops.mpam_llc_size;
}

struct rdt_resource *mpam_resctrl_get_resource(enum resctrl_resource_level l)
{
	if (l >= RDT_NUM_RESOURCES)
		return NULL;

	return &mpam_resctrl_exports[l]->resctrl_res;
}

bool mpam_resctrl_mbm_total_enabled(void)
{
	return (mpam_resctrl_events[QOS_L3_MBM_TOTAL_EVENT_ID] != NULL);
}

bool mpam_resctrl_llc_occupancy_enabled(void)
{
	return (mpam_resctrl_events[QOS_L3_OCCUP_EVENT_ID] != NULL);
}

static void mpam_resctrl_pick_event_l3_occup(void)
{
	/*
	 * as the name suggests, resctrl can only use this if your cache is
	 * called 'l3'.
	 */
	struct mpam_class *class = mpam_resctrl_exports[RDT_RESOURCE_L3];
	if (!class)
		return;

	if (!mpam_has_feature(mpam_feat_msmon_csu, class->features))
		return;

	mpam_resctrl_events[QOS_L3_OCCUP_EVENT_ID] = class;

	exposed_mon_capable = true;
	class->resctrl_res.mon_capable = true;
}

static void mpam_resctrl_pick_event_mbm_total(void)
{
	u64 num_counters;
	struct mpam_class *class;

	/*
	 * as the name suggests, resctrl can only use this if your cache is
	 * called 'l3'.
	 */
	class = mpam_resctrl_exports[RDT_RESOURCE_L3];
	if (!class)
		return;

	/*
	 * to measure bandwidth in a resctrl like way, we need to leave a
	 * counter running all the time. This is really unlikely.
	 */
	num_counters = resctrl_arch_num_rmid_idx();

	if (mpam_has_feature(mpam_feat_msmon_mbwu, class->features)) {
		if (class->num_mbwu_mon >= num_counters) {
			/*
			 * We don't support this use of monitors, let the
			 * world know this platform could make use of them
			 * if we did!
			 */
			pr_info_once("Platform has candidate class for unsupported event: MBM_TOTAL!");
		}
	}
}

/* Find what we can can export as MBA */
static void mpam_resctrl_pick_mba(void)
{
	u8 level, resctrl_llc;
	struct mpam_class *class;
	struct mpam_class *candidate = NULL;

	/* At least two partitions ... */
	if (mpam_sysprops.max_partid <= 1)
		return;

	if (mpam_resctrl_exports[RDT_RESOURCE_L3])
		resctrl_llc = 3;
	else if (mpam_resctrl_exports[RDT_RESOURCE_L2])
		resctrl_llc = 2;
	else
		resctrl_llc = 0;

	rcu_read_lock();
	list_for_each_entry_rcu(class, &mpam_classes_rcu, classes_list_rcu) {
		if (class->type == MPAM_CLASS_UNKNOWN)
			continue;

		level = class->resctrl_res.cache_level;
		if (level < resctrl_llc)
			continue;

		if (!mpam_has_feature(mpam_feat_msmon_mbwu, class->features))
			continue;

		/*
		 * There are two ways we can generate delays for MBA, either with
		 * the mbw portion bitmap, or the mbw max control.
		 */
		if (!mpam_has_feature(mpam_feat_mbw_part, class->features) ||
		    !mpam_has_feature(mpam_feat_mbw_max, class->features))
			continue;

		/* pick the class 'closest' to resctrl_llc */
		if (!candidate || (level < candidate->resctrl_res.cache_level))
			candidate = class;
	}
	rcu_read_unlock();

	if (candidate)
		mpam_resctrl_exports[RDT_RESOURCE_MBA] = candidate;
}


/* Test whether we can export MPAM_CLASS_CACHE:{2,3}? */
static void mpam_resctrl_pick_caches(void)
{
	u8 level;
	struct mpam_class *class;

	rcu_read_lock();
	list_for_each_entry_rcu(class, &mpam_classes_rcu, classes_list_rcu) {
		if (class->type != MPAM_CLASS_CACHE)
			continue;

		level = class->resctrl_res.cache_level;
		if (level != 2 && level != 3)
			continue;

		if (!mpam_has_feature(mpam_feat_cpor_part, class->features) &&
		    !mpam_has_feature(mpam_feat_msmon_csu, class->features))
			continue;

		if (!mpam_has_feature(mpam_feat_msmon_csu, class->features) &&
		    mpam_sysprops.max_partid <= 1)
			continue;

		if (class->cpbm_wd > RESCTRL_MAX_CBM)
			continue;

		if (level == 2) {
			mpam_resctrl_exports[RDT_RESOURCE_L2] = class;
			class->resctrl_res.name = "L2";

			if (!mpam_llc_class)
				mpam_llc_class = class;
		} else {
			mpam_resctrl_exports[RDT_RESOURCE_L3] = class;
			class->resctrl_res.name = "L3";

			mpam_llc_class = class;
		}
	}
	rcu_read_unlock();
}

static void mpam_convert_resctrl_cfg(struct mpam_class *class,
				     resctrl_config_t resctrl_cfg,
				     struct mpam_component_cfg_update *cfg)
{
	if (class == mpam_resctrl_exports[RDT_RESOURCE_MBA]) {
		u64 range;

		/* For MBA cfg is a percentage of .. */
		if (class->resctrl_mba_uses_mbw_part) {
			/* .. the number of bits we can set */
			range = class->mbw_pbm_bits;
			cfg->feat = mpam_feat_mbw_part;
		} else {
			/* .. the number of fractions we can represent */
			range = (1ULL << class->bwa_wd) - 1;
			cfg->feat = mpam_feat_mbw_max;
		}

		cfg->mpam_cfg = (resctrl_cfg * range) / MAX_MBA_BW;
	} else {
		/*
		 * Nothing clever here as mpam_resctrl_pick_caches()
		 * capped the size at RESCTRL_MAX_CBM.
		 */
		cfg->mpam_cfg = resctrl_cfg;
		cfg->feat = mpam_feat_cpor_part;
	}
}

static void resource_reset_cfg(struct mpam_class *class, struct mpam_component *comp)
{
	int i;

	for (i = 0; i < mpam_resctrl_num_closid(); i++)
		comp->resctrl_cfg[i] = class->resctrl_res.default_ctrl;
}

static int resource_alloc_cfg(struct mpam_class *class)
{
	struct rdt_resource *res;
	struct mpam_component *comp;
	u16 num_cfgs = mpam_resctrl_num_closid();

	res = &class->resctrl_res;
	if (!res->alloc_capable)
		return 0;

	list_for_each_entry(comp, &class->components, class_list) {
		comp->resctrl_cfg = kcalloc(num_cfgs,
					    sizeof(*comp->resctrl_cfg),
					    GFP_KERNEL);
		if (!comp->resctrl_cfg)
			return -ENOMEM;

		resource_reset_cfg(class, comp);
	}

	return 0;
}

static int mpam_resctrl_resource_init(struct mpam_class *class)
{
	struct rdt_resource *res;

	res = &class->resctrl_res;

	if (class == mpam_resctrl_exports[RDT_RESOURCE_MBA]) {
		res->default_ctrl = MAX_MBA_BW;
		res->data_width = 3;
		res->membw.delay_linear = true;
		res->name = "MB";

		/* TODO: kill these */
		res->format_str = "%d=%0*u";
		res->fflags = RFTYPE_RES_MB;

		if (mpam_has_feature(mpam_feat_mbw_part, class->features)) {
			class->resctrl_mba_uses_mbw_part = true;

			/*
			 * The maximum throttling is the number of bits we can
			 * unset in the bitmap. We never clear all of them, so the
			 * minimum is one bit, as a percentage.
			 */
			res->membw.min_bw = MAX_MBA_BW / class->mbw_pbm_bits;
		} else {
			/* we're using mpam_feat_mbw_max's */
			class->resctrl_mba_uses_mbw_part = false;

			/*
			 * The maximum throttling is the number of fractions we
			 * can represent with the implemented bits. We never
			 * set 0. The minimum is the LSB, as a percentage.
			 */
			res->membw.min_bw = MAX_MBA_BW / ((1ULL << class->bwa_wd) - 1);
		}

		/* Just in case we have an excessive number of bits */
		if (!res->membw.min_bw)
			res->membw.min_bw = 1;

		/*
		 * because its linear with no offset, the granule is the same
		 * as the smallest value
		 */
		res->membw.bw_gran = res->membw.min_bw;

		/* We will only pick a class that can monitor and control */
		res->alloc_capable = true;
		exposed_alloc_capable = true;
		res->mon_capable = true;
	} else {
		res->cache.cbm_len = class->cpbm_wd;
		res->cache.arch_has_sparse_bitmaps = true;

		/* mpam_devices will reject empty bitmaps */
		res->cache.min_cbm_bits = 1;

		/* TODO: kill these properties off as they are derivatives */
		res->format_str = "%d=%0*x";
		res->fflags = RFTYPE_RES_CACHE;
		res->default_ctrl = BIT_MASK(class->cpbm_wd) - 1;
		res->data_width = (class->cpbm_wd + 3) / 4;

		/*
		 * Which bits are shared with other ...things...
		 * Unknown devices use partid-0 which uses all the bitmap
		 * fields. Until we configured the SMMU and GIC not to do this
		 * 'all the bits' is the correct answer here.
		 */
		res->cache.shareable_bits = res->default_ctrl;

		if (mpam_has_feature(mpam_feat_cpor_part, class->features)) {
			res->alloc_capable = true;
			exposed_alloc_capable = true;
		}

		/*
		 * While this is a CPU-interface feature of MPAM, we only tell
		 * resctrl about it for caches, as that seems to be how x86
		 * works, and thus what resctrl expects.
		 */
		res->cdp_capable = true;
	}

	return resource_alloc_cfg(class);
}

void mpam_resctrl_reset_resources(void)
{
	int i;
	struct mpam_class *class;
	struct mpam_component *comp;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		class = mpam_resctrl_exports[i];

		if (!class->resctrl_res.alloc_capable)
			continue;

		list_for_each_entry(comp, &class->components, class_list)
			resource_reset_cfg(class, comp);
	}

	mpam_reset_devices();
}

static int mpam_resctrl_allocate_dummy_resource(enum resctrl_resource_level l)
{
	struct mpam_class *class;

	class = kzalloc(sizeof(*class), GFP_KERNEL);
	if (!class)
		return -ENOMEM;
	INIT_LIST_HEAD(&class->resctrl_res.domains);
	INIT_LIST_HEAD(&class->resctrl_res.evt_list);
	INIT_LIST_HEAD(&class->components);

	/* list_emtpy(&class->classes_list_rcu) is how we spot dummy classes */
	INIT_LIST_HEAD(&class->classes_list_rcu);

	mpam_resctrl_exports[l] = class;

	return 0;
}

int mpam_resctrl_init(void)
{
	int rc;
	enum resctrl_resource_level i;

	mpam_resctrl_pick_caches();
	mpam_resctrl_pick_mba();

	mpam_resctrl_pick_event_l3_occup();
	mpam_resctrl_pick_event_mbm_total();

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		if (mpam_resctrl_exports[i])
			rc = mpam_resctrl_resource_init(mpam_resctrl_exports[i]);
		else
			rc = mpam_resctrl_allocate_dummy_resource(i);
		if (rc)
			return rc;

		mpam_resctrl_exports[i]->resctrl_res.rid = i;
	}

	if (!exposed_alloc_capable && !exposed_mon_capable)
		return -EOPNOTSUPP;

	return resctrl_init();
}

void mpam_resctrl_get_config(struct rdt_resource *res, struct rdt_domain *d,
			     u16 hw_closid, u32 *value)
{
	struct mpam_component *comp;

	if (!res->alloc_capable || hw_closid >= mpam_resctrl_num_closid())
		*value = res->default_ctrl;

	comp = container_of(d, struct mpam_component, resctrl_domain);
	*value = comp->resctrl_cfg[hw_closid];
}

int mpam_resctrl_cpu_online(unsigned int cpu)
{
	int i;
	bool do_online_domain;
	struct rdt_domain *dom;
	struct rdt_resource *res;
	struct mpam_class *class;
	struct mpam_component *comp;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		class = mpam_resctrl_exports[i];

		/* dummy resource, we leave its domain list empty */
		if (list_empty(&class->classes_list_rcu))
			continue;

		res = &class->resctrl_res;
		list_for_each_entry(comp, &class->components, class_list) {
			if (!cpumask_test_cpu(cpu, &comp->fw_affinity))
				continue;

			dom = &comp->resctrl_domain;
			do_online_domain = cpumask_empty(&dom->cpu_mask);
			cpumask_set_cpu(cpu, &dom->cpu_mask);

			if (do_online_domain){
				list_add(&dom->list, &res->domains);
				resctrl_online_domain(res, dom);
			}
		}
	}

	return resctrl_online_cpu(cpu);
}

static void reset_this_cpus_defaults(int cpu)
{
	mpam_set_default_partid(cpu, 0, 0);
	mpam_set_default_pmg(cpu, 0, 0);

	_mpam_thread_switch(current);
}

int mpam_resctrl_cpu_offline(unsigned int cpu)
{
	int i;
	struct rdt_domain *dom;
	struct rdt_resource *res;
	struct mpam_class *class;
	struct mpam_component *comp;

	resctrl_offline_cpu(cpu);
	reset_this_cpus_defaults(cpu);

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		class = mpam_resctrl_exports[i];

		/* dummy resource */
		if (list_empty(&class->classes_list_rcu))
			continue;

		res = &class->resctrl_res;
		list_for_each_entry(comp, &class->components, class_list) {
			if (!cpumask_test_cpu(cpu, &comp->fw_affinity))
				continue;

			dom = &comp->resctrl_domain;
			cpumask_clear_cpu(cpu, &dom->cpu_mask);

			if (cpumask_empty(&dom->cpu_mask)) {
				resctrl_offline_domain(res, dom);
				list_del(&dom->list);
			}
		}
	}

	return 0;
}

void resctrl_arch_update_cpu_defaults(void *info)
{
	u16 partid_i, partid_d;
	struct resctrl_cpu_sync *d = info;

	if (d) {
		partid_i = hwclosid_val(d->closid_code);
		partid_d = hwclosid_val(d->closid_data);
		mpam_set_default_partid(smp_processor_id(), partid_d, partid_i);
		mpam_set_default_pmg(smp_processor_id(), d->rmid, d->rmid);
	}

	resctrl_sched_in();
}

struct mpam_component_cfg_update *
mpam_resctrl_get_converted_config(struct mpam_class *class,
				  struct mpam_component *comp, u16 partid,
				  struct mpam_component_cfg_update *cfg)
{
	resctrl_config_t resctrl_cfg;

	/* Not exported as a configurable resource: reset */
	if (!class->resctrl_res.alloc_capable)
		return NULL;

	/* Out of range: reset */
	if (partid >= mpam_resctrl_num_closid())
		return NULL;

	resctrl_cfg = comp->resctrl_cfg[partid];
	/* resctrl:reset value, keep the mpam:reset value */
	if (resctrl_cfg == class->resctrl_res.default_ctrl)
		return NULL;

	cfg->partid = partid;
	mpam_convert_resctrl_cfg(class, resctrl_cfg, cfg);

	return cfg;
}

int mpam_resctrl_update_one(struct rdt_resource *r, struct rdt_domain *d,
			    u16 hw_closid, u32 resctrl_val)
{
	struct mpam_class *class;
	struct mpam_component *comp;
	struct mpam_component_cfg_update cfg;
	struct mpam_component_cfg_update *cfg_p;

	lockdep_assert_cpus_held();

	class = container_of(r, struct mpam_class, resctrl_res);
	comp = container_of(d, struct mpam_component, resctrl_domain);

	if (hw_closid >= mpam_resctrl_num_closid())
		return -EINVAL;

	comp->resctrl_cfg[hw_closid] = resctrl_val;
	cfg_p = mpam_resctrl_get_converted_config(class, comp, hw_closid, &cfg);

	return mpam_component_apply_all(class, comp, cfg_p);
}

/* call with cpus_read_lock() held */
struct rdt_domain *mpam_resctrl_find_domain(struct rdt_resource *r, int id)
{
	struct mpam_class *class;
	struct mpam_component *comp;

	if (!r)
		return NULL;

	class = container_of(r, struct mpam_class, resctrl_res);
	comp = mpam_component_get(class, id, false);
	if (IS_ERR(comp))
		return NULL;

	return &comp->resctrl_domain;
}

int mpam_resctrl_rmid_read(u16 closid, u32 rmid, enum resctrl_event_id eventid,
			   u64 *res)
{
	u16 mon_id;
	int err = -EINVAL, cpu;
	unsigned long timeout;
	struct rdt_domain *dom;
	struct mpam_class *class;
	struct mpam_component *comp;
	struct mpam_component_cfg_update mon_cfg;

	lockdep_assert_cpus_held();

	*res = 0;
	class = mpam_resctrl_events[eventid];
	if ( !class)
		return -EINVAL;

	/* rmid_read seems to be implicitly for 'this' domain */
	cpu = get_cpu();
	dom = resctrl_get_domain_from_cpu(cpu, &class->resctrl_res);
	comp = container_of(dom, struct mpam_component, resctrl_domain);
	put_cpu();

	if (eventid == QOS_L3_OCCUP_EVENT_ID) {
		err = mpam_alloc_csu_mon(class);
		if (err < 0)
			return err;
		mon_id = err;

		mon_cfg.mon = mon_id;
		mon_cfg.partid = closid;
		mon_cfg.pmg = rmid;
		mon_cfg.match_pmg = true;
		mon_cfg.feat = mpam_feat_msmon_csu;

		err = mpam_component_configure_mon(class, comp, &mon_cfg);
		if (err) {
			mpam_free_csu_mon(class, mon_id);
			return err;
		}

		timeout = READ_ONCE(jiffies) + (NRDY_TIMEOUT*SEC_CONVERSION);
		do {
			if (time_after(READ_ONCE(jiffies), timeout)) {
				err = -ETIMEDOUT;
				break;
			}

			err = mpam_component_read_mon(class, comp, &mon_cfg, res);
			if (err == -EBUSY)
				cond_resched();
		} while (err == -EBUSY);

		mpam_free_csu_mon(class, mon_id);
	}

	return err;
}
