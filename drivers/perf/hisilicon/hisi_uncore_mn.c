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
	struct device_node *djtag_node = mn_hwmod_data->djtag_node;
	u32 reg_offset, value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;
	reg_offset = mn_hwmod_data->mn_hwcfg.counter_reg0_off + (idx * 4);

	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				djtag_node, &value);

	return value;
}

u64 hisi_mn_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct device_node *djtag_node;
	struct hisi_pmu *pmn_pmu = to_hisi_pmu(event->pmu);
	struct hisi_mn_data *mn_hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;
	u32 cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return 0;
	}

	mn_hwmod_data = pmn_pmu->hwmod_data;

	/* Check if the MN data is initialized for this SCCL */
	if (!mn_hwmod_data->djtag_node) {
		pr_err("SCCL=%d not initialized!\n", pmn_pmu->scl_id);
		return 0;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = mn_hwmod_data->djtag_node;
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
	struct device_node *djtag_node;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.evtype_reg0_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_value = 0, value = 0;
	u32 cfg_en;

	event_value = (val -
			HISI_HWEVENT_MN_EO_BARR_REQ);

	/* Value to write to event type register */
	val = event_value << (8 * idx);

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Set the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id,
			   cfg_en,
			   reg_offset,
			   djtag_node, &value);

	value &= ~(0xff << (8 * idx));
	value |= val;

	hisi_djtag_writereg(module_id,
			    cfg_en,
			    reg_offset,
			    value,
			    djtag_node);
}

u32 hisi_write_mn_counter(struct hisi_pmu *pmn_pmu, int idx, u32 value)
{
	struct device_node *djtag_node;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 reg_offset, cfg_en;
	int ret = 0;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = mn_hwmod_data->mn_hwcfg.counter_reg0_off +
							(idx * 4);
	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	ret = hisi_djtag_writereg(module_id,
				  cfg_en,
				  reg_offset,
				  value,
				  djtag_node);
	if (!ret)
		ret = value;

	return ret;
}

void hisi_enable_mn_counter(struct hisi_pmu *pmn_pmu, int idx)
{
	struct device_node *djtag_node;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.event_ctrl_reg_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_en = mn_hwmod_data->mn_hwcfg.event_enable;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Set the event_bus_en bit in MN_EVENT_CTRL to enable counting
	 */
	hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				djtag_node,
				&value);

	value |= event_en;
	hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
}

void hisi_disable_mn_counter(struct hisi_pmu *pmn_pmu, int idx)
{
	struct device_node *djtag_node;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.event_ctrl_reg_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 event_en = mn_hwmod_data->mn_hwcfg.event_enable;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Clear the event_bus_en bit in MN event control
	 */
	hisi_djtag_readreg(module_id,
			   cfg_en,
			   reg_offset,
			   djtag_node, &value);

	value &= ~(event_en);
	hisi_djtag_writereg(module_id,
			    cfg_en,
			    reg_offset,
			    value,
			    djtag_node);
}

void hisi_clear_mn_event_idx(struct hisi_pmu *pmn_pmu, int idx)
{
	struct device_node *djtag_node;
	void *bitmap_addr;
	struct hisi_mn_data *mn_hwmod_data = pmn_pmu->hwmod_data;
	u32 reg_offset = mn_hwmod_data->mn_hwcfg.evtype_reg0_off;
	u32 module_id = mn_hwmod_data->mn_hwcfg.module_id;
	u32 value, cfg_en;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = mn_hwmod_data->hisi_mn_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	cfg_en = mn_hwmod_data->mn_hwcfg.bank_cfgen;

	/*
	 * Clear the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id,
			   cfg_en,
			   reg_offset,
			   djtag_node, &value);

	value &= ~(0xff << (8 * idx));
	value |= (0xff << (8 * idx));
	hisi_djtag_writereg(module_id,
			    cfg_en,
			    reg_offset,
			    value,
			    djtag_node);
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
	if (pmn_pmu->hwmod_data) {
		kfree(pmn_pmu->hwmod_data);
		pmn_pmu->hwmod_data = NULL;
	}
}

static int init_hisi_mn_hwcfg(struct device_node *node,
				struct hisi_mn_data *pmn_data)
{
	struct hisi_mn_hwcfg *pmn_hwcfg = &pmn_data->mn_hwcfg;
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
	pr_err("init_hisi_mn_hwcfg: fail to read DT properties!\n");
	return -EINVAL;
}

static int init_hisi_mn_data(struct platform_device *pdev,
					struct hisi_pmu *pmn_pmu)
{
	struct device *dev = &pdev->dev;
	struct device_node *parent_node = dev->of_node;
	struct device_node *djtag_node;
	struct hisi_mn_data *mn_hwmod_data;
	int ret;

	mn_hwmod_data = kzalloc(sizeof(struct hisi_mn_data),
						 GFP_KERNEL);
	if (!mn_hwmod_data)
		return -ENOMEM;

	djtag_node = of_parse_phandle(parent_node, "djtag", 0);
	if (IS_ERR(djtag_node)) {
		pr_info("mn_device_probe-node without djtag!\n");
		goto fail;
	}

	/* Set the djtag node */
	mn_hwmod_data->djtag_node = djtag_node;

	pmn_pmu->hw_events.events = devm_kcalloc(&pdev->dev,
					pmn_pmu->num_counters,
					sizeof(*pmn_pmu->hw_events.events),
					GFP_KERNEL);
	if (!pmn_pmu->hw_events.events) {
		ret = -ENOMEM;
		goto free_mem;
	}

	raw_spin_lock_init(&pmn_pmu->hw_events.pmu_lock);

	pmn_pmu->hwmod_data = mn_hwmod_data;

	ret = init_hisi_mn_hwcfg(parent_node, mn_hwmod_data);
	if (ret)
		goto free_mem;

	return 0;

fail:
	pr_err("init_hisi_mn_data: fail to read DT properties!\n");
	ret = -EINVAL;

free_mem:
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
				"event=0x316"),
	HISI_PMU_EVENT_ATTR_STR(ec_barrier_req,
				"event=0x317"),
	HISI_PMU_EVENT_ATTR_STR(dvm_op_req,
				"event=0x318"),
	HISI_PMU_EVENT_ATTR_STR(dvm_sync_req,
				"event=0x319"),
	HISI_PMU_EVENT_ATTR_STR(read_req,
				"event=0x31A"),
	HISI_PMU_EVENT_ATTR_STR(write_req,
				"event=0x31B"),
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

static const struct attribute_group *hisi_mn_pmu_attr_groups[] = {
	&hisi_mn_attr_group,
	&hisi_mn_format_group,
	&hisi_mn_events_group,
	NULL
};

static int hisi_mn_pmu_init(struct platform_device *pdev,
					struct hisi_pmu *pmn_pmu)
{
	struct device *dev = &pdev->dev;

	if (device_property_read_u32(dev, "num-events",
				     &pmn_pmu->num_events)) {
		pr_err("mn_device_probe: Cant read num-events from DT!\n");
		return -EINVAL;
	}

	if (device_property_read_u32(dev, "num-counters",
				     &pmn_pmu->num_counters)) {
		pr_err("mn_device_probe: Cant read num-counters from DT!\n");
		return -EINVAL;
	}

	/* Find the SCL ID */
	if (device_property_read_u32(dev, "scl-id",
					&pmn_pmu->scl_id)) {
		pr_err("mn_device_probe: cant read scl-id!\n");
		return -EINVAL;
	}

	if (pmn_pmu->scl_id == 0 ||
		pmn_pmu->scl_id >= MAX_UNITS) {
		pr_err("mn_device_probe: Invalid SCL=%d!\n",
						pmn_pmu->scl_id);
		return -EINVAL;
	}

	pmn_pmu->pmu_type = SCCL_SPECIFIC;
	pmn_pmu->name = kasprintf(GFP_KERNEL, "hisi_mn%d",
						pmn_pmu->scl_id);
	pmn_pmu->hwmod_type = HISI_MN;
	pmn_pmu->plat_device = pdev;
	pmn_pmu->set_evtype = hisi_set_mn_evtype;
	pmn_pmu->get_event_idx = hisi_mn_get_event_idx;
	pmn_pmu->clear_event_idx = hisi_clear_mn_event_idx;
	pmn_pmu->event_update = hisi_mn_event_update;
	pmn_pmu->enable_counter = hisi_enable_mn_counter;
	pmn_pmu->disable_counter = hisi_disable_mn_counter;
	pmn_pmu->write_counter = hisi_write_mn_counter;

	return 0;
}

static int hisi_pmu_mn_dev_probe(struct platform_device *pdev)
{
	struct hisi_pmu *pmn_pmu = NULL;
	int ret;

	pmn_pmu = hisi_pmu_alloc(pdev);
	if (IS_ERR(pmn_pmu))
		return PTR_ERR(pmn_pmu);

	ret = hisi_mn_pmu_init(pdev, pmn_pmu);

	ret = init_hisi_mn_data(pdev, pmn_pmu);
	if (ret)
		goto fail_init;

	/* Register with perf PMU */
	pmn_pmu->pmu = (struct pmu) {
		.name = pmn_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.pmu_enable = hisi_uncore_pmu_enable,
		.pmu_disable = hisi_uncore_pmu_disable,
		.event_init = hisi_uncore_pmu_event_init,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_mn_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(pmn_pmu, pdev, pmn_pmu->name);
	if (ret) {
		pr_err("hisi_uncore_pmu_init FAILED!!\n");
		goto fail;
	}

	return 0;

fail:
	hisi_free_mn_data(pmn_pmu);

fail_init:
	if (pmn_pmu)
		devm_kfree(&pdev->dev, pmn_pmu);

	return ret;
}

static int hisi_pmu_mn_dev_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mn_of_match[] = {
	{ .compatible = "hisilicon,hisi-pmu-mn", },
	{},
};
MODULE_DEVICE_TABLE(of, mn_of_match);

static struct platform_driver hisi_pmu_mn_driver = {
	.driver = {
		.name = "hisi-pmu-mn",
		.of_match_table = mn_of_match,
	},
	.probe = hisi_pmu_mn_dev_probe,
	.remove = hisi_pmu_mn_dev_remove,
};
module_platform_driver(hisi_pmu_mn_driver);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x MN PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shaokun Zhang");
