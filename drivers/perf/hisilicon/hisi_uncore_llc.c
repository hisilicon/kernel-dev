/*
 * HiSilicon SoC LLC Hardware event counters support
 *
 * Copyright (C) 2016 Huawei Technologies Limited
 * Author: Anurup M <anurup.m@huawei.com>
 *
 * This code is based heavily on the ARMv7 perf event code.
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
#include <linux/module.h>
#include <linux/bitmap.h>
#include <linux/of.h>
#include <linux/perf_event.h>
#include "hisi_uncore_llc.h"

/* Map cfg_en values for LLC Banks */
const int llc_cfgen_map[] = { HISI_LLC_BANK0_CFGEN, HISI_LLC_BANK1_CFGEN,
				HISI_LLC_BANK2_CFGEN, HISI_LLC_BANK3_CFGEN
};

struct hisi_pmu *hisi_uncore_llc;

static inline int hisi_llc_counter_valid(int idx)
{
	return (idx >= ARMV8_HISI_IDX_LLC_COUNTER0 &&
			idx < ARMV8_HISI_IDX_LLC_COUNTER_MAX);
}

u64 hisi_llc_event_update(struct perf_event *event,
				struct hw_perf_event *hwc, int idx)
{
	struct device_node *djtag_node;
	struct hisi_pmu *pllc_pmu = to_hisi_pmu(event->pmu);
	struct hisi_hwmod_unit *punit = NULL;
	hisi_llc_data *llc_hwmod_data = NULL;
	u64 delta, delta_ovflw = 0, prev_raw_count, new_raw_count = 0;
	int cfg_en;
	u32 raw_event_code = hwc->config_base;
	u32 scclID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 llc_idx = scclID - 1;
	u32 ovfl_cnt = 0;
	int i, j = 0;

	if (!scclID || (HISI_SCCL_MASK < scclID)) {
		pr_err("Invalid DieID=%d in event code!\n", scclID);
		return 0;
	}

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	punit = &pllc_pmu->hwmod_pmu_unit[llc_idx];
	llc_hwmod_data = punit->hwmod_data;

	/* Check if the LLC data is initialized for this SCCL */
	if (!llc_hwmod_data->djtag_node) {
		pr_err("LLC: scclID=%d not initialized!\n", scclID);
		return 0;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = llc_hwmod_data->djtag_node;

	do {
		prev_raw_count = local64_read(&hwc->prev_count);
		for (i = 0; i < llc_hwmod_data->num_banks; i++, j++) {
			cfg_en = llc_hwmod_data->bank[i].cfg_en;

			new_raw_count =
					hisi_read_llc_counter(idx,
							djtag_node, cfg_en);
			/* update the interrupt count also */
			ovfl_cnt =
				atomic_read(
				&llc_hwmod_data->bank[i].cntr_ovflw[idx]);
			if (0 != ovfl_cnt) {
				delta_ovflw =
					(0xFFFFFFFF - (u32)prev_raw_count) +
							 ((ovfl_cnt - 1) * 0xFFFFFFFF);
				pr_debug("Counter overflow detected. delta_ovflow=%llu\n",
								delta_ovflw);

				/* Set the overflow count to 0 */
				atomic_set(
				&llc_hwmod_data->bank[i].cntr_ovflw[idx],
									0);
				delta = ((new_raw_count + delta_ovflw) &
							HISI_ARMV8_MAX_PERIOD);
			} else {
				delta = (new_raw_count - prev_raw_count) &
							HISI_ARMV8_MAX_PERIOD;
			}

			local64_add(delta, &event->count);

			pr_debug("LLC: delta for event:0x%x is %llu\n",
							raw_event_code, delta);
		}
	} while (local64_cmpxchg(
			&hwc->prev_count, prev_raw_count, new_raw_count) !=
								prev_raw_count);

	return new_raw_count;
}

/*
 * Enable interrupts for counter overlow
 */
int hisi_init_llc_hw_perf_event(struct hisi_pmu *pllc_pmu,
					struct hw_perf_event *hwc)
{
	struct device_node *djtag_node;
	struct hisi_hwmod_unit *punit = NULL;
	hisi_llc_data *llc_hwmod_data = NULL;
	u32 raw_event_code = hwc->config_base;
	atomic_t *active_events;
	u32 scclID = (raw_event_code & HISI_SCCL_MASK) >> 20;
	u32 llc_idx = scclID - 1;
	u32 value, cfg_en, i;

	if (!scclID || (HISI_SCCL_MAX < scclID)) {
		pr_err("LLC: Invalid DieID=%d in event code=%d!\n",
						scclID, raw_event_code);
		return -EINVAL;
	}

	punit = &pllc_pmu->hwmod_pmu_unit[llc_idx];
	llc_hwmod_data = punit->hwmod_data;

	active_events = &punit->active_events;

	/* Check if the LLC data is initialized for this SCCL */
	if (!llc_hwmod_data->djtag_node) {
		pr_err("LLC: scclID=%d not initialized!\n", scclID);
		return -EINVAL;
	}

	/* Find the djtag device node of the SCCL */
	djtag_node = llc_hwmod_data->djtag_node;

	if (!atomic_inc_not_zero(active_events)) {
		mutex_lock(&punit->reserve_mutex);
		if (atomic_read(active_events) == 0) {
			/* Enable interrupts for counter overlow */
			for (i = 0; i < llc_hwmod_data->num_banks; i++) {
				cfg_en = llc_hwmod_data->bank[i].cfg_en;

				hisi_djtag_readreg(HISI_LLC_MODULE_ID,
						cfg_en,
						HISI_LLC_BANK_INTM,
						djtag_node, &value);
				if (value) {
					hisi_djtag_writereg(HISI_LLC_MODULE_ID,
						cfg_en,
						HISI_LLC_BANK_INTM,
						0x0,
						djtag_node);
				}
			}
			atomic_inc(active_events);
		}
		mutex_unlock(&punit->reserve_mutex);
	}

	return 0;
}

void hisi_set_llc_evtype(hisi_llc_data *llc_hwmod_data,
						int idx, u32 val)
{
	struct device_node *djtag_node;
	u32 reg_offset;
	u32 value = 0;
	int cfg_en;
	u32 event_value;
	int i;

	event_value = (val -
			HISI_HWEVENT_LLC_READ_ALLOCATE);

	/* Select the appropriate Event select register */
	if (idx <= 3)
		reg_offset = HISI_LLC_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_LLC_EVENT_TYPE1_REG_OFF;

	/* Value to write to event type register */
	val = event_value << (8 * idx);

	/* Find the djtag device node of the Unit */
	djtag_node = llc_hwmod_data->djtag_node;

	/*
	 * Set the event in LLC_EVENT_TYPEx Register
	 * for all LLC banks
	 */
	for (i = 0; i < llc_hwmod_data->num_banks; i++) {
		cfg_en = llc_hwmod_data->bank[i].cfg_en;
		hisi_djtag_readreg(HISI_LLC_MODULE_ID,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value &= ~(0xff << (8 * idx));
		value |= val;

		hisi_djtag_writereg(HISI_LLC_MODULE_ID,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}
}

u32 hisi_write_llc_counter(hisi_llc_data *llc_hwmod_data,
					int idx, u32 value)
{
	struct device_node *djtag_node;
	int cfg_en;
	u32 reg_offset = 0;
	int i, ret = 0;

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_LLC_COUNTER0_REG_OFF +
					(idx * 4);

	/* Find the djtag device node of the Unit */
	djtag_node = llc_hwmod_data->djtag_node;

	for (i = 0; i < llc_hwmod_data->num_banks; i++) {
		cfg_en = llc_hwmod_data->bank[i].cfg_en;
		ret = hisi_djtag_writereg(HISI_LLC_MODULE_ID,
					cfg_en,
					reg_offset,
					value,
					djtag_node);
		if (0 == ret)
			ret = value;
	}

	return ret;
}

irqreturn_t hisi_llc_event_handle_irq(int irq_num, void *dev)
{
	pr_debug("- hisi_llc_event_handle_irq -\n");
	return IRQ_HANDLED;

	/* TBD */
}

int hisi_enable_llc_counter(hisi_llc_data *llc_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;
	int cfg_en;
	int i, ret = 0;

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = llc_hwmod_data->djtag_node;

	/*
	 * Set the event_bus_en bit in LLC AUCNTRL to enable counting
	 * for all LLC banks
	 */
	for (i = 0; i < llc_hwmod_data->num_banks; i++) {
		cfg_en = llc_hwmod_data->bank[i].cfg_en;
		ret = hisi_djtag_readreg(HISI_LLC_MODULE_ID,
				cfg_en,
				HISI_LLC_AUCTRL_REG_OFF,
				djtag_node, &value);

		value |= HISI_LLC_AUCTRL_EVENT_BUS_EN;
		ret = hisi_djtag_writereg(HISI_LLC_MODULE_ID,
				cfg_en,
				HISI_LLC_AUCTRL_REG_OFF,
				value,
				djtag_node);
	}

	return ret;
}

void hisi_disable_llc_counter(hisi_llc_data *llc_hwmod_data, int idx)
{
	struct device_node *djtag_node;
	u32 value = 0;
	int cfg_en;
	int i;

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	/* Find the djtag device node of the Unit */
	djtag_node = llc_hwmod_data->djtag_node;

	/*
	 * Clear the event_bus_en bit in LLC AUCNTRL if no other
	 * event counting for all LLC banks
	 */
	for (i = 0; i < llc_hwmod_data->num_banks; i++) {
		cfg_en = llc_hwmod_data->bank[i].cfg_en;
		hisi_djtag_readreg(HISI_LLC_MODULE_ID,
				cfg_en,
				HISI_LLC_AUCTRL_REG_OFF,
				djtag_node, &value);

		value &= ~(HISI_LLC_AUCTRL_EVENT_BUS_EN);
		hisi_djtag_writereg(HISI_LLC_MODULE_ID,
				cfg_en,
				HISI_LLC_AUCTRL_REG_OFF,
				value,
				djtag_node);
	}
}

void hisi_clear_llc_event_idx(struct hisi_hwmod_unit *punit,
							int idx)
{
	struct device_node *djtag_node;
	void *bitmap_addr;
	hisi_llc_data *llc_hwmod_data = punit->hwmod_data;
	u32 cfg_en, value, reg_offset;
	int i;

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return;
	}

	bitmap_addr = llc_hwmod_data->hisi_llc_event_used_mask;

	__clear_bit(idx, bitmap_addr);

	/* Clear Counting in LLC event config register */
	if (idx <= 3)
		reg_offset = HISI_LLC_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_LLC_EVENT_TYPE1_REG_OFF;

	djtag_node = llc_hwmod_data->djtag_node;

	/*
	 * Clear the event in LLC_EVENT_TYPEx Register
	 * for all LLC banks
	 */
	for (i = 0; i < llc_hwmod_data->num_banks; i++) {
		cfg_en = llc_hwmod_data->bank[i].cfg_en;

		hisi_djtag_readreg(HISI_LLC_MODULE_ID,
				cfg_en,
				reg_offset,
				djtag_node, &value);

		value &= ~(0xff << (8 * idx));
		value |= (0xff << (8 * idx));
		hisi_djtag_writereg(HISI_LLC_MODULE_ID,
				cfg_en,
				reg_offset,
				value,
				djtag_node);
	}

	return;
}

int hisi_llc_get_event_idx(struct hisi_hwmod_unit *punit)
{
	hisi_llc_data *llc_hwmod_data = punit->hwmod_data;
	int event_idx;

	event_idx =
		find_first_zero_bit(
			llc_hwmod_data->hisi_llc_event_used_mask,
					HISI_ARMV8_MAX_CFG_LLC_CNTR);

	if (event_idx == HISI_ARMV8_MAX_CFG_LLC_CNTR) {
		return -EAGAIN;
	}

	__set_bit(event_idx,
		llc_hwmod_data->hisi_llc_event_used_mask);

	pr_debug("LLC:event_idx=%d\n", event_idx);

	return event_idx;
}


void hisi_uncore_pmu_write_evtype(struct hisi_hwmod_unit *punit,
						int idx, u32 val)
{
	val &= HISI_ARMV8_EVTYPE_EVENT;

	/* Select event based on Counter Module */
	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		hisi_set_llc_evtype(punit->hwmod_data, idx, val);
	}
}

u32 hisi_read_llc_counter(int idx, struct device_node *djtag_node, int bank)
{
	u32 reg_offset = 0;
	u32 value;

	if (!hisi_llc_counter_valid(idx)) {
		pr_err("Unsupported event index:%d!\n", idx);
		return -EINVAL;
	}

	reg_offset = HISI_LLC_COUNTER0_REG_OFF + (idx * 4);

	hisi_djtag_readreg(HISI_LLC_MODULE_ID, /* ModuleID  */
			bank,
			reg_offset, /* Register Offset */
			djtag_node, &value);

	return value;
}

static int init_hisi_llc_banks(hisi_llc_data *pllc_data,
				struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	int i, j, ret, irq, num_irqs = 0;

	pllc_data->num_banks = NUM_LLC_BANKS;
	for (i = 0; i < NUM_LLC_BANKS; i++)
		pllc_data->bank[i].cfg_en = llc_cfgen_map[i];

	/* Get the number of IRQ's for LLC banks in a Totem */
	while ((res =
		platform_get_resource(pdev, IORESOURCE_IRQ, num_irqs))) {
		num_irqs++;
	}

	if (num_irqs > NUM_LLC_BANKS) {
		pr_err("LLC: Invalid IRQ numbers in dts.\n");
		return -EINVAL;
	}

	for (i = 0; i < num_irqs; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(dev, "failed to get irq index %d\n", i);
			return -ENODEV;
		}

		ret = request_irq(irq, hisi_llc_event_handle_irq,
				IRQF_NOBALANCING,
				"hisi-pmu", pllc_data);
		if (ret) {
			pr_err("LLC: unable to request IRQ%d for HISI PMU" \
					"Stub counters\n", irq);
			goto err;
		}
		pr_debug("LLC:IRQ:%d assigned to bank:%d\n", irq, i);

		pllc_data->bank[i].irq = irq;
	}

	return 0;
err:
	for (j = 0; j < i; j++) {
		irq = platform_get_irq(pdev, j);
		if (irq < 0) {
			dev_err(dev, "failed to get irq index %d\n", i);
			continue;
		}
		free_irq(irq, NULL);
	}

	return ret;
}

static int init_hisi_llc_data(struct platform_device *pdev,
					struct hisi_pmu *pllc_pmu,
							int *punit_id)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *djtag_node;
	struct hisi_hwmod_unit *punit;
	hisi_llc_data *llc_hwmod_data = NULL;
	struct of_phandle_args arg;
	int ret, sccl_id;

	ret = of_parse_phandle_with_fixed_args(node,
						"djtag", 1, 0, &arg);
	if (!ret) {
		if (arg.args[0] > 0  && arg.args[0] <= MAX_UNITS) {
			sccl_id = arg.args[0];
			djtag_node = arg.np;
			if (MAX_UNITS < sccl_id) {
				pr_err("LLC: llc_device_probe - Invalid SCCL ID=%d..\n!",
						sccl_id);
				return -EINVAL;
			}
		} else
			return -EINVAL;
	} else {
		pr_err("LLC: llc_device_probe - node without djtag..\n!");
		return -EINVAL;
	}

	llc_hwmod_data = kzalloc(sizeof(hisi_llc_data), GFP_KERNEL);
	if (!llc_hwmod_data)
		return -ENOMEM;

	llc_hwmod_data->djtag_node = djtag_node;
	punit = &pllc_pmu->hwmod_pmu_unit[sccl_id - 1];

	ret = hisi_pmu_unit_init(pdev, punit, sccl_id,
						HISI_ARMV8_MAX_CFG_LLC_CNTR);
	if (ret) {
		kfree(llc_hwmod_data);
		return ret;
	}

	ret = init_hisi_llc_banks(llc_hwmod_data, pdev);
	if (ret) {
		kfree(llc_hwmod_data);
		return ret;
	}

	punit->hwmod_data = llc_hwmod_data;

	*punit_id = sccl_id - 1;
	return 0;
}

static void hisi_free_llc_data(struct hisi_hwmod_unit *punit)
{
	if (punit->hwmod_data)
		kfree(punit->hwmod_data);
}

PMU_FORMAT_ATTR(event, "config:0-11");
PMU_FORMAT_ATTR(bank, "config:12-15");
PMU_FORMAT_ATTR(module, "config:16-19");
PMU_FORMAT_ATTR(cluster, "config:20-23");
PMU_FORMAT_ATTR(socket, "config:24-25");

#define HISI_UNCORE_EVENT_DESC(_name, _config)			\
{								\
	.attr   = __ATTR(_name, 0444, uncore_event_show, NULL), \
	.config = _config,					\
}

static struct attribute *hisi_llc_format_attr[] = {
	&format_attr_event.attr,
	&format_attr_bank.attr,
	&format_attr_module.attr,
	&format_attr_cluster.attr,
	&format_attr_socket.attr,
	NULL,
};

static struct attribute_group hisi_llc_format_group = {
	.name = "format",
	.attrs = hisi_llc_format_attr,
};

EVENT_ATTR_STR(l3c_read_allocate,
			"event=0x301,bank=?,module=0x4,cluster=?,socket=0x1");
EVENT_ATTR_STR(l3c_write_allocate,
			"event=0x302,bank=?,module=0x4,cluster=?,socket=0x1");
EVENT_ATTR_STR(l3c_read_noallocate,
			"event=0x303,bank=?,module=0x4,cluster=?,socket=0x1");
EVENT_ATTR_STR(l3c_write_noallocate,
			"event=0x304,bank=?,module=0x4,cluster=?,socket=0x1");
EVENT_ATTR_STR(l3c_read_hit,
			"event=0x305,bank=?,module=0x4,cluster=?,socket=0x1");
EVENT_ATTR_STR(l3c_write_hit,
			"event=0x306,bank=?,module=0x4,cluster=?,socket=0x1");


static struct attribute *hisi_llc_events_attr[] = {
	EVENT_PTR(l3c_read_allocate),
	EVENT_PTR(l3c_write_allocate),
	EVENT_PTR(l3c_read_noallocate),
	EVENT_PTR(l3c_write_noallocate),
	EVENT_PTR(l3c_read_hit),
	EVENT_PTR(l3c_write_hit),
	NULL,
};

static struct attribute_group hisi_llc_events_group = {
	.name = "events",
	.attrs = hisi_llc_events_attr,
};

static struct attribute *hisi_llc_attrs[] = {
	NULL,
};

struct attribute_group hisi_llc_attr_group = {
	.attrs = hisi_llc_attrs,
};

static const struct attribute_group *hisi_llc_pmu_attr_groups[] = {
	&hisi_llc_attr_group,
	&hisi_llc_format_group,
	&hisi_llc_events_group,
	NULL
};

static int hisi_pmu_uncore_cpu_notifier(struct notifier_block *self,
					unsigned long action, void *hcpu)
{
	pr_debug("-- hisi_pmu_uncore_cpu_notifier --\n");

	return NOTIFY_OK;
	/* TBD */
}

void hisi_llc_pmu_init(struct platform_device *pdev,
					struct hisi_pmu *pllc_pmu)
{
	pllc_pmu->pmu_type = SCCL_SPECIFIC;
	pllc_pmu->name = "HISI_L3C";
	pllc_pmu->num_counters = HISI_ARMV8_MAX_CFG_LLC_CNTR;
	pllc_pmu->num_events = HISI_LLC_MAX_EVENTS;
	pllc_pmu->hwmod_type = HISI_LLC;
}

static int hisi_pmu_llc_dev_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_pmu *pllc_pmu = hisi_uncore_llc;
	struct hisi_hwmod_unit *punit;
	int unit_id;

	/* Allocate and Register PMU for the first time */
	if (!hisi_uncore_llc) {
		pllc_pmu = hisi_pmu_alloc(pdev);
		if (IS_ERR(pllc_pmu))
			return PTR_ERR(pllc_pmu);
		hisi_llc_pmu_init(pdev, pllc_pmu);
	}

	ret = init_hisi_llc_data(pdev, pllc_pmu, &unit_id);
	if (ret)
		goto fail_init;

    pllc_pmu->num_units++;

	pllc_pmu->plat_device = pdev;
	hisi_llc_pmu_init(pdev, pllc_pmu);

	if (!hisi_uncore_llc) {
		/* First active LLC in the chip registers the pmu */
		pllc_pmu->pmu = (struct pmu) {
				.name		= "HISI-L3C-PMU",
		/*		.task_ctx_nr	= perf_invalid_context, */
				.pmu_enable 	= hisi_uncore_pmu_enable,
				.pmu_disable	= hisi_uncore_pmu_disable,
				.event_init 	= hisi_uncore_pmu_event_init,
				.add			= hisi_uncore_pmu_add,
				.del			= hisi_uncore_pmu_del,
				.start			= hisi_uncore_pmu_start,
				.stop			= hisi_uncore_pmu_stop,
				.read			= hisi_uncore_pmu_read,
				.attr_groups	= hisi_llc_pmu_attr_groups,
		};

		ret = hisi_uncore_pmu_setup(pllc_pmu, pdev, "hisi_l3c");
		if (ret) {
			pr_err("----- hisi_uncore_pmu_init FAILED!! -----\n");
			goto fail;
		}

		pllc_pmu->cpu_nb = (struct notifier_block) {
			.notifier_call	= hisi_pmu_uncore_cpu_notifier,
			/*
			 * to migrate uncore events, our notifier should be executed
			 * before perf core's notifier.
			 */
			.priority	= CPU_PRI_PERF + 1,
		};

		ret = register_cpu_notifier(&pllc_pmu->cpu_nb);
		if (ret)
			goto fail;

		hisi_uncore_llc = pllc_pmu;
	}

	return 0;

fail:
	punit = &pllc_pmu->hwmod_pmu_unit[unit_id];
	hisi_free_llc_data(punit);

fail_init:
	if (!hisi_uncore_llc) {
		devm_kfree(&pdev->dev, pllc_pmu);
	}

	return ret;
}

static int hisi_pmu_llc_dev_remove(struct platform_device *pdev)
{
	if (hisi_uncore_llc)
		devm_kfree(&pdev->dev, hisi_uncore_llc);

	return 0;
}

static struct of_device_id llc_of_match[] = {
	{ .compatible = "hisilicon,hip05-llc", },
	{ .compatible = "hisilicon,hip06-llc", },
	{},
};
MODULE_DEVICE_TABLE(of, llc_of_match);

static struct platform_driver hisi_pmu_llc_driver = {
	.driver = {
		.name = "hisi-llc-perf",
		.of_match_table = llc_of_match,
	},
	.probe = hisi_pmu_llc_dev_probe,
	.remove = hisi_pmu_llc_dev_remove,
};
module_platform_driver(hisi_pmu_llc_driver);

MODULE_DESCRIPTION("HiSilicon ARMv8 LLC PMU driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anurup M");
