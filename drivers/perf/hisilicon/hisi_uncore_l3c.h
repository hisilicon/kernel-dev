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

#define HISI_L3C_MODULE_ID	0x04

#define HISI_L3C_BANK0_CFGEN  0x02
#define HISI_L3C_BANK1_CFGEN  0x04
#define HISI_L3C_BANK2_CFGEN  0x01
#define HISI_L3C_BANK3_CFGEN  0x08

#define HISI_L3C_AUCTRL_REG_OFF 0x04
#define HISI_L3C_AUCTRL_EVENT_BUS_EN 0x1000000

#define HISI_L3C_EVENT_TYPE0_REG_OFF 0x140
#define HISI_L3C_EVENT_TYPE1_REG_OFF 0x144

#define HISI_MAX_CFG_L3C_CNTR	0x08

#define HISI_L3C_COUNTER0_REG_OFF 0x170

#define HISI_L3C_MAX_EVENTS 22

#define NUM_L3C_BANKS 4

struct hisi_hwc_prev_counter {
	local64_t prev_count;
};

struct hisi_l3c_hwc_data_info {
	u32 num_banks;
	struct hisi_hwc_prev_counter *hwc_prev_counters;
};

struct l3c_bank_info {
	u32 cfg_en;
};

struct hisi_l3c_data {
	struct device_node *djtag_node;
	DECLARE_BITMAP(hisi_l3c_event_used_mask,
				HISI_MAX_CFG_L3C_CNTR);
	u32 num_banks;
	struct l3c_bank_info bank[MAX_BANKS];
};

int hisi_l3c_get_event_idx(struct hisi_hwmod_unit *);
void hisi_clear_l3c_event_idx(struct hisi_hwmod_unit *,	int);
void hisi_set_l3c_evtype(struct hisi_l3c_data *, int, u32);
u32 hisi_read_l3c_counter(int, struct device_node *, int);
u32 hisi_write_l3c_counter(struct hisi_l3c_data *, int, u32);
u64 hisi_l3c_event_update(struct perf_event *,
				struct hw_perf_event *, int);
void hisi_disable_l3c_counter(struct hisi_l3c_data *, int);
int hisi_enable_l3c_counter(struct hisi_l3c_data *, int);

#endif /* __HISI_UNCORE_L3C_H__ */
