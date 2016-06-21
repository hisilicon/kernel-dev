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

/* Map cfg_en values for L3C Banks */
const int l3c_cfgen_map[] = { HISI_L3C_BANK0_CFGEN, HISI_L3C_BANK1_CFGEN,
				HISI_L3C_BANK2_CFGEN, HISI_L3C_BANK3_CFGEN
};

static struct hisi_pmu *hisi_uncore_l3c;

static inline int hisi_l3c_counter_valid(int idx)
{
	return (idx >= HISI_IDX_L3C_COUNTER0 &&
			idx <= HISI_IDX_L3C_COUNTER_MAX);
}

u64 hisi_l3c_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct device_node *djtag_node;
	struct hisi_pmu *pl3c_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	struct hisi_l3c_data *l3c_hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;
	int cfg_en;
	u32 raw_event_code = hwc->config_base;
	u32 scclID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 l3c_idx = scclID - 1;
	int i;

	if (!scclID || (scclID >= HISI_SCCL_MASK)) {
		pr_err("Invalid SCCL=%d in event code!\n", scclID);
		return 0;
	}

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return 0;
	}

	punit = &pl3c_pmu->hwmod_pmu_unit[l3c_idx];
	l3c_hwmod_data = punit->hwmod_data;

	/* Check if the L3C data is initialized for this SCCL */
	if (!l3c_hwmod_data->djtag_node) {
		pr_err("SCCL=%d not initialized!\n", scclID);
		return 0;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = l3c_hwmod_data->djtag_node;

	do {
		prev_raw_count = local64_read(&hwc->prev_count);
		for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
			cfg_en = l3c_hwmod_data->bank[i].cfg_en;

			new_raw_count =
					hisi_read_l3c_counter(idx,
							djtag_node, cfg_en);
			delta = (new_raw_count - prev_raw_count) &
							HISI_MAX_PERIOD;

			local64_add(delta, &event->count);

			pr_debug("delta for event:0x%x is %llu\n",
						raw_event_code, delta);
		}
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
								prev_raw_count);

	return new_raw_count;
}

void hisi_set_l3c_evtype(struct hisi_l3c_data *l3c_hwmod_data,
						int idx, u32 val)
{
	struct device_node *djtag_node;
	u32 reg_offset;
	u32 value = 0;
	int cfg_en;
	u32 event_value;
	int i;

	event_value = (val -
			HISI_HWEVENT_L3C_READ_ALLOCATE);

	/* Select the appropriate Event select register */
	if (idx <= 3)
		reg_offset = HISI_L3C_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_L3C_EVENT_TYPE1_REG_OFF;

	/* Value to write to event type register */
	val = event_value << (8 * idx);

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Set the event in L3C_EVENT_TYPEx Register
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
		cfg_en = l3c_hwmod_data->bank[i].cfg_en;
		hisi_djtag_readreg(HISI_L3C_MODULE_ID,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value &= ~(0xff << (8 * idx));
		value |= val;

		hisi_djtag_writereg(HISI_L3C_MODULE_ID,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}
}

u32 hisi_write_l3c_counter(struct hisi_l3c_data *l3c_hwmod_data,
					int idx, u32 value)
{
	struct device_node *djtag_node;
	int cfg_en;
	u32 reg_offset = 0;
	int i, ret = 0;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_L3C_COUNTER0_REG_OFF +
					(idx * 4);

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
		cfg_en = l3c_hwmod_data->bank[i].cfg_en;
		ret = hisi_djtag_writereg(HISI_L3C_MODULE_ID,
					cfg_en,
					reg_offset,
					value,
					djtag_node);
		if (!ret)
			ret = value;
	}

	return ret;
}

int hisi_enable_l3c_counter(struct hisi_l3c_data *l3c_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;
	int cfg_en;
	int i, ret = 0;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Set the event_bus_en bit in L3C AUCNTRL to enable counting
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
		cfg_en = l3c_hwmod_data->bank[i].cfg_en;
		ret = hisi_djtag_readreg(HISI_L3C_MODULE_ID,
				cfg_en,
				HISI_L3C_AUCTRL_REG_OFF,
				djtag_node, &value);

		value |= HISI_L3C_AUCTRL_EVENT_BUS_EN;
		ret = hisi_djtag_writereg(HISI_L3C_MODULE_ID,
				cfg_en,
				HISI_L3C_AUCTRL_REG_OFF,
				value,
				djtag_node);
	}

	return ret;
}

void hisi_disable_l3c_counter(struct hisi_l3c_data *l3c_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;
	int cfg_en;
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
	for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
		cfg_en = l3c_hwmod_data->bank[i].cfg_en;
		hisi_djtag_readreg(HISI_L3C_MODULE_ID,
				cfg_en,
				HISI_L3C_AUCTRL_REG_OFF,
				djtag_node, &value);

		value &= ~(HISI_L3C_AUCTRL_EVENT_BUS_EN);
		hisi_djtag_writereg(HISI_L3C_MODULE_ID,
				cfg_en,
				HISI_L3C_AUCTRL_REG_OFF,
				value,
				djtag_node);
	}
}

void hisi_clear_l3c_event_idx(struct hisi_hwmod_unit *punit,
							int idx)
{
	struct device_node *djtag_node;
	void *bitmap_addr;
	struct hisi_l3c_data *l3c_hwmod_data = punit->hwmod_data;
	u32 cfg_en, value, reg_offset;
	int i;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = l3c_hwmod_data->hisi_l3c_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Clear Counting in L3C event config register */
	if (idx <= 3)
		reg_offset = HISI_L3C_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_L3C_EVENT_TYPE1_REG_OFF;

	djtag_node = l3c_hwmod_data->djtag_node;

	/*
	 * Clear the event in L3C_EVENT_TYPEx Register
	 * for all L3C banks
	 */
	for (i = 0; i < l3c_hwmod_data->num_banks; i++) {
		cfg_en = l3c_hwmod_data->bank[i].cfg_en;

		hisi_djtag_readreg(HISI_L3C_MODULE_ID,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value &= ~(0xff << (8 * idx));
		value |= (0xff << (8 * idx));
		hisi_djtag_writereg(HISI_L3C_MODULE_ID,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}
}

int hisi_l3c_get_event_idx(struct hisi_hwmod_unit *punit)
{
	struct hisi_l3c_data *l3c_hwmod_data = punit->hwmod_data;
	int event_idx;

	event_idx =
		find_first_zero_bit(
			l3c_hwmod_data->hisi_l3c_event_used_mask,
					HISI_MAX_CFG_L3C_CNTR);

	if (event_idx == HISI_MAX_CFG_L3C_CNTR)
		return -EAGAIN;

	__set_bit(event_idx,
		l3c_hwmod_data->hisi_l3c_event_used_mask);

	pr_debug("event_idx=%d\n", event_idx);

	return event_idx;
}

u32 hisi_read_l3c_counter(int idx, struct device_node *djtag_node, int bank)
{
	u32 reg_offset = 0;
	u32 value;

	if (!hisi_l3c_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_L3C_COUNTER0_REG_OFF + (idx * 4);

	hisi_djtag_readreg(HISI_L3C_MODULE_ID, /* ModuleID  */
			bank,
			reg_offset, /* Register Offset */
			djtag_node, &value);

	return value;
}

static int init_hisi_l3c_banks(struct hisi_l3c_data *pl3c_data,
				struct platform_device *pdev)
{
	int i;

	pl3c_data->num_banks = NUM_L3C_BANKS;
	for (i = 0; i < NUM_L3C_BANKS; i++)
		pl3c_data->bank[i].cfg_en = l3c_cfgen_map[i];

	return 0;
}

static int init_hisi_l3c_data(struct platform_device *pdev,
					struct hisi_pmu *pl3c_pmu,
							int *punit_id)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *djtag_node;
	struct hisi_hwmod_unit *punit;
	struct hisi_l3c_data *l3c_hwmod_data;
	struct of_phandle_args arg;
	int ret, sccl_id;

	ret = of_parse_phandle_with_fixed_args(node,
						"djtag", 1, 0, &arg);
	if (!ret) {
		if (arg.args[0] > 0  && arg.args[0] <= MAX_UNITS) {
			sccl_id = arg.args[0];
			djtag_node = arg.np;
			if (sccl_id >= MAX_UNITS) {
				pr_err("l3c_device_probe-Invalid SCCL=%d!\n",
						sccl_id);
				return -EINVAL;
			}
		} else
			return -EINVAL;
	} else {
		pr_err("l3c_device_probe-node without djtag!\n");
		return -EINVAL;
	}

	l3c_hwmod_data = kzalloc(sizeof(struct hisi_l3c_data), GFP_KERNEL);
	if (!l3c_hwmod_data)
		return -ENOMEM;

	l3c_hwmod_data->djtag_node = djtag_node;
	punit = &pl3c_pmu->hwmod_pmu_unit[sccl_id - 1];

	ret = hisi_pmu_unit_init(pdev, punit, sccl_id,
					HISI_MAX_CFG_L3C_CNTR);
	if (ret) {
		kfree(l3c_hwmod_data);
		return ret;
	}

	ret = init_hisi_l3c_banks(l3c_hwmod_data, pdev);
	if (ret) {
		kfree(l3c_hwmod_data);
		return ret;
	}

	punit->hwmod_data = l3c_hwmod_data;

	*punit_id = sccl_id - 1;
	return 0;
}

static void hisi_free_l3c_data(struct hisi_hwmod_unit *punit)
{
	kfree(punit->hwmod_data);
}

void hisi_l3c_pmu_init(struct platform_device *pdev,
					struct hisi_pmu *pl3c_pmu)
{
	pl3c_pmu->pmu_type = SCCL_SPECIFIC;
	pl3c_pmu->name = "hip05_l3c";
	pl3c_pmu->num_counters = HISI_MAX_CFG_L3C_CNTR;
	pl3c_pmu->num_events = HISI_L3C_MAX_EVENTS;
	pl3c_pmu->hwmod_type = HISI_L3C;
}

static int hisi_pmu_l3c_dev_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_pmu *pl3c_pmu = hisi_uncore_l3c;
	struct hisi_hwmod_unit *punit;
	int unit_id;

	/* Allocate and Register PMU for the first time */
	if (!hisi_uncore_l3c) {
		pl3c_pmu = hisi_pmu_alloc(pdev);
		if (IS_ERR(pl3c_pmu))
			return PTR_ERR(pl3c_pmu);
		hisi_l3c_pmu_init(pdev, pl3c_pmu);
	}

	ret = init_hisi_l3c_data(pdev, pl3c_pmu, &unit_id);
	if (ret)
		goto fail_init;

	pl3c_pmu->num_units++;

	pl3c_pmu->plat_device = pdev;
	hisi_l3c_pmu_init(pdev, pl3c_pmu);

	if (!hisi_uncore_l3c) {
		/* First active L3C in the chip registers the pmu */
		pl3c_pmu->pmu = (struct pmu) {
				.name		= "hip05_l3c",
				.task_ctx_nr	= perf_invalid_context,
				.pmu_enable = hisi_uncore_pmu_enable,
				.pmu_disable = hisi_uncore_pmu_disable,
				.event_init = hisi_uncore_pmu_event_init,
				.add = hisi_uncore_pmu_add,
				.del = hisi_uncore_pmu_del,
				.start = hisi_uncore_pmu_start,
				.stop = hisi_uncore_pmu_stop,
				.read = hisi_uncore_pmu_read,
		};

		ret = hisi_uncore_pmu_setup(pl3c_pmu, pdev, "hip05_l3c");
		if (ret) {
			pr_err("hisi_uncore_pmu_init FAILED!!\n");
			goto fail;
		}

		hisi_uncore_l3c = pl3c_pmu;
	}

	return 0;

fail:
	punit = &pl3c_pmu->hwmod_pmu_unit[unit_id];
	hisi_free_l3c_data(punit);

fail_init:
	if (!hisi_uncore_l3c)
		devm_kfree(&pdev->dev, pl3c_pmu);

	return ret;
}

static int hisi_pmu_l3c_dev_remove(struct platform_device *pdev)
{
	if (hisi_uncore_l3c)
		devm_kfree(&pdev->dev, hisi_uncore_l3c);

	return 0;
}

static const struct of_device_id l3c_of_match[] = {
	{ .compatible = "hisilicon,hip05-l3c", },
	{},
};
MODULE_DEVICE_TABLE(of, l3c_of_match);

static struct platform_driver hisi_pmu_l3c_driver = {
	.driver = {
		.name = "hip05-l3c-pmu",
		.of_match_table = l3c_of_match,
	},
	.probe = hisi_pmu_l3c_dev_probe,
	.remove = hisi_pmu_l3c_dev_remove,
};
module_platform_driver(hisi_pmu_l3c_driver);

MODULE_DESCRIPTION("HiSilicon HIP05 L3C PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anurup M");
