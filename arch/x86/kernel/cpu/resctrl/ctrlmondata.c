/*
 * Resource Director Technology(RDT)
 * - Cache Allocation code.
 *
 * Copyright (C) 2016 Intel Corporation
 *
 * Authors:
 *    Fenghua Yu <fenghua.yu@intel.com>
 *    Tony Luck <tony.luck@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * More information about RDT be found in the Intel (R) x86 Architecture
 * Software Developer Manual June 2016, volume 3, section 17.17.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/kernfs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include "internal.h"

static void apply_config(struct rdt_hw_domain *hw_dom,
			 struct resctrl_staged_config *cfg,
			 cpumask_var_t cpu_mask)
{
	u32 hw_closid_val = hwclosid_val(cfg->hw_closid);
	struct rdt_domain *dom = &hw_dom->resctrl;

	if (cfg->new_ctrl != hw_dom->ctrl_val[hw_closid_val]) {
		cpumask_set_cpu(cpumask_any(&dom->cpu_mask), cpu_mask);
		hw_dom->ctrl_val[hw_closid_val] = cfg->new_ctrl;
		cfg->have_new_ctrl = false;
	}
}

int resctrl_arch_update_domains(struct rdt_resource *r)
{
	struct resctrl_staged_config *cfg;
	struct rdt_hw_domain *hw_dom;
	bool msr_param_init = false;
	struct msr_param msr_param;
	cpumask_var_t cpu_mask;
	struct rdt_domain *d;
	u32 hw_closid_val;
	int cpu, i;

	lockdep_assert_cpus_held();

	if (!zalloc_cpumask_var(&cpu_mask, GFP_KERNEL))
		return -ENOMEM;

	msr_param.res = r;

	list_for_each_entry(d, &r->domains, list) {
		hw_dom = resctrl_to_arch_dom(d);
		for (i = 0; i < ARRAY_SIZE(d->staged_config); i++) {
			cfg = &hw_dom->resctrl.staged_config[i];
			if (!cfg->have_new_ctrl)
				continue;

			apply_config(hw_dom, cfg, cpu_mask);

			hw_closid_val = hwclosid_val(cfg->hw_closid);
			if (!msr_param_init) {
				msr_param.low = hw_closid_val;
				msr_param.high = hw_closid_val;
				msr_param_init = true;
			} else {
				msr_param.low = min(msr_param.low,
						    hw_closid_val);
				msr_param.high = max(msr_param.high,
						     hw_closid_val);
			}
		}
	}

	msr_param.high += 1;

	if (cpumask_empty(cpu_mask))
		goto done;
	cpu = get_cpu();
	/* Update resource control msr on this CPU if it's in cpu_mask. */
	if (cpumask_test_cpu(cpu, cpu_mask))
		rdt_ctrl_update(&msr_param);
	/* Update resource control msr on other CPUs. */
	smp_call_function_many(cpu_mask, rdt_ctrl_update, &msr_param, 1);
	put_cpu();

done:
	free_cpumask_var(cpu_mask);

	return 0;
}

void resctrl_arch_get_config(struct rdt_resource *r, struct rdt_domain *d,
			     hw_closid_t hw_closid, u32 *value)
{
	u32 hw_closid_val = hwclosid_val(hw_closid);
	struct rdt_hw_domain *hw_dom = resctrl_to_arch_dom(d);

	*value = hw_dom->ctrl_val[hw_closid_val];
}
