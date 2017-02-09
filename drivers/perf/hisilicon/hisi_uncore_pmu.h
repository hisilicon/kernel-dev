/*
 * HiSilicon SoC Hardware event counters support
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
#ifndef __HISI_UNCORE_PMU_H__
#define __HISI_UNCORE_PMU_H__

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/local64.h>
#include "djtag.h"

#undef pr_fmt
#define pr_fmt(fmt)     "hisi_pmu: " fmt

#define HISI_EVTYPE_EVENT	0xfff
#define HISI_MAX_PERIOD ((1LLU << 32) - 1)

#define MAX_BANKS 8
#define MAX_COUNTERS 30
#define MAX_UNITS 8

#define GET_CNTR_IDX(hwc) (hwc->idx)
#define to_hisi_pmu(c)	(container_of(c, struct hisi_pmu, pmu))

#define HISI_PMU_FORMAT_ATTR(_name, _config)		\
	(&((struct dev_ext_attribute[]) {		\
		{ .attr = __ATTR(_name, 0444,		\
				 hisi_format_sysfs_show,\
				 NULL),			\
		  .var = (void *) _config,		\
		}					\
	})[0].attr.attr)

#define HISI_PMU_EVENT_ATTR_STR(_name, _str)		\
	(&((struct perf_pmu_events_attr[]) {		\
		{ .attr = __ATTR(_name, 0444,		\
				 hisi_event_sysfs_show,	\
				 NULL),			\
		  .event_str = _str,			\
		}					\
	  })[0].attr.attr)

struct hisi_pmu;

struct hisi_uncore_ops {
	void (*set_evtype)(struct hisi_pmu *, int, u32);
	void (*clear_evtype)(struct hisi_pmu *, int);
	void (*set_event_period)(struct perf_event *);
	int (*get_event_idx)(struct hisi_pmu *);
	void (*clear_event_idx)(struct hisi_pmu *, int);
	u64 (*event_update)(struct perf_event *,
			     struct hw_perf_event *, int);
	u32 (*read_counter)(struct hisi_pmu *, int, int);
	u32 (*write_counter)(struct hisi_pmu *,
				struct hw_perf_event *, u32);
	void (*enable_counter)(struct hisi_pmu *, int);
	void (*disable_counter)(struct hisi_pmu *, int);
	void (*start_counters)(struct hisi_pmu *);
	void (*stop_counters)(struct hisi_pmu *);
	void (*start_hrtimer)(struct hisi_pmu *);
	void (*stop_hrtimer)(struct hisi_pmu *);
};

/* Generic pmu struct for different pmu types */
struct hisi_pmu {
	const char *name;
	struct perf_event **hw_perf_events;
	struct list_head active_list; /* Active events list */
	struct hisi_uncore_ops *ops;
	struct hrtimer hrtimer; /* hrtimer to handle the
				 * counter overflow
				 */
	u64 hrt_duration; /* hrtimer timeout */
	struct device *dev;
	void *hwmod_data; /* Hardware module specific data */
	cpumask_t cpu;
	raw_spinlock_t lock;
	struct pmu pmu;
	u32 scl_id;
	int num_counters;
	int num_events;
	int num_active;
};

void hisi_uncore_pmu_read(struct perf_event *event);
int hisi_uncore_pmu_add(struct perf_event *event, int flags);
void hisi_uncore_pmu_del(struct perf_event *event, int flags);
void hisi_uncore_pmu_start(struct perf_event *event, int flags);
void hisi_uncore_pmu_stop(struct perf_event *event, int flags);
void hisi_pmu_set_event_period(struct perf_event *event);
int hisi_uncore_pmu_event_init(struct perf_event *event);
int hisi_uncore_pmu_setup(struct hisi_pmu *hisi_pmu, const char *pmu_name);
void hisi_uncore_pmu_enable(struct pmu *pmu);
void hisi_uncore_pmu_disable(struct pmu *pmu);
struct hisi_pmu *hisi_pmu_alloc(struct device *dev);
ssize_t hisi_event_sysfs_show(struct device *dev,
			      struct device_attribute *attr, char *buf);
ssize_t hisi_format_sysfs_show(struct device *dev,
			       struct device_attribute *attr, char *buf);
ssize_t hisi_cpumask_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *buf);
ssize_t hisi_hrtimer_interval_sysfs_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf);
ssize_t hisi_hrtimer_interval_sysfs_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count);
void hisi_hrtimer_init(struct hisi_pmu *hisi_pmu, u64 timer_interval);
void hisi_hrtimer_start(struct hisi_pmu *hisi_pmu);
void hisi_hrtimer_stop(struct hisi_pmu *hisi_pmu);
int hisi_djtag_readreg(int module_id, int bank, u32 offset,
		       struct hisi_djtag_client *client, u32 *value);
int hisi_djtag_writereg(int module_id, int bank, u32 offset,
			u32 value, struct hisi_djtag_client *client);
#endif /* __HISI_UNCORE_PMU_H__ */
