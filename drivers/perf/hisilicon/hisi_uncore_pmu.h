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

#define HISI_SCCL_MAX	(1 << 4)
#define HISI_SCCL_MASK	(0xF00000)
#define HISI_SCCL_SHIFT 20

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

enum hisi_hwmod_type {
	HISI_L3C = 0x0,
};

/* Event granularity */
enum hisi_pmu_type {
	CORE_SPECIFIC,
	CCL_SPECIFIC, /* For future use */
	SCCL_SPECIFIC,
	CHIP_SPECIFIC, /* For future use */
};

struct hisi_pmu_hw_events {
	struct perf_event **events;
	raw_spinlock_t pmu_lock;
};

/* Hardware module information */
struct hisi_hwmod_unit {
	   int unit_id;
	   struct hisi_pmu_hw_events hw_events;
	   void *hwmod_data;
};

/* Generic pmu struct for different pmu types */
struct hisi_pmu {
	const char *name;
	enum hisi_pmu_type pmu_type;
	enum hisi_hwmod_type hwmod_type;
	int num_counters;
	int	num_events;
	struct perf_event *events[MAX_COUNTERS];
	int num_units;
	struct hisi_hwmod_unit hwmod_pmu_unit[MAX_UNITS];
	struct pmu pmu; /* for custom pmu ops */
	struct platform_device *plat_device;
};

u64 hisi_pmu_event_update(struct perf_event *,
				struct hw_perf_event *, int);
int hisi_pmu_enable_counter(struct hisi_hwmod_unit *, int);
void hisi_pmu_disable_counter(struct hisi_hwmod_unit *, int);
int hisi_pmu_write_counter(struct hisi_hwmod_unit *, int, u32);
void hisi_pmu_write_evtype(int, u32);
int hisi_pmu_get_event_idx(struct hw_perf_event *,
				struct hisi_hwmod_unit *);
void hisi_pmu_clear_event_idx(struct hw_perf_event *,
				struct hisi_hwmod_unit *, int);
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
struct hisi_pmu *hisi_uncore_pmu_alloc(struct platform_device *);
int hisi_uncore_pmu_setup(struct hisi_pmu *hisi_pmu,
				struct platform_device *, char *);
void hisi_uncore_pmu_write_evtype(struct hisi_hwmod_unit *, int, u32);
int hisi_uncore_pmu_event_init(struct perf_event *);
int hisi_djtag_readreg(int, int, u32, struct device_node *, u32 *);
int hisi_djtag_writereg(int, int, u32, u32, struct device_node *);
int hisi_pmu_unit_init(struct platform_device *,
				struct hisi_hwmod_unit *,
						int, int);
struct hisi_pmu *hisi_pmu_alloc(struct platform_device *);
#endif /* __HISI_UNCORE_PMU_H__ */
