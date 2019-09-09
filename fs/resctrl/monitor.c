// SPDX-License-Identifier: GPL-2.0
/*
 * Resource Director Technology(RDT)
 * - Monitoring code
 *
 * Copyright (C) 2017 Intel Corporation
 *
 * Author:
 *    Vikas Shivappa <vikas.shivappa@intel.com>
 *
 * This replaces the cqm.c based on perf but we reuse a lot of
 * code and datastructures originally from Peter Zijlstra and Matt Fleming.
 *
 * More information about RDT be found in the Intel (R) x86 Architecture
 * Software Developer Manual June 2016, volume 3, section 17.17.
 */

#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "internal.h"

struct rmid_entry {
	hw_closid_t			hw_closid;
	u32				rmid;
	int				busy;
	struct list_head		list;
};

/**
 * @rmid_free_lru    A least recently used list of free RMIDs
 *     These RMIDs are guaranteed to have an occupancy less than the
 *     threshold occupancy
 */
static LIST_HEAD(rmid_free_lru);

/**
 * @rmid_limbo_count     count of currently unused but (potentially)
 *     dirty RMIDs.
 *     This counts RMIDs that no one is currently using but that
 *     may have a occupancy value > resctrl_rmid_realloc_threshold. User can
 *     change the threshold occupancy value.
 */
static unsigned int rmid_limbo_count;

/**
 * @rmid_entry - The entry in the limbo and free lists.
 */
static struct rmid_entry	*rmid_ptrs;

/*
 * Global boolean for rdt_monitor which is true if any
 * resource monitoring is enabled.
 */
bool rdt_mon_capable;

/*
 * Global to indicate which monitoring events are enabled.
 */
unsigned int rdt_mon_features;

/*
 * This is the threshold cache occupancy (in bytes) at which we will consider
 * an RMID available for re-allocation.
 */
unsigned int resctrl_rmid_realloc_threshold;

static inline struct rmid_entry *__rmid_entry(rmid_idx_t idx)
{
	rmid_idx_t found_idx;
	struct rmid_entry *entry;

	entry = &rmid_ptrs[idx];

	found_idx = resctrl_arch_rmid_idx_encode(entry->hw_closid, entry->rmid);
	WARN_ON_ONCE(found_idx != idx);

	return entry;
}


static bool rmid_dirty(struct rmid_entry *entry)
{
	u64 val = 0;

	if (resctrl_arch_rmid_read(entry->hw_closid, entry->rmid, QOS_L3_OCCUP_EVENT_ID, &val))
		return true;

	return val >= resctrl_rmid_realloc_threshold;
}

/*
 * Check the RMIDs that are marked as busy for this domain. If the
 * reported LLC occupancy is below the threshold clear the busy bit and
 * decrement the count. If the busy count gets to zero on an RMID, we
 * free the RMID
 */
void __check_limbo(struct rdt_domain *d, bool force_free)
{
	u32 rmid_idx_limit = resctrl_arch_num_rmid_idx();
	struct rmid_entry *entry;
	u32 crmid = 1, nrmid;

	/*
	 * Skip RMID 0 and start from RMID 1 and check all the RMIDs that
	 * are marked as busy for occupancy < threshold. If the occupancy
	 * is less than the threshold decrement the busy counter of the
	 * RMID and move it to the free list when the counter reaches 0.
	 */
	for (;;) {
		nrmid = find_next_bit(d->rmid_busy_llc, rmid_idx_limit, crmid);
		if (nrmid >= rmid_idx_limit)
			break;

		entry = __rmid_entry(nrmid);
		if (force_free || !rmid_dirty(entry)) {
			clear_bit(entry->rmid, d->rmid_busy_llc);
			if (!--entry->busy) {
				rmid_limbo_count--;
				list_add_tail(&entry->list, &rmid_free_lru);
			}
		}
		crmid = nrmid + 1;
	}
}

bool has_busy_rmid(struct rdt_resource *r, struct rdt_domain *d)
{
	u32 idx_limit = resctrl_arch_num_rmid_idx();

	return find_first_bit(d->rmid_busy_llc, idx_limit) != idx_limit;
}

static struct rmid_entry *resctrl_find_free_rmid(hw_closid_t hw_closid)
{
	rmid_idx_t iter_idx, tgt_idx;
	struct rmid_entry *iter;

	if (list_empty(&rmid_free_lru))
		return rmid_limbo_count ? ERR_PTR(-EBUSY) : ERR_PTR(-ENOSPC);

	list_for_each_entry(iter, &rmid_free_lru, list) {
		iter_idx = resctrl_arch_rmid_idx_encode(iter->hw_closid,
							iter->rmid);
		tgt_idx = resctrl_arch_rmid_idx_encode(hw_closid, iter->rmid);

		if (iter_idx == tgt_idx)
			return iter;
	}

	return ERR_PTR(-ENOSPC);
}

/*
 * As of now the RMIDs allocation is global.
 * However we keep track of which packages the RMIDs
 * are used to optimize the limbo list management.
 */
int alloc_rmid(hw_closid_t hw_closid)
{
	struct rmid_entry *entry;

	lockdep_assert_held(&rdtgroup_mutex);

	entry = resctrl_find_free_rmid(hw_closid);
	if (!IS_ERR(entry)) {
		list_del(&entry->list);
		return entry->rmid;
	}

	return PTR_ERR(entry);
}

static void add_rmid_to_limbo(struct rmid_entry *entry)
{
	struct rdt_resource *r = resctrl_arch_get_resource(RDT_RESOURCE_L3);
	struct rdt_domain *d;
	int cpu, ret;
	u64 val = 0;

	lockdep_assert_cpus_held();

	entry->busy = 0;
	cpu = get_cpu();
	list_for_each_entry(d, &r->domains, list) {
		if (cpumask_test_cpu(cpu, &d->cpu_mask)) {
			ret = resctrl_arch_rmid_read(entry->hw_closid,
						     entry->rmid,
						     QOS_L3_OCCUP_EVENT_ID,
						    &val);
			if (ret || val <= resctrl_rmid_realloc_threshold)
				continue;
		}

		/*
		 * For the first limbo RMID in the domain,
		 * setup up the limbo worker.
		 */
		if (!has_busy_rmid(r, d))
			cqm_setup_limbo_handler(d, CQM_LIMBOCHECK_INTERVAL, -1);
		set_bit(entry->rmid, d->rmid_busy_llc);
		entry->busy++;
	}
	put_cpu();

	if (entry->busy)
		rmid_limbo_count++;
	else
		list_add_tail(&entry->list, &rmid_free_lru);
}

void free_rmid(hw_closid_t hw_closid, u32 rmid)
{
	struct rmid_entry *entry;
	rmid_idx_t idx = resctrl_arch_rmid_idx_encode(hw_closid, rmid);

	if (!rmid)
		return;

	lockdep_assert_held(&rdtgroup_mutex);

	entry = __rmid_entry(idx);

	if (resctrl_arch_is_llc_occupancy_enabled())
		add_rmid_to_limbo(entry);
	else
		list_add_tail(&entry->list, &rmid_free_lru);
}

static int __mon_event_count(u32 rmid, struct rmid_read *rr)
{
	hw_closid_t hw_closid;
	struct mbm_state *m;
	u64 tval = 0;

	hw_closid = resctrl_closid_cdp_map(rr->rgrp->closid, CDP_BOTH);
	rr->err = resctrl_arch_rmid_read(hw_closid, rmid, rr->evtid, &tval);
	if (rr->err)
		return -EINVAL;

	switch (rr->evtid) {
	case QOS_L3_OCCUP_EVENT_ID:
		rr->val += tval;
		return 0;
	case QOS_L3_MBM_TOTAL_EVENT_ID:
		m = &rr->d->mbm_total[rmid];
		break;
	case QOS_L3_MBM_LOCAL_EVENT_ID:
		m = &rr->d->mbm_local[rmid];
		break;
	default:
		/*
		 * Code would never reach here because
		 * an invalid event id would fail the resctrl_arch_rmid_read()
		 */
		return -EINVAL;
	}

	if (rr->first) {
		memset(m, 0, sizeof(struct mbm_state));
		m->prev_bw_msr = m->prev_msr = tval;
		return 0;
	}

	m->chunks += tval - m->prev_msr;
	m->prev_msr = tval;

	rr->val += m->chunks;
	return 0;
}

/*
 * Supporting function to calculate the memory bandwidth
 * and delta bandwidth in MBps.
 */
static void mbm_bw_count(u32 rmid, struct rmid_read *rr)
{
	struct mbm_state *m = &rr->d->mbm_local[rmid];
	u64 tval = 0, cur_bw, chunks;
	hw_closid_t hw_closid;

	hw_closid = resctrl_closid_cdp_map(rr->rgrp->closid, CDP_BOTH);
	if (resctrl_arch_rmid_read(hw_closid, rmid, rr->evtid, &tval))
		return;

	chunks = tval - m->prev_bw_msr;
	m->chunks += chunks;
	cur_bw = chunks / SZ_1M;

	if (m->delta_comp)
		m->delta_bw = abs(cur_bw - m->prev_bw);
	m->delta_comp = false;
	m->prev_bw = cur_bw;
	m->prev_bw_msr = tval;
}

/*
 * This is called via IPI to read the CQM/MBM counters
 * on a domain.
 */
void mon_event_count(void *info)
{
	struct rdtgroup *rdtgrp, *entry;
	struct rmid_read *rr = info;
	struct list_head *head;

	rdtgrp = rr->rgrp;

	if (__mon_event_count(rdtgrp->mon.rmid, rr))
		return;

	/*
	 * For Ctrl groups read data from child monitor groups.
	 */
	head = &rdtgrp->mon.crdtgrp_list;

	if (rdtgrp->type == RDTCTRL_GROUP) {
		list_for_each_entry(entry, head, mon.crdtgrp_list) {
			if (__mon_event_count(entry->mon.rmid, rr))
				return;
		}
	}
}

/*
 * Feedback loop for MBA software controller (mba_sc)
 *
 * mba_sc is a feedback loop where we periodically read MBM counters and
 * adjust the bandwidth percentage values via the IA32_MBA_THRTL_MSRs so
 * that:
 *
 *   current bandwdith(cur_bw) < user specified bandwidth(user_bw)
 *
 * This uses the MBM counters to measure the bandwidth and MBA throttle
 * MSRs to control the bandwidth for a particular rdtgrp. It builds on the
 * fact that resctrl rdtgroups have both monitoring and control.
 *
 * The frequency of the checks is 1s and we just tag along the MBM overflow
 * timer. Having 1s interval makes the calculation of bandwidth simpler.
 *
 * Although MBA's goal is to restrict the bandwidth to a maximum, there may
 * be a need to increase the bandwidth to avoid uncecessarily restricting
 * the L2 <-> L3 traffic.
 *
 * Since MBA controls the L2 external bandwidth where as MBM measures the
 * L3 external bandwidth the following sequence could lead to such a
 * situation.
 *
 * Consider an rdtgroup which had high L3 <-> memory traffic in initial
 * phases -> mba_sc kicks in and reduced bandwidth percentage values -> but
 * after some time rdtgroup has mostly L2 <-> L3 traffic.
 *
 * In this case we may restrict the rdtgroup's L2 <-> L3 traffic as its
 * throttle MSRs already have low percentage values.  To avoid
 * unnecessarily restricting such rdtgroups, we also increase the bandwidth.
 */
static void update_mba_bw(struct rdtgroup *rgrp, struct rdt_domain *dom_mbm)
{
	u32 closid, rmid, cur_msr_val, new_msr_val;
	struct mbm_state *pmbm_data, *cmbm_data;
	u32 cur_bw, delta_bw, user_bw;
	struct rdt_resource *r_mba;
	struct rdt_domain *dom_mba;
	struct list_head *head;
	struct rdtgroup *entry;
	hw_closid_t hw_closid;

	r_mba = resctrl_arch_get_resource(RDT_RESOURCE_MBA);
	closid = rgrp->closid;
	rmid = rgrp->mon.rmid;
	pmbm_data = &dom_mbm->mbm_local[rmid];

	dom_mba = resctrl_get_domain_from_cpu(smp_processor_id(), r_mba);
	if (!dom_mba) {
		pr_warn_once("Failure to get domain for MBA update\n");
		return;
	}

	cur_bw = pmbm_data->prev_bw;
	user_bw = dom_mba->mba_sc[closid].mbps_val;
	delta_bw = pmbm_data->delta_bw;

	/* MBA monitor resource doesn't support CDP, we know this is both */
	hw_closid = resctrl_closid_cdp_map(closid, CDP_BOTH);
	resctrl_arch_get_config(r_mba, dom_mba, hw_closid, &cur_msr_val);

	/*
	 * For Ctrl groups read data from child monitor groups.
	 */
	head = &rgrp->mon.crdtgrp_list;
	list_for_each_entry(entry, head, mon.crdtgrp_list) {
		cmbm_data = &dom_mbm->mbm_local[entry->mon.rmid];
		cur_bw += cmbm_data->prev_bw;
		delta_bw += cmbm_data->delta_bw;
	}

	/*
	 * Scale up/down the bandwidth linearly for the ctrl group.  The
	 * bandwidth step is the bandwidth granularity specified by the
	 * hardware.
	 *
	 * The delta_bw is used when increasing the bandwidth so that we
	 * dont alternately increase and decrease the control values
	 * continuously.
	 *
	 * For ex: consider cur_bw = 90MBps, user_bw = 100MBps and if
	 * bandwidth step is 20MBps(> user_bw - cur_bw), we would keep
	 * switching between 90 and 110 continuously if we only check
	 * cur_bw < user_bw.
	 */
	if (cur_msr_val > r_mba->membw.min_bw && user_bw < cur_bw) {
		new_msr_val = cur_msr_val - r_mba->membw.bw_gran;
	} else if (cur_msr_val < MAX_MBA_BW &&
		   (user_bw > (cur_bw + delta_bw))) {
		new_msr_val = cur_msr_val + r_mba->membw.bw_gran;
	} else {
		return;
	}

	resctrl_arch_update_one(r_mba, dom_mba, hw_closid, new_msr_val);

	/*
	 * Delta values are updated dynamically package wise for each
	 * rdtgrp everytime the throttle MSR changes value.
	 *
	 * This is because (1)the increase in bandwidth is not perfectly
	 * linear and only "approximately" linear even when the hardware
	 * says it is linear.(2)Also since MBA is a core specific
	 * mechanism, the delta values vary based on number of cores used
	 * by the rdtgrp.
	 */
	pmbm_data->delta_comp = true;
	list_for_each_entry(entry, head, mon.crdtgrp_list) {
		cmbm_data = &dom_mbm->mbm_local[entry->mon.rmid];
		cmbm_data->delta_comp = true;
	}
}

static void mbm_update(struct rdt_domain *d, struct rdtgroup *prgrp, int rmid)
{
	struct rmid_read rr;

	rr.first = false;
	rr.d = d;

	/* The parent control group is used to find closid. */
	rr.rgrp = prgrp;

	/*
	 * This is protected from concurrent reads from user
	 * as both the user and we hold the global mutex.
	 */
	if (resctrl_arch_is_mbm_total_enabled()) {
		rr.evtid = QOS_L3_MBM_TOTAL_EVENT_ID;
		__mon_event_count(rmid, &rr);
	}
	if (resctrl_arch_is_mbm_local_enabled()) {
		rr.evtid = QOS_L3_MBM_LOCAL_EVENT_ID;

		/*
		 * Call the MBA software controller only for the
		 * control groups and when user has enabled
		 * the software controller explicitly.
		 */
		if (!is_mba_sc(NULL))
			__mon_event_count(rmid, &rr);
		else
			mbm_bw_count(rmid, &rr);
	}
}

/*
 * Handler to scan the limbo list and move the RMIDs
 * to free list whose occupancy < threshold_occupancy.
 */
void cqm_handle_limbo(struct work_struct *work)
{
	unsigned long delay = msecs_to_jiffies(CQM_LIMBOCHECK_INTERVAL);
	int cpu = smp_processor_id();
	struct rdt_resource *r;
	struct rdt_domain *d;

	mutex_lock(&rdtgroup_mutex);

	r = resctrl_arch_get_resource(RDT_RESOURCE_L3);
	d = container_of(work, struct rdt_domain, cqm_limbo.work);

	__check_limbo(d, false);

	if (has_busy_rmid(r, d))
		schedule_delayed_work_on(cpu, &d->cqm_limbo, delay);

	mutex_unlock(&rdtgroup_mutex);
}

/*
 * Schedule the limbo handler to run for this domain in @delay_ms.
 * If @exclude_cpu is not -1, pick any other cpu.
 */
void cqm_setup_limbo_handler(struct rdt_domain *dom, unsigned long delay_ms,
			     int exclude_cpu)
{
	unsigned long delay = msecs_to_jiffies(delay_ms);
	int cpu;

	if (exclude_cpu == -1)
		cpu = cpumask_any(&dom->cpu_mask);
	else
		cpu = cpumask_any_but(&dom->cpu_mask, exclude_cpu);

	dom->cqm_work_cpu = cpu;

	if (cpu < nr_cpu_ids)
		schedule_delayed_work_on(cpu, &dom->cqm_limbo, delay);
}

void mbm_handle_overflow(struct work_struct *work)
{
	unsigned long delay = msecs_to_jiffies(MBM_OVERFLOW_INTERVAL);
	struct rdtgroup *prgrp, *crgrp;
	int cpu = smp_processor_id();
	struct list_head *head;
	struct rdt_domain *d;

	mutex_lock(&rdtgroup_mutex);

	if (!resctrl_mounted)
		goto out_unlock;

	d = container_of(work, struct rdt_domain, mbm_over.work);

	list_for_each_entry(prgrp, &rdt_all_groups, rdtgroup_list) {
		mbm_update(d, prgrp, prgrp->mon.rmid);

		head = &prgrp->mon.crdtgrp_list;
		list_for_each_entry(crgrp, head, mon.crdtgrp_list)
			mbm_update(d, prgrp, crgrp->mon.rmid);

		if (is_mba_sc(NULL))
			update_mba_bw(prgrp, d);
	}

	schedule_delayed_work_on(cpu, &d->mbm_over, delay);

out_unlock:
	mutex_unlock(&rdtgroup_mutex);
}

/*
 * Schedule the overflow handler to run for this domain in @delay_ms.
 * If @exclude_cpu is not -1, pick any other cpu.
 */
void mbm_setup_overflow_handler(struct rdt_domain *dom, unsigned long delay_ms,
				int exclude_cpu)
{
	unsigned long delay = msecs_to_jiffies(delay_ms);
	int cpu;

	if (!resctrl_mounted)
		return;

	if (exclude_cpu == -1)
		cpu = cpumask_any(&dom->cpu_mask);
	else
		cpu = cpumask_any_but(&dom->cpu_mask, exclude_cpu);

	dom->mbm_work_cpu = cpu;

	if (cpu < nr_cpu_ids)
		schedule_delayed_work_on(cpu, &dom->mbm_over, delay);
}

static int dom_data_init(struct rdt_resource *r)
{
	u32 nr_rmid_idxs = resctrl_arch_num_rmid_idx();
	struct rmid_entry *entry = NULL;
	int i;

	rmid_ptrs = kcalloc(nr_rmid_idxs, sizeof(struct rmid_entry),
			    GFP_KERNEL);
	if (!rmid_ptrs)
		return -ENOMEM;

	for (i = 0; i < nr_rmid_idxs; i++) {
		entry = &rmid_ptrs[i];
		INIT_LIST_HEAD(&entry->list);

		resctrl_arch_rmid_idx_decode(i, &entry->hw_closid, &entry->rmid);
		list_add_tail(&entry->list, &rmid_free_lru);
	}

	/*
	 * CLOSID 0 RMID 0 is special and is always allocated. It's used for
	 * all tasks that are not monitored.
	 */
	entry = __rmid_entry(0);
	list_del(&entry->list);

	return 0;
}

struct mon_evt *alloc_mon_evt(char *name, enum resctrl_event_id id)
{
	struct mon_evt *evt;

	evt = kzalloc(sizeof(*evt), GFP_KERNEL);
	if (!evt)
		return NULL;

	INIT_LIST_HEAD(&evt->list);
	evt->name = name;
	evt->evtid = id;
	return evt;
}
/*
 * Initialize the event list for the resource.
 *
 * Note that MBM events are also part of RDT_RESOURCE_L3 resource
 * because as per the SDM the total and local memory bandwidth
 * are enumerated as part of L3 monitoring.
 * FixMe: Check evt NULL return and handle it.
 */
static void l3_mon_evt_init(struct rdt_resource *r)
{
	struct mon_evt *evt;

	if (resctrl_arch_is_llc_occupancy_enabled()) {
		evt = alloc_mon_evt("llc_occupancy", QOS_L3_OCCUP_EVENT_ID);
		list_add_tail(&evt->list, &r->evt_list);
	}
	if (resctrl_arch_is_mbm_total_enabled()) {
		evt = alloc_mon_evt("mbm_total_bytes",
				    QOS_L3_MBM_TOTAL_EVENT_ID);
		list_add_tail(&evt->list, &r->evt_list);
	}
	if (resctrl_arch_is_mbm_local_enabled()) {
		evt = alloc_mon_evt("mbm_local_bytes",
				    QOS_L3_MBM_LOCAL_EVENT_ID);
		list_add_tail(&evt->list, &r->evt_list);
	}
}

int resctrl_mon_resource_init(void)
{
	int ret;
	struct rdt_resource *r;
	enum resctrl_resource_level i;
	u32 num_rmid = resctrl_arch_system_num_rmid();
	u32 rmid_cache_size = resctrl_arch_max_rmid_threshold();

	/*
	 * A reasonable upper limit on the max threshold is the number
	 * of lines tagged per RMID if all RMIDs have the same number of
	 * lines tagged in the LLC.
	 *
	 * For a 35MB LLC and 56 RMIDs, this is ~1.8% of the LLC.
	 */
	resctrl_rmid_realloc_threshold = rmid_cache_size / num_rmid;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		r = resctrl_arch_get_resource(i);
		if (!r->mon_capable)
			continue;

		ret = dom_data_init(r);
		if (ret)
			return ret;

		l3_mon_evt_init(r);
	}

	return 0;
}

int rdt_get_mon_l3_config(struct rdt_resource *r)
{
	INIT_LIST_HEAD(&r->evt_list);
	r->mon_capable = true;

	return 0;
}
