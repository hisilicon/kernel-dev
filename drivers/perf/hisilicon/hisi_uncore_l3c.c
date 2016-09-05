/*
 * HiSilicon SoC L3C Hardware event counters support
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/perf_event.h>
#include "hisi_uncore_l3c.h"

static inline int hisi_l3c_counter_valid(int idx)
{
	return (idx >= HISI_IDX_L3C_COUNTER0 &&
			idx <= HISI_IDX_L3C_COUNTER_MAX);
}

static u32 hisi_read_l3c_counter(struct hisi_l3c_data *l3c_hwmod_data,
								int idx,
								int bank)
{
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	struct device_node *djtag_node = l3c_hwmod_data->djtag_node;
	u32 reg_offset, value;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = l3c_hwmod_data->l3c_hwcfg.counter_reg0_off + (idx * 4);

	hisi_djtag_readreg(module_id,
			bank,
			reg_offset,
			djtag_node, &value);

	return value;
}

u64 hisi_l3c_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct device_node *djtag_node;
	struct hisi_pmu *pl3c_pmu = to_hisi_pmu(event->pmu);
	struct hisi_l3c_data *l3c_hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;
	u32 cfg_en;
	int i;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return 0;
	}

	l3c_hwmod_data = pl3c_pmu->hwmod_data;

	/* Check if the L3C data is initialized for this SCCL */
	if (!l3c_hwmod_data->djtag_node) {
		pr_err("SCL=%d not initialized!\n", pl3c_pmu->scl_id);
		return 0;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = l3c_hwmod_data->djtag_node;

	do {
		prev_raw_count = local64_read(&hwc->prev_count);
		for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
			cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];

			new_raw_count =
					hisi_read_l3c_counter(l3c_hwmod_data,
								      idx,
								      cfg_en);
			delta = (new_raw_count - prev_raw_count) &
							HISI_MAX_PERIOD;

			local64_add(delta, &event->count);
		}
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
								prev_raw_count);

	return new_raw_count;
}

void hisi_set_l3c_evtype(struct hisi_pmu *pl3c_pmu, int idx, u32 val)
{
	struct device_node *djtag_node;
	struct hisi_l3c_data *l3c_hwmod_data = pl3c_pmu->hwmod_data;
	u32 reg_offset = l3c_hwmod_data->l3c_hwcfg.evtype_reg0_off;
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	u32 event_value, value = 0;
	u32 cfg_en;
	int i;

	event_value = (val -
			HISI_HWEVENT_L3C_READ_ALLOCATE);

	/* Select the appropriate Event select register */
	if (idx > 3)
		reg_offset += 4;

	/* Value to write to event type register */
	val = event_value << (8 * idx);

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Set the event in L3C_EVENT_TYPEx Register
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
		cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];
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
}

u32 hisi_write_l3c_counter(struct hisi_pmu *pl3c_pmu, int idx, u32 value)
{
	struct device_node *djtag_node;
	struct hisi_l3c_data *l3c_hwmod_data = pl3c_pmu->hwmod_data;
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	u32 reg_offset, cfg_en;
	int i, ret = 0;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = l3c_hwmod_data->l3c_hwcfg.counter_reg0_off +
					(idx * 4);

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
		cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];
		ret = hisi_djtag_writereg(module_id,
					cfg_en,
					reg_offset,
					value,
					djtag_node);
		if (!ret)
			ret = value;
	}

	return ret;
}

void hisi_enable_l3c_counter(struct hisi_pmu *pl3c_pmu, int idx)
{
	struct device_node *djtag_node;
	struct hisi_l3c_data *l3c_hwmod_data = pl3c_pmu->hwmod_data;
	u32 reg_offset = l3c_hwmod_data->l3c_hwcfg.auctrl_reg_off;
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	u32 auctrl_eventen = l3c_hwmod_data->l3c_hwcfg.auctrl_event_enable;
	u32 value, cfg_en;
	int i;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Set the event_bus_en bit in L3C AUCNTRL to enable counting
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
		cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];
		hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value |= auctrl_eventen;
		hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}
}

void hisi_disable_l3c_counter(struct hisi_pmu *pl3c_pmu, int idx)
{
	struct device_node *djtag_node;
	struct hisi_l3c_data *l3c_hwmod_data = pl3c_pmu->hwmod_data;
	u32 reg_offset = l3c_hwmod_data->l3c_hwcfg.auctrl_reg_off;
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	u32 auctrl_eventen = l3c_hwmod_data->l3c_hwcfg.auctrl_event_enable;
	u32 value, cfg_en;
	int i;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Clear the event_bus_en bit in L3C AUCNTRL if no other
	 * event counting for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
		cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];
		hisi_djtag_readreg(module_id,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value &= ~(auctrl_eventen);
		hisi_djtag_writereg(module_id,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}
}

void hisi_clear_l3c_event_idx(struct hisi_pmu *phisi_pmu,
							int idx)
{
	struct device_node *djtag_node;
	void *bitmap_addr;
	struct hisi_l3c_data *l3c_hwmod_data = phisi_pmu->hwmod_data;
	u32 reg_offset = l3c_hwmod_data->l3c_hwcfg.evtype_reg0_off;
	u32 module_id = l3c_hwmod_data->l3c_hwcfg.module_id;
	u32 cfg_en, value;
	int i;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = l3c_hwmod_data->hisi_l3c_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Clear Counting in L3C event config register */
	if (idx > 3)
		reg_offset += 4;

	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Clear the event in L3C_EVENT_TYPEx Register
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->l3c_hwcfg.num_banks; i++) {
		cfg_en = l3c_hwmod_data->l3c_hwcfg.bank_cfgen[i];

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
}

int hisi_l3c_get_event_idx(struct hisi_pmu *pl3c_pmu)
{
	struct hisi_l3c_data *l3c_hwmod_data = pl3c_pmu->hwmod_data;
	int event_idx;

	event_idx =
		find_first_zero_bit(
			l3c_hwmod_data->hisi_l3c_event_used_mask,
					 pl3c_pmu->num_counters);

	if (event_idx == HISI_MAX_CFG_L3C_CNTR)
		return -EAGAIN;

	__set_bit(event_idx,
		l3c_hwmod_data->hisi_l3c_event_used_mask);

	return event_idx;
}

static void hisi_free_l3c_data(struct hisi_pmu *pl3c_pmu)
{
	if (pl3c_pmu->hwmod_data) {
		kfree(pl3c_pmu->hwmod_data);
		pl3c_pmu->hwmod_data = NULL;
	}
}

static int init_hisi_l3c_hwcfg(struct device_node *node,
				struct hisi_l3c_data *pl3c_data)
{
	struct hisi_l3c_hwcfg *pl3c_hwcfg = &pl3c_data->l3c_hwcfg;
	const u32 *cfgen_map = NULL;
	u32 cfgen_map_len;

	if (of_property_read_u32(node, "counter-reg",
				     &pl3c_hwcfg->counter_reg0_off))
		goto fail;

	if (of_property_read_u32(node, "auctrl-reg",
				     &pl3c_hwcfg->auctrl_reg_off))
		goto fail;

	if (of_property_read_u32(node, "auctrl-event-en",
				     &pl3c_hwcfg->auctrl_event_enable))
		goto fail;

	if (of_property_read_u32(node, "evtype-reg",
				     &pl3c_hwcfg->evtype_reg0_off))
		goto fail;

	if (of_property_read_u32(node, "num-banks",
				     &pl3c_hwcfg->num_banks))
		goto fail;

	if (of_property_read_u32(node, "module-id",
				     &pl3c_hwcfg->module_id))
		goto fail;

	cfgen_map = of_get_property(node, "cfgen-map", &cfgen_map_len);
	of_property_read_u32_array(node, "cfgen-map",
					&pl3c_hwcfg->bank_cfgen[0],
					pl3c_hwcfg->num_banks);

	return 0;

fail:
	pr_err("init_hisi_l3c_hwcfg: fail to read DT properties!\n");
	return -EINVAL;
}

static int init_hisi_l3c_data(struct platform_device *pdev,
					struct hisi_pmu *pl3c_pmu)
{
	struct device *dev = &pdev->dev;
	struct device_node *parent_node = dev->of_node;
	struct device_node *djtag_node;
	struct hisi_l3c_data *l3c_hwmod_data = NULL;
	int ret;

	l3c_hwmod_data = kzalloc(sizeof(struct hisi_l3c_data),
							GFP_KERNEL);
	if (!l3c_hwmod_data)
		return -ENOMEM;

	djtag_node = of_parse_phandle(parent_node, "djtag", 0);
	if (IS_ERR(djtag_node)) {
		pr_info("l3c_device_probe-node without djtag!\n");
		goto fail;
	}

	/* Set the djtag node */
	l3c_hwmod_data->djtag_node = djtag_node;

	pl3c_pmu->hw_events.events = devm_kcalloc(&pdev->dev,
					pl3c_pmu->num_counters,
					sizeof(*pl3c_pmu->hw_events.events),
					GFP_KERNEL);
	if (!pl3c_pmu->hw_events.events) {
		ret = -ENOMEM;
		goto free_mem;
	}

	raw_spin_lock_init(&pl3c_pmu->hw_events.pmu_lock);

	pl3c_pmu->hwmod_data = l3c_hwmod_data;

	ret = init_hisi_l3c_hwcfg(parent_node, l3c_hwmod_data);
	if (ret)
		goto free_mem;

	return 0;

fail:
	pr_err("init_hisi_l3c_data: fail to read DT properties!\n");
	ret = -EINVAL;

free_mem:
	hisi_free_l3c_data(pl3c_pmu);
	return ret;
}

static struct attribute *hisi_l3c_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	NULL,
};

static struct attribute_group hisi_l3c_format_group = {
	.name = "format",
	.attrs = hisi_l3c_format_attr,
};

static struct attribute *hisi_l3c_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(read_allocate,
					"event=0x300"),
	HISI_PMU_EVENT_ATTR_STR(write_allocate,
					"event=0x301"),
	HISI_PMU_EVENT_ATTR_STR(read_noallocate,
					"event=0x302"),
	HISI_PMU_EVENT_ATTR_STR(write_noallocate,
					"event=0x303"),
	HISI_PMU_EVENT_ATTR_STR(read_hit, "event=0x304"),
	HISI_PMU_EVENT_ATTR_STR(write_hit, "event=0x305"),
	NULL,
};

static struct attribute_group hisi_l3c_events_group = {
	.name = "events",
	.attrs = hisi_l3c_events_attr,
};

static struct attribute *hisi_l3c_attrs[] = {
	NULL,
};

struct attribute_group hisi_l3c_attr_group = {
	.attrs = hisi_l3c_attrs,
};

static const struct attribute_group *hisi_l3c_pmu_attr_groups[] = {
	&hisi_l3c_attr_group,
	&hisi_l3c_format_group,
	&hisi_l3c_events_group,
	NULL
};

static int hisi_l3c_pmu_init(struct platform_device *pdev,
					struct hisi_pmu *pl3c_pmu)
{
	struct device *dev = &pdev->dev;

	if (device_property_read_u32(dev, "num-events",
				     &pl3c_pmu->num_events)) {
		pr_err("l3c_device_probe: Cant read num-events from DT!\n");
		return -EINVAL;
	}

	if (device_property_read_u32(dev, "num-counters",
				     &pl3c_pmu->num_counters)) {
		pr_err("l3c_device_probe: Cant read num-counters from DT!\n");
		return -EINVAL;
	}

	/* Find the SCL ID */
	if (device_property_read_u32(dev, "scl-id",
					&pl3c_pmu->scl_id)) {
		pr_err("l3c_device_probe: cant read scl-id!\n");
		return -EINVAL;
	}

	if (pl3c_pmu->scl_id == 0 ||
		pl3c_pmu->scl_id >= MAX_UNITS) {
		pr_err("l3c_device_probe: Invalid SCL=%d!\n",
						pl3c_pmu->scl_id);
		return -EINVAL;
	}

	pl3c_pmu->pmu_type = SCCL_SPECIFIC;
	pl3c_pmu->name = kasprintf(GFP_KERNEL, "hisi_l3c%d",
						pl3c_pmu->scl_id);
	pl3c_pmu->hwmod_type = HISI_L3C;
	pl3c_pmu->plat_device = pdev;
	pl3c_pmu->set_evtype = hisi_set_l3c_evtype;
	pl3c_pmu->get_event_idx = hisi_l3c_get_event_idx;
	pl3c_pmu->clear_event_idx = hisi_clear_l3c_event_idx;
	pl3c_pmu->event_update = hisi_l3c_event_update;
	pl3c_pmu->enable_counter = hisi_enable_l3c_counter;
	pl3c_pmu->disable_counter = hisi_disable_l3c_counter;
	pl3c_pmu->write_counter = hisi_write_l3c_counter;

	return 0;
}

static int hisi_pmu_l3c_dev_probe(struct platform_device *pdev)
{
	struct hisi_pmu *pl3c_pmu = NULL;
	int ret;

	pl3c_pmu = hisi_pmu_alloc(pdev);
	if (IS_ERR(pl3c_pmu))
		return PTR_ERR(pl3c_pmu);

	ret = hisi_l3c_pmu_init(pdev, pl3c_pmu);
	if (ret)
		return ret;

	ret = init_hisi_l3c_data(pdev, pl3c_pmu);
	if (ret)
		goto fail_init;

	/* Register with perf PMU */
	pl3c_pmu->pmu = (struct pmu) {
		.name = pl3c_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.pmu_enable = hisi_uncore_pmu_enable,
		.pmu_disable = hisi_uncore_pmu_disable,
		.event_init = hisi_uncore_pmu_event_init,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_l3c_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(pl3c_pmu, pdev, pl3c_pmu->name);
	if (ret) {
		pr_err("hisi_uncore_pmu_init FAILED!!\n");
		goto fail;
	}

	return 0;

fail:
	hisi_free_l3c_data(pl3c_pmu);

fail_init:
	if (pl3c_pmu)
		devm_kfree(&pdev->dev, pl3c_pmu);

	return ret;
}

static int hisi_pmu_l3c_dev_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id l3c_of_match[] = {
	{ .compatible = "hisilicon,hisi-pmu-l3c", },
	{},
};
MODULE_DEVICE_TABLE(of, l3c_of_match);

static struct platform_driver hisi_pmu_l3c_driver = {
	.driver = {
		.name = "hisi-pmu-l3c",
		.of_match_table = l3c_of_match,
	},
	.probe = hisi_pmu_l3c_dev_probe,
	.remove = hisi_pmu_l3c_dev_remove,
};
module_platform_driver(hisi_pmu_l3c_driver);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x L3C PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anurup M");
