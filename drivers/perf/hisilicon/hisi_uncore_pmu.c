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
#include "hisi_uncore_pmu.h"

/*
 * PMU format attributes
 */
ssize_t hisi_format_sysfs_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct dev_ext_attribute *eattr;

	eattr = container_of(attr, struct dev_ext_attribute, attr);
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

/*
 * sysfs cpumask attributes
 */
ssize_t hisi_cpumask_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pmu *pmu = dev_get_drvdata(dev);
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(pmu);

	return cpumap_print_to_pagebuf(true, buf, &hisi_pmu->cpu);
}

/* djtag read interface - Call djtag driver to access SoC registers */
int hisi_djtag_readreg(int module_id, int bank, u32 offset,
		       struct hisi_djtag_client *client, u32 *value)
{
	int ret;
	u32 chain_id = 0;

	while (bank != 1) {
		bank = (bank >> 0x1);
		chain_id++;
	}

	ret = hisi_djtag_readl(client, offset, module_id, chain_id, value);
	if (ret)
		dev_err(&client->dev, "read failed, ret=%d!\n", ret);

	return ret;
}

/* djtag write interface - Call djtag driver  to access SoC registers */
int hisi_djtag_writereg(int module_id, int bank, u32 offset,
			u32 value, struct hisi_djtag_client *client)
{
	int ret;
	u32 chain_id = 0;

	while (bank != 1) {
		bank = (bank >> 0x1);
		chain_id++;
	}

	ret = hisi_djtag_writel(client, offset, module_id,
				(1 << chain_id), value);
	if (ret)
		dev_err(&client->dev, "write failed, ret=%d!\n", ret);

	return ret;
}

static int pmu_map_event(struct perf_event *event)
{
	return (int)(event->attr.config & HISI_EVTYPE_EVENT);
}

static int hisi_hw_perf_event_init(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);
	struct device *dev = hisi_pmu->dev;
	struct perf_event *sibling;
	int mapping;

	mapping = pmu_map_event(event);
	if (mapping < 0) {
		dev_err(dev, "event %x:%llx not supported\n",
			event->attr.type, event->attr.config);
		return mapping;
	}

	/*
	 * We don't assign an index until we actually place the event onto
	 * hardware. Use -1 to signify that we haven't decided where to put it
	 * yet.
	 */
	hwc->idx		= -1;
	hwc->config		= 0;
	hwc->event_base		= 0;

	/* For HiSilicon SoC update config_base based on event encoding */
	hwc->config_base = event->attr.config;

	/*
	 * We must NOT create groups containing mixed PMUs, although
	 * software events are acceptable
	 */
	if (event->group_leader->pmu != event->pmu &&
	    !is_software_event(event->group_leader))
		return -EINVAL;

	list_for_each_entry(sibling, &event->group_leader->sibling_list,
			    group_entry)
		if (sibling->pmu != event->pmu && !is_software_event(sibling))
			return -EINVAL;

	return 0;
}

int hisi_uncore_pmu_event_init(struct perf_event *event)
{
	int err;
	struct hisi_pmu *hisi_pmu;

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

	if (event->cpu < 0)
		return -EINVAL;

	hisi_pmu = to_hisi_pmu(event->pmu);
	event->cpu = cpumask_first(&hisi_pmu->cpu);

	err = hisi_hw_perf_event_init(event);

	return err;
}

/*
 * Enable counter and set the counter to count
 * the event that we're interested in.
 */
static void hisi_uncore_pmu_enable_event(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	/*
	 * Set event in Event select registers.
	 */
	if (hisi_pmu->ops->set_evtype)
		hisi_pmu->ops->set_evtype(hisi_pmu, GET_CNTR_IDX(hwc),
					  hwc->config_base);

	/* Enable the hardware event counting */
	if (hisi_pmu->ops->enable_counter)
		hisi_pmu->ops->enable_counter(hisi_pmu, GET_CNTR_IDX(hwc));
}

/*
 * Disable counting and clear the event.
 */
static void hisi_uncore_pmu_disable_event(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	/* Disable the hardware event counting */
	if (hisi_pmu->ops->disable_counter)
		hisi_pmu->ops->disable_counter(hisi_pmu, GET_CNTR_IDX(hwc));

	/*
	 * Clear event in Event select registers.
	 */
	if (hisi_pmu->ops->clear_evtype)
		hisi_pmu->ops->clear_evtype(hisi_pmu, GET_CNTR_IDX(hwc));
}

void hisi_pmu_set_event_period(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	/*
	 * The Hisilicon PMU counters have a period of 2^32. We reduce it
	 * to 2^31 to account for the extreme interrupt latency. So we
	 * could hopefully handle the overflow interrupt before another
	 * 2^31 events occur and the counter overtakes its previous value.
	 */
	u64 val = 1ULL << 31;

	local64_set(&hwc->prev_count, val);

	/* Write start value to the hardware event counter */
	hisi_pmu->ops->write_counter(hisi_pmu, hwc, (u32) val);
}

void hisi_uncore_pmu_start(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

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

	hisi_uncore_pmu_enable_event(event);
	perf_event_update_userpage(event);
}

void hisi_uncore_pmu_stop(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	hisi_uncore_pmu_disable_event(event);
	WARN_ON_ONCE(hwc->state & PERF_HES_STOPPED);
	hwc->state |= PERF_HES_STOPPED;

	if (hwc->state & PERF_HES_UPTODATE)
		return;

	/* Read hardware counter and update the Perf counter statistics */
	hisi_pmu->ops->event_update(event, hwc, GET_CNTR_IDX(hwc));
	hwc->state |= PERF_HES_UPTODATE;
}

int hisi_uncore_pmu_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);
	int idx;

	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;

	/* Get an available counter index for counting. */
	idx = hisi_pmu->ops->get_event_idx(hisi_pmu);
	if (idx < 0)
		return -EAGAIN;

	event->hw.idx = idx;
	hisi_pmu->hw_perf_events[idx] = event;

	if (flags & PERF_EF_START)
		hisi_uncore_pmu_start(event, PERF_EF_RELOAD);

	return 0;
}

void hisi_uncore_pmu_del(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	hisi_uncore_pmu_stop(event, PERF_EF_UPDATE);
	hisi_pmu->ops->clear_event_idx(hisi_pmu, GET_CNTR_IDX(hwc));
	perf_event_update_userpage(event);
	hisi_pmu->hw_perf_events[GET_CNTR_IDX(hwc)] = NULL;
}

struct hisi_pmu *hisi_pmu_alloc(struct device *dev)
{
	struct hisi_pmu *hisi_pmu;

	hisi_pmu = devm_kzalloc(dev, sizeof(*hisi_pmu), GFP_KERNEL);
	if (!hisi_pmu)
		return ERR_PTR(-ENOMEM);

	return hisi_pmu;
}

void hisi_uncore_pmu_read(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(event->pmu);

	/* Read hardware counter and update the Perf counter statistics */
	hisi_pmu->ops->event_update(event, hwc, GET_CNTR_IDX(hwc));
}

void hisi_uncore_pmu_enable(struct pmu *pmu)
{
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(pmu);

	if (hisi_pmu->ops->start_counters)
		hisi_pmu->ops->start_counters(hisi_pmu);
}

void hisi_uncore_pmu_disable(struct pmu *pmu)
{
	struct hisi_pmu *hisi_pmu = to_hisi_pmu(pmu);

	if (hisi_pmu->ops->stop_counters)
		hisi_pmu->ops->stop_counters(hisi_pmu);
}

int hisi_uncore_pmu_setup(struct hisi_pmu *hisi_pmu, const char *pmu_name)
{
	/* Register the events with perf */
	return perf_pmu_register(&hisi_pmu->pmu, pmu_name, -1);
}
