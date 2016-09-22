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
#include <linux/soc/hisilicon/djtag.h>
#include <linux/types.h>
#include <asm/local64.h>

#undef pr_fmt
#define pr_fmt(fmt)     "hisi_pmu: " fmt

#define HISI_DJTAG_MOD_MASK (0xFFFF)
#define HISI_CNTR_SCCL_MASK    (0xF00)

#define HISI_EVTYPE_EVENT	0xfff
#define HISI_MAX_PERIOD ((1LLU << 32) - 1)

#define MAX_BANKS 8
#define MAX_COUNTERS 30
#define MAX_UNITS 8

#define GET_CNTR_IDX(hwc) (hwc->idx)
#define to_hisi_pmu(c)	(container_of(c, struct hisi_pmu, pmu))

#define GET_UNIT_IDX(event_code)		\
	(((event_code & HISI_SCCL_MASK) >>	\
			   HISI_SCCL_SHIFT) - 1)

#define HISI_PMU_FORMAT_ATTR(_name, _config)		\
	(&((struct dev_ext_attribute[]) {		\
		{ .attr = __ATTR(_name, S_IRUGO,	\
			hisi_format_sysfs_show, NULL),	\
		  .var = (void *) _config,		\
		}					\
	})[0].attr.attr)

#define HISI_PMU_EVENT_ATTR_STR(_name, _str)		\
	(&((struct perf_pmu_events_attr[]) {		\
		{ .attr = __ATTR(_name, S_IRUGO,	\
			 hisi_event_sysfs_show, NULL),	\
		  .event_str = _str,			\
		}					\
	  })[0].attr.attr)

struct hisi_pmu;

struct hisi_uncore_ops {
	void (*set_evtype)(struct hisi_pmu *, int, u32);
	int (*get_event_idx)(struct hisi_pmu *);
	void (*clear_event_idx)(struct hisi_pmu *, int);
	u64 (*event_update)(struct perf_event *,
			     struct hw_perf_event *, int);
	u32 (*read_counter)(struct hisi_pmu *, int, int);
	u32 (*write_counter)(struct hisi_pmu *, int, u32);
	void (*enable_counter)(struct hisi_pmu *, int);
	void (*disable_counter)(struct hisi_pmu *, int);
};

struct hisi_pmu_hw_events {
	struct perf_event **events;
	raw_spinlock_t pmu_lock;
};

/* Generic pmu struct for different pmu types */
struct hisi_pmu {
	const char *name;
	struct hisi_pmu_hw_events hw_events;
	struct hisi_uncore_ops *ops;
	struct platform_device *plat_device;
	void *hwmod_data; /* Hardware module specific data */
	struct pmu pmu;
	u32 scl_id;
	int num_counters;
	int num_events;
	int num_units;
};

int hisi_pmu_write_counter(struct hisi_pmu *, int, u32);
void hisi_uncore_pmu_read(struct perf_event *);
void hisi_uncore_pmu_del(struct perf_event *, int);
int hisi_uncore_pmu_add(struct perf_event *, int);
void hisi_uncore_pmu_start(struct perf_event *, int);
void hisi_uncore_pmu_stop(struct perf_event *, int);
void hisi_pmu_event_set_period(struct perf_event *);
void hisi_uncore_pmu_enable_event(struct perf_event *);
void hisi_uncore_pmu_disable_event(struct perf_event *);
void hisi_uncore_pmu_enable(struct pmu *);
void hisi_uncore_pmu_disable(struct pmu *);
struct hisi_pmu *hisi_uncore_pmu_alloc(struct device *);
int hisi_uncore_pmu_setup(struct hisi_pmu *hisi_pmu, const char *);
int hisi_uncore_pmu_event_init(struct perf_event *);
int hisi_djtag_readreg(int, int, u32, struct hisi_djtag_client *, u32 *);
int hisi_djtag_writereg(int, int, u32, u32, struct hisi_djtag_client *);
struct hisi_pmu *hisi_pmu_alloc(struct device *);
int hisi_uncore_common_fwprop_read(struct device *, struct hisi_pmu *);
ssize_t hisi_event_sysfs_show(struct device *,
				  struct device_attribute *, char *);
ssize_t hisi_format_sysfs_show(struct device *,
				  struct device_attribute *, char *);
#endif /* __HISI_UNCORE_PMU_H__ */
