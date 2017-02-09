/*
 * HiSilicon SoC L3C Hardware event counters support
 *
 * Copyright (C) 2017 Huawei Technologies Limited
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
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/perf_event.h>
#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon L3C event types.
 */
enum armv8_hisi_l3c_event_types {
	HISI_HWEVENT_L3C_READ_ALLOCATE		= 0x0,
	HISI_HWEVENT_L3C_WRITE_ALLOCATE		= 0x01,
	HISI_HWEVENT_L3C_READ_NOALLOCATE	= 0x02,
	HISI_HWEVENT_L3C_WRITE_NOALLOCATE	= 0x03,
	HISI_HWEVENT_L3C_READ_HIT		= 0x04,
	HISI_HWEVENT_L3C_WRITE_HIT		= 0x05,
	HISI_HWEVENT_L3C_EVENT_MAX		= 0x15,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_l3c_counters {
	HISI_IDX_L3C_COUNTER0		= 0x0,
	HISI_IDX_L3C_COUNTER_MAX	= 0x8,
};

#define HISI_MAX_CFG_L3C_CNTR	0x08
#define L3C_EVTYPE_REG_OFF 0x140
#define L3C_EVCTRL_REG_OFF 0x04
#define L3C_CNT0_REG_OFF 0x170
#define L3C_EVENT_EN 0x1000000

/*
 * Default timer frequency to poll and avoid counter overflow.
 * CPU speed = 2.4Ghz, Therefore Access time = 0.4ns
 * L1 cache - 2 way set associative
 * L2  - 16 way set associative
 * L3  - 16 way set associative. L3 cache has 4 banks.
 *
 * Overflow time = 2^31 * (acces time L1 + access time L2 + access time L3)
 * = 2^31 * ((2 * 0.4ns) + (16 * 0.4ns) + (4 * 16 * 0.4ns)) = 70 seconds
 *
 * L3 cache is also used by devices like PCIe, SAS etc. at
 * the same time. So the overflow time could be even smaller.
 * So on a safe side we use a timer interval of 10sec
 */
#define L3C_HRTIMER_INTERVAL (10LL * MSEC_PER_SEC)

#define GET_MODULE_ID(hwmod_data) hwmod_data->l3c_hwcfg.module_id
#define GET_BANK_SEL(hwmod_data) hwmod_data->l3c_hwcfg.bank_select

struct hisi_l3c_hwcfg {
	u32 module_id;
	u32 bank_select;
	u32 bank_id;
};

struct hisi_l3c_data {
	struct hisi_djtag_client *client;
	DECLARE_BITMAP(event_used_mask, HISI_MAX_CFG_L3C_CNTR);
	struct hisi_l3c_hwcfg l3c_hwcfg;
};

struct hisi_l3c_hw_diff {
	u32 (*get_bank_id)(u32 module_id, u32 bank_select);
};

/* hip05/06 chips L3C bank identifier */
static u32 l3c_bankid_map_v1[MAX_BANKS] = {
	0x02, 0x04, 0x01, 0x08,
};

/* hip07 chip L3C bank identifier */
static u32 l3c_bankid_map_v2[MAX_BANKS] = {
	0x01, 0x02, 0x03, 0x04,
};

/* Return the L3C bank index to use in PMU name */
static u32 get_l3c_bank_v1(u32 module_id, u32 bank_select)
{
	u32 i;

	/*
	 * For v1 chip (hip05/06) the index of bank_select
	 * in the bankid_map gives the bank index.
	 */
	for (i = 0 ; i < MAX_BANKS; i++)
		if (l3c_bankid_map_v1[i] == bank_select)
			break;

	return i;
}

/* Return the L3C bank index to use in PMU name */
static u32 get_l3c_bank_v2(u32 module_id, u32 bank_select)
{
	u32 i;

	/*
	 * For v2 chip (hip07) each bank has different
	 * module ID. So index of module ID in the
	 * bankid_map gives the bank index.
	 */
	for (i = 0 ; i < MAX_BANKS; i++)
		if (l3c_bankid_map_v2[i] == module_id)
			break;

	return i;
}

static inline int hisi_l3c_counter_valid(int idx)
{
	return (idx >= HISI_IDX_L3C_COUNTER0 &&
			idx < HISI_IDX_L3C_COUNTER_MAX);
}

/* Select the counter register offset from the index */
static inline u32 get_counter_reg_off(int cntr_idx)
{
	return (L3C_CNT0_REG_OFF + (cntr_idx * 4));
}

static u32 hisi_l3c_read_counter(struct hisi_l3c_data *l3c_data, int cntr_idx)
{
	struct hisi_djtag_client *client = l3c_data->client;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 reg_off, value;

	reg_off = get_counter_reg_off(cntr_idx);
	hisi_djtag_readreg(module_id, bank_sel, reg_off, client, &value);

	return value;
}

static u64 hisi_l3c_event_update(struct perf_event *event,
				 struct hw_perf_event *hwc, int idx)
{
	struct hisi_pmu *l3c_pmu = to_hisi_pmu(event->pmu);
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;

	if (!hisi_l3c_counter_valid(idx)) {
		dev_err(l3c_pmu->dev, "Unsupported event index:%d!\n", idx);
		return 0;
	}

	do {
		/* Get count from the L3C bank / submodule */
		new_raw_count += hisi_l3c_read_counter(l3c_data, idx);
		prev_raw_count = local64_read(&hwc->prev_count);

		/*
		 *  compute the delta
		 */
		delta = (new_raw_count - prev_raw_count) & HISI_MAX_PERIOD;

		local64_add(delta, &event->count);
	} while (local64_cmpxchg(&hwc->prev_count, prev_raw_count,
				 new_raw_count) != prev_raw_count);

	return new_raw_count;
}

static void hisi_l3c_set_evtype(struct hisi_pmu *l3c_pmu, int idx, u32 val)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_djtag_client *client = l3c_data->client;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 reg_off = L3C_EVTYPE_REG_OFF;
	u32 event_value, value = 0;

	event_value = (val - HISI_HWEVENT_L3C_READ_ALLOCATE);

	/*
	 * Select the appropriate Event select register(L3C_EVENT_TYPEx).
	 * There are 2 Event Select registers for the 8 hardware counters.
	 * For the first 4 hardware counters, the L3C_EVTYPE_REG_OFF is chosen.
	 * For the next 4 hardware counters, the second register is chosen.
	 */
	if (idx > 3)
		reg_off += 4;

	/*
	 * Value to write to event select register
	 * Each byte in the 32 bit select register is used to
	 * configure the event code. Each byte correspond to a
	 * counter register to use.
	 */
	val = event_value << (8 * idx);

	/*
	 * Set the event in L3C_EVENT_TYPEx Register
	 * for all L3C banks
	 */
	hisi_djtag_readreg(module_id, bank_sel, reg_off, client, &value);
	value &= ~(0xff << (8 * idx));
	value |= val;
	hisi_djtag_writereg(module_id, bank_sel, reg_off, value, client);
}

static void hisi_l3c_clear_evtype(struct hisi_pmu *l3c_pmu, int idx)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_djtag_client *client = l3c_data->client;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 reg_off = L3C_EVTYPE_REG_OFF;
	u32 value;

	if (!hisi_l3c_counter_valid(idx)) {
		dev_err(l3c_pmu->dev,
			"Unsupported event index:%d!\n", idx);
		return;
	}

	/*
	 * Clear Counting in L3C event config register.
	 * Select the appropriate Event select register(L3C_EVENT_TYPEx).
	 * There are 2 Event Select registers for the 8 hardware counters.
	 * For the first 4 hardware counters, the L3C_EVTYPE_REG_OFF is chosen.
	 * For the next 4 hardware counters, the second register is chosen.
	 */
	if (idx > 3)
		reg_off += 4;

	/*
	 * Clear the event in L3C_EVENT_TYPEx Register
	 */
	hisi_djtag_readreg(module_id, bank_sel, reg_off, client, &value);
	value &= ~(0xff << (8 * idx));
	value |= (0xff << (8 * idx));
	hisi_djtag_writereg(module_id, bank_sel, reg_off, value, client);
}

static u32 hisi_l3c_write_counter(struct hisi_pmu *l3c_pmu,
				  struct hw_perf_event *hwc, u32 value)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_djtag_client *client = l3c_data->client;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 reg_off;
	int idx = GET_CNTR_IDX(hwc);
	int ret;

	if (!hisi_l3c_counter_valid(idx)) {
		dev_err(l3c_pmu->dev,
			"Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_off = get_counter_reg_off(idx);
	ret = hisi_djtag_writereg(module_id, bank_sel, reg_off, value, client);
	if (!ret)
		ret = value;

	return ret;
}

static void hisi_l3c_start_counters(struct hisi_pmu *l3c_pmu)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_djtag_client *client = l3c_data->client;
	unsigned long *used_mask = l3c_data->event_used_mask;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 num_counters = l3c_pmu->num_counters;
	u32 value;
	int enabled = bitmap_weight(used_mask, num_counters);

	if (!enabled)
		return;

	/*
	 * Set the event_bus_en bit in L3C AUCNTRL to start counting
	 * for the L3C bank
	 */
	hisi_djtag_readreg(module_id, bank_sel, L3C_EVCTRL_REG_OFF,
			   client, &value);
	value |= L3C_EVENT_EN;
	hisi_djtag_writereg(module_id, bank_sel, L3C_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_l3c_stop_counters(struct hisi_pmu *l3c_pmu)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_djtag_client *client = l3c_data->client;
	u32 module_id = GET_MODULE_ID(l3c_data);
	u32 bank_sel = GET_BANK_SEL(l3c_data);
	u32 value;

	/*
	 * Clear the event_bus_en bit in L3C AUCNTRL
	 */
	hisi_djtag_readreg(module_id, bank_sel, L3C_EVCTRL_REG_OFF,
			   client, &value);
	value &= ~(L3C_EVENT_EN);
	hisi_djtag_writereg(module_id, bank_sel, L3C_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_l3c_clear_event_idx(struct hisi_pmu *l3c_pmu, int idx)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	void *bitmap_addr;

	if (!hisi_l3c_counter_valid(idx)) {
		dev_err(l3c_pmu->dev, "Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = l3c_data->event_used_mask;
	clear_bit(idx, bitmap_addr);
}

static int hisi_l3c_get_event_idx(struct hisi_pmu *l3c_pmu)
{
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	unsigned long *used_mask = l3c_data->event_used_mask;
	u32 num_counters = l3c_pmu->num_counters;
	int event_idx;

	event_idx = find_first_zero_bit(used_mask, num_counters);
	if (event_idx == num_counters)
		return -EAGAIN;

	set_bit(event_idx, used_mask);

	return event_idx;
}

/* Handle differences in L3C hw in v1/v2 chips */
static const struct hisi_l3c_hw_diff l3c_hw_v1 = {
	.get_bank_id = get_l3c_bank_v1,
};

/* Handle differences in L3C hw in v1/v2 chips */
static const struct hisi_l3c_hw_diff l3c_hw_v2 = {
	.get_bank_id = get_l3c_bank_v2,
};

static const struct of_device_id l3c_of_match[] = {
	{ .compatible = "hisilicon,hip05-pmu-l3c-v1", &l3c_hw_v1},
	{ .compatible = "hisilicon,hip06-pmu-l3c-v1", &l3c_hw_v1},
	{ .compatible = "hisilicon,hip07-pmu-l3c-v2", &l3c_hw_v2},
	{},
};
MODULE_DEVICE_TABLE(of, l3c_of_match);

static int hisi_l3c_get_module_id_prop(struct device *dev,
				       struct  hisi_l3c_hwcfg *l3c_hwcfg)
{
	u32 module_id[2];
	int ret;

	ret = device_property_read_u32_array(dev, "hisilicon,module-id",
					     module_id, 2);
	if (ret < 0) {
		dev_err(dev, "Could not read module-id!\n");
		return -EINVAL;
	}
	l3c_hwcfg->module_id = module_id[0];
	l3c_hwcfg->bank_select = module_id[1];

	return 0;
}

static int hisi_l3c_init_data(struct hisi_pmu *l3c_pmu,
			      struct hisi_djtag_client *client)
{
	struct hisi_l3c_data *l3c_data;
	struct hisi_l3c_hwcfg *l3c_hwcfg;
	const struct hisi_l3c_hw_diff *l3c_hw;
	struct device *dev = &client->dev;
	int ret;

	l3c_data = devm_kzalloc(dev, sizeof(*l3c_data), GFP_KERNEL);
	if (!l3c_data)
		return -ENOMEM;

	/* Set the djtag Identifier */
	l3c_data->client = client;

	l3c_pmu->hw_perf_events = devm_kcalloc(dev, l3c_pmu->num_counters,
					       sizeof(*l3c_pmu->hw_perf_events),
					       GFP_KERNEL);
	if (!l3c_pmu->hw_perf_events)
		return -ENOMEM;

	raw_spin_lock_init(&l3c_pmu->lock);
	l3c_pmu->hwmod_data = l3c_data;

	if (dev->of_node) {
		const struct of_device_id *of_id;

		of_id = of_match_device(l3c_of_match, dev);
		if (!of_id) {
			dev_err(dev, "DT: Match device fail!\n");
			return -EINVAL;
		}
		l3c_hw = of_id->data;
	} else
		return -EINVAL;

	l3c_hwcfg = &l3c_data->l3c_hwcfg;

	/* Get the L3C Module ID to identify the bank index */
	ret = hisi_l3c_get_module_id_prop(dev, l3c_hwcfg);
	if (ret < 0)
		return -EINVAL;

	/* Get the L3C bank index to set the pmu name */
	l3c_hwcfg->bank_id = l3c_hw->get_bank_id(l3c_hwcfg->module_id,
						 l3c_hwcfg->bank_select);
	if (l3c_hwcfg->bank_id == MAX_BANKS) {
		dev_err(dev, "Invalid bank-select!\n");
		return -EINVAL;
	}

	return 0;
}

static struct attribute *hisi_l3c_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	NULL,
};

static const struct attribute_group hisi_l3c_format_group = {
	.name = "format",
	.attrs = hisi_l3c_format_attr,
};

static struct attribute *hisi_l3c_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(read_allocate, "event=0x0"),
	HISI_PMU_EVENT_ATTR_STR(write_allocate, "event=0x01"),
	HISI_PMU_EVENT_ATTR_STR(read_noallocate, "event=0x02"),
	HISI_PMU_EVENT_ATTR_STR(write_noallocate, "event=0x03"),
	HISI_PMU_EVENT_ATTR_STR(read_hit, "event=0x04"),
	HISI_PMU_EVENT_ATTR_STR(write_hit, "event=0x05"),
	NULL,
};

static const struct attribute_group hisi_l3c_events_group = {
	.name = "events",
	.attrs = hisi_l3c_events_attr,
};

static struct attribute *hisi_l3c_attrs[] = {
	NULL,
};

static const struct attribute_group hisi_l3c_attr_group = {
	.attrs = hisi_l3c_attrs,
};

static DEVICE_ATTR(cpumask, 0444, hisi_cpumask_sysfs_show, NULL);

static DEVICE_ATTR(hrtimer_interval, 0444, hisi_hrtimer_interval_sysfs_show,
		   NULL);

static struct attribute *hisi_l3c_hrtimer_interval_attrs[] = {
	&dev_attr_hrtimer_interval.attr,
	NULL,
};

static const struct attribute_group hisi_l3c_hrtimer_interval_attr_group = {
	.attrs = hisi_l3c_hrtimer_interval_attrs,
};

static struct attribute *hisi_l3c_cpumask_attrs[] = {
	&dev_attr_cpumask.attr,
	NULL,
};

static const struct attribute_group hisi_l3c_cpumask_attr_group = {
	.attrs = hisi_l3c_cpumask_attrs,
};

static const struct attribute_group *hisi_l3c_pmu_attr_groups[] = {
	&hisi_l3c_attr_group,
	&hisi_l3c_format_group,
	&hisi_l3c_events_group,
	&hisi_l3c_cpumask_attr_group,
	&hisi_l3c_hrtimer_interval_attr_group,
	NULL,
};

static struct hisi_uncore_ops hisi_uncore_l3c_ops = {
	.set_evtype = hisi_l3c_set_evtype,
	.clear_evtype = hisi_l3c_clear_evtype,
	.set_event_period = hisi_pmu_set_event_period,
	.get_event_idx = hisi_l3c_get_event_idx,
	.clear_event_idx = hisi_l3c_clear_event_idx,
	.event_update = hisi_l3c_event_update,
	.start_counters = hisi_l3c_start_counters,
	.stop_counters = hisi_l3c_stop_counters,
	.write_counter = hisi_l3c_write_counter,
};

/* Initialize hrtimer to poll for avoiding counter overflow */
static void hisi_l3c_hrtimer_init(struct hisi_pmu *l3c_pmu)
{
	INIT_LIST_HEAD(&l3c_pmu->active_list);
	l3c_pmu->ops->start_hrtimer = hisi_hrtimer_start;
	l3c_pmu->ops->stop_hrtimer = hisi_hrtimer_stop;
	hisi_hrtimer_init(l3c_pmu, L3C_HRTIMER_INTERVAL);
}

static int hisi_l3c_pmu_init(struct hisi_pmu *l3c_pmu,
			     struct hisi_djtag_client *client)
{
	struct device *dev = &client->dev;
	struct hisi_l3c_data *l3c_data = l3c_pmu->hwmod_data;
	struct hisi_l3c_hwcfg *l3c_hwcfg = &l3c_data->l3c_hwcfg;

	l3c_pmu->num_events = HISI_HWEVENT_L3C_EVENT_MAX;
	l3c_pmu->num_counters = HISI_IDX_L3C_COUNTER_MAX;
	l3c_pmu->num_active = 0;
	l3c_pmu->scl_id = hisi_djtag_get_sclid(client);

	l3c_pmu->name = kasprintf(GFP_KERNEL, "hisi_l3c%u_%u",
				  l3c_hwcfg->bank_id, l3c_pmu->scl_id);
	l3c_pmu->ops = &hisi_uncore_l3c_ops;
	l3c_pmu->dev = dev;

	/* Pick one core to use for cpumask attributes */
	cpumask_set_cpu(smp_processor_id(), &l3c_pmu->cpu);

	/* Use hrtimer to poll for avoiding counter overflow */
	hisi_l3c_hrtimer_init(l3c_pmu);

	return 0;
}

static int hisi_pmu_l3c_dev_probe(struct hisi_djtag_client *client)
{
	struct hisi_pmu *l3c_pmu;
	struct device *dev = &client->dev;
	int ret;

	l3c_pmu = hisi_pmu_alloc(dev);
	if (!l3c_pmu)
		return -ENOMEM;

	ret = hisi_l3c_init_data(l3c_pmu, client);
	if (ret)
		return ret;

	ret = hisi_l3c_pmu_init(l3c_pmu, client);
	if (ret)
		return ret;

	l3c_pmu->pmu = (struct pmu) {
		.name = l3c_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.event_init = hisi_uncore_pmu_event_init,
		.pmu_enable = hisi_uncore_pmu_enable,
		.pmu_disable = hisi_uncore_pmu_disable,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_l3c_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(l3c_pmu, l3c_pmu->name);
	if (ret) {
		dev_err(dev, "hisi_uncore_pmu_init FAILED!!\n");
		return ret;
	}

	/* Set the drv data to L3C pmu */
	dev_set_drvdata(dev, l3c_pmu);

	return 0;
}

static int hisi_pmu_l3c_dev_remove(struct hisi_djtag_client *client)
{
	struct hisi_pmu *l3c_pmu;
	struct device *dev = &client->dev;

	l3c_pmu = dev_get_drvdata(dev);
	perf_pmu_unregister(&l3c_pmu->pmu);

	return 0;
}

static struct hisi_djtag_driver hisi_pmu_l3c_driver = {
	.driver = {
		.name = "hisi-pmu-l3c",
		.of_match_table = l3c_of_match,
	},
	.probe = hisi_pmu_l3c_dev_probe,
	.remove = hisi_pmu_l3c_dev_remove,
};

static int __init hisi_pmu_l3c_init(void)
{
	int rc;

	rc = hisi_djtag_register_driver(THIS_MODULE, &hisi_pmu_l3c_driver);
	if (rc < 0) {
		pr_err("hisi pmu L3C init failed, rc=%d\n", rc);
		return rc;
	}

	return 0;
}
module_init(hisi_pmu_l3c_init);

static void __exit hisi_pmu_l3c_exit(void)
{
	hisi_djtag_unregister_driver(&hisi_pmu_l3c_driver);

}
module_exit(hisi_pmu_l3c_exit);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x L3C PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anurup M");
