/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2017 Hisilicon Limited
 * Author: Anurup M <anurup.m@huawei.com>
 *         Shaokun Zhang <zhangshaokun@hisilicon.com>
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
#ifndef __HISI_UNCORE_PMU_H__
#define __HISI_UNCORE_PMU_H__

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/local64.h>

#undef pr_fmt
#define pr_fmt(fmt)     "hisi_pmu: " fmt

#define GET_EVENTID(ev)   (ev->hw.config_base & 0xff)
#define to_hisi_pmu(p)	(container_of(p, struct hisi_pmu, pmu))
#define HISI_MAX_PERIOD(nr) (BIT_ULL(nr) - 1)

#define HISI_PMU_ATTR(_name, _func, _config)				\
	(&((struct dev_ext_attribute[]) {				\
		{ __ATTR(_name, 0444, _func, NULL), (void *)_config }   \
	})[0].attr.attr)

#define HISI_PMU_FORMAT_ATTR(_name, _config)		\
	HISI_PMU_ATTR(_name, hisi_format_sysfs_show, (void *)_config)
#define HISI_PMU_EVENT_ATTR(_name, _config)		\
	HISI_PMU_ATTR(_name, hisi_event_sysfs_show, (unsigned long)_config)

struct hisi_pmu;

struct hisi_uncore_ops {
	void (*write_evtype)(struct hisi_pmu *, int, u32);
	int (*get_event_idx)(struct perf_event *);
	u64 (*read_counter)(struct hisi_pmu *, struct hw_perf_event *);
	void (*write_counter)(struct hisi_pmu *, struct hw_perf_event *, u64);
	void (*enable_counter)(struct hisi_pmu *, struct hw_perf_event *);
	void (*disable_counter)(struct hisi_pmu *, struct hw_perf_event *);
	void (*enable_counter_int)(struct hisi_pmu *, struct hw_perf_event *);
	void (*disable_counter_int)(struct hisi_pmu *, struct hw_perf_event *);
	void (*start_counters)(struct hisi_pmu *);
	void (*stop_counters)(struct hisi_pmu *);
};

struct hisi_pmu_hwevents {
	struct perf_event **hw_events;
	unsigned long *used_mask;
};

/* Generic pmu struct for different pmu types */
struct hisi_pmu {
	const char *name;
	struct pmu pmu;
	const struct hisi_uncore_ops *ops;
	struct hisi_pmu_hwevents pmu_events;
	cpumask_t cpus;
	struct device *dev;
	struct hlist_node node;
	u32 scl_id;
	u32 ccl_id;
	/* Hardware information for different pmu types */
	void __iomem *base;
	union {
		u32 ddrc_chn_id;
		u32 l3c_tag_id;
		u32 hha_uid;
	};
	int num_counters;
	int num_events;
	int counter_bits;
};

int hisi_uncore_pmu_counter_valid(struct hisi_pmu *hisi_pmu, int idx);
int hisi_uncore_pmu_get_event_idx(struct perf_event *event);
void hisi_uncore_pmu_clear_event_idx(struct hisi_pmu *hisi_pmu, int idx);
void hisi_uncore_pmu_read(struct perf_event *event);
int hisi_uncore_pmu_add(struct perf_event *event, int flags);
void hisi_uncore_pmu_del(struct perf_event *event, int flags);
void hisi_uncore_pmu_start(struct perf_event *event, int flags);
void hisi_uncore_pmu_stop(struct perf_event *event, int flags);
void hisi_uncore_pmu_set_event_period(struct perf_event *event);
u64 hisi_uncore_pmu_event_update(struct perf_event *event);
int hisi_uncore_pmu_event_init(struct perf_event *event);
int hisi_uncore_pmu_setup(struct hisi_pmu *hisi_pmu, const char *pmu_name);
void hisi_uncore_pmu_enable(struct pmu *pmu);
void hisi_uncore_pmu_disable(struct pmu *pmu);
struct hisi_pmu *hisi_pmu_alloc(struct device *dev, u32 num_cntrs);
ssize_t hisi_event_sysfs_show(struct device *dev,
			      struct device_attribute *attr, char *buf);
ssize_t hisi_format_sysfs_show(struct device *dev,
			       struct device_attribute *attr, char *buf);
ssize_t hisi_cpumask_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *buf);
void hisi_read_scl_and_ccl_id(u32 *scl_id, u32 *ccl_id);
#endif /* __HISI_UNCORE_PMU_H__ */
