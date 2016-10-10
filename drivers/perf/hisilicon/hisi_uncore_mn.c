/*
 * HiSilicon SoC MN Hardware event counters support
 *
 * Copyright (C) 2016 Huawei Technologies Limited
 * Author: Shaokun Zhang <zhangshaokun@hisilicon.com>
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/perf_event.h>
#include "hisi_uncore_mn.h"

static inline int hisi_mn_counter_valid(int idx)
{
	return (idx >= HISI_IDX_MN_COUNTER0 &&
			idx <= HISI_IDX_MN_COUNTER_MAX);
}

static u32 hisi_read_mn_counter(struct hisi_mn_data *mn_hwmod_data,
								int idx)
{
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	struct hisi_djtag_client *client = mn_hwmod_data->client;
	u32 reg_offset, value, cfg_en;

	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;
	reg_offset = mn_hwmod_data->mn_hwcfg.counter_reg0_off + (idx * 4);

	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				client, &value);

	return value;
}

u64 hisi_mn_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct hisi_pmu *pmn_pmu = to_hisi_pmu(event->pmu);
	struct hisi_mn_data *mn_hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;
	u32 cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(pmn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return 0;
	}

	mn_hwmod_data = pmn_pmu->hwmod_data;

	/* Check if the MN data is initialized for this SCCL */
	if (!mn_hwmod_data->client) {
		dev_err(pmn_pmu->dev,
			"SCL=%d not initialized!\n", pmn_pmu->scl_id);
		return 0;
	}

	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	do {
		prev_raw_count = local64_read(&hwc->prev_count);
		new_raw_count =
			hisi_read_mn_counter(mn_hwmod_data, idx);
		delta = (new_raw_count - prev_raw_count) &
						HISI_MAX_PERIOD;

		local64_add(delta, &event->count);
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
							prev_raw_count);

	return new_raw_count;
}

void hisi_set_mn_evtype(struct hisi_pmu *pmn_pmu, int idx, u32 val)
{
	struct hisi_djtag_client *client;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.evtype_reg0_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_value, value = 0;
	u32 cfg_en;

	event_value = (val -
			HISI_HWEVENT_MN_EO_BARR_REQ);

	/* Value to write to event type register */
	val = event_value << (8 * idx);

	/* Find the djtag Identifier of the Unit */
	client = mn_hwmod_data->client;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Set the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id,
			   cfg_en,
			   reg_offset,
			   client, &value);

	value &= ~(0xff << (8 * idx));
	value |= val;

	hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				client);
}

u32 hisi_write_mn_counter(struct hisi_pmu *pmn_pmu,
				struct hw_perf_event *hwc, u32 value)
{
	struct hisi_djtag_client *client;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 reg_offset, cfg_en;
	int ret;
	int idx = GET_CNTR_IDX(hwc);

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(pmn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = mn_hwmod_data->mn_hwcfg.counter_reg0_off +
							(idx * 4);
	/* Find the djtag Identifier of the Unit */
	client = mn_hwmod_data->client;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	ret = hisi_djtag_writereg(module_id,
				  cfg_en,
				  reg_offset,
				  value,
				  client);

	return ret;
}

void hisi_enable_mn_counter(struct hisi_pmu *pmn_pmu, int idx)
{
	struct hisi_djtag_client *client;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.event_ctrl_reg_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_en = mn_hwmod_data->mn_hwcfg.event_enable;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(pmn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag Identifier of the Unit */
	client = mn_hwmod_data->client;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Set the event_bus_en bit in MN_EVENT_CTRL to enable counting
	 */
	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				client,
				&value);

	value |= event_en;
	hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				client);
}

void hisi_disable_mn_counter(struct hisi_pmu *pmn_pmu, int idx)
{
	struct hisi_djtag_client *client;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.event_ctrl_reg_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_en = mn_hwmod_data->mn_hwcfg.event_enable;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(pmn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag Identifier of the Unit */
	client = mn_hwmod_data->client;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Clear the event_bus_en bit in MN event control
	 */
	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				client, &value);

	value &= ~(event_en);
	hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				client);
}

void hisi_clear_mn_event_idx(struct hisi_pmu *pmn_pmu, int idx)
{
	struct hisi_djtag_client *client;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.evtype_reg0_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	void *bitmap_addr;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(pmn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = mn_hwmod_data->hisi_mn_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Find the djtag Identifier of the Unit */
	client = mn_hwmod_data->client;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Clear the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				client, &value);

	value &= ~(0xff << (8 * idx));
	value |= (0xff << (8 * idx));
	hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				client);
}

int hisi_mn_get_event_idx(struct hisi_pmu *pmn_pmu)
{
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	int event_idx;

	event_idx =
		find_first_zero_bit(
			mn_hwmod_data->hisi_mn_event_used_mask,
					HISI_MAX_CFG_MN_CNTR);

	if (event_idx == HISI_MAX_CFG_MN_CNTR)
		return -EAGAIN;

	__set_bit(event_idx,
		mn_hwmod_data->hisi_mn_event_used_mask);

	return event_idx;
}

static void hisi_free_mn_data(struct hisi_pmu *pmn_pmu)
{
	kfree(pmn_pmu->hwmod_data);
	pmn_pmu->hwmod_data = NULL;
}

static int init_hisi_mn_hwcfg(struct device *dev,
				struct hisi_mn_data *pmn_data)
{
	struct hisi_mn_hwcfg *pmn_hwcfg = &pmn_data->mn_hwcfg;
	struct device_node *node = dev->of_node;
	const u32 *cfgen_map = NULL;
	u32 cfgen_map_len;

	if (of_property_read_u32(node, "counter-reg",
				     &pmn_hwcfg->counter_reg0_off))
		goto fail;

	if (of_property_read_u32(node, "evctrl-reg",
				     &pmn_hwcfg->event_ctrl_reg_off))
		goto fail;

	if (of_property_read_u32(node, "event-en",
				     &pmn_hwcfg->event_enable))
		goto fail;

	if (of_property_read_u32(node, "evtype-reg",
				     &pmn_hwcfg->evtype_reg0_off))
		goto fail;

	if (of_property_read_u32(node, "module-id",
				     &pmn_hwcfg->module_id))
		goto fail;

	cfgen_map = of_get_property(node, "cfgen-map", &cfgen_map_len);
	of_property_read_u32(node, "cfgen-map",
					&pmn_hwcfg->bank_cfgen);

	return 0;

fail:
	dev_err(dev, "Fail to read DT properties!\n");
	return -EINVAL;
}

static int init_hisi_mn_data(struct device *dev,
					struct hisi_pmu *pmn_pmu,
					struct hisi_djtag_client *client)
{
	struct hisi_mn_data *mn_hwmod_data;
	int ret;

	mn_hwmod_data = kzalloc(sizeof(struct hisi_mn_data),
							GFP_KERNEL);
	if (!mn_hwmod_data)
		return -ENOMEM;

	/* Set the djtag Identifier */
	mn_hwmod_data->client = client;

	pmn_pmu->hw_events.events = devm_kcalloc(dev,
					pmn_pmu->num_counters,
					sizeof(*pmn_pmu->hw_events.events),
					GFP_KERNEL);
	if (!pmn_pmu->hw_events.events) {
		ret = -ENOMEM;
		goto fail;
	}

	raw_spin_lock_init(&pmn_pmu->hw_events.pmu_lock);

	pmn_pmu->hwmod_data = mn_hwmod_data;

	ret = init_hisi_mn_hwcfg(dev, mn_hwmod_data);
	if (ret)
		goto fail;

	return 0;

fail:
	hisi_free_mn_data(pmn_pmu);
	return ret;
}

static struct attribute *hisi_mn_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	NULL,
};

static struct attribute_group hisi_mn_format_group = {
	.name = "format",
	.attrs = hisi_mn_format_attr,
};

static struct attribute *hisi_mn_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(eo_barrier_req,
				"event=0x0"),
	HISI_PMU_EVENT_ATTR_STR(ec_barrier_req,
				"event=0x01"),
	HISI_PMU_EVENT_ATTR_STR(dvm_op_req,
				"event=0x02"),
	HISI_PMU_EVENT_ATTR_STR(dvm_sync_req,
				"event=0x03"),
	HISI_PMU_EVENT_ATTR_STR(read_req,
				"event=0x04"),
	HISI_PMU_EVENT_ATTR_STR(write_req,
				"event=0x05"),
	NULL,
};

static struct attribute_group hisi_mn_events_group = {
	.name = "events",
	.attrs = hisi_mn_events_attr,
};

static struct attribute *hisi_mn_attrs[] = {
	NULL,
};

struct attribute_group hisi_mn_attr_group = {
	.attrs = hisi_mn_attrs,
};

static DEVICE_ATTR(cpumask, S_IRUGO, hisi_cpumask_sysfs_show, NULL);

static struct attribute *hisi_mn_cpumask_attrs[] = {
	&dev_attr_cpumask.attr,
	NULL,
};

static const struct attribute_group hisi_mn_cpumask_attr_group = {
	.attrs = hisi_mn_cpumask_attrs,
};

static const struct attribute_group *hisi_mn_pmu_attr_groups[] = {
	&hisi_mn_attr_group,
	&hisi_mn_format_group,
	&hisi_mn_events_group,
	&hisi_mn_cpumask_attr_group,
	NULL,
};

static struct hisi_uncore_ops hisi_uncore_mn_ops = {
	.set_evtype = hisi_set_mn_evtype,
	.set_event_period = hisi_pmu_set_event_period,
	.get_event_idx = hisi_mn_get_event_idx,
	.clear_event_idx = hisi_clear_mn_event_idx,
	.event_update = hisi_mn_event_update,
	.enable_counter = hisi_enable_mn_counter,
	.disable_counter = hisi_disable_mn_counter,
	.write_counter = hisi_write_mn_counter,
};

static int hisi_mn_pmu_init(struct device *dev,
					struct hisi_pmu *pmn_pmu)
{
	int ret;
	/* Read common PMU properties */
	ret = hisi_uncore_common_fwprop_read(dev, pmn_pmu);
	if (ret)
		return ret;

	pmn_pmu->name = kasprintf(GFP_KERNEL, "hisi_mn%d",
						pmn_pmu->scl_id);
	pmn_pmu->ops = &hisi_uncore_mn_ops;

	pmn_pmu->dev = dev;
	/* Pick one core to use for cpumask attributes */
	cpumask_set_cpu(smp_processor_id(), &pmn_pmu->cpu);

	return 0;
}

static int hisi_pmu_mn_dev_probe(struct hisi_djtag_client *client)
{
	struct hisi_pmu *pmn_pmu = NULL;
	struct device *dev = &client->dev;
	int ret;

	pmn_pmu = hisi_pmu_alloc(dev);
	if (IS_ERR(pmn_pmu))
		return PTR_ERR(pmn_pmu);

	ret = hisi_mn_pmu_init(dev, pmn_pmu);
	if (ret)
		return ret;

	ret = init_hisi_mn_data(dev, pmn_pmu, client);
	if (ret)
		goto fail_init;

	/* Register with perf PMU */
	pmn_pmu->pmu = (struct pmu) {
		.name = pmn_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.event_init = hisi_uncore_pmu_event_init,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_mn_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(pmn_pmu, pmn_pmu->name);
	if (ret) {
		dev_err(pmn_pmu->dev, "hisi_uncore_pmu_init FAILED!!\n");
		goto fail;
	}

	return 0;

fail:
	hisi_free_mn_data(pmn_pmu);

fail_init:
	if (pmn_pmu)
		devm_kfree(dev, pmn_pmu);
	dev_err(pmn_pmu->dev, "%s failed\n", __func__);

	return ret;
}

static const struct of_device_id mn_of_match[] = {
	{ .compatible = "hisilicon,hisi-pmu-mn", },
	{},
};
MODULE_DEVICE_TABLE(of, mn_of_match);

static struct hisi_djtag_driver hisi_pmu_mn_driver = {
	.driver = {
		.name = "hisi-pmu-mn",
		.of_match_table = mn_of_match,
	},
	.probe = hisi_pmu_mn_dev_probe,
};

static int __init hisi_pmu_mn_init(void)
{
	int rc;

	rc = hisi_djtag_register_driver(THIS_MODULE, &hisi_pmu_mn_driver);
	if (rc < 0) {
		pr_err("hisi pmu mn init failed, rc=%d\n", rc);
		return rc;
	}

	return 0;
}
module_init(hisi_pmu_mn_init);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x MN PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shaokun Zhang");
