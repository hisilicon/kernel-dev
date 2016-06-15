/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2016 Huawei Technologies Limited
 * Author: Anurup M <anurup.m@huawei.com>
 *
 * This code is based heavily on the ARMv7 perf event code.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bitmap.h>
#include <linux/of.h>
#include <linux/perf_event.h>
#include "hisi_uncore_pmu.h"
#include "hisi_uncore_llc.h"

ssize_t hisi_events_sysfs_show(struct device *dev,
				  struct device_attribute *attr, char *page)
{
	struct perf_pmu_events_attr *pmu_attr =
		container_of(attr, struct perf_pmu_events_attr, attr);

	if (pmu_attr->event_str)
		return sprintf(page, "%s", pmu_attr->event_str);

	return 0;
}

/* djtag read interface - Call djtag driver */
int hisi_djtag_readreg(int module_id, int bank, u32 offset,
				struct device_node *djtag_node, u32 *pvalue)
{
	int ret;
	u32 chain_id = 0;

	while (bank != 1) {
		bank = (bank >> 0x1);
		chain_id++;
	}

	ret = djtag_readl(djtag_node, offset, module_id, chain_id, pvalue);
	if (ret)
		pr_info("Djtag:%s Read failed!\n", djtag_node->full_name);

	return ret;
}

/* djtag write interface */
int hisi_djtag_writereg(int module_id, int bank,
				u32 offset, u32 value,
				struct device_node *djtag_node)
{
	int ret;

	ret = djtag_writel(djtag_node, offset, module_id, 0, value);
	if (ret)
		pr_info("Djtag:%s Write failed!\n", djtag_node->full_name);

	return ret;
}

int hisi_uncore_pmu_enable_intens(struct hisi_hwmod_unit *punit, int idx)
{
	return 0;
}

int hisi_uncore_pmu_disable_intens(struct hisi_hwmod_unit *punit, int idx)
{
	return 0;
}

int hisi_pmu_get_event_idx(struct hw_perf_event *hwc,
						struct hisi_hwmod_unit *punit)
{
	int event_idx = -1;
	u32 raw_event_code = hwc->config_base;
	unsigned long evtype = raw_event_code & HISI_ARMV8_EVTYPE_EVENT;

	/* If event type is LLC events */
	if (evtype >= HISI_HWEVENT_LLC_READ_ALLOCATE &&
			evtype <= HISI_HWEVENT_LLC_DGRAM_1B_ECC) {
		event_idx = hisi_llc_get_event_idx(punit);
	}

	return event_idx;
}

void hisi_pmu_clear_event_idx(struct hw_perf_event *hwc,
					struct hisi_hwmod_unit *punit,
								int idx)
{
	u32 raw_event_code = hwc->config_base;
	unsigned long evtype = raw_event_code & HISI_ARMV8_EVTYPE_EVENT;

	if (evtype >= HISI_HWEVENT_LLC_READ_ALLOCATE &&
			evtype <= HISI_HWEVENT_LLC_DGRAM_1B_ECC) {
		hisi_clear_llc_event_idx(punit, idx);
	}

	return;
}

static int
__hw_perf_event_init(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	int mapping;
	int err;

	mapping = pmu_map_event(event);

	pr_debug("---event code=0x%llx ---\n", event->attr.config);
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
	 * Store the event encoding into the config_base field.
	 */
	/* For HiSilicon SoC LLC update config_base based on event encoding */
	if (mapping >= HISI_HWEVENT_LLC_READ_ALLOCATE &&
				mapping < HISI_HWEVENT_EVENT_MAX) {
		hwc->config_base = event->attr.config;
	}

	/*
	 * Limit the sample_period to half of the counter width. That way, the
	 * new counter value is far less likely to overtake the previous one
	 * unless you have some serious IRQ latency issues.
	 */
	hwc->sample_period  = HISI_ARMV8_MAX_PERIOD >> 1;
	hwc->last_period    = hwc->sample_period;
	local64_set(&hwc->period_left, hwc->sample_period);

	/* Initialize event counter variables to support multiple
	 * HiSilicon Soc SCCL and banks */
	if (mapping >= HISI_HWEVENT_LLC_READ_ALLOCATE &&
				mapping <= HISI_HWEVENT_LLC_DGRAM_1B_ECC) {
		/* Enable Interrupts for Counter overflow handling */
		err = hisi_init_llc_hw_perf_event(phisi_pmu, hwc);
		if (err)
			return err;
	} else {
		return -EINVAL;
	}

	return 0;
}

void hw_perf_event_destroy(struct perf_event *event)
{
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	atomic_t *active_events = NULL;
	struct mutex *reserve_mutex = NULL;
	struct hw_perf_event *hwc = &event->hw;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];
	active_events = &punit->active_events;
	reserve_mutex = &punit->reserve_mutex;

	if (atomic_dec_and_mutex_lock(active_events, reserve_mutex)) {
		/* FIXME: Release IRQ here */
		mutex_unlock(reserve_mutex);
	}
}

int hisi_uncore_pmu_event_init(struct perf_event *event)
{
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	u32 raw_event_code = event->attr.config;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	int err;
	int cpu;

	pr_debug("--- hisi_uncore_llc_pmu_event_init event->attr.type=%d "\
			"event->pmu->type=%d event->attach_state=%d "\
			"pmu_type=%d pmu_name=%s unitID=%d ---\n",
			event->attr.type, event->pmu->type, event->attach_state,
				phisi_pmu->pmu_type, phisi_pmu->name, unitID);

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/* we do not support sampling */
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

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];

	/*
	 * Following the example set by other "uncore" PMUs, we accept any CPU
	 * and rewrite its affinity dynamically rather than having perf core
	 * handle cpu == -1 and pid == -1 for this case.
	 *
	 * The perf core will pin online CPUs for the duration of this call and
	 * the event being installed into its context, so the PMU's CPU can't
	 * change under our feet.
	 */
	cpu = cpumask_first(&punit->cpus);
	if (event->cpu < 0 || cpu < 0)
		return -EINVAL;
	event->cpu = cpu;

	event->destroy = hw_perf_event_destroy;

	err = __hw_perf_event_init(event);
	if (err)
		hw_perf_event_destroy(event);

	return err;
}


/* Read hardware counter and update the Perf counter statistics */
u64 hisi_uncore_pmu_event_update(struct perf_event *event,
					struct hw_perf_event *hwc,
							int idx) {
	u64 new_raw_count = 0;
	int cntr_idx = idx & ~(HISI_CNTR_SCCL_MASK);

	/*
	 * Identify Event type and read appropriate hardware counter
	 * and sum the values
	 */
	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= cntr_idx &&
			 cntr_idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		 new_raw_count = hisi_llc_event_update(event, hwc, idx);
	}

	return new_raw_count;
}

void hisi_uncore_pmu_enable(struct pmu *pmu)
{
	/* TBD */
	pr_debug("--- hisi_uncore_pmu_enable CPU=%d ---..\n",
					smp_processor_id());
	/* Enable all the PMU counters. */
}

void hisi_uncore_pmu_disable(struct pmu *pmu)
{
	/* TBD */
	pr_debug("--- hisi_uncore_pmu_disable CPU=%d ---..\n",
					smp_processor_id());

	/* Disable all the PMU counters. */
}

int hisi_pmu_enable_counter(struct hisi_hwmod_unit *punit,
							int idx)
{
	int ret = 0;

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
			idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		ret = hisi_enable_llc_counter(punit->hwmod_data, idx);
	}

	return ret;
}

void hisi_pmu_disable_counter(struct hisi_hwmod_unit *punit,
							int idx)
{
	int cntr_idx = idx & ~(HISI_CNTR_SCCL_MASK);

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= cntr_idx &&
		 cntr_idx <=	ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		hisi_disable_llc_counter(punit->hwmod_data, idx);
	}
}

void hisi_uncore_pmu_enable_event(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	int idx = hwc->idx;

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];

	/*
	 * Enable counter and set the counter to count
	 * the event that we're interested in.
	 */

	/*
	 * Disable counter
	 */
	hisi_pmu_disable_counter(punit, idx);

	/*
	 * Set event (if destined for Hisilicon SoC counters).
	 */
	hisi_uncore_pmu_write_evtype(punit, idx, hwc->config_base);

	/*
	 * Enable counter
	 */
	hisi_pmu_enable_counter(punit, idx);

}

int hisi_pmu_write_counter(struct hisi_hwmod_unit *punit,
						int idx,
						u32 value)
{
	int ret = 0;

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
			idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		ret = hisi_write_llc_counter(punit->hwmod_data, idx, value);
	}

	return ret;
}

void hisi_pmu_event_set_period(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	int idx = hwc->idx;

	/*
	 * The Hisilicon PMU counters have a period of 2^32. To account for the
	 * possiblity of extreme interrupt latency we program for a period of
	 * half that. Hopefully we can handle the interrupt before another 2^31
	 * events occur and the counter overtakes its previous value.
	 */
	u64 val = 1ULL << 31;

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];
	local64_set(&hwc->prev_count, val);
	hisi_pmu_write_counter(punit, idx, val);;
}

void hisi_uncore_pmu_start(struct perf_event *event,
						int pmu_flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	struct hisi_pmu_hw_events *hw_events = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	unsigned long flags;

	if (!unitID || (unitID >= HISI_SCCL_MAX)) {
		pr_err("LLC: Invalid unitID=%d in event code=%d!\n",
						unitID, raw_event_code);
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
	struct hisi_hwmod_unit *punit = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	int idx = hwc->idx;

	if (hwc->state & PERF_HES_STOPPED)
		return;

	punit = &phisi_pmu->hwmod_pmu_unit[unit_idx];

	/*
	 * We always reprogram the counter, so ignore PERF_EF_UPDATE. See
	 * hisi_uncore_llc_pmu_start()
	 */
	hisi_pmu_disable_counter(punit, idx);
	hisi_uncore_pmu_event_update(event, hwc, idx);
	hwc->state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;

}

int hisi_uncore_pmu_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisipmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	hisi_llc_data *llc_hwmod_data = NULL;
	struct hisi_pmu_hw_events *hw_events = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 unitID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = unitID - 1;
	int idx, err = 0;

	if (!unitID || (HISI_SCCL_MAX < unitID)) {
		pr_err("LLC: Invalid unitID=%d in event code=%d!\n",
						unitID, raw_event_code);
		return -EINVAL;
	}

	punit = &phisipmu->hwmod_pmu_unit[unit_idx];
	llc_hwmod_data = punit->hwmod_data;
	hw_events = &punit->hw_events;

	if (!unitID || (HISI_SCCL_MAX < unitID)) {
		pr_err("Invalid unitID=%d in event code!\n", unitID);
		return -EINVAL;
	}

	/* If we don't have a space for the counter then finish early. */
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
	struct hisi_hwmod_unit *punit = NULL;
	hisi_llc_data *llc_hwmod_data = NULL;
	struct hisi_pmu_hw_events *hw_events = NULL;
	u32 raw_event_code = hwc->config_base;
	u32 scclID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 unit_idx = scclID - 1;
	int idx = hwc->idx;

	punit = &phisipmu->hwmod_pmu_unit[unit_idx];
	llc_hwmod_data = punit->hwmod_data;
	hw_events = &punit->hw_events;

	hisi_uncore_pmu_stop(event, PERF_EF_UPDATE);
	hw_events->events[idx] = NULL;
	clear_bit(idx, hw_events->used_mask);

	hisi_pmu_clear_event_idx(hwc, punit, idx);

	perf_event_update_userpage(event);
}

int pmu_map_event(struct perf_event *event)
{
	return (int)(event->attr.config & HISI_ARMV8_EVTYPE_EVENT);;
}

struct hisi_pmu *hisi_pmu_alloc(struct platform_device *pdev)
{
	struct hisi_pmu *phisipmu;

	phisipmu = devm_kzalloc(&pdev->dev, sizeof(*phisipmu), GFP_KERNEL);
	if (!phisipmu)
		return ERR_PTR(-ENOMEM);

	return phisipmu;
}

struct hisi_pmu *hisi_pmu_free(struct platform_device *pdev,
					hisi_hwmod_type hwmod_type,
						hisi_pmu_type pmu_type)
{
	return 0;
}

int hisi_pmu_unit_init(struct platform_device *pdev,
				struct hisi_hwmod_unit *punit,
						int unit_id,
						int num_counters)
{
	int ret;

	punit->hw_events.events = devm_kcalloc(&pdev->dev,
				     num_counters,
				     sizeof(*punit->hw_events.events),
							     GFP_KERNEL);
	if (!punit->hw_events.events) {
		return -ENOMEM;
	}

	punit->hw_events.used_mask = devm_kcalloc(&pdev->dev,
					BITS_TO_LONGS(num_counters),
					sizeof(*punit->hw_events.used_mask),
								GFP_KERNEL);
	if (!punit->hw_events.used_mask) {
		ret = -ENOMEM;
		goto fail;
	}

	raw_spin_lock_init(&punit->hw_events.pmu_lock);
	atomic_set(&punit->active_events, 0);
	mutex_init(&punit->reserve_mutex);

	punit->unit_id = unit_id;
	cpumask_set_cpu(smp_processor_id(), &punit->cpus);

	return 0;

fail:
	if (punit->hw_events.events)
		devm_kfree(&pdev->dev, punit->hw_events.events);

	if (punit->hw_events.used_mask)
		devm_kfree(&pdev->dev, punit->hw_events.used_mask);

	return ret;
}

void hisi_uncore_pmu_read(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;

	hisi_uncore_pmu_event_update(event, hwc, idx);
}

int hisi_uncore_pmu_setup(struct hisi_pmu *hisipmu,
				struct platform_device *pdev,
						char *pmu_name)
{
	int ret;

	ret = perf_pmu_register(&hisipmu->pmu, pmu_name, PERF_TYPE_RAW);
	if (ret)
		goto fail;

	return 0;

fail:
	return ret;
}

