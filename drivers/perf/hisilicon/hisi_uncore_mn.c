/*
 * HiSilicon SoC MN Hardware event counters support
 *
 * Copyright (C) 2017 Huawei Technologies Limited
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
#include <linux/acpi.h>
#include <linux/bitmap.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/perf_event.h>
#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon MN event types.
 */
enum armv8_hisi_mn_event_types {
	HISI_HWEVENT_MN_EO_BARR_REQ	= 0x0,
	HISI_HWEVENT_MN_EC_BARR_REQ	= 0x01,
	HISI_HWEVENT_MN_DVM_OP_REQ	= 0x02,
	HISI_HWEVENT_MN_DVM_SYNC_REQ	= 0x03,
	HISI_HWEVENT_MN_READ_REQ	= 0x04,
	HISI_HWEVENT_MN_WRITE_REQ	= 0x05,
	HISI_HWEVENT_MN_EVENT_MAX	= 0x08,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_mn_counters {
	HISI_IDX_MN_COUNTER0	= 0x0,
	HISI_IDX_MN_COUNTER_MAX	= 0x4,
};

#define HISI_MAX_CFG_MN_CNTR	0x04
#define MN1_EVTYPE_REG_OFF 0x48
#define MN1_EVCTRL_REG_OFF 0x40
#define MN1_CNT0_REG_OFF 0x30
#define MN1_EVENT_EN 0x01
#define MN1_BANK_SELECT 0x01

#define MN1_INTM_REG_OFF 0x060
#define MN1_INTS_REG_OFF 0x068
#define MN1_INTC_REG_OFF 0x06C
#define MN1_INTM_UNMASK_ALL 0x0

#define GET_MODULE_ID(hwmod_data) hwmod_data->mn_hwcfg.module_id

struct hisi_mn_hwcfg {
	u32 module_id;
};

struct hisi_mn_data {
	struct hisi_djtag_client *client;
	DECLARE_BITMAP(event_used_mask, HISI_MAX_CFG_MN_CNTR);
	struct hisi_mn_hwcfg mn_hwcfg;
};

static inline int hisi_mn_counter_valid(int idx)
{
	return (idx >= HISI_IDX_MN_COUNTER0 &&
			idx < HISI_IDX_MN_COUNTER_MAX);
}

/* Select the counter register offset from the index */
static inline u32 get_counter_reg_off(int cntr_idx)
{
	return (MN1_CNT0_REG_OFF + (cntr_idx * 4));
}

static u32 hisi_mn_read_counter(struct hisi_mn_data *mn_data, int cntr_idx)
{
	struct hisi_djtag_client *client = mn_data->client;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 reg_off, value;

	reg_off = get_counter_reg_off(cntr_idx);
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, reg_off,
			   client, &value);

	return value;
}

static u64 hisi_mn_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct hisi_pmu *mn_pmu = to_hisi_pmu(event->pmu);
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	u64 delta, prev_raw_count, new_raw_count = 0;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(mn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return 0;
	}

	do {
		/* Get count from the MN */
		prev_raw_count = local64_read(&hwc->prev_count);
		new_raw_count =	hisi_mn_read_counter(mn_data, idx);
		delta = (new_raw_count - prev_raw_count) & HISI_MAX_PERIOD;

		local64_add(delta, &event->count);
	} while (local64_cmpxchg(&hwc->prev_count, prev_raw_count,
				 new_raw_count) != prev_raw_count);

	return new_raw_count;
}

static irqreturn_t hisi_pmu_mn_isr(int irq, void *dev_id)
{
	struct hisi_pmu *mn_pmu = dev_id;
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	struct perf_event *event;
	unsigned long flags;
	unsigned long overflown;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 ints = 0;
	int idx;

	raw_spin_lock_irqsave(&mn_pmu->lock, flags);

	/* Read the INTS register */
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_INTS_REG_OFF,
			   client, &ints);
	if (!ints) {
		raw_spin_unlock_irqrestore(&mn_pmu->lock, flags);
		return IRQ_NONE;
	}
	overflown = ints;

	/* Find the counter index which overflowed and handle them */
	for_each_set_bit(idx, &overflown, HISI_MAX_CFG_MN_CNTR) {
		/* Clear the IRQ status flag */
		hisi_djtag_writereg(module_id, MN1_BANK_SELECT,
				    MN1_INTC_REG_OFF, (1 << idx),
				    client);

		/* Get the corresponding event struct */
		event = mn_pmu->hw_perf_events[idx];
		if (!event)
			continue;

		hisi_mn_event_update(event, &event->hw, idx);
		hisi_pmu_set_event_period(event);
		perf_event_update_userpage(event);
	}

	raw_spin_unlock_irqrestore(&mn_pmu->lock, flags);
	return IRQ_HANDLED;
}

static void hisi_mn_set_evtype(struct hisi_pmu *mn_pmu, int idx, u32 val)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 event_value, value = 0;

	event_value = (val - HISI_HWEVENT_MN_EO_BARR_REQ);

	/*
	 * Value to write to event select register
	 * Each byte in the 32 bit select register is used to
	 * configure the event code. Each byte correspond to a
	 * counter register to use.
	 */
	val = event_value << (8 * idx);

	/*
	 * Set the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_EVTYPE_REG_OFF,
			   client, &value);
	value &= ~(0xff << (8 * idx));
	value |= val;
	hisi_djtag_writereg(module_id, MN1_BANK_SELECT, MN1_EVTYPE_REG_OFF,
			    value, client);
}

static void hisi_mn_clear_evtype(struct hisi_pmu *mn_pmu, int idx)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 value;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(mn_pmu->dev,
				"Unsupported event index:%d!\n", idx);
		return;
	}

	/*
	 * Clear the event in MN_EVENT_TYPE Register
	 */
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_EVTYPE_REG_OFF,
			   client, &value);
	value &= ~(0xff << (8 * idx));
	value |= (0xff << (8 * idx));
	hisi_djtag_writereg(module_id, MN1_BANK_SELECT, MN1_EVTYPE_REG_OFF,
			    value, client);
}

static u32 hisi_mn_write_counter(struct hisi_pmu *mn_pmu,
				 struct hw_perf_event *hwc, u32 value)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 reg_off;
	int idx = GET_CNTR_IDX(hwc);
	int ret;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(mn_pmu->dev, "Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_off = get_counter_reg_off(idx);
	ret = hisi_djtag_writereg(module_id, MN1_BANK_SELECT, reg_off,
				  value, client);
	if (!ret)
		ret = value;

	return ret;
}

static void hisi_mn_start_counters(struct hisi_pmu *mn_pmu)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	unsigned long *used_mask = mn_data->event_used_mask;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 num_counters = mn_pmu->num_counters;
	u32 value;
	int enabled = bitmap_weight(used_mask, num_counters);

	if (!enabled)
		return;

	/*
	 * Set the event_bus_en bit in MN_EVENT_CTRL to start counting
	 * for the L3C bank
	 */
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_EVCTRL_REG_OFF,
			   client, &value);
	value |= MN1_EVENT_EN;
	hisi_djtag_writereg(module_id, MN1_BANK_SELECT, MN1_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_mn_stop_counters(struct hisi_pmu *mn_pmu)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	u32 module_id = GET_MODULE_ID(mn_data);
	u32 value;

	/*
	 * Clear the event_bus_en bit in MN_EVENT_CTRL
	 */
	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_EVCTRL_REG_OFF,
			   client, &value);
	value &= ~(MN1_EVENT_EN);
	hisi_djtag_writereg(module_id, MN1_BANK_SELECT, MN1_EVCTRL_REG_OFF,
			    value, client);
}

static void hisi_mn_clear_event_idx(struct hisi_pmu *mn_pmu, int idx)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	void *bitmap_addr;

	if (!hisi_mn_counter_valid(idx)) {
		dev_err(mn_pmu->dev, "Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = mn_data->event_used_mask;
	clear_bit(idx, bitmap_addr);
}

static int hisi_mn_get_event_idx(struct hisi_pmu *mn_pmu)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	const unsigned long *used_mask = mn_data->event_used_mask;
	u32 num_counters = mn_pmu->num_counters;
	int event_idx;

	event_idx = find_first_zero_bit(used_mask, num_counters);
	if (event_idx == num_counters)
		return -EAGAIN;

	set_bit(event_idx, mn_data->event_used_mask);

	return event_idx;
}

static void hisi_mn_enable_interrupts(u32 module_id,
				      struct hisi_djtag_client *client)
{
	u32 intm = 0;

	hisi_djtag_readreg(module_id, MN1_BANK_SELECT, MN1_INTM_REG_OFF,
			   client, &intm);
	if (intm)
		hisi_djtag_writereg(module_id, MN1_BANK_SELECT,
				    MN1_INTM_REG_OFF, MN1_INTM_UNMASK_ALL,
				    client);
}

static int hisi_mn_init_irq(int irq, struct hisi_pmu *mn_pmu,
			    struct hisi_djtag_client *client)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	u32 module_id = GET_MODULE_ID(mn_data);
	struct device *dev = &client->dev;
	int rc;

	rc = devm_request_irq(dev, irq, hisi_pmu_mn_isr,
			      IRQF_NOBALANCING | IRQF_NO_THREAD,
			      dev_name(dev), mn_pmu);
	if (rc) {
		dev_err(dev, "Could not request IRQ:%d\n", irq);
		return rc;
	}

	/* Overflow interrupt also should use the same CPU */
	rc = irq_set_affinity(irq, &mn_pmu->cpu);
	if (rc) {
		dev_err(dev, "could not set IRQ affinity!\n");
		return rc;
	}

	/*
	 * Unmask all interrupts in Mask register
	 * Enable all IRQ's
	 */
	hisi_mn_enable_interrupts(module_id, client);

	return 0;
}

static const struct of_device_id mn_of_match[] = {
	{ .compatible = "hisilicon,hip05-pmu-mn-v1", },
	{ .compatible = "hisilicon,hip06-pmu-mn-v1", },
	{ .compatible = "hisilicon,hip07-pmu-mn-v2", },
	{},
};
MODULE_DEVICE_TABLE(of, mn_of_match);

static const struct acpi_device_id hisi_mn_pmu_acpi_match[] = {
	{ "HISI0221", },
	{ "HISI0222", },
	{},
};
MODULE_DEVICE_TABLE(acpi, hisi_l3c_pmu_acpi_match);

static int hisi_mn_init_irqs_fdt(struct device *dev,
				 struct hisi_pmu *mn_pmu)
{
	struct hisi_mn_data *mn_data = mn_pmu->hwmod_data;
	struct hisi_djtag_client *client = mn_data->client;
	int irq = -1, num_irqs, i;

	num_irqs = of_irq_count(dev->of_node);
	for (i = 0; i < num_irqs; i++) {
		irq = of_irq_get(dev->of_node, i);
		if (irq < 0)
			dev_info(dev, "No IRQ resource!\n");
	}

	if (irq < 0)
		return 0;

	/* The last entry in the IRQ list to be chosen
	 * This is as per mbigen-v2 IRQ mapping
	 */
	return hisi_mn_init_irq(irq, mn_pmu, client);
}

static int hisi_mn_init_data(struct hisi_pmu *mn_pmu,
			     struct hisi_djtag_client *client)
{
	struct hisi_mn_data *mn_data;
	struct hisi_mn_hwcfg *mn_hwcfg;
	struct device *dev = &client->dev;
	int ret;

	mn_data = devm_kzalloc(dev, sizeof(*mn_data), GFP_KERNEL);
	if (!mn_data)
		return -ENOMEM;

	/* Set the djtag Identifier */
	mn_data->client = client;
	mn_pmu->hw_perf_events = devm_kcalloc(dev, mn_pmu->num_counters,
					      sizeof(*mn_pmu->hw_perf_events),
					      GFP_KERNEL);
	if (!mn_pmu->hw_perf_events)
		return -ENOMEM;

	raw_spin_lock_init(&mn_pmu->lock);
	mn_pmu->hwmod_data = mn_data;
	mn_hwcfg = &mn_data->mn_hwcfg;

	if (dev->of_node) {
		const struct of_device_id *of_id;

		of_id = of_match_device(mn_of_match, dev);
		if (!of_id) {
			dev_err(dev, "DT: Match device fail!\n");
			return -EINVAL;
		}

		ret = hisi_mn_init_irqs_fdt(dev, mn_pmu);
		if (ret)
			return ret;
	} else if (ACPI_COMPANION(dev)) {
		const struct acpi_device_id *acpi_id;

		acpi_id = acpi_match_device(hisi_mn_pmu_acpi_match, dev);
		if (!acpi_id) {
			dev_err(dev, "ACPI: Match device fail!\n");
			return -EINVAL;
		}
	} else
		return -EINVAL;

	ret = device_property_read_u32(dev, "hisilicon,module-id",
				       &mn_hwcfg->module_id);
	if (ret < 0) {
		dev_err(dev, "DT: Could not read module-id!\n");
		return -EINVAL;
	}

	return 0;
}

static struct attribute *hisi_mn_format_attr[] = {
	HISI_PMU_FORMAT_ATTR(event, "config:0-11"),
	NULL,
};

static const struct attribute_group hisi_mn_format_group = {
	.name = "format",
	.attrs = hisi_mn_format_attr,
};

static struct attribute *hisi_mn_events_attr[] = {
	HISI_PMU_EVENT_ATTR_STR(eo_barrier_req, "event=0x0"),
	HISI_PMU_EVENT_ATTR_STR(ec_barrier_req,	"event=0x01"),
	HISI_PMU_EVENT_ATTR_STR(dvm_op_req, "event=0x02"),
	HISI_PMU_EVENT_ATTR_STR(dvm_sync_req, "event=0x03"),
	HISI_PMU_EVENT_ATTR_STR(read_req, "event=0x04"),
	HISI_PMU_EVENT_ATTR_STR(write_req, "event=0x05"),
	NULL,
};

static const struct attribute_group hisi_mn_events_group = {
	.name = "events",
	.attrs = hisi_mn_events_attr,
};

static struct attribute *hisi_mn_attrs[] = {
	NULL,
};

static const struct attribute_group hisi_mn_attr_group = {
	.attrs = hisi_mn_attrs,
};

static DEVICE_ATTR(cpumask, 0444, hisi_cpumask_sysfs_show, NULL);

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
	.set_evtype = hisi_mn_set_evtype,
	.clear_evtype = hisi_mn_clear_evtype,
	.set_event_period = hisi_pmu_set_event_period,
	.get_event_idx = hisi_mn_get_event_idx,
	.clear_event_idx = hisi_mn_clear_event_idx,
	.event_update = hisi_mn_event_update,
	.start_counters = hisi_mn_start_counters,
	.stop_counters = hisi_mn_stop_counters,
	.write_counter = hisi_mn_write_counter,
};

static int hisi_mn_pmu_init(struct hisi_pmu *mn_pmu,
				struct hisi_djtag_client *client)
{
	struct device *dev = &client->dev;

	mn_pmu->num_events = HISI_HWEVENT_MN_EVENT_MAX;
	mn_pmu->num_counters = HISI_IDX_MN_COUNTER_MAX;
	mn_pmu->scl_id = hisi_djtag_get_sclid(client);

	mn_pmu->name = kasprintf(GFP_KERNEL, "hisi_mn_%d", mn_pmu->scl_id);
	mn_pmu->ops = &hisi_uncore_mn_ops;
	mn_pmu->dev = dev;

	/* Pick one core to use for cpumask attributes */
	cpumask_set_cpu(smp_processor_id(), &mn_pmu->cpu);

	return 0;
}

static int hisi_pmu_mn_dev_probe(struct hisi_djtag_client *client)
{
	struct hisi_pmu *mn_pmu;
	struct device *dev = &client->dev;
	int ret;

	mn_pmu = hisi_pmu_alloc(dev);
	if (!mn_pmu)
		return -ENOMEM;

	ret = hisi_mn_pmu_init(mn_pmu, client);
	if (ret)
		return ret;

	ret = hisi_mn_init_data(mn_pmu, client);
	if (ret)
		return ret;

	mn_pmu->pmu = (struct pmu) {
		.name = mn_pmu->name,
		.task_ctx_nr = perf_invalid_context,
		.event_init = hisi_uncore_pmu_event_init,
		.pmu_enable = hisi_uncore_pmu_enable,
		.pmu_disable = hisi_uncore_pmu_disable,
		.add = hisi_uncore_pmu_add,
		.del = hisi_uncore_pmu_del,
		.start = hisi_uncore_pmu_start,
		.stop = hisi_uncore_pmu_stop,
		.read = hisi_uncore_pmu_read,
		.attr_groups = hisi_mn_pmu_attr_groups,
	};

	ret = hisi_uncore_pmu_setup(mn_pmu, mn_pmu->name);
	if (ret) {
		dev_err(mn_pmu->dev, "hisi_uncore_pmu_init FAILED!!\n");
		return ret;
	}

	/* Set the drv data to MN pmu */
	dev_set_drvdata(dev, mn_pmu);

	return 0;
}

static int hisi_pmu_mn_dev_remove(struct hisi_djtag_client *client)
{
	struct hisi_pmu *mn_pmu = NULL;
	struct device *dev = &client->dev;

	mn_pmu = dev_get_drvdata(dev);
	perf_pmu_unregister(&mn_pmu->pmu);

	return 0;
}

static struct hisi_djtag_driver hisi_pmu_mn_driver = {
	.driver = {
		.name = "hisi-pmu-mn",
		.of_match_table = mn_of_match,
		.acpi_match_table = ACPI_PTR(hisi_mn_pmu_acpi_match),
	},
	.probe = hisi_pmu_mn_dev_probe,
	.remove = hisi_pmu_mn_dev_remove,
};

static int __init hisi_pmu_mn_init(void)
{
	int rc;

	rc = hisi_djtag_register_driver(THIS_MODULE, &hisi_pmu_mn_driver);
	if (rc < 0) {
		pr_err("hisi pmu MN init failed, rc=%d\n", rc);
		return rc;
	}

	return 0;
}
module_init(hisi_pmu_mn_init);

static void __exit hisi_pmu_mn_exit(void)
{
	hisi_djtag_unregister_driver(&hisi_pmu_mn_driver);
}
module_exit(hisi_pmu_mn_exit);

MODULE_DESCRIPTION("HiSilicon SoC HIP0x MN PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shaokun Zhang");
