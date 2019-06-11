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
#include <linux/rculist.h>
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

static int mpam_resctrl_resource_init(struct mpam_resctrl_res *res)
{
	/* TODO: initialise the resctrl resources */

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

	/* TODO: pick MPAM classes to map to resctrl resources */

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

static int mpam_resctrl_setup_domain(unsigned int cpu,
				     struct mpam_resctrl_res *res)
{
	struct mpam_resctrl_dom *dom;
	struct mpam_class *class = res->class;
	struct mpam_component *comp_iter, *comp;

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
		kfree(dom);
	}

	return 0;
}
