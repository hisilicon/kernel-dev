/*
 * HiSilicon SoC MN Hardware event counters support
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
#include "hisi_uncore_mn.h"

static struct hisi_pmu *hisi_uncore_mn;

static inline int hisi_mn_counter_valid(int idx)
{
	return (idx >= HISI_IDX_MN_COUNTER0 &&
			idx <= HISI_IDX_MN_COUNTER_MAX);
}

u64 hisi_mn_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct device_node *djtag_node;
	struct hisi_pmu *pmn_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit;
	struct hisi_mn_data *mn_hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;
	u32 raw_event_code = hwc->config_base;
	u32 scclID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 mn_idx = scclID - 1;

	if (!scclID || (scclID >= HISI_SCCL_MASK)) {
		pr_err("Invalid SCCL=%d in event code!\n", scclID);
		return 0;
	}

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return 0;
	}

	punit = &pmn_pmu->hwmod_pmu_unit[mn_idx];
	mn_hwmod_data = punit->hwmod_data;

	/* Check if the MN data is initialized for this SCCL */
	if (!mn_hwmod_data->djtag_node) {
		pr_err("SCCL=%d not initialized!\n", scclID);
		return 0;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = mn_hwmod_data->djtag_node;

	do {
		prev_raw_count = local64_read(&hwc->prev_count);
		new_raw_count =
			hisi_read_mn_counter(idx,
					     djtag_node, HISI_MN_CFGEN);
		delta = (new_raw_count - prev_raw_count) &
			HISI_MAX_PERIOD;

		local64_add(delta, &event->count);
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
							prev_raw_count);

	return new_raw_count;
}

void hisi_set_mn_evtype(struct hisi_mn_data *mn_hwmod_data,
						int idx, u32 val)
{
	struct device_node *djtag_node;
	u32 reg_offset;
	u32 value1 = 0;
	u32 value2 = 0;
	u32 event_value;

	event_value = (val -
			HISI_HWEVENT_MN_EO_BARR_REQ);


	/* Value to write to event type register */
	value1 = event_value << (8 * idx);

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;

	/*
	 * Set the event in MN_EVENT_TYPE Register
	 */
	reg_offset = HISI_MN_EVENT_TYPE_REG_OFF;

	hisi_djtag_readreg(HISI_MN1_MODULE_ID,
			   HISI_MN_CFGEN,
			   reg_offset,
			   djtag_node, &value2);

	value2 &= ~(0xff << (8 * idx));
	value2 |= value1;

	hisi_djtag_writereg(HISI_MN1_MODULE_ID,
			    HISI_MN_CFGEN,
			    reg_offset,
			    value2,
			    djtag_node);
}

u32 hisi_write_mn_counter(struct hisi_mn_data *mn_hwmod_data,
					int idx, u32 value)
{
	struct device_node *djtag_node;
	u32 reg_offset = 0;
	int ret = 0;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_MN_COUNTER0_REG_OFF +
					(idx * 4);

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;
	ret = hisi_djtag_writereg(HISI_MN1_MODULE_ID,
				  HISI_MN_CFGEN,
				  reg_offset,
				  value,
				  djtag_node);
	if (!ret)
		ret = value;

	return ret;
}

void hisi_enable_mn_counter(struct hisi_mn_data *mn_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;

	/*
	 * Set the event_bus_en bit in MN_EVENT_CTRL to enable counting
	 */
	hisi_djtag_readreg(HISI_MN1_MODULE_ID,
				 HISI_MN_CFGEN,
				 HISI_MN_EVENT_TYPE_REG_OFF,
				 djtag_node, &value);

	value |= HISI_MN_EVENT_EN;
	hisi_djtag_writereg(HISI_MN1_MODULE_ID,
				 HISI_MN_CFGEN,
				 HISI_MN_EVENT_TYPE_REG_OFF,
				 value,
				 djtag_node);
}

void hisi_disable_mn_counter(struct hisi_mn_data *mn_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = mn_hwmod_data->djtag_node;

	/*
	 * Clear the event_bus_en bit in MN event control
	 */
	hisi_djtag_readreg(HISI_MN1_MODULE_ID,
			   HISI_MN_CFGEN,
			   HISI_MN_EVENT_TYPE_REG_OFF,
			   djtag_node, &value);

	value &= ~(HISI_MN_EVENT_EN);
	hisi_djtag_writereg(HISI_MN1_MODULE_ID,
			    HISI_MN_CFGEN,
			    HISI_MN_EVENT_TYPE_REG_OFF,
			    value,
			    djtag_node);
}

void hisi_clear_mn_event_idx(struct hisi_hwmod_unit *punit,
							int idx)
{
	struct device_node *djtag_node;
	void *bitmap_addr;
	struct hisi_mn_data *mn_hwmod_data = punit->hwmod_data;
	u32 value, reg_offset;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = mn_hwmod_data->hisi_mn_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Clear Counting in MN event config register */
	reg_offset = HISI_MN_EVENT_TYPE_REG_OFF;

	djtag_node = mn_hwmod_data->djtag_node;

	/*
	 * Clear the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(HISI_MN1_MODULE_ID,
			   HISI_MN_CFGEN,
			   reg_offset,
			   djtag_node, &value);

	value &= ~(0xff << (8 * idx));
	value |= (0xff << (8 * idx));
	hisi_djtag_writereg(HISI_MN1_MODULE_ID,
			    HISI_MN_CFGEN,
			    reg_offset,
			    value,
			    djtag_node);
}

int hisi_mn_get_event_idx(struct hisi_hwmod_unit *punit)
{
	struct hisi_mn_data *mn_hwmod_data = punit->hwmod_data;
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

u32 hisi_read_mn_counter(int idx, struct device_node *djtag_node, int bank)
{
	u32 reg_offset = 0;
	u32 value;

	if (!hisi_mn_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_MN_COUNTER0_REG_OFF + (idx * 4);

	hisi_djtag_readreg(HISI_MN1_MODULE_ID, /* ModuleID  */
			bank,
			reg_offset, /* Register Offset */
			djtag_node, &value);

	return value;
}

static int init_hisi_mn_data(struct platform_device *pdev,
					struct hisi_pmu *pmn_pmu,
							int *punit_id)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *djtag_node;
	struct hisi_hwmod_unit *punit;
	struct hisi_mn_data *mn_hwmod_data;
	struct of_phandle_args arg;
	int ret, sccl_id;

	ret = of_parse_phandle_with_fixed_args(node,
						"djtag", 1, 0, &arg);
	if (!ret) {
		if (arg.args[0] > 0  && arg.args[0] <= MAX_UNITS) {
			sccl_id = arg.args[0];
			djtag_node = arg.np;
			if (sccl_id >= MAX_UNITS) {
				pr_err("mn_device_probe-Invalid SCCL=%d!\n",
						sccl_id);
				return -EINVAL;
			}
		} else
			return -EINVAL;
	} else {
		pr_err("mn_device_probe-node without djtag!\n");
		return -EINVAL;
	}

	mn_hwmod_data = kzalloc(sizeof(struct hisi_mn_data), GFP_KERNEL);
	if (!mn_hwmod_data)
		return -ENOMEM;

	mn_hwmod_data->djtag_node = djtag_node;
	punit = &pmn_pmu->hwmod_pmu_unit[sccl_id - 1];

	ret = hisi_pmu_unit_init(pdev, punit, sccl_id,
					HISI_MAX_CFG_MN_CNTR);
	if (ret) {
		kfree(mn_hwmod_data);
		return ret;
	}

	mn_hwmod_data->num_banks = 0;
	punit->hwmod_data = mn_hwmod_data;
	*punit_id = sccl_id - 1;

	return 0;
}

static void hisi_free_mn_data(struct hisi_hwmod_unit *punit)
{
	kfree(punit->hwmod_data);
}

static struct attribute *hisi_mn_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	HISI_PMU_FORMAT_ATTR(bank, "config:12-15"),
	HISI_PMU_FORMAT_ATTR(cpu_cluster, "config:16-19"),
	HISI_PMU_FORMAT_ATTR(cpu_die, "config:20-23"),
	NULL,
};

static struct attribute_group hisi_mn_format_group = {
	.name = "format",
	.attrs = hisi_mn_format_attr,
};

static struct attribute *hisi_mn_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(eo_barrier_req,
			"event=0x316,cpu_die=?"),
	HISI_PMU_EVENT_ATTR_STR(ec_barrier_req,
			"event=0x317,cpu_die=?"),
	HISI_PMU_EVENT_ATTR_STR(dvm_op_req,
			"event=0x318,cpu_die=?"),
	HISI_PMU_EVENT_ATTR_STR(dvm_sync_req,
			"event=0x319,cpu_die=?"),
	HISI_PMU_EVENT_ATTR_STR(read_req,
			"event=0x31A,cpu_die=?"),
	HISI_PMU_EVENT_ATTR_STR(write_req,
			"event=0x31B,cpu_die=?"),
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

void hisi_mn_pmu_init(struct platform_device *pdev,
					struct hisi_pmu *pmn_pmu)
{
	pmn_pmu->pmu_type = SCCL_SPECIFIC;
	pmn_pmu->name = "hisi_mn";
	pmn_pmu->num_counters = HISI_MAX_CFG_MN_CNTR;
	pmn_pmu->num_events = HISI_MN_MAX_EVENTS;
	pmn_pmu->hwmod_type = HISI_MN;
	pmn_pmu->plat_device = pdev;
}

static int hisi_pmu_mn_dev_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_pmu *pmn_pmu = hisi_uncore_mn;
	struct hisi_hwmod_unit *punit;
	int unit_id;

	/* Allocate and Register PMU for the first time */
	if (!hisi_uncore_mn) {
		pmn_pmu = hisi_pmu_alloc(pdev);
		if (IS_ERR(pmn_pmu))
			return PTR_ERR(pmn_pmu);
		hisi_mn_pmu_init(pdev, pmn_pmu);
	}

	ret = init_hisi_mn_data(pdev, pmn_pmu, &unit_id);
	if (ret)
		goto fail_init;

	pmn_pmu->num_units++;

	if (!hisi_uncore_mn) {
		/* First active MN in the chip registers the pmu */
		pmn_pmu->pmu = (struct pmu) {
				.name		= "hisi_mn",
				.task_ctx_nr	= perf_invalid_context,
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

		ret = hisi_uncore_pmu_setup(pmn_pmu, pdev, "hisi_mn");
		if (ret) {
			pr_err("hisi_uncore_pmu_init FAILED!!\n");
			goto fail;
		}

		hisi_uncore_mn = pmn_pmu;
	}

	return 0;

fail:
	punit = &pmn_pmu->hwmod_pmu_unit[unit_id];
	hisi_free_mn_data(punit);

fail_init:
	if (!hisi_uncore_mn)
		devm_kfree(&pdev->dev, pmn_pmu);

	return ret;
}

static int hisi_pmu_mn_dev_remove(struct platform_device *pdev)
{
	if (hisi_uncore_mn)
		devm_kfree(&pdev->dev, hisi_uncore_mn);

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
MODULE_AUTHOR("Anurup M");
