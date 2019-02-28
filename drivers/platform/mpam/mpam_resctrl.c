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

	return &mpam_resctrl_exports[l]->resctrl_res;
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
		} else {
			mpam_resctrl_exports[RDT_RESOURCE_L3] = class;
			class->resctrl_res.name = "L3";
		}
	}
	rcu_read_unlock();
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

	if (class != mpam_resctrl_exports[RDT_RESOURCE_MBA]) {
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


static int mpam_resctrl_allocate_dummy_resource(enum resctrl_resource_level l)
{
	struct mpam_class *class;

	class = kzalloc(sizeof(*class), GFP_KERNEL);
	if (!class)
		return -ENOMEM;
	INIT_LIST_HEAD(&class->resctrl_res.domains);
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

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		if (mpam_resctrl_exports[i])
			rc = mpam_resctrl_resource_init(mpam_resctrl_exports[i]);
		else
			rc = mpam_resctrl_allocate_dummy_resource(i);
		if (rc)
			return rc;

		mpam_resctrl_exports[i]->resctrl_res.rid = i;
	}

	if (!exposed_alloc_capable || exposed_mon_capable)
		return -EOPNOTSUPP;

	return 0;
}

