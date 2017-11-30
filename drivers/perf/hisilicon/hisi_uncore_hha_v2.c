/*
 * HiSilicon SoC HHA Hardware event counters support
 *
 * Copyright (C) 2017 Hisilicon Limited
 * Author: Shaokun Zhang<zhangshaokun@hisilicon.com>
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
#include <linux/acpi.h>
#include <linux/bitmap.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/perf_event.h>
#include "hisi_uncore_pmu_v2.h"

/*
 * ARMv8 HiSilicon HHA event types.
 */
enum armv8_hisi_hha_event_types {
	HISI_HWEVENT_HHA_RX_UNLINKFILT          = 0x0,
	HISI_HWEVENT_HHA_TX_RETRY               = 0x01,
	HISI_HWEVENT_HHA_TX_PGRANT              = 0x02,
	HISI_HWEVENT_HHA_SNP_LLC                = 0x06,
	HISI_HWEVENT_HHA_TX_RD_DDRC             = 0x08,
	HISI_HWEVENT_HHA_TX_WR_DDRC             = 0x09,
	HISI_HWEVENT_HHA_EVENT_MAX              = 0x29,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_hha_counters {
	HISI_IDX_HHA_COUNTER0		= 0x0,
	HISI_IDX_HHA_COUNTER_MAX	= 0xC,
};

#define HHA_EVTYPE_REG_OFF 0x100
#define HHA_EVCTRL_REG_OFF 0xC0
#define HHA_CNT0_REG_OFF 0x10c
#define HHA_EVENT_EN 0x1
#define HHA_CFG_EN	0x1

/*
 * Default timer frequency to poll and avoid counter overflow.
 * CPU speed = 2.4Ghz, Therefore Access time = 0.4ns
 * L1 cache - 2 way set associative
 * L2  - 16 way set associative
 * L3  - 16 way set associative. L3 cache has 4 banks.
 *
 * Overflow time = 2^31 * (access time L1 + access time L2 + access time L3)
 * = 2^31 * ((2 * 0.4ns) + (16 * 0.4ns) + (4 * 16 * 0.4ns)) = 70 seconds
 *
 * L3 cache is also used by devices like PCIe, SAS etc. at
 * the same time. So the overflow time could be even smaller.
 * So on a safe side we use a timer interval of 10sec
 */
#define HHA_HRTIMER_INTERVAL (10LL * MSEC_PER_SEC)

#define GET_MODULE_ID(hwmod_data) hwmod_data->module_id
#define HHA_EVTYPE_REG(idx)	(HHA_EVTYPE_REG_OFF + (idx <= 3 ? 0 : 4 * \
						       (idx / 4)))

struct hisi_hha_data {
	struct hisi_djtag_client *client;
	DECLARE_BITMAP(event_used_mask, HISI_IDX_HHA_COUNTER_MAX);
	u32 module_id;
};

static inline int hisi_hha_counter_valid(int idx)
{
	return (idx >= HISI_IDX_HHA_COUNTER0 && idx < HISI_IDX_HHA_COUNTER_MAX);
}

/* Select the counter register offset from the index */
static inline u32 get_counter_reg_off(int cntr_idx)
{
	return (HHA_CNT0_REG_OFF + (cntr_idx * 8));
}

static u64 hisi_hha_read_counter(struct hisi_pmu_v2 *hha_pmu, int cntr_idx)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	struct hisi_djtag_client *client = hha_data->client;
	u32 module_id = GET_MODULE_ID(hha_data);
	u32 reg_off;
	u64 lower, upper, value;

	lower = upper = value = 0;
	reg_off = get_counter_reg_off(cntr_idx);
	hisi_djtag_readreg(module_id, HHA_CFG_EN, reg_off, client, (u32*)&lower);
	hisi_djtag_readreg(module_id, HHA_CFG_EN, reg_off + 0x4, client,
			   (u32*) &upper);
	value = ((upper & 0xFFFF) << 32) + lower;

	return value;
}

static void hisi_hha_set_event_period(struct perf_event *event)
{
	struct hisi_pmu_v2 *hha_pmu = to_hisi_pmu_v2(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	u64 value;
	int idx = GET_CNTR_IDX(hwc);

	/*
	 * For Hisilicon HHA PMU we save the current counter value
	 * to prev_count, as we have enabled continuous counting for
	 * HHA.
	 */

	value = hisi_hha_read_counter(hha_pmu, idx);
	local64_set(&hwc->prev_count, value);
}

static void hisi_hha_set_evtype(struct hisi_pmu_v2 *hha_pmu, int idx, u32 val)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	struct hisi_djtag_client *client = hha_data->client;
	u32 module_id = GET_MODULE_ID(hha_data);
	u32 reg_off, event_value, value = 0;

	event_value = (val - HISI_HWEVENT_HHA_RX_UNLINKFILT);

	/*
	 * Select the appropriate Event select register(HHA_EVENT_TYPEx).
	 * There are 2 Event Select registers for the 8 hardware counters.
	 * For the first 4 hardware counters, the HHA_EVTYPE_REG_OFF is chosen.
	 * For the next 4 hardware counters, the second register is chosen.
	 */
	reg_off = HHA_EVTYPE_REG(idx);

	/*
	 * Write the event code in HHA_EVENT_TYPEx Register
	 * Each byte in the 32 bit event select register is used to configure
	 * the event code. Each byte correspond to a counter register to use.
	 * Use (idx % 4) to select the byte to update in event select register
	 * with the event code.
	 */
	val = event_value << (8 * (idx % 4));

	hisi_djtag_readreg(module_id, HHA_CFG_EN, reg_off, client, &value);
	value &= ~(0xff << (8 * (idx % 4)));
	value |= val;
	hisi_djtag_writereg(module_id, HHA_CFG_EN, reg_off, value, client);
}

static void hisi_hha_clear_evtype(struct hisi_pmu_v2 *hha_pmu, int idx)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	struct hisi_djtag_client *client = hha_data->client;
	u32 module_id = GET_MODULE_ID(hha_data);
	u32 reg_off, value;

	if (!hisi_hha_counter_valid(idx)) {
		dev_err(hha_pmu->dev, "Unsupported event index:%d!\n", idx);
		return;
	}

	/*
	 * Clear Counting in HHA event config register.
	 * Select the appropriate Event select register(HHA_EVENT_TYPEx).
	 * There are 3 Event Select registers for the 12 hardware counters.
	 * For the first 4 hardware counters, the HHA_EVTYPE_REG_OFF is chosen.
	 * For the next 4 hardware counters, the second register is chosen.
	 */
	reg_off = HHA_EVTYPE_REG(idx);

	/*
	 * Clear the event in HHA_EVENT_TYPEx Register
	 * Each byte in the 32 bit event select register is used to configure
	 * the event code. Each byte correspond to a counter register to use.
	 * Use (idx % 4) to select the byte to clear in event select register
	 * with the vale 0xff.
	 */
	hisi_djtag_readreg(module_id, HHA_CFG_EN, reg_off, client, &value);
	value &= ~(0xff << (8 * (idx % 4)));
	value |= (0xff << (8 * (idx % 4)));
	hisi_djtag_writereg(module_id, HHA_CFG_EN, reg_off, value, client);
}

/* counter reg in hha is RO */
static void hisi_hha_write_counter(struct hisi_pmu_v2 *hha_pmu,
				   struct hw_perf_event *hwc, u32 value)
{
	return;
}

static void hisi_hha_start_counters(struct hisi_pmu_v2 *hha_pmu)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	struct hisi_djtag_client *client = hha_data->client;
	unsigned long *used_mask = hha_data->event_used_mask;
	u32 module_id = GET_MODULE_ID(hha_data);
	u32 num_counters = hha_pmu->num_counters;
	u32 value;
	int enabled = bitmap_weight(used_mask, num_counters);

	if (!enabled)
		return;

	/*
	 * Set the event_bus_en bit HHA AUCNTRL to start counting
	 */
	hisi_djtag_readreg(module_id, HHA_CFG_EN, HHA_EVCTRL_REG_OFF,
			   client, &value);
	value |= HHA_EVENT_EN;
	hisi_djtag_writereg(module_id, HHA_CFG_EN, HHA_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_hha_stop_counters(struct hisi_pmu_v2 *hha_pmu)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	struct hisi_djtag_client *client = hha_data->client;
	u32 module_id = GET_MODULE_ID(hha_data);
	u32 value;

	/*
	 * Clear the event_bus_en bit in HHA AUCNTRL
	 */
	hisi_djtag_readreg(module_id, HHA_CFG_EN, HHA_EVCTRL_REG_OFF,
			   client, &value);
	value &= ~(HHA_EVENT_EN);
	hisi_djtag_writereg(module_id, HHA_CFG_EN, HHA_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_hha_clear_event_idx(struct hisi_pmu_v2 *hha_pmu, int idx)
{
	struct hisi_hha_data *hha_data = hha_pmu->hwmod_data;
	if (!hisi_hha_counter_valid(idx)) {
		dev_err(hha_pmu->dev, "Unsupported event index:%d!\n", idx);
		return;
	}
	clear_bit(idx, hha_data->event_used_mask);
}

static int hisi_hha_get_event_idx(struct perf_event *event)
{
	struct hisi_pmu_v2 *hha_pmu = to_hisi_pmu_v2(event->pmu);
	unsigned long *used_mask = hha_pmu->pmu_events.used_mask;
	u32 num_counters = hha_pmu->num_counters;
	int event_idx;

	event_idx = find_first_zero_bit(used_mask, num_counters);
	if (event_idx == num_counters)
		return -EAGAIN;

	set_bit(event_idx, used_mask);

	return event_idx;
}

static const struct of_device_id hha_of_match[] = {
	{ .compatible = "hisilicon,hip05-pmu-hha-v1", },
	{ .compatible = "hisilicon,hip06-pmu-hha-v1", },
	{ .compatible = "hisilicon,hip07-pmu-hha-v2", },
	{},
};
MODULE_DEVICE_TABLE(of, hha_of_match);

static const struct acpi_device_id hisi_hha_pmu_acpi_match[] = {
	{ "HISI0241", },
	{ "HISI0242", },
	{},
};
MODULE_DEVICE_TABLE(acpi, hisi_hha_pmu_acpi_match);

static int hisi_hha_get_module_instance_id(struct device *dev,
					   struct  hisi_hha_data *hha_data)
{
	int ret;

	ret = device_property_read_u32(dev, "hisilicon,module-id",
				       &hha_data->module_id);
	if (ret < 0) {
		dev_err(dev, "Could not read module-id!\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_hha_pmu_init_data(struct hisi_pmu_v2 *hha_pmu,
			      struct hisi_djtag_client *client)
{
	struct hisi_hha_data *hha_data;
	struct device *dev = &client->dev;
	int ret;

	hha_data = devm_kzalloc(dev, sizeof(*hha_data), GFP_KERNEL);
	if (!hha_data)
		return -ENOMEM;

	/* Set the djtag Identifier */
	hha_data->client = client;
	hha_pmu->hwmod_data = hha_data;

	if (dev->of_node) {
		const struct of_device_id *of_id;
		of_id = of_match_device(hha_of_match, dev);
		if (!of_id) {
			dev_err(dev, "DT: Match device fail!\n");
			return -EINVAL;
		}
	} else if (ACPI_COMPANION(dev)) {
		const struct acpi_device_id *acpi_id;

		acpi_id = acpi_match_device(hisi_hha_pmu_acpi_match, dev);
		if (!acpi_id) {
			dev_err(dev, "ACPI: Match device fail!\n");
			return -EINVAL;
		}
	} else
		return -EINVAL;

	/* Get the HHA Module ID */
	ret = hisi_hha_get_module_instance_id(dev, hha_data);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static struct attribute *hisi_hha_format_attr[] = {
	HISI_PMU_FORMAT_ATTR_V2(event, "config:0-7"),
	NULL,
};

static const struct attribute_group hisi_hha_format_group = {
	.name = "format",
	.attrs = hisi_hha_format_attr,
};

static struct attribute *hisi_hha_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR_V2(snoop_l3c, "event=0x6"),
	HISI_PMU_EVENT_ATTR_STR_V2(tx_rd_ddrc, "event=0x08"),
	HISI_PMU_EVENT_ATTR_STR_V2(tx_wr_ddrc, "event=0x09"),
	HISI_PMU_EVENT_ATTR_STR_V2(rx_outer, "event=0x0b"),
	HISI_PMU_EVENT_ATTR_STR_V2(rx_inner, "event=0xc"),
	HISI_PMU_EVENT_ATTR_STR_V2(deal_128_rd, "event=0x11"),
	HISI_PMU_EVENT_ATTR_STR_V2(deal_128_wr, "event=0x12"),
	HISI_PMU_EVENT_ATTR_STR_V2(backinvalid_num, "event=0x15"),
	HISI_PMU_EVENT_ATTR_STR_V2(dir_cache_hit, "event=0x20"),
	HISI_PMU_EVENT_ATTR_STR_V2(dir_cache_relookup, "event=0x21"),
	HISI_PMU_EVENT_ATTR_STR_V2(dir_lookup, "event=0x24"),
	HISI_PMU_EVENT_ATTR_STR_V2(dir_comflict_cancel, "event=0x25"),
	NULL,
};

static const struct attribute_group hisi_hha_events_group = {
	.name = "events",
	.attrs = hisi_hha_events_attr,
};

static struct attribute *hisi_hha_attrs[] = {
	NULL,
};

static const struct attribute_group hisi_hha_attr_group = {
	.attrs = hisi_hha_attrs,
};

static DEVICE_ATTR(cpumask, 0444, hisi_cpumask_sysfs_show_v2, NULL);

static struct attribute *hisi_hha_cpumask_attrs[] = {
	&dev_attr_cpumask.attr,
	NULL,
};

static const struct attribute_group hisi_hha_cpumask_attr_group = {
	.attrs = hisi_hha_cpumask_attrs,
};

static const struct attribute_group *hisi_hha_pmu_attr_groups[] = {
	&hisi_hha_attr_group,
	&hisi_hha_format_group,
	&hisi_hha_events_group,
	&hisi_hha_cpumask_attr_group,
	NULL,
};

static struct hisi_uncore_ops_v2 hisi_uncore_HHA_ops = {
	.set_evtype = hisi_hha_set_evtype,
	.clear_evtype = hisi_hha_clear_evtype,
	.set_event_period = hisi_hha_set_event_period,
	.get_event_idx = hisi_hha_get_event_idx,
	.clear_event_idx = hisi_hha_clear_event_idx,
	.event_update = hisi_uncore_pmu_event_update_v2,
	.start_counters = hisi_hha_start_counters,
	.stop_counters = hisi_hha_stop_counters,
	.write_counter = hisi_hha_write_counter,
};

/* Initialize hrtimer to poll for avoiding counter overflow */
static void hisi_hha_pmu_hrtimer_init(struct hisi_pmu_v2 *hha_pmu)
{
	INIT_LIST_HEAD(&hha_pmu->active_list);
	hha_pmu->ops->start_hrtimer = hisi_hrtimer_start;
	hha_pmu->ops->stop_hrtimer = hisi_hrtimer_stop;
	hisi_hrtimer_init(hha_pmu, HHA_HRTIMER_INTERVAL);
}

static int hisi_hha_pmu_dev_probe(struct hisi_pmu_v2 *hha_pmu,
				  struct hisi_djtag_client *client)
{
	struct device *dev = &client->dev;
	struct hisi_hha_data *hha_data;
	int ret, hha_id;

	ret = hisi_hha_pmu_init_data(hha_pmu, client);
	if (ret)
		return ret;

	hha_data = hha_pmu->hwmod_data;
	hha_pmu->num_events = HISI_HWEVENT_HHA_EVENT_MAX;
	hha_pmu->num_counters = HISI_IDX_HHA_COUNTER_MAX;
	hha_pmu->num_active = 0;
	hha_pmu->scl_id = hisi_djtag_get_sclid(client);
	hha_id = (hha_data->module_id) / 16 - 1;
	hha_pmu->name = kasprintf(GFP_KERNEL, "hisi_hha%u_%u",
				  hha_id, hha_pmu->scl_id);
	hha_pmu->ops = &hisi_uncore_HHA_ops;
	hha_pmu->dev = dev;

	/* Pick one core to use for cpumask attributes */
	cpumask_set_cpu(smp_processor_id(), &hha_pmu->cpus);

	/*
	 * Use poll method to avoid counter overflow as overflow IRQ
	 * is not supported in v1,v2 hardware.
	 */
	hisi_hha_pmu_hrtimer_init(hha_pmu);

	return 0;
}

static int hisi_hha_pmu_probe(struct hisi_djtag_client *client)
{
	struct hisi_pmu_v2 *hha_pmu;
	struct device *dev = &client->dev;
	int ret;

	hha_pmu = hisi_pmu_alloc_v2(dev, HISI_IDX_HHA_COUNTER_MAX);
	if (!hha_pmu)
		return -ENOMEM;

	ret = hisi_hha_pmu_dev_probe(hha_pmu, client);
	if (ret)
		return ret;

	hha_pmu->pmu = (struct pmu) {
		.name = hha_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.event_init = hisi_uncore_pmu_event_init_v2,
		.pmu_enable = hisi_uncore_pmu_enable_v2,
		.pmu_disable = hisi_uncore_pmu_disable_v2,
		.add = hisi_uncore_pmu_add_v2,
		.del = hisi_uncore_pmu_del_v2,
		.start = hisi_uncore_pmu_start_v2,
		.stop = hisi_uncore_pmu_stop_v2,
		.read = hisi_uncore_pmu_read_v2,
		.attr_groups = hisi_hha_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup_v2(hha_pmu, hha_pmu->name);
	if (ret) {
		dev_err(dev, "hisi_uncore_pmu_init FAILED!!\n");
		kfree(hha_pmu->name);
		return ret;
	}

	/* Set the drv data to HHA pmu */
	dev_set_drvdata(dev, hha_pmu);

	return 0;
}

static int hisi_hha_pmu_remove(struct hisi_djtag_client *client)
{
	struct hisi_pmu_v2 *hha_pmu;
	struct device *dev = &client->dev;

	hha_pmu = dev_get_drvdata(dev);
	perf_pmu_unregister(&hha_pmu->pmu);
	kfree(hha_pmu->name);

	return 0;
}

static struct hisi_djtag_driver hisi_hha_pmu_driver = {
	.driver = {
		.name = "hisi-pmu-hha",
		.of_match_table = hha_of_match,
		.acpi_match_table = ACPI_PTR(hisi_hha_pmu_acpi_match),
	},
	.probe = hisi_hha_pmu_probe,
	.remove = hisi_hha_pmu_remove,
};

static int __init hisi_hha_pmu_init(void)
{
	int ret;

	ret = hisi_djtag_register_driver(THIS_MODULE, &hisi_hha_pmu_driver);
	if (ret < 0) {
		pr_err("hisi pmu HHA init failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}
module_init(hisi_hha_pmu_init);

static void __exit hisi_hha_pmu_exit(void)
{
	hisi_djtag_unregister_driver(&hisi_hha_pmu_driver);

}
module_exit(hisi_hha_pmu_exit);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x HHA PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shaokun Zhang");
