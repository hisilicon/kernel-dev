/*
 * HiSilicon SoC DDRC Hardware event counters support
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
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/perf_event.h>
#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon DDRC event types.
 */
enum armv8_hisi_ddrc_event_types {
	HISI_HWEVENT_DDRC_FLUX_WR	= 0x0,
	HISI_HWEVENT_DDRC_FLUX_RD	= 0x01,
	HISI_HWEVENT_DDRC_FLUX_WCMD	= 0x02,
	HISI_HWEVENT_DDRC_FLUX_RCMD	= 0x03,
	HISI_HWEVENT_DDRC_EVENT_MAX	= 0x0C,
};

#define HISI_DDRC_CTRL_PERF_REG_OFF	0x010
#define HISI_DDRC_CFG_PERF_REG_OFF	0x270
#define HISI_DDRC_FLUX_WR_REG_OFF	0x380

struct hisi_ddrc_hwcfg {
	u32 channel_id;
};

struct hisi_ddrc_data {
	void __iomem *regs_base;
	DECLARE_BITMAP(event_used_mask,	HISI_HWEVENT_DDRC_EVENT_MAX);
	struct hisi_ddrc_hwcfg ddrc_hwcfg;
};

static inline int hisi_ddrc_counter_valid(int idx, struct hisi_pmu *ddrc_pmu)
{
	return (idx >= 0 && idx < HISI_HWEVENT_DDRC_EVENT_MAX);
}

/*
 * Select the counter register offset from the event code
 * In DDRC there are no special counter registers, the
 * count is read form the statistics register itself
 */
static inline u32 get_counter_reg_off(int event_code)
{
	return (HISI_DDRC_FLUX_WR_REG_OFF + (event_code * 4));
}

static u32 hisi_read_ddrc_counter(struct hisi_ddrc_data *ddrc_data,
					unsigned long event_code)
{
	u32 value;
	u32 reg_off;

	reg_off = get_counter_reg_off(event_code);

	value = readl(ddrc_data->regs_base + reg_off);

	return value;
}

static u64 hisi_ddrc_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct hisi_pmu *ddrc_pmu = to_hisi_pmu(event->pmu);
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;

	do {
		/* Read the count from the statistics register */
		new_raw_count =
			hisi_read_ddrc_counter(ddrc_data,
						hwc->config_base);
		prev_raw_count = local64_read(&hwc->prev_count);

		/*
		 *  compute the delta
		 */
		delta = (new_raw_count - prev_raw_count) &
						HISI_MAX_PERIOD;

		local64_add(delta, &event->count);
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
							prev_raw_count);

	return new_raw_count;
}

static u32 hisi_write_ddrc_counter(struct hisi_pmu *ddrc_pmu,
				struct hw_perf_event *hwc, u32 value)
{
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	u32 reg_off;
	u32 event_code = hwc->config_base;

	if (!(event_code >= HISI_HWEVENT_DDRC_FLUX_WR &&
		event_code < HISI_HWEVENT_DDRC_EVENT_MAX)) {
		dev_err(ddrc_pmu->dev, "Unknown DDR evevnt!");
		return 0;
	}

	if (!ddrc_data->regs_base) {
		dev_err(ddrc_pmu->dev,
				"DDR reg address not mapped!\n");
		return 0;
	}

	reg_off = get_counter_reg_off(event_code);

	writel(value, ddrc_data->regs_base + reg_off);

	return value;
}

static void hisi_ddrc_set_event_period(struct perf_event *event)
{
	struct hisi_pmu *ddrc_pmu = to_hisi_pmu(event->pmu);
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	struct hw_perf_event *hwc = &event->hw;
	u32 event_code = hwc->config_base;
	u32 reg_off, value;

	/*
	 * For Hisilicon DDRC PMU we save the current counter value
	 * to prev_count, as we have enabled continuous counting for
	 * DDRC.
	 */
	reg_off = get_counter_reg_off(event_code);
	value = readl(ddrc_data->regs_base + reg_off);
	local64_set(&hwc->prev_count, value);
}

static void hisi_clear_ddrc_event_idx(struct hisi_pmu *ddrc_pmu, int idx)
{
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	void *bitmap_addr;

	if (!hisi_ddrc_counter_valid(idx, ddrc_pmu)) {
		dev_err(ddrc_pmu->dev,
			"%s:Unsupported event index:%d!\n", __func__, idx);
		return;
	}

	bitmap_addr = ddrc_data->event_used_mask;

	clear_bit(idx, bitmap_addr);
}

static int hisi_ddrc_get_event_idx(struct hisi_pmu *ddrc_pmu)
{
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	unsigned long *used_mask = ddrc_data->event_used_mask;
	u32 num_counters = ddrc_pmu->num_counters;
	int event_idx;

	event_idx = find_first_zero_bit(used_mask, num_counters);
	if (event_idx == num_counters)
		return -EAGAIN;

	set_bit(event_idx, used_mask);

	return event_idx;
}

static void hisi_free_ddrc_data(struct hisi_pmu *ddrc_pmu)
{
	kfree(ddrc_pmu->hwmod_data);
	ddrc_pmu->hwmod_data = NULL;
}

static void init_hisi_ddrc(void __iomem *reg_base)
{
	u32 value;

	writel(0, reg_base + HISI_DDRC_CTRL_PERF_REG_OFF);

	value = readl(reg_base + HISI_DDRC_CFG_PERF_REG_OFF);
	value &= 0x2fffffff;
	writel(value, reg_base + HISI_DDRC_CFG_PERF_REG_OFF);

	/* Enable Continuous counting */
	writel(1, reg_base + HISI_DDRC_CTRL_PERF_REG_OFF);
}

static int init_hisi_ddrc_hwcfg_fdt(struct device *dev,
					struct hisi_ddrc_data *ddrc_data)
{
	struct hisi_ddrc_hwcfg *ddrc_hwcfg;

	ddrc_hwcfg = &ddrc_data->ddrc_hwcfg;
	if (of_property_read_u32(dev->of_node, "ch-id",
					 &ddrc_hwcfg->channel_id)) {
		dev_err(dev, "DT: Cant read ch-id!\n");
		kfree(ddrc_data);
		return -EINVAL;
	}

	return 0;
}

static int init_hisi_ddrc_data(struct platform_device *pdev,
					struct hisi_pmu *ddrc_pmu)
{
	struct device *dev = &pdev->dev;
	struct hisi_ddrc_data *ddrc_data;
	struct resource *res;
	int ret;

	ddrc_data = devm_kzalloc(dev, sizeof(struct hisi_ddrc_data),
							GFP_KERNEL);
	if (!ddrc_data)
		return -ENOMEM;

	ddrc_pmu->hwmod_data = ddrc_data;

	ddrc_pmu->hw_events.events = devm_kcalloc(dev,
					ddrc_pmu->num_counters,
					sizeof(*ddrc_pmu->hw_events.events),
					GFP_KERNEL);
	if (!ddrc_pmu->hw_events.events) {
		ret = -ENOMEM;
		goto fail;
	}

	raw_spin_lock_init(&ddrc_pmu->hw_events.pmu_lock);

	if (dev->of_node) {
		ret = init_hisi_ddrc_hwcfg_fdt(dev, ddrc_data);
		if (ret)
			return ret;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	/* Continue for zero entries */
	if (!res) {
		dev_err(dev, "No DDR reg resources!\n");
		return -EINVAL;
	}

	if (!resource_size(res)) {
		dev_err(dev, "Zero DDR reg entry!\n");
		return -EINVAL;
	}

	ddrc_data->regs_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(ddrc_data->regs_base))
		return PTR_ERR(ddrc_data->regs_base);

	init_hisi_ddrc(ddrc_data->regs_base);

	return 0;

fail:
	hisi_free_ddrc_data(ddrc_pmu);
	return ret;
}

static struct attribute *hisi_ddrc_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	NULL,
};

static struct attribute_group hisi_ddrc_format_group = {
	.name = "format",
	.attrs = hisi_ddrc_format_attr,
};

static struct attribute *hisi_ddrc_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(flux_write, "event=0x00"),
	HISI_PMU_EVENT_ATTR_STR(flux_read, "event=0x01"),
	HISI_PMU_EVENT_ATTR_STR(flux_write_cmd, "event=0x02"),
	HISI_PMU_EVENT_ATTR_STR(flux_read_cmd, "event=0x03"),
	HISI_PMU_EVENT_ATTR_STR(fluxid_write, "event=0x04"),
	HISI_PMU_EVENT_ATTR_STR(fluxid_read, "event=0x05"),
	HISI_PMU_EVENT_ATTR_STR(fluxid_write_cmd, "event=0x06"),
	HISI_PMU_EVENT_ATTR_STR(fluxid_read_cmd, "event=0x07"),
	HISI_PMU_EVENT_ATTR_STR(write_latency_cnt0, "event=0x08"),
	HISI_PMU_EVENT_ATTR_STR(read_latency_cnt0, "event=0x09"),
	HISI_PMU_EVENT_ATTR_STR(write_latency_cnt1, "event=0x0A"),
	HISI_PMU_EVENT_ATTR_STR(read_latency_cnt1, "event=0x0B"),
	HISI_PMU_EVENT_ATTR_STR(read_latency_cnt_inher,	"event=0x0C"),
	NULL,
};

static struct attribute_group hisi_ddrc_events_group = {
	.name = "events",
	.attrs = hisi_ddrc_events_attr,
};

static struct attribute *hisi_ddrc_attrs[] = {
	NULL,
};

static struct attribute_group hisi_ddrc_attr_group = {
	.attrs = hisi_ddrc_attrs,
};

static DEVICE_ATTR(cpumask, 0444, hisi_cpumask_sysfs_show, NULL);

static struct attribute *hisi_ddrc_cpumask_attrs[] = {
	&dev_attr_cpumask.attr,
	NULL,
};

static const struct attribute_group hisi_ddrc_cpumask_attr_group = {
	.attrs = hisi_ddrc_cpumask_attrs,
};

static const struct attribute_group *hisi_ddrc_pmu_attr_groups[] = {
	&hisi_ddrc_attr_group,
	&hisi_ddrc_format_group,
	&hisi_ddrc_events_group,
	&hisi_ddrc_cpumask_attr_group,
	NULL,
};

static struct hisi_uncore_ops hisi_uncore_ddrc_ops = {
	.set_event_period = hisi_ddrc_set_event_period,
	.get_event_idx = hisi_ddrc_get_event_idx,
	.clear_event_idx = hisi_clear_ddrc_event_idx,
	.event_update = hisi_ddrc_event_update,
	.write_counter = hisi_write_ddrc_counter,
};

static int hisi_ddrc_pmu_init(struct device *dev, struct hisi_pmu *ddrc_pmu)
{
	struct hisi_ddrc_data *ddrc_data = ddrc_pmu->hwmod_data;
	struct hisi_ddrc_hwcfg *ddrc_hwcfg;

	ddrc_hwcfg = &ddrc_data->ddrc_hwcfg;

	if (dev->of_node) {
		if (of_property_read_u32(dev->of_node, "scl-id",
						&ddrc_pmu->scl_id)) {
			dev_err(dev, "DT: Cant read scl-id!\n");
			return -EINVAL;
		}
	}

	ddrc_pmu->name = kasprintf(GFP_KERNEL, "hisi_ddrc%u_%u",
			ddrc_hwcfg->channel_id, ddrc_pmu->scl_id);

	ddrc_pmu->num_events = HISI_HWEVENT_DDRC_EVENT_MAX;
	/* For DDRC the num of counters = num of events */
	ddrc_pmu->num_counters = HISI_HWEVENT_DDRC_EVENT_MAX;
	ddrc_pmu->ops = &hisi_uncore_ddrc_ops;
	ddrc_pmu->dev = dev;

	/* Pick one core to use for cpumask attributes */
	cpumask_set_cpu(smp_processor_id(), &ddrc_pmu->cpu);

	return 0;
}

static const struct of_device_id ddrc_of_match[] = {
	{ .compatible = "hisilicon,hisi-pmu-ddrc-v1", },
	{},
};
MODULE_DEVICE_TABLE(of, ddrc_of_match);

static int hisi_pmu_ddrc_dev_probe(struct platform_device *pdev)
{
	struct hisi_pmu *ddrc_pmu;
	struct device *dev = &pdev->dev;
	const struct of_device_id *of_id;
	int ret;

	of_id = of_match_device(ddrc_of_match, dev);
	if (!of_id)
		return -EINVAL;

	ddrc_pmu = hisi_pmu_alloc(dev);
	if (!ddrc_pmu)
		return -ENOMEM;

	ret = init_hisi_ddrc_data(pdev, ddrc_pmu);
	if (ret)
		return ret;

	ret = hisi_ddrc_pmu_init(dev, ddrc_pmu);
	if (ret)
		return ret;

	ddrc_pmu->pmu = (struct pmu) {
		.name = ddrc_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.event_init = hisi_uncore_pmu_event_init,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_ddrc_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(ddrc_pmu, ddrc_pmu->name);
	if (ret) {
		dev_err(ddrc_pmu->dev, "hisi_uncore_pmu_init FAILED!!\n");
		return ret;
	}

	platform_set_drvdata(pdev, ddrc_pmu);

	return 0;
}

static int hisi_pmu_ddrc_dev_remove(struct platform_device *pdev)
{
	struct hisi_pmu *ddrc_pmu = platform_get_drvdata(pdev);

	perf_pmu_unregister(&ddrc_pmu->pmu);
	hisi_free_ddrc_data(ddrc_pmu);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver hisi_pmu_ddrc_driver = {
	.driver = {
		.name = "hisi-pmu-ddrc",
		.of_match_table = ddrc_of_match,
	},
	.probe = hisi_pmu_ddrc_dev_probe,
	.remove = hisi_pmu_ddrc_dev_remove,
};
module_platform_driver(hisi_pmu_ddrc_driver);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x DDRC PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anurup M");
