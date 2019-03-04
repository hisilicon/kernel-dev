// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.

#define pr_fmt(fmt) "mpam: " fmt

#include <linux/arm_mpam.h>
#include <linux/cacheinfo.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/resctrl.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/mpam.h>

#include "mpam_internal.h"

/*
 * The classes we've picked to map to resctrl resources.
 * Class pointer may be NULL.
 */
static struct mpam_resctrl_res mpam_resctrl_exports[RDT_NUM_RESOURCES];

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

struct rdt_resource *mpam_resctrl_get_resource(enum resctrl_resource_level l)
{
	if (l >= RDT_NUM_RESOURCES)
		return NULL;

	return &mpam_resctrl_exports[l].resctrl_res;
}

/* Test whether we can export MPAM_CLASS_CACHE:{2,3}? */
static void mpam_resctrl_pick_caches(void)
{
	struct mpam_class *class;
	struct mpam_resctrl_res *res;

	mpam_class_list_lock_held();

	list_for_each_entry(class, &mpam_classes, classes_list) {
		if (class->type != MPAM_CLASS_CACHE)
			continue;

		if (class->level != 2 && class->level != 3)
			continue;

		if (!mpam_has_feature(mpam_feat_cpor_part, class->features) &&
		    !mpam_has_feature(mpam_feat_msmon_csu, class->features))
			continue;

		if (!mpam_has_feature(mpam_feat_msmon_csu, class->features) &&
		    mpam_sysprops.max_partid <= 1)
			continue;

		if (class->cpbm_wd > RESCTRL_MAX_CBM)
			continue;

		if (class->level == 2) {
			res = &mpam_resctrl_exports[RDT_RESOURCE_L2];
			res->resctrl_res.name = "L2";
		} else {
			res = &mpam_resctrl_exports[RDT_RESOURCE_L3];
			res->resctrl_res.name = "L3";
		}
		res->class = class;
	}
}

static int mpam_resctrl_resource_init(struct mpam_resctrl_res *res)
{
	struct mpam_class *class = res->class;
	struct rdt_resource *r = &res->resctrl_res;

	/* Is this one of the two well-known caches? */
	if (class != mpam_resctrl_exports[RDT_RESOURCE_MBA].class) {
		r->cache.cbm_len = class->cpbm_wd;
		r->cache.arch_has_sparse_bitmaps = true;

		/* mpam_devices will reject empty bitmaps */
		r->cache.min_cbm_bits = 1;

		/* TODO: kill these properties off as they are derivatives */
		r->format_str = "%d=%0*x";
		r->fflags = RFTYPE_RES_CACHE;
		r->default_ctrl = BIT_MASK(class->cpbm_wd) - 1;
		r->data_width = (class->cpbm_wd + 3) / 4;

		/*
		 * Which bits are shared with other ...things...
		 * Unknown devices use partid-0 which uses all the bitmap
		 * fields. Until we configured the SMMU and GIC not to do this
		 * 'all the bits' is the correct answer here.
		 */
		r->cache.shareable_bits = r->default_ctrl;

		if (mpam_has_feature(mpam_feat_cpor_part, class->features)) {
			r->alloc_capable = true;
			exposed_alloc_capable = true;
		}

		/*
		 * While this is a CPU-interface feature of MPAM, we only tell
		 * resctrl about it for caches, as that seems to be how x86
		 * works, and thus what resctrl expects.
		 */
		r->cdp_capable = true;
	}

	return 0;
}

/* Called with the mpam classes lock held */
int mpam_resctrl_setup(void)
{
	int rc;
	struct mpam_resctrl_res *res;
	enum resctrl_resource_level i;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		res = &mpam_resctrl_exports[i];
		INIT_LIST_HEAD(&res->resctrl_res.domains);
		INIT_LIST_HEAD(&res->resctrl_res.evt_list);
		res->resctrl_res.rid = i;
	}

	mpam_resctrl_pick_caches();

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		res = &mpam_resctrl_exports[i];
		if (!res->class)
			continue;	// dummy resource

		rc = mpam_resctrl_resource_init(res);
		if (rc)
			return rc;
	}

	if (!exposed_alloc_capable && !exposed_mon_capable)
		return -EOPNOTSUPP;

	return 0;
}

static void mpam_update_from_resctrl_cfg(struct mpam_resctrl_res *res,
					 resctrl_config_t resctrl_cfg,
					 mpam_config_t *mpam_cfg)
{
	/* Is this one of the two well-known caches? */
	if (res != &mpam_resctrl_exports[RDT_RESOURCE_MBA]) {
		/*
		 * Nothing clever here as mpam_resctrl_pick_caches()
		 * capped the size at RESCTRL_MAX_CBM.
		 */
		mpam_cfg->cpbm = resctrl_cfg;
		mpam_set_feature(mpam_feat_cpor_part, &mpam_cfg->valid);
	}
}

static void resource_reset_cfg(struct mpam_resctrl_res *res,
			       struct mpam_resctrl_dom *dom)
{
	int i;
	struct rdt_resource *r = &res->resctrl_res;

	for (i = 0; i < mpam_resctrl_num_closid(); i++) {
		dom->resctrl_cfg[i] = r->default_ctrl;
		mpam_update_from_resctrl_cfg(res, r->default_ctrl,
					     &dom->comp->cfg[i]);
	}
}

void resctrl_arch_reset_resources(void)
{
	int i;
	struct rdt_domain *d;
	struct mpam_resctrl_dom *dom;
	struct mpam_resctrl_res *res;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		res = &mpam_resctrl_exports[i];

		if (!res->class)
			continue;	// dummy resource

		if (!res->resctrl_res.alloc_capable)
			continue;

		list_for_each_entry(d, &res->resctrl_res.domains, list) {
			dom = container_of(d, struct mpam_resctrl_dom,
					   resctrl_dom);
			resource_reset_cfg(res, dom);
		}
	}

	mpam_reset_devices();
}

static int mpam_resctrl_setup_domain(unsigned int cpu,
				     struct mpam_resctrl_res *res)
{
	struct mpam_resctrl_dom *dom;
	struct mpam_class *class = res->class;
	struct mpam_component *comp_iter, *comp;
	u16 num_cfgs = mpam_resctrl_num_closid();

	comp = NULL;
	list_for_each_entry(comp_iter, &class->components, class_list) {
		if (cpumask_test_cpu(cpu, &comp_iter->fw_affinity)) {
			comp = comp_iter;
			break;
		}
	}

	/* cpu with unknown exported component? */
	if (WARN_ON_ONCE(!comp))
		return 0;

	dom = kzalloc_node(sizeof(*dom), GFP_KERNEL, cpu_to_node(cpu));
	if (!dom)
		return -ENOMEM;

	dom->comp = comp;
	INIT_LIST_HEAD(&dom->resctrl_dom.list);
	dom->resctrl_dom.id = comp->comp_id;
	cpumask_set_cpu(cpu, &dom->resctrl_dom.cpu_mask);
	dom->resctrl_cfg = kcalloc(num_cfgs, sizeof(*dom->resctrl_cfg),
				   GFP_KERNEL);
	if (!dom->resctrl_cfg) {
		kfree(dom);
		return -ENOMEM;
	}

	/* Resctrl expects resources to be reset over domain offline/online */
	resource_reset_cfg(res, dom);

	/* TODO: this list should be sorted */
	list_add_tail(&dom->resctrl_dom.list, &res->resctrl_res.domains);

	return 0;
}

/* Like resctrl_get_domain_from_cpu(), but for offline CPUs */
static struct mpam_resctrl_dom *
mpam_get_domain_from_cpu(int cpu, struct mpam_resctrl_res *res)
{
	struct rdt_domain *d;
	struct mpam_resctrl_dom *dom;

	list_for_each_entry(d, &res->resctrl_res.domains, list) {
		dom = container_of(d, struct mpam_resctrl_dom, resctrl_dom);

		if (cpumask_test_cpu(cpu, &dom->comp->fw_affinity))
			return dom;
	}

	return NULL;
}

int mpam_resctrl_cpu_online(unsigned int cpu)
{
	int i;
	struct mpam_resctrl_dom *dom;
	struct mpam_resctrl_res *res;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		res = &mpam_resctrl_exports[i];

		if (!res->class)
			continue;	// dummy_resource;

		dom = mpam_get_domain_from_cpu(cpu, res);
		if (dom)
			cpumask_set_cpu(cpu, &dom->resctrl_dom.cpu_mask);
		else
			mpam_resctrl_setup_domain(cpu, res);
	}

	return 0;
}

int mpam_resctrl_cpu_offline(unsigned int cpu)
{
	int i;
	struct rdt_domain *d;
	struct mpam_resctrl_res *res;
	struct mpam_resctrl_dom *dom;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		res = &mpam_resctrl_exports[i];

		if (!res->class)
			continue;	// dummy resource

		d = resctrl_get_domain_from_cpu(cpu, &res->resctrl_res);

		/* cpu with unknown exported component? */
		if (WARN_ON_ONCE(!d))
			continue;

		cpumask_clear_cpu(cpu, &d->cpu_mask);

		if (!cpumask_empty(&d->cpu_mask))
			continue;

		list_del(&d->list);
		dom = container_of(d, struct mpam_resctrl_dom, resctrl_dom);
		kfree(dom->resctrl_cfg);
		kfree(dom);
	}

	return 0;
}
