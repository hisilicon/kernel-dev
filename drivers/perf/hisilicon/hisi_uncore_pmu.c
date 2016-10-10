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
				struct hisi_djtag_client *client, u32 *pvalue)
{
	int ret;
	u32 chain_id = 0;

	while (bank != 1) {
		bank = (bank >> 0x1);
		chain_id++;
	}

	ret = hisi_djtag_readl(client, offset, module_id,
						chain_id, pvalue);
	if (ret)
		dev_err(&client->dev, "read failed, ret=%d!\n", ret);

	return ret;
}

/* djtag write interface - Call djtag driver  to access SoC registers */
int hisi_djtag_writereg(int module_id, int bank,
				u32 offset, u32 value,
				struct hisi_djtag_client *client)
{
	int ret;

	ret = hisi_djtag_writel(client, offset, module_id,
						HISI_DJTAG_MOD_MASK, value);
	if (ret)
		dev_err(&client->dev, "write failed, ret=%d!\n", ret);

	return ret;
}

static int pmu_map_event(struct perf_event *event)
{
	return (int)(event->attr.config & HISI_EVTYPE_EVENT);
}

static int
__hw_perf_event_init(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct device *dev = phisi_pmu->dev;
	int mapping;

	mapping = pmu_map_event(event);
	if (mapping < 0) {
		dev_err(dev, "event %x:%llx not supported\n", event->attr.type,
							 event->attr.config);
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

	/* For HiSilicon SoC L3C update config_base based on event encoding */
	hwc->config_base = event->attr.config;

	return 0;
}

int hisi_uncore_pmu_event_init(struct perf_event *event)
{
	int err;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);

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

	if (event->cpu < 0)
		return -EINVAL;

	event->cpu = cpumask_first(&phisi_pmu->cpu);

	err = __hw_perf_event_init(event);

	return err;
}

void hisi_uncore_pmu_enable(struct pmu *pmu)
{
	/* Enable all the PMU counters. */
}

void hisi_uncore_pmu_disable(struct pmu *pmu)
{
	/* Disable all the PMU counters. */
}

/*
 * Enable counter and set the counter to count
 * the event that we're interested in.
 */
void hisi_uncore_pmu_enable_event(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);

	/* Enable the hardware event counting */
	phisi_pmu->ops->disable_counter(phisi_pmu, GET_CNTR_IDX(hwc));

	/*
	 * Set event (if destined for Hisilicon SoC counters).
	 */
	phisi_pmu->ops->set_evtype(phisi_pmu, GET_CNTR_IDX(hwc),
						hwc->config_base);

	/* Disable the hardware event counting */
	phisi_pmu->ops->enable_counter(phisi_pmu, GET_CNTR_IDX(hwc));
}

void hisi_pmu_event_set_period(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);

	/*
	 * The Hisilicon PMU counters have a period of 2^32. To account for the
	 * possiblity of extreme interrupt latency we program for a period of
	 * half that. Hopefully we can handle the interrupt before another 2^31
	 * events occur and the counter overtakes its previous value.
	 */
	u64 val = 1ULL << 31;

	local64_set(&hwc->prev_count, val);

	/* Write to the hardware event counter */
	phisi_pmu->ops->write_counter(phisi_pmu, hwc, val);
}

void hisi_uncore_pmu_start(struct perf_event *event,
						int pmu_flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_pmu_hw_events *hw_events;
	unsigned long flags;

	hw_events = &phisi_pmu->hw_events;

	if (WARN_ON_ONCE(!(hwc->state & PERF_HES_STOPPED)))
		return;

	WARN_ON_ONCE(!(hwc->state & PERF_HES_UPTODATE));
	hwc->state = 0;

	hisi_pmu_event_set_period(event);

	if (flags & PERF_EF_RELOAD) {
		u64 prev_raw_count =  local64_read(&hwc->prev_count);

		phisi_pmu->ops->write_counter(phisi_pmu, hwc,
						(u32)prev_raw_count);
	}

	hisi_uncore_pmu_enable_event(event);
	perf_event_update_userpage(event);
}

void hisi_uncore_pmu_stop(struct perf_event *event,
						int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);

	if (hwc->state & PERF_HES_UPTODATE)
		return;

	/*
	 * We always reprogram the counter, so ignore PERF_EF_UPDATE. See
	 * hisi_uncore_pmu_start()
	 */
	phisi_pmu->ops->disable_counter(phisi_pmu, GET_CNTR_IDX(hwc));

	WARN_ON_ONCE(hwc->state & PERF_HES_STOPPED);
	hwc->state |= PERF_HES_STOPPED;
	if (hwc->state & PERF_HES_UPTODATE)
		return;

	/* Read hardware counter and update the Perf counter statistics */
	phisi_pmu->ops->event_update(event, hwc, GET_CNTR_IDX(hwc));
	hwc->state |= PERF_HES_UPTODATE;
}

int hisi_uncore_pmu_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_pmu_hw_events *hw_events;
	int idx;

	hw_events = &phisi_pmu->hw_events;

	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;

	/* If we don't have a free counter then return early. */
	idx = phisi_pmu->ops->get_event_idx(phisi_pmu);
	if (idx < 0)
		return -EAGAIN;

	event->hw.idx = idx;
	hw_events->events[idx] = event;

	if (flags & PERF_EF_START)
		hisi_uncore_pmu_start(event, PERF_EF_RELOAD);

	/* Propagate our changes to the userspace mapping. */
	perf_event_update_userpage(event);

	return 0;
}

void hisi_uncore_pmu_del(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);
	struct hisi_pmu_hw_events *hw_events;

	hw_events = &phisi_pmu->hw_events;

	hisi_uncore_pmu_stop(event, PERF_EF_UPDATE);

	phisi_pmu->ops->clear_event_idx(phisi_pmu, GET_CNTR_IDX(hwc));
	perf_event_update_userpage(event);
	hw_events->events[GET_CNTR_IDX(hwc)] = NULL;
}

struct hisi_pmu *hisi_pmu_alloc(struct device *dev)
{
	struct hisi_pmu *phisi_pmu;

	phisi_pmu = devm_kzalloc(dev, sizeof(*phisi_pmu), GFP_KERNEL);
	if (!phisi_pmu)
		return ERR_PTR(-ENOMEM);

	return phisi_pmu;
}

void hisi_uncore_pmu_read(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hisi_pmu *phisi_pmu = to_hisi_pmu(event->pmu);

	/* Read hardware counter and update the Perf counter statistics */
	phisi_pmu->ops->event_update(event, hwc, GET_CNTR_IDX(hwc));
}

int hisi_uncore_common_fwprop_read(struct device *dev,
					struct hisi_pmu *phisi_pmu)
{
	if (device_property_read_u32(dev, "num-events",
					&phisi_pmu->num_events)) {
		dev_err(dev, "Cant read num-events from DT!\n");
		return -EINVAL;
	}

	if (device_property_read_u32(dev, "num-counters",
				     &phisi_pmu->num_counters)) {
		dev_err(dev, "Cant read num-counters from DT!\n");
		return -EINVAL;
	}

	/* Find the SCL ID */
	if (device_property_read_u32(dev, "scl-id",
					&phisi_pmu->scl_id)) {
		dev_err(dev, "Cant read scl-id!\n");
		return -EINVAL;
	}

	if (phisi_pmu->scl_id == 0 ||
		phisi_pmu->scl_id >= MAX_UNITS) {
		dev_err(dev, "Invalid SCL=%d!\n",
					phisi_pmu->scl_id);
		return -EINVAL;
	}

	return 0;
}

int hisi_uncore_pmu_setup(struct hisi_pmu *hisipmu,
					const char *pmu_name)
{
	int ret;

	/* Register the events with perf */
	ret = perf_pmu_register(&hisipmu->pmu, pmu_name, -1);
	if (ret)
		return ret;

	return 0;
}
