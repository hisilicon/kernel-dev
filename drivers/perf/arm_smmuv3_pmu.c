/* Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * This driver adds support for perf events to use the Performance
 * Monitor Counter Groups (PMCG) associated with an SMMUv3 node
 * to monitor that node.
 *
 * Devices are named smmu_0_<phys_addr_page> where <phys_addr_page>
 * is the physical page address of the SMMU PMCG.
 * For example, the SMMU PMCG at 0xff88840000 is named smmu_0_ff88840
 *
 * Filtering by stream id is done by specifying filtering parameters
 * with the event. options are:
 *   filter_enable    - 0 = no filtering, 1 = filtering enabled
 *   filter_span      - 0 = exact match, 1 = pattern match
 *   filter_sec       - filter applies to non-secure (0) or secure (1) namespace
 *   filter_stream_id - pattern to filter against
 * Further filtering information is available in the SMMU documentation.
 *
 * Example: perf stat -e smmu_0_ff88840/transaction,filter_enable=1,
 *                       filter_span=1,filter_stream_id=0x42/ -a pwd
 * Applies filter pattern 0x42 to transaction events.
 *
 * SMMU events are not attributable to a CPU, so task mode and sampling
 * are not supported.
 */

#include <linux/acpi.h>
#include <linux/acpi_iort.h>
#include <linux/bitops.h>
#include <linux/cpuhotplug.h>
#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/msi.h>
#include <linux/perf_event.h>
#include <linux/platform_device.h>
#include <linux/smp.h>
#include <linux/sysfs.h>
#include <linux/types.h>

#include <asm/local64.h>

#define SMMU_PMCG_EVCNTR0               0x0
#define SMMU_PMCG_EVCNTR(n, stride)     (SMMU_PMCG_EVCNTR0 + (n) * (stride))
#define SMMU_PMCG_EVTYPER0              0x400
#define SMMU_PMCG_EVTYPER(n)            (SMMU_PMCG_EVTYPER0 + (n) * 4)
#define SMMU_PMCG_EVTYPER_SEC_SID_SHIFT       30
#define SMMU_PMCG_EVTYPER_SID_SPAN_SHIFT      29
#define SMMU_PMCG_EVTYPER_EVENT_MASK          GENMASK(15, 0)
#define SMMU_PMCG_SVR0                  0x600
#define SMMU_PMCG_SVR(n, stride)        (SMMU_PMCG_SVR0 + (n) * (stride))
#define SMMU_PMCG_SMR0                  0xA00
#define SMMU_PMCG_SMR(n)                (SMMU_PMCG_SMR0 + (n) * 4)
#define SMMU_PMCG_CNTENSET0             0xC00
#define SMMU_PMCG_CNTENCLR0             0xC20
#define SMMU_PMCG_INTENSET0             0xC40
#define SMMU_PMCG_INTENCLR0             0xC60
#define SMMU_PMCG_OVSCLR0               0xC80
#define SMMU_PMCG_OVSSET0               0xCC0
#define SMMU_PMCG_CAPR                  0xD88
#define SMMU_PMCG_SCR                   0xDF8
#define SMMU_PMCG_CFGR                  0xE00
#define SMMU_PMCG_CFGR_SID_FILTER_TYPE        BIT(23)
#define SMMU_PMCG_CFGR_CAPTURE                BIT(22)
#define SMMU_PMCG_CFGR_MSI                    BIT(21)
#define SMMU_PMCG_CFGR_RELOC_CTRS             BIT(20)
#define SMMU_PMCG_CFGR_SIZE_MASK              GENMASK(13, 8)
#define SMMU_PMCG_CFGR_SIZE_SHIFT             8
#define SMMU_PMCG_CFGR_COUNTER_SIZE_32        31
#define SMMU_PMCG_CFGR_NCTR_MASK              GENMASK(5, 0)
#define SMMU_PMCG_CFGR_NCTR_SHIFT             0
#define SMMU_PMCG_CR                    0xE04
#define SMMU_PMCG_CR_ENABLE                   BIT(0)
#define SMMU_PMCG_CEID0                 0xE20
#define SMMU_PMCG_CEID1                 0xE28
#define SMMU_PMCG_IRQ_CTRL              0xE50
#define SMMU_PMCG_IRQ_CTRL_IRQEN              BIT(0)
#define SMMU_PMCG_IRQ_CTRLACK           0xE54
#define SMMU_PMCG_IRQ_CTRLACK_IRQEN           BIT(0)
#define SMMU_PMCG_IRQ_CFG0              0xE58
#define SMMU_PMCG_IRQ_CFG0_ADDR_MASK          GENMASK(51, 2)
#define SMMU_PMCG_IRQ_CFG1              0xE60
#define SMMU_PMCG_IRQ_CFG2              0xE64
#define SMMU_PMCG_IRQ_STATUS            0xE68
#define SMMU_PMCG_IRQ_STATUS_IRQ_ABT          BIT(0)
#define SMMU_PMCG_AIDR                  0xE70

#define SMMU_COUNTER_RELOAD             BIT(31)
#define SMMU_DEFAULT_FILTER_SEC         0
#define SMMU_DEFAULT_FILTER_SPAN        1
#define SMMU_DEFAULT_FILTER_STREAM_ID   GENMASK(31, 0)

#define SMMU_MAX_COUNTERS               64
#define SMMU_MAX_EVENT_ID               128
#define SMMU_NUM_EVENTS_U32             (SMMU_MAX_EVENT_ID / sizeof(u32))

#define SMMU_PA_SHIFT                   12

/* Events */
#define SMMU_PMU_CYCLES                 0
#define SMMU_PMU_TRANSACTION            1
#define SMMU_PMU_TLB_MISS               2
#define SMMU_PMU_CONFIG_CACHE_MISS      3
#define SMMU_PMU_TRANS_TABLE_WALK       4
#define SMMU_PMU_CONFIG_STRUCT_ACCESS   5
#define SMMU_PMU_PCIE_ATS_TRANS_RQ      6
#define SMMU_PMU_PCIE_ATS_TRANS_PASSED  7

/* Common MSI config fields */
#define MSI_CFG0_ADDR_SHIFT             2
#define MSI_CFG0_ADDR_MASK              0x3fffffffffffUL
#define MSI_CFG2_SH_SHIFT               4
#define MSI_CFG2_SH_NSH                 (0UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_SH_OSH                 (2UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_SH_ISH                 (3UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_MEMATTR_SHIFT          0
#define MSI_CFG2_MEMATTR_DEVICE_nGnRE   (0x1 << MSI_CFG2_MEMATTR_SHIFT)

static int cpuhp_state_num;

struct smmu_pmu {
	struct hlist_node node;
	struct perf_event *events[SMMU_MAX_COUNTERS];
	DECLARE_BITMAP(used_counters, SMMU_MAX_COUNTERS);
	DECLARE_BITMAP(supported_events, SMMU_MAX_EVENT_ID);
	unsigned int irq;
	unsigned int on_cpu;
	struct pmu pmu;
	unsigned int num_counters;
	struct platform_device *pdev;
	void __iomem *reg_base;
	void __iomem *reloc_base;
	u64 counter_present_mask;
	u64 counter_mask;
	bool reg_size_32;
};

#define to_smmu_pmu(p) (container_of(p, struct smmu_pmu, pmu))

#define SMMU_PMU_EVENT_ATTR_EXTRACTOR(_name, _config, _size, _shift)    \
	static inline u32 get_##_name(struct perf_event *event)         \
	{                                                               \
		return (event->attr._config >> (_shift)) &              \
			GENMASK_ULL((_size) - 1, 0);                    \
	}

SMMU_PMU_EVENT_ATTR_EXTRACTOR(event, config, 16, 0);
SMMU_PMU_EVENT_ATTR_EXTRACTOR(filter_stream_id, config1, 32, 0);
SMMU_PMU_EVENT_ATTR_EXTRACTOR(filter_span, config1, 1, 32);
SMMU_PMU_EVENT_ATTR_EXTRACTOR(filter_sec, config1, 1, 33);
SMMU_PMU_EVENT_ATTR_EXTRACTOR(filter_enable, config1, 1, 34);

static inline void smmu_pmu_enable(struct pmu *pmu)
{
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(pmu);

	writel(SMMU_PMCG_CR_ENABLE, smmu_pmu->reg_base + SMMU_PMCG_CR);
	writel(SMMU_PMCG_IRQ_CTRL_IRQEN,
	       smmu_pmu->reg_base + SMMU_PMCG_IRQ_CTRL);
}

static inline void smmu_pmu_disable(struct pmu *pmu)
{
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(pmu);

	writel(0, smmu_pmu->reg_base + SMMU_PMCG_CR);
	writel(0, smmu_pmu->reg_base + SMMU_PMCG_IRQ_CTRL);
}

static inline void smmu_pmu_counter_set_value(struct smmu_pmu *smmu_pmu,
					      u32 idx, u64 value)
{
	if (smmu_pmu->reg_size_32)
		writel(value, smmu_pmu->reloc_base + SMMU_PMCG_EVCNTR(idx, 4));
	else
		writeq(value, smmu_pmu->reloc_base + SMMU_PMCG_EVCNTR(idx, 8));
}

static inline u64 smmu_pmu_counter_get_value(struct smmu_pmu *smmu_pmu, u32 idx)
{
	u64 value;

	if (smmu_pmu->reg_size_32)
		value = readl(smmu_pmu->reloc_base + SMMU_PMCG_EVCNTR(idx, 4));
	else
		value = readq(smmu_pmu->reloc_base + SMMU_PMCG_EVCNTR(idx, 8));

	return value;
}

static inline void smmu_pmu_counter_enable(struct smmu_pmu *smmu_pmu, u32 idx)
{
	writeq(BIT(idx), smmu_pmu->reg_base + SMMU_PMCG_CNTENSET0);
}

static inline void smmu_pmu_counter_disable(struct smmu_pmu *smmu_pmu, u32 idx)
{
	writeq(BIT(idx), smmu_pmu->reg_base + SMMU_PMCG_CNTENCLR0);
}

static inline void smmu_pmu_interrupt_enable(struct smmu_pmu *smmu_pmu, u32 idx)
{
	writeq(BIT(idx), smmu_pmu->reg_base + SMMU_PMCG_INTENSET0);
}

static inline void smmu_pmu_interrupt_disable(struct smmu_pmu *smmu_pmu,
					      u32 idx)
{
	writeq(BIT(idx), smmu_pmu->reg_base + SMMU_PMCG_INTENCLR0);
}

static inline void smmu_pmu_set_evtyper(struct smmu_pmu *smmu_pmu, u32 idx,
					u32 val)
{
	writel(val, smmu_pmu->reg_base + SMMU_PMCG_EVTYPER(idx));
}

static inline void smmu_pmu_set_smr(struct smmu_pmu *smmu_pmu, u32 idx, u32 val)
{
	writel(val, smmu_pmu->reg_base + SMMU_PMCG_SMR(idx));
}

static inline u64 smmu_pmu_getreset_ovsr(struct smmu_pmu *smmu_pmu)
{
	u64 result = readq_relaxed(smmu_pmu->reloc_base + SMMU_PMCG_OVSSET0);

	writeq(result, smmu_pmu->reloc_base + SMMU_PMCG_OVSCLR0);
	return result;
}

static inline bool smmu_pmu_has_overflowed(struct smmu_pmu *smmu_pmu, u64 ovsr)
{
	return !!(ovsr & smmu_pmu->counter_present_mask);
}

static void smmu_pmu_event_update(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(event->pmu);
	u64 delta, prev, now;
	u32 idx = hwc->idx;

	do {
		prev = local64_read(&hwc->prev_count);
		now = smmu_pmu_counter_get_value(smmu_pmu, idx);
	} while (local64_cmpxchg(&hwc->prev_count, prev, now) != prev);

	/* handle overflow. */
	delta = now - prev;
	delta &= smmu_pmu->counter_mask;

	local64_add(delta, &event->count);
}

static void smmu_pmu_set_period(struct smmu_pmu *smmu_pmu,
				struct hw_perf_event *hwc)
{
	u32 idx = hwc->idx;
	u64 new;

	/*
	 * We limit the max period to half the max counter value of the smallest
	 * counter size, so that even in the case of extreme interrupt latency
	 * the counter will (hopefully) not wrap past its initial value.
	 */
	new = SMMU_COUNTER_RELOAD;

	local64_set(&hwc->prev_count, new);
	smmu_pmu_counter_set_value(smmu_pmu, idx, new);
}

static irqreturn_t smmu_pmu_handle_irq(int irq_num, void *data)
{
	struct smmu_pmu *smmu_pmu = data;
	u64 ovsr;
	unsigned int idx;

	ovsr = smmu_pmu_getreset_ovsr(smmu_pmu);
	if (!smmu_pmu_has_overflowed(smmu_pmu, ovsr))
		return IRQ_NONE;

	for_each_set_bit(idx, (unsigned long *)&ovsr, smmu_pmu->num_counters) {
		struct perf_event *event = smmu_pmu->events[idx];
		struct hw_perf_event *hwc;

		if (WARN_ON_ONCE(!event))
			continue;

		smmu_pmu_event_update(event);
		hwc = &event->hw;

		smmu_pmu_set_period(smmu_pmu, hwc);
	}

	return IRQ_HANDLED;
}

static unsigned int smmu_pmu_get_event_idx(struct smmu_pmu *smmu_pmu)
{
	unsigned int idx;
	unsigned int num_ctrs = smmu_pmu->num_counters;

	idx = find_first_zero_bit(smmu_pmu->used_counters, num_ctrs);
	if (idx == num_ctrs)
		/* The counters are all in use. */
		return -EAGAIN;

	set_bit(idx, smmu_pmu->used_counters);

	return idx;
}

/*
 * Implementation of abstract pmu functionality required by
 * the core perf events code.
 */

static int smmu_pmu_event_init(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct perf_event *sibling;
	struct smmu_pmu *smmu_pmu;
	u32 event_id;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	smmu_pmu = to_smmu_pmu(event->pmu);

	if (hwc->sample_period) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
				    "Sampling not supported\n");
		return -EOPNOTSUPP;
	}

	if (event->cpu < 0) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
				    "Per-task mode not supported\n");
		return -EOPNOTSUPP;
	}

	/* We cannot filter accurately so we just don't allow it. */
	if (event->attr.exclude_user || event->attr.exclude_kernel ||
	    event->attr.exclude_hv || event->attr.exclude_idle) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
				    "Can't exclude execution levels\n");
		return -EOPNOTSUPP;
	}

	/* Verify specified event is supported on this PMU */
	event_id = get_event(event);
	if ((event_id >= SMMU_MAX_EVENT_ID) ||
	    (!test_bit(event_id, smmu_pmu->supported_events))) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
				    "Invalid event %d for this PMU\n",
				    event_id);
		return -EINVAL;
	}

	/* Don't allow groups with mixed PMUs, except for s/w events */
	if (event->group_leader->pmu != event->pmu &&
	    !is_software_event(event->group_leader)) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
			 "Can't create mixed PMU group\n");
		return -EINVAL;
	}

	list_for_each_entry(sibling, &event->group_leader->sibling_list,
			    group_entry)
		if (sibling->pmu != event->pmu &&
		    !is_software_event(sibling)) {
			dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
				 "Can't create mixed PMU group\n");
			return -EINVAL;
		}

	/* Ensure all events in a group are on the same cpu */
	if ((event->group_leader != event) &&
	    (event->cpu != event->group_leader->cpu)) {
		dev_dbg_ratelimited(&smmu_pmu->pdev->dev,
			 "Can't create group on CPUs %d and %d",
			 event->cpu, event->group_leader->cpu);
		return -EINVAL;
	}

	hwc->idx = -1;

	/*
	 * Ensure all events are on the same cpu so all events are in the
	 * same cpu context, to avoid races on pmu_enable etc.
	 */
	event->cpu = smmu_pmu->on_cpu;

	return 0;
}

static void smmu_pmu_event_start(struct perf_event *event, int flags)
{
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;
	u32 evtyper;
	u32 filter_sec;
	u32 filter_span;
	u32 filter_stream_id;

	hwc->state = 0;

	smmu_pmu_set_period(smmu_pmu, hwc);

	if (get_filter_enable(event)) {
		filter_sec = get_filter_sec(event);
		filter_span = get_filter_span(event);
		filter_stream_id = get_filter_stream_id(event);
	} else {
		filter_sec = SMMU_DEFAULT_FILTER_SEC;
		filter_span = SMMU_DEFAULT_FILTER_SPAN;
		filter_stream_id = SMMU_DEFAULT_FILTER_STREAM_ID;
	}

	evtyper = get_event(event) |
		  filter_span << SMMU_PMCG_EVTYPER_SID_SPAN_SHIFT |
		  filter_sec << SMMU_PMCG_EVTYPER_SEC_SID_SHIFT;

	smmu_pmu_set_evtyper(smmu_pmu, idx, evtyper);
	smmu_pmu_set_smr(smmu_pmu, idx, filter_stream_id);
	smmu_pmu_interrupt_enable(smmu_pmu, idx);
	smmu_pmu_counter_enable(smmu_pmu, idx);
}

static void smmu_pmu_event_stop(struct perf_event *event, int flags)
{
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;

	if (hwc->state & PERF_HES_STOPPED)
		return;

	smmu_pmu_interrupt_disable(smmu_pmu, idx);
	smmu_pmu_counter_disable(smmu_pmu, idx);

	if (flags & PERF_EF_UPDATE)
		smmu_pmu_event_update(event);
	hwc->state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;
}

static int smmu_pmu_event_add(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	int idx;
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(event->pmu);

	idx = smmu_pmu_get_event_idx(smmu_pmu);
	if (idx < 0)
		return idx;

	hwc->idx = idx;
	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;
	smmu_pmu->events[idx] = event;
	local64_set(&hwc->prev_count, 0);

	if (flags & PERF_EF_START)
		smmu_pmu_event_start(event, flags);

	/* Propagate changes to the userspace mapping. */
	perf_event_update_userpage(event);

	return 0;
}

static void smmu_pmu_event_del(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(event->pmu);
	int idx = hwc->idx;

	smmu_pmu_event_stop(event, flags | PERF_EF_UPDATE);
	smmu_pmu->events[idx] = NULL;
	clear_bit(idx, smmu_pmu->used_counters);

	perf_event_update_userpage(event);
}

static void smmu_pmu_event_read(struct perf_event *event)
{
	smmu_pmu_event_update(event);
}

/* cpumask */

static ssize_t smmu_pmu_cpumask_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(dev_get_drvdata(dev));

	return cpumap_print_to_pagebuf(true, buf, cpumask_of(smmu_pmu->on_cpu));
}

static struct device_attribute smmu_pmu_cpumask_attr =
		__ATTR(cpumask, 0444, smmu_pmu_cpumask_show, NULL);

static struct attribute *smmu_pmu_cpumask_attrs[] = {
	&smmu_pmu_cpumask_attr.attr,
	NULL,
};

static struct attribute_group smmu_pmu_cpumask_group = {
	.attrs = smmu_pmu_cpumask_attrs,
};

/* Events */

ssize_t smmu_pmu_event_show(struct device *dev,
			    struct device_attribute *attr, char *page)
{
	struct perf_pmu_events_attr *pmu_attr;

	pmu_attr = container_of(attr, struct perf_pmu_events_attr, attr);

	return sprintf(page, "event=0x%02llx\n", pmu_attr->id);
}

#define SMMU_EVENT_ATTR(_name, _id)					  \
	(&((struct perf_pmu_events_attr[]) {				  \
		{ .attr = __ATTR(_name, 0444, smmu_pmu_event_show, NULL), \
		  .id = _id, }						  \
	})[0].attr.attr)

static struct attribute *smmu_pmu_events[] = {
	SMMU_EVENT_ATTR(cycles, SMMU_PMU_CYCLES),
	SMMU_EVENT_ATTR(transaction, SMMU_PMU_TRANSACTION),
	SMMU_EVENT_ATTR(tlb_miss, SMMU_PMU_TLB_MISS),
	SMMU_EVENT_ATTR(config_cache_miss, SMMU_PMU_CONFIG_CACHE_MISS),
	SMMU_EVENT_ATTR(trans_table_walk, SMMU_PMU_TRANS_TABLE_WALK),
	SMMU_EVENT_ATTR(config_struct_access, SMMU_PMU_CONFIG_STRUCT_ACCESS),
	SMMU_EVENT_ATTR(pcie_ats_trans_rq, SMMU_PMU_PCIE_ATS_TRANS_RQ),
	SMMU_EVENT_ATTR(pcie_ats_trans_passed, SMMU_PMU_PCIE_ATS_TRANS_PASSED),
	NULL
};

static umode_t smmu_pmu_event_is_visible(struct kobject *kobj,
					 struct attribute *attr, int unused)
{
	struct device *dev = kobj_to_dev(kobj);
	struct smmu_pmu *smmu_pmu = to_smmu_pmu(dev_get_drvdata(dev));
	struct perf_pmu_events_attr *pmu_attr;

	pmu_attr = container_of(attr, struct perf_pmu_events_attr, attr.attr);

	if (test_bit(pmu_attr->id, smmu_pmu->supported_events))
		return attr->mode;

	return 0;
}
static struct attribute_group smmu_pmu_events_group = {
	.name = "events",
	.attrs = smmu_pmu_events,
	.is_visible = smmu_pmu_event_is_visible,
};

/* Formats */
PMU_FORMAT_ATTR(event,		   "config:0-15");
PMU_FORMAT_ATTR(filter_stream_id,  "config1:0-31");
PMU_FORMAT_ATTR(filter_span,	   "config1:32");
PMU_FORMAT_ATTR(filter_sec,	   "config1:33");
PMU_FORMAT_ATTR(filter_enable,	   "config1:34");

static struct attribute *smmu_pmu_formats[] = {
	&format_attr_event.attr,
	&format_attr_filter_stream_id.attr,
	&format_attr_filter_span.attr,
	&format_attr_filter_sec.attr,
	&format_attr_filter_enable.attr,
	NULL
};

static struct attribute_group smmu_pmu_format_group = {
	.name = "format",
	.attrs = smmu_pmu_formats,
};

static const struct attribute_group *smmu_pmu_attr_grps[] = {
	&smmu_pmu_cpumask_group,
	&smmu_pmu_events_group,
	&smmu_pmu_format_group,
	NULL,
};

/*
 * Generic device handlers
 */

static unsigned int get_num_counters(struct smmu_pmu *smmu_pmu)
{
	u32 cfgr = readl(smmu_pmu->reg_base + SMMU_PMCG_CFGR);

	return ((cfgr & SMMU_PMCG_CFGR_NCTR_MASK) >> SMMU_PMCG_CFGR_NCTR_SHIFT)
		+ 1;
}

static int smmu_pmu_offline_cpu(unsigned int cpu, struct hlist_node *node)
{
	struct smmu_pmu *smmu_pmu;
	unsigned int target;

	smmu_pmu = hlist_entry_safe(node, struct smmu_pmu, node);
	if (cpu != smmu_pmu->on_cpu)
		return 0;

	target = cpumask_any_but(cpu_online_mask, cpu);
	if (target >= nr_cpu_ids)
		return 0;

	perf_pmu_migrate_context(&smmu_pmu->pmu, cpu, target);
	smmu_pmu->on_cpu = target;
	WARN_ON(irq_set_affinity(smmu_pmu->irq, cpumask_of(target)));

	return 0;
}

static void smmu_pmu_free_msis(void *data)
{
	struct device *dev = data;

	platform_msi_domain_free_irqs(dev);
}

static void smmu_pmu_write_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	phys_addr_t doorbell;
	struct device *dev = msi_desc_to_dev(desc);
	struct smmu_pmu *smmu_pmu = dev_get_drvdata(dev);

	doorbell = (((u64)msg->address_hi) << 32) | msg->address_lo;

//	doorbell &= SMMU_PMCG_IRQ_CFG0_ADDR_MASK << MSI_CFG0_ADDR_SHIFT; // ?
	doorbell &= SMMU_PMCG_IRQ_CFG0_ADDR_MASK;

	writeq_relaxed(doorbell, smmu_pmu->reg_base + SMMU_PMCG_IRQ_CFG0);
	writel_relaxed(msg->data, smmu_pmu->reg_base + SMMU_PMCG_IRQ_CFG1);
	writel_relaxed(MSI_CFG2_MEMATTR_DEVICE_nGnRE, smmu_pmu->reg_base + SMMU_PMCG_IRQ_CFG2);
}

static void smmu_pmu_setup_msis(struct smmu_pmu *smmu_pmu)
{
	struct msi_desc *desc;
	int ret, nvec = 1;
	struct platform_device *pdev = smmu_pmu->pdev;
	struct device *dev = &pdev->dev;

	/* Clear the MSI address regs */
	writeq_relaxed(0, smmu_pmu->reg_base + SMMU_PMCG_IRQ_CFG0);

	ret = platform_msi_domain_alloc_irqs(&pdev->dev, nvec, smmu_pmu_write_msi_msg);
	if (ret) {
		dev_warn(dev, "failed to allocate MSIs\n");
		return;
	}

	for_each_msi_entry(desc, dev) {
		smmu_pmu->irq = desc->irq;
	}

	/* Add callback to free MSIs on teardown */
	devm_add_action(dev, smmu_pmu_free_msis, dev);
}

static int smmu_pmu_reset(struct smmu_pmu *smmu_pmu)
{
	unsigned int i;
	int irq, ret;
	struct platform_device *pdev = smmu_pmu->pdev;

	for (i = 0; i < smmu_pmu->num_counters; i++) {
		smmu_pmu_counter_disable(smmu_pmu, i);
		smmu_pmu_interrupt_disable(smmu_pmu, i);
	}
	smmu_pmu_disable(&smmu_pmu->pmu);

	if (smmu_pmu->irq <= 0)
		smmu_pmu_setup_msis(smmu_pmu);

	/* Request interrupt lines */
	irq = smmu_pmu->irq;
	if(irq) {
		ret = devm_request_irq(&pdev->dev, irq, smmu_pmu_handle_irq,
			       IRQF_NOBALANCING | IRQF_SHARED | IRQF_NO_THREAD,
			       "smmu-pmu", smmu_pmu);
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to request IRQ%d for SMMU PMU counters\n", irq);
			return ret;
		}
	}

	for (i = 0; i < smmu_pmu->num_counters; i++) {
		smmu_pmu_counter_enable(smmu_pmu, i);
		smmu_pmu_interrupt_enable(smmu_pmu, i);
	}
	smmu_pmu_enable(&smmu_pmu->pmu);

	return 0;
}

static int smmu_pmu_probe(struct platform_device *pdev)
{
	struct smmu_pmu *smmu_pmu;
	struct resource *mem_resource_0, *mem_resource_1;
	void __iomem *mem_map_0, *mem_map_1;
	unsigned int reg_size;
	int err;
	int irq;
	u32 ceid[SMMU_NUM_EVENTS_U32];
	u64 ceid_64;

	smmu_pmu = devm_kzalloc(&pdev->dev, sizeof(*smmu_pmu), GFP_KERNEL);
	if (!smmu_pmu)
		return -ENOMEM;

	smmu_pmu->pdev = pdev;
	platform_set_drvdata(pdev, smmu_pmu);
	smmu_pmu->pmu = (struct pmu) {
		.task_ctx_nr    = perf_invalid_context,
		.pmu_enable	= smmu_pmu_enable,
		.pmu_disable	= smmu_pmu_disable,
		.event_init	= smmu_pmu_event_init,
		.add		= smmu_pmu_event_add,
		.del		= smmu_pmu_event_del,
		.start		= smmu_pmu_event_start,
		.stop		= smmu_pmu_event_stop,
		.read		= smmu_pmu_event_read,
		.attr_groups	= smmu_pmu_attr_grps,
	};

	mem_resource_0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	mem_map_0 = devm_ioremap(&pdev->dev, mem_resource_0->start,
				 resource_size(mem_resource_0));
	if (IS_ERR(mem_map_0)) {
		dev_err(&pdev->dev, "Can't map SMMU PMU @%pa\n",
			&mem_resource_0->start);
		return PTR_ERR(mem_map_0);
	}

	smmu_pmu->reg_base = mem_map_0;
	smmu_pmu->pmu.name =
		devm_kasprintf(&pdev->dev, GFP_KERNEL, "smmu_0_%llx",
			       (mem_resource_0->start) >> SMMU_PA_SHIFT);

	if (!smmu_pmu->pmu.name) {
		dev_err(&pdev->dev, "Failed to create PMU name");
		return -EINVAL;
	}

	ceid_64 = readq(smmu_pmu->reg_base + SMMU_PMCG_CEID0);
	ceid[0] = ceid_64 & GENMASK(31, 0);
	ceid[1] = ceid_64 >> 32;
	ceid_64 = readq(smmu_pmu->reg_base + SMMU_PMCG_CEID1);
	ceid[2] = ceid_64 & GENMASK(31, 0);
	ceid[3] = ceid_64 >> 32;
	bitmap_from_u32array(smmu_pmu->supported_events, SMMU_MAX_EVENT_ID,
			     ceid, SMMU_NUM_EVENTS_U32);

	/* Determine if page 1 is present */
	if (readl(smmu_pmu->reg_base + SMMU_PMCG_CFGR) &
	    SMMU_PMCG_CFGR_RELOC_CTRS) {
		mem_resource_1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		mem_map_1 = devm_ioremap(&pdev->dev, mem_resource_1->start,
					 resource_size(mem_resource_1));

		if (IS_ERR(mem_map_1)) {
			dev_err(&pdev->dev, "Can't map SMMU PMU @%pa\n",
				&mem_resource_1->start);
			return PTR_ERR(mem_map_1);
		}
		smmu_pmu->reloc_base = mem_map_1;
	} else {
		smmu_pmu->reloc_base = smmu_pmu->reg_base;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq > 0)
		smmu_pmu->irq = irq;

	err = smmu_pmu_reset(smmu_pmu);
	if(err)
		return err;

	/* Pick one CPU to be the preferred one to use */
	smmu_pmu->on_cpu = smp_processor_id();
	WARN_ON(irq_set_affinity(smmu_pmu->irq, cpumask_of(smmu_pmu->on_cpu)));

	smmu_pmu->num_counters = get_num_counters(smmu_pmu);
	smmu_pmu->pdev = pdev;
	smmu_pmu->counter_present_mask = GENMASK(smmu_pmu->num_counters - 1, 0);
	reg_size = (readl(smmu_pmu->reg_base + SMMU_PMCG_CFGR) &
		    SMMU_PMCG_CFGR_SIZE_MASK) >> SMMU_PMCG_CFGR_SIZE_SHIFT;
	smmu_pmu->reg_size_32 = (reg_size == SMMU_PMCG_CFGR_COUNTER_SIZE_32);
	smmu_pmu->counter_mask = GENMASK_ULL(reg_size, 0);

	err = cpuhp_state_add_instance_nocalls(cpuhp_state_num,
					       &smmu_pmu->node);
	if (err) {
		dev_err(&pdev->dev, "Error %d registering hotplug", err);
		return err;
	}

	err = perf_pmu_register(&smmu_pmu->pmu, smmu_pmu->pmu.name, -1);
	if (err) {
		dev_err(&pdev->dev, "Error %d registering SMMU PMU\n", err);
		goto out_unregister;
	}

	dev_info(&pdev->dev, "Registered SMMU PMU @ %pa using %d counters\n",
		 &mem_resource_0->start, smmu_pmu->num_counters);

	return err;

out_unregister:
	cpuhp_state_remove_instance_nocalls(cpuhp_state_num, &smmu_pmu->node);
	return err;
}

static int smmu_pmu_remove(struct platform_device *pdev)
{
	struct smmu_pmu *smmu_pmu = platform_get_drvdata(pdev);

	perf_pmu_unregister(&smmu_pmu->pmu);
	cpuhp_state_remove_instance_nocalls(cpuhp_state_num, &smmu_pmu->node);

	return 0;
}

static void smmu_pmu_shutdown(struct platform_device *pdev)
{
	struct smmu_pmu *smmu_pmu = platform_get_drvdata(pdev);

	smmu_pmu_disable(&smmu_pmu->pmu);
}

static struct platform_driver smmu_pmu_driver = {
	.driver = {
		.name = "arm-smmu-pmu",
	},
	.probe = smmu_pmu_probe,
	.remove = smmu_pmu_remove,
	.shutdown = smmu_pmu_shutdown,
};

static int __init arm_smmu_pmu_init(void)
{
	cpuhp_state_num = cpuhp_setup_state_multi(CPUHP_AP_ONLINE_DYN,
				      "perf/arm/smmupmu:online",
				      NULL,
				      smmu_pmu_offline_cpu);
	if (cpuhp_state_num < 0)
		return cpuhp_state_num;

	return platform_driver_register(&smmu_pmu_driver);
}
module_init(arm_smmu_pmu_init);

static void __exit arm_smmu_pmu_exit(void)
{
	platform_driver_unregister(&smmu_pmu_driver);
}

module_exit(arm_smmu_pmu_exit);
MODULE_LICENSE("GPL v2");
