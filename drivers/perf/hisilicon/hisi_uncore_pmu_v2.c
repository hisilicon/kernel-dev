/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2017 Hisilicon Limited
 * Author: Anurup M <anurup.m@huawei.com>
 *
 * This code is based on the uncore PMUs like arm-cci and arm-ccn.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/bitmap.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/perf_event.h>

#include "hisi_uncore_pmu_v2.h"

/*
 * PMU format attributes
 */
ssize_t hisi_format_sysfs_show_v2(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct dev_ext_attribute *eattr;

	eattr = container_of(attr, struct dev_ext_attribute, attr);
	return sprintf(buf, "%s\n", (char *) eattr->var);
}

/*
 * PMU event attributes
 */
ssize_t hisi_event_sysfs_show_v2(struct device *dev,
			      struct device_attribute *attr, char *page)
{
	struct perf_pmu_events_attr *pmu_attr =
		container_of(attr, struct perf_pmu_events_attr, attr);

	return sprintf(page, "%s", pmu_attr->event_str);
}

/*
 * sysfs cpumask attributes
 */
ssize_t hisi_cpumask_sysfs_show_v2(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pmu *pmu = dev_get_drvdata(dev);
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(pmu);

	return cpumap_print_to_pagebuf(true, buf, &hisi_pmu->cpus);
}

/* The counter overflow IRQ is not supported for some PMUs
 * use hrtimer to periodically poll and avoid overflow
 */
static enum hrtimer_restart hisi_hrtimer_callback(struct hrtimer *hrtimer)
{
	struct hisi_pmu_v2 *hisi_pmu = container_of(hrtimer,
						 struct hisi_pmu_v2, hrtimer);
	struct perf_event *event;
	unsigned long flags;

	/* Return if no active events */
	if (!hisi_pmu->num_active)
		return HRTIMER_NORESTART;

	local_irq_save(flags);

	/* Update event count for each active event */
	list_for_each_entry(event, &hisi_pmu->active_list, active_entry) {
		/* Read hardware counter and update the Perf event counter */
		hisi_pmu->ops->event_update(event);
	}

	local_irq_restore(flags);
	hrtimer_forward_now(hrtimer, ms_to_ktime(hisi_pmu->hrt_duration));
	return HRTIMER_RESTART;
}

void hisi_hrtimer_init(struct hisi_pmu_v2 *hisi_pmu, u64 timer_interval)
{
	/* hr timer clock initalization */
	hrtimer_init(&hisi_pmu->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hisi_pmu->hrtimer.function = &hisi_hrtimer_callback;
	hisi_pmu->hrt_duration = timer_interval;
}

void hisi_hrtimer_start(struct hisi_pmu_v2 *hisi_pmu)
{
	hrtimer_start(&hisi_pmu->hrtimer,
		      ms_to_ktime(hisi_pmu->hrt_duration),
		      HRTIMER_MODE_REL_PINNED);
}

void hisi_hrtimer_stop(struct hisi_pmu_v2 *hisi_pmu)
{
	hrtimer_cancel(&hisi_pmu->hrtimer);
}

/* djtag read interface - Call djtag driver to access SoC registers */
void hisi_djtag_readreg(int module_id, int bank, u32 offset,
			struct hisi_djtag_client *client, u32 *value)
{
	u32 chain_id = 0;

	/* Get the chain_id from bank_select, to use in djtag */
	if (bank != 1)
		chain_id = DJTAG_GET_CHAIN_ID(bank);

	hisi_djtag_readl(client, offset, module_id, chain_id, value);
}

/* djtag write interface - Call djtag driver  to access SoC registers */
void hisi_djtag_writereg(int module_id, int bank, u32 offset,
			 u32 value, struct hisi_djtag_client *client)
{
	u32 chain_id = 0;

	/* Get the chain_id from bank_select, to use in djtag */
	if (bank != 1)
		chain_id = DJTAG_GET_CHAIN_ID(bank);

	hisi_djtag_writel(client, offset, module_id, (1 << chain_id), value);
}

static bool hisi_validate_event_group(struct perf_event *event)
{
	struct perf_event *sibling, *leader = event->group_leader;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);
	int counters = 1; /* Include count for the event */

	/*
	 * We must NOT create groups containing mixed PMUs, although
	 * software events are acceptable
	 */
	if (leader->pmu != event->pmu && !is_software_event(leader))
		return false;

	/* Increment counter for the leader */
	counters++;

	list_for_each_entry(sibling, &event->group_leader->sibling_list,
			    group_entry) {
		if (is_software_event(sibling))
			continue;
		if (sibling->pmu != event->pmu)
			return false;
		/* Increment counter for each sibling */
		counters++;
	}

	/* The group can not count events more than the counters in the HW */
	return counters <= hisi_pmu->num_counters;
}

int hisi_uncore_pmu_event_init_v2(struct perf_event *event)
{
	struct hisi_pmu_v2 *hisi_pmu;
	struct hw_perf_event *hwc = &event->hw;
	int cpu;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/*
	 * We do not support sampling as the counters are all
	 * shared by all CPU cores in a CPU die(SCCL). Also we
	 * do not support attach to a task(per-process mode)
	 */
	if (is_sampling_event(event) || event->attach_state & PERF_ATTACH_TASK)
		return -EOPNOTSUPP;

	/* counters do not have these bits */
	if (event->attr.exclude_user	||
	    event->attr.exclude_kernel	||
	    event->attr.exclude_host	||
	    event->attr.exclude_guest	||
	    event->attr.exclude_hv	||
	    event->attr.exclude_idle)
		return -EINVAL;

	/*
	 *  The uncore counters not specific to any CPU, so cannot
	 *  support per-task
	 */
	if (event->cpu < 0)
		return -EINVAL;

	/*
	 * Validate if the events in group does not exceed the
	 * available counters in hardware.
	 */
	if (!hisi_validate_event_group(event))
		return -EINVAL;

	/*
	 * We don't assign an index until we actually place the event onto
	 * hardware. Use -1 to signify that we haven't decided where to put it
	 * yet.
	 */
	hwc->idx		= -1;
	hwc->config		= event->attr.config;

	/* Select an available CPU to monitor events in this PMU */
	hisi_pmu = to_hisi_pmu_v2(event->pmu);
	cpu = cpumask_first(&hisi_pmu->cpus);
	if (cpu >= nr_cpu_ids)
		return -EINVAL;

	/* Enforce to use the same CPU for all events in this PMU */
	event->cpu = cpu;

	return 0;
}

/*
 * Enable counter and set the counter to count
 * the event that we're interested in.
 */
static void hisi_uncore_pmu_enable_event_v2(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	/*
	 * Set event in Event select registers.
	 */
	hisi_pmu->ops->set_evtype(hisi_pmu, GET_CNTR_IDX(hwc),
				  GET_EVENTID(event));

	/* Enable the hardware event counting */
	if (hisi_pmu->ops->enable_counter)
		hisi_pmu->ops->enable_counter(hisi_pmu, GET_CNTR_IDX(hwc));
}

/*
 * Disable counting and clear the event.
 */
static void hisi_uncore_pmu_disable_event_v2(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	/* Disable the hardware event counting */
	if (hisi_pmu->ops->disable_counter)
		hisi_pmu->ops->disable_counter(hisi_pmu, GET_CNTR_IDX(hwc));

	/*
	 * Clear event in Event select registers.
	 */
	hisi_pmu->ops->clear_evtype(hisi_pmu, GET_CNTR_IDX(hwc));
}

void hisi_uncore_pmu_set_event_period_v2(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	/*
	 * The Hisilicon PMU counters have a period of 2^32. We reduce it
	 * to 2^31 to account for the extreme interrupt latency. So we
	 * could hopefully handle the overflow interrupt before another
	 * 2^31 events occur and the counter overtakes its previous value.
	 */
	u64 val = 1ULL << (hisi_pmu->counter_bits - 1);

	local64_set(&hwc->prev_count, val);

	/* Write start value to the hardware event counter */
	hisi_pmu->ops->write_counter(hisi_pmu, hwc, (u32) val);
}

u64 hisi_uncore_pmu_event_update_v2(struct perf_event *event)
{
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = GET_CNTR_IDX(hwc);
	u64 delta, prev_raw_count, new_raw_count;

	do {
		/* Read the count from the counter register */
		new_raw_count = hisi_pmu->ops->read_counter(hisi_pmu, idx);
		prev_raw_count = local64_read(&hwc->prev_count);
	} while (local64_cmpxchg(&hwc->prev_count, prev_raw_count,
				 new_raw_count) != prev_raw_count);
	/*
	 * compute the delta
	 */
	delta = (new_raw_count - prev_raw_count) &
		HISI_MAX_PERIOD(hisi_pmu->counter_bits);
	local64_add(delta, &event->count);

	return new_raw_count;
}

void hisi_uncore_pmu_start_v2(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	if (WARN_ON_ONCE(!(hwc->state & PERF_HES_STOPPED)))
		return;

	WARN_ON_ONCE(!(hwc->state & PERF_HES_UPTODATE));
	hwc->state = 0;
	hisi_pmu->ops->set_event_period(event);

	if (flags & PERF_EF_RELOAD) {
		u64 prev_raw_count =  local64_read(&hwc->prev_count);

		hisi_pmu->ops->write_counter(hisi_pmu, hwc,
					     (u32) prev_raw_count);
	}

	/* Start hrtimer when the first event is started in this PMU */
	if (hisi_pmu->ops->start_hrtimer) {
		hisi_pmu->num_active++;
		list_add_tail(&event->active_entry, &hisi_pmu->active_list);

		if (hisi_pmu->num_active == 1)
			hisi_pmu->ops->start_hrtimer(hisi_pmu);
	}

	hisi_uncore_pmu_enable_event_v2(event);
	perf_event_update_userpage(event);
}

void hisi_uncore_pmu_stop_v2(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	hisi_uncore_pmu_disable_event_v2(event);
	WARN_ON_ONCE(hwc->state & PERF_HES_STOPPED);
	hwc->state |= PERF_HES_STOPPED;

	/* Stop hrtimer when the last event is stopped in this PMU */
	if (hisi_pmu->ops->stop_hrtimer) {
		hisi_pmu->num_active--;
		list_del(&event->active_entry);

		if (hisi_pmu->num_active == 0)
			hisi_pmu->ops->stop_hrtimer(hisi_pmu);
	}

	if (hwc->state & PERF_HES_UPTODATE)
		return;

	/* Read hardware counter and update the Perf counter statistics */
	hisi_pmu->ops->event_update(event);
	hwc->state |= PERF_HES_UPTODATE;
}

int hisi_uncore_pmu_add_v2(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);
	int idx;

	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;

	/* Get an available counter index for counting. */
	idx = hisi_pmu->ops->get_event_idx(event);
	if (idx < 0)
		return -EAGAIN;

	event->hw.idx = idx;
	hisi_pmu->pmu_events.hw_events[idx] = event;

	if (flags & PERF_EF_START)
		hisi_uncore_pmu_start_v2(event, PERF_EF_RELOAD);

	return 0;
}

void hisi_uncore_pmu_del_v2(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	hisi_uncore_pmu_stop_v2(event, PERF_EF_UPDATE);
	hisi_pmu->ops->clear_event_idx(hisi_pmu, GET_CNTR_IDX(hwc));
	perf_event_update_userpage(event);
	hisi_pmu->pmu_events.hw_events[GET_CNTR_IDX(hwc)] = NULL;
}

struct hisi_pmu_v2 *hisi_pmu_alloc_v2(struct device *dev, u32 num_cntrs)
{
	struct hisi_pmu_v2 *hisi_pmu;
	struct hisi_pmu_v2_hwevents *pmu_events;

	hisi_pmu = devm_kzalloc(dev, sizeof(*hisi_pmu), GFP_KERNEL);
	if (!hisi_pmu)
		return ERR_PTR(-ENOMEM);

	pmu_events = &hisi_pmu->pmu_events;
	pmu_events->hw_events = devm_kcalloc(dev,
					     num_cntrs,
					     sizeof(*pmu_events->hw_events),
					     GFP_KERNEL);
	if (!pmu_events->hw_events)
		return ERR_PTR(-ENOMEM);

	pmu_events->used_mask = devm_kcalloc(dev,
					     BITS_TO_LONGS(num_cntrs),
					     sizeof(*pmu_events->used_mask),
					     GFP_KERNEL);
	if (!pmu_events->used_mask)
		return ERR_PTR(-ENOMEM);

	return hisi_pmu;
}

void hisi_uncore_pmu_read_v2(struct perf_event *event)
{
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(event->pmu);

	/* Read hardware counter and update the Perf counter statistics */
	hisi_pmu->ops->event_update(event);
}

void hisi_uncore_pmu_enable_v2(struct pmu *pmu)
{
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(pmu);

	hisi_pmu->ops->start_counters(hisi_pmu);
}

void hisi_uncore_pmu_disable_v2(struct pmu *pmu)
{
	struct hisi_pmu_v2 *hisi_pmu = to_hisi_pmu_v2(pmu);

	hisi_pmu->ops->stop_counters(hisi_pmu);
}

int hisi_uncore_pmu_setup_v2(struct hisi_pmu_v2 *hisi_pmu, const char *pmu_name)
{
	/* Register the events with perf */
	return perf_pmu_register(&hisi_pmu->pmu, pmu_name, -1);
}
