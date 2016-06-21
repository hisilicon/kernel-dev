/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2016 Huawei Technologies Limited
 * Author: Anurup M <anurup.m@huawei.com>
 *
 * This code is based on the uncore PMU's like arm-cci and
 * arm-ccn.
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
#include "hisi_uncore_l3c.h"
#include "hisi_uncore_pmu.h"

/*
 * PMU format attributes
 */
ssize_t hisi_format_sysfs_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct dev_ext_attribute *eattr;

	eattr = container_of(attr, struct dev_ext_attribute,
					     attr);
	return sprintf(buf, "%s\n", (char *) eattr->var);
}

/*
 * PMU event attributes
 */
ssize_t hisi_event_sysfs_show(struct device *dev,
				  struct device_attribute *attr, char *page)
{
	struct perf_pmu_events_attr *pmu_attr =
		container_of(attr, struct perf_pmu_events_attr, attr);

	if (pmu_attr->event_str)
		return sprintf(page, "%s", pmu_attr->event_str);

	return 0;
}

/* djtag read interface - Call djtag driver to access SoC registers */
int hisi_djtag_readreg(int module_id, int bank, u32 offset,
				struct device_node *djtag_node, u32 *pvalue)
{
	int ret;
	u32 chain_id = 0;

	while (bank != 1) {
		bank = (bank >> 0x1);
		chain_id++;
	}

	ret = hisi_djtag_readl(djtag_node, offset, module_id,
							chain_id, pvalue);
	if (ret)
		pr_warn("Djtag:%s Read failed!\n", djtag_node->full_name);

	return ret;
}

/* djtag write interface - Call djtag driver  to access SoC registers */
int hisi_djtag_writereg(int module_id, int bank,
				u32 offset, u32 value,
				struct device_node *djtag_node)
{
	int ret;

	ret = hisi_djtag_writel(djtag_node, offset, module_id,
						HISI_DJTAG_MOD_MASK, value);
	if (ret)
		pr_warn("Djtag:%s Write failed!\n", djtag_node->full_name);

	return ret;
}

void hisi_uncore_pmu_write_evtype(struct hisi_hwmod_unit *punit,
						int idx, u32 val)
{
	/* Select event based on Hardware counter Module */
	if (idx >= HISI_IDX_L3C_COUNTER0 &&
		idx <= HISI_IDX_L3C_COUNTER_MAX)
		hisi_set_l3c_evtype(punit->hwmod_data, idx, val);
}

int hisi_pmu_get_event_idx(struct hw_perf_event *hwc,
						struct hisi_hwmod_unit *punit)
{
	int event_idx = -1;
	u32 raw_event_code = hwc->config_base;
	unsigned long evtype = raw_event_code & HISI_EVTYPE_EVENT;

	/* Get the available hardware event counter index */
	/* If event type is L3C events */
	if (evtype >= HISI_HWEVENT_L3C_READ_ALLOCATE &&
			evtype <= HISI_HWEVENT_L3C_EVENT_MAX) {
		event_idx = hisi_l3c_get_event_idx(punit);
	}

	return event_idx;
}

void hisi_pmu_clear_event_idx(struct hw_perf_event *hwc,
					struct hisi_hwmod_unit *punit,
								int idx)
{
	/* Release the hardware event counter index */
	u32 raw_event_code = hwc->config_base;
	unsigned long evtype = raw_event_code & HISI_EVTYPE_EVENT;

	if (evtype >= HISI_HWEVENT_L3C_READ_ALLOCATE &&
			evtype <= HISI_HWEVENT_L3C_EVENT_MAX) {
		hisi_clear_l3c_event_idx(punit, idx);
	}
}

static int pmu_map_event(struct perf_event *event)
{
	return (int)(event->attr.config & HISI_EVTYPE_EVENT);
}

static int
__hw_perf_event_init(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	int mapping;

	mapping = pmu_map_event(event);
	if (mapping < 0) {
		pr_debug("event %x:%llx not supported\n", event->attr.type,
							 event->attr.config);
		return mapping;
	}

	/*
	 * We don't assign an index until we actually place the event onto
	 * hardware. Use -1 to signify that we haven't decided where to put it
	 * yet.
	 */
	hwc->idx		= -1;
	hwc->config_base	= 0;
	hwc->config		= 0;
	hwc->event_base		= 0;

	/*
	 * For HiSilicon SoC store the event encoding into the config_base
	 * field.
	 */
	/* For HiSilicon SoC L3C update config_base based on event encoding */
	if (mapping >= HISI_HWEVENT_L3C_READ_ALLOCATE &&
				mapping <= HISI_HWEVENT_L3C_EVENT_MAX) {
		hwc->config_base = event->attr.config;
	} else {
		return -EINVAL;
	}

	/*
	 * Limit the sample_period to half of the counter width. That way, the
	 * new counter value is far less likely to overtake the previous one
	 * unless you have some serious IRQ latency issues.
	 */
	hwc->sample_period  = HISI_MAX_PERIOD >> 1;
	hwc->last_period    = hwc->sample_period;
	local64_set(&hwc->period_left, hwc->sample_period);

	return 0;
}

int hisi_uncore_pmu_event_init(struct perf_event *event)
{
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	u32 raw_event_code = event->attr.config;
	int err;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/* we do not support sampling as the counters are all
	 * shared by all CPU cores in a CPU die(SCCL). Also we
	 * donot support attach to a task(per-process mode)
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

//	if (event->cpu < 0)
//		return -EINVAL;

	/* FIXME: event->cpu = cpumask_first(&pmu_dev->parent->cpu);
	 */

	punit = &phisi_pmu->hwmod_pmu_unit[GET_UNIT_IDX(raw_event_code)];

	err = __hw_perf_event_init(event);

	return err;
}


/* Read hardware counter and update the Perf counter statistics */
u64 hisi_uncore_pmu_event_update(struct perf_event *event,
					struct hw_perf_event *hwc,
							int idx) {
	u64 new_raw_count = 0;
	int cntr_idx = idx & ~(HISI_CNTR_SCCL_MASK);

	/*
	 * Identify Event type and read appropriate hardware
	 * counter and sum the values
	 */
	if (cntr_idx >= HISI_IDX_L3C_COUNTER0 &&
		cntr_idx <= HISI_IDX_L3C_COUNTER_MAX)
		new_raw_count = hisi_l3c_event_update(event, hwc, idx);

	return new_raw_count;
}

void hisi_uncore_pmu_enable(struct pmu *pmu)
{
	/* Enable all the PMU counters. */
}

void hisi_uncore_pmu_disable(struct pmu *pmu)
{
	/* Disable all the PMU counters. */
}

int hisi_pmu_enable_counter(struct hisi_hwmod_unit *punit,
							int idx)
{
	int ret = 0;

	/* Enable the hardware event counting */
	if (idx >= HISI_IDX_L3C_COUNTER0 &&
		idx <= HISI_IDX_L3C_COUNTER_MAX)
		ret = hisi_enable_l3c_counter(punit->hwmod_data, idx);

	return ret;
}

void hisi_pmu_disable_counter(struct hisi_hwmod_unit *punit,
							int idx)
{
	/* Disable the hardware event counting */
	int cntr_idx = idx & ~(HISI_CNTR_SCCL_MASK);

	if (cntr_idx >= HISI_IDX_L3C_COUNTER0 &&
		 cntr_idx <= HISI_IDX_L3C_COUNTER_MAX)
		hisi_disable_l3c_counter(punit->hwmod_data, idx);
}

/*
 * Enable counter and set the counter to count
 * the event that we're interested in.
 */
void hisi_uncore_pmu_enable_event(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;

	punit = &phisi_pmu->hwmod_pmu_unit[GET_UNIT_IDX(hwc->config_base)];

	/*
	 * Disable counter
	 */
	hisi_pmu_disable_counter(punit, GET_CNTR_IDX(hwc));

	/*
	 * Set event (if destined for Hisilicon SoC counters).
	 */
	hisi_uncore_pmu_write_evtype(punit, GET_CNTR_IDX(hwc),
						hwc->config_base);

	/*
	 * Enable counter
	 */
	hisi_pmu_enable_counter(punit, GET_CNTR_IDX(hwc));

}

int hisi_pmu_write_counter(struct hisi_hwmod_unit *punit,
						int idx,
						u32 value)
{
	int ret = 0;

	/* Write to the hardware event counter */
	if (idx >= HISI_IDX_L3C_COUNTER0 &&
		idx <= HISI_IDX_L3C_COUNTER_MAX)
		ret = hisi_write_l3c_counter(punit->hwmod_data, idx, value);

	return ret;
}

void hisi_pmu_event_set_period(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;

	/*
	 * The Hisilicon PMU counters have a period of 2^32. To account for the
	 * possiblity of extreme interrupt latency we program for a period of
	 * half that. Hopefully we can handle the interrupt before another 2^31
	 * events occur and the counter overtakes its previous value.
	 */
	u64 val = 1ULL << 31;

	punit = &phisi_pmu->hwmod_pmu_unit[GET_UNIT_IDX(hwc->config_base)];
	local64_set(&hwc->prev_count, val);
	hisi_pmu_write_counter(punit, GET_CNTR_IDX(hwc), val);
}

void hisi_uncore_pmu_start(struct perf_event *event,
						int pmu_flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	struct hisi_pmu_hw_events *hw_events;
	u32 unit_idx = GET_UNIT_IDX(hwc->config_base);
	unsigned long flags;

	if (unit_idx >= (HISI_SCCL_MAX - 1)) {
		pr_err("Invalid unitID=%d in event code=%lu!\n",
					unit_idx + 1, hwc->config_base);
		return;
	}

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];
	hw_events = &punit->hw_events;

	/*
	 * To handle interrupt latency, we always reprogram the period
	 * regardlesss of PERF_EF_RELOAD.
	 */
	if (pmu_flags & PERF_EF_RELOAD)
		WARN_ON_ONCE(!(hwc->state & PERF_HES_UPTODATE));

	hwc->state = 0;

	raw_spin_lock_irqsave(&hw_events->pmu_lock, flags);

	hisi_pmu_event_set_period(event);
	hisi_uncore_pmu_enable_event(event);

	raw_spin_unlock_irqrestore(&hw_events->pmu_lock, flags);
}

void hisi_uncore_pmu_stop(struct perf_event *event,
						int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;

	if (hwc->state & PERF_HES_STOPPED)
		return;

	punit = &phisi_pmu->hwmod_pmu_unit[GET_UNIT_IDX(hwc->config_base)];

	/*
	 * We always reprogram the counter, so ignore PERF_EF_UPDATE. See
	 * hisi_uncore_pmu_start()
	 */
	hisi_pmu_disable_counter(punit, GET_CNTR_IDX(hwc));
	hisi_uncore_pmu_event_update(event, hwc, GET_CNTR_IDX(hwc));
	hwc->state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;

}

int hisi_uncore_pmu_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisipmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	struct hisi_pmu_hw_events *hw_events;
	u32 unit_idx = GET_UNIT_IDX(hwc->config_base);
	int idx, err = 0;

	if (unit_idx >= (HISI_SCCL_MAX - 1)) {
		pr_err("Invalid unitID=%d in event code=%lu\n",
					unit_idx + 1, hwc->config_base);
		return -EINVAL;
	}

	punit = &phisipmu->hwmod_pmu_unit[unit_idx];
	hw_events = &punit->hw_events;

	/* If we don't have a free counter then return early. */
	idx = hisi_pmu_get_event_idx(hwc, punit);
	if (idx < 0) {
		err = idx;
		goto out;
	}

	event->hw.idx = idx;
	hw_events->events[idx] = event;

	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;
	if (flags & PERF_EF_START)
		hisi_uncore_pmu_start(event, PERF_EF_RELOAD);

	/* Propagate our changes to the userspace mapping. */
	perf_event_update_userpage(event);

out:
	return err;
}

void hisi_uncore_pmu_del(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisipmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	struct hisi_pmu_hw_events *hw_events;

	punit = &phisipmu->hwmod_pmu_unit[GET_UNIT_IDX(hwc->config_base)];
	hw_events = &punit->hw_events;

	hisi_uncore_pmu_stop(event, PERF_EF_UPDATE);
	hw_events->events[GET_CNTR_IDX(hwc)] = NULL;

	hisi_pmu_clear_event_idx(hwc, punit, GET_CNTR_IDX(hwc));

	perf_event_update_userpage(event);
}

struct hisi_pmu *hisi_pmu_alloc(struct platform_device *pdev)
{
	struct hisi_pmu *phisipmu;

	phisipmu = devm_kzalloc(&pdev->dev, sizeof(*phisipmu), GFP_KERNEL);
	if (!phisipmu)
		return ERR_PTR(-ENOMEM);

	return phisipmu;
}

int hisi_pmu_unit_init(struct platform_device *pdev,
				struct hisi_hwmod_unit *punit,
						int unit_id,
						int num_counters)
{
	punit->hw_events.events = devm_kcalloc(&pdev->dev,
				     num_counters,
				     sizeof(*punit->hw_events.events),
							     GFP_KERNEL);
	if (!punit->hw_events.events)
		return -ENOMEM;

	raw_spin_lock_init(&punit->hw_events.pmu_lock);

	punit->unit_id = unit_id;

	return 0;
}

void hisi_uncore_pmu_read(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	hisi_uncore_pmu_event_update(event, hwc, GET_CNTR_IDX(hwc));
}

int hisi_uncore_pmu_setup(struct hisi_pmu *hisipmu,
				struct platform_device *pdev,
						char *pmu_name)
{
	int ret;

	/* Register the events with perf */
	ret = perf_pmu_register(&hisipmu->pmu, pmu_name, -1);
	if (ret)
		return ret;

	return 0;
}
