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
#ifndef __HISI_UNCORE_L3C_H__
#define __HISI_UNCORE_L3C_H__

#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon L3C RAW event types.
 */
enum armv8_hisi_l3c_event_types {
	HISI_HWEVENT_L3C_READ_ALLOCATE		= 0x300,
	HISI_HWEVENT_L3C_WRITE_ALLOCATE		= 0x301,
	HISI_HWEVENT_L3C_READ_NOALLOCATE	= 0x302,
	HISI_HWEVENT_L3C_WRITE_NOALLOCATE	= 0x303,
	HISI_HWEVENT_L3C_READ_HIT		= 0x304,
	HISI_HWEVENT_L3C_WRITE_HIT		= 0x305,
	HISI_HWEVENT_L3C_EVENT_MAX		= 0x315,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_l3c_counters {
	HISI_IDX_L3C_COUNTER0		= 0x0,
	HISI_IDX_L3C_COUNTER_MAX	= 0x7,
};

#define HISI_MAX_CFG_L3C_CNTR	0x08

struct hisi_l3c_hwcfg {
	u32 auctrl_reg_off;
	u32 evtype_reg0_off;
	u32 counter_reg0_off;
	u32 auctrl_event_enable;
	u32 module_id;
	u32 num_banks;
	u32 bank_cfgen[MAX_BANKS];
};

struct hisi_l3c_data {
	struct device_node *djtag_node;
	DECLARE_BITMAP(hisi_l3c_event_used_mask,
				HISI_MAX_CFG_L3C_CNTR);
	struct hisi_l3c_hwcfg l3c_hwcfg;
};

int hisi_l3c_get_event_idx(struct hisi_pmu *);
void hisi_clear_l3c_event_idx(struct hisi_pmu *, int);
void hisi_set_l3c_evtype(struct hisi_pmu *, int, u32);
u32 hisi_write_l3c_counter(struct hisi_pmu *, int, u32);
u64 hisi_l3c_event_update(struct perf_event *,
				struct hw_perf_event *, int);
void hisi_disable_l3c_counter(struct hisi_pmu *, int);
void hisi_enable_l3c_counter(struct hisi_pmu *, int);

#endif /* __HISI_UNCORE_L3C_H__ */
