/*
 * HiSilicon SoC MN Hardware event counters support
 *
 * Copyright (C) 2016 Huawei Technologies Limited
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
#ifndef __HISI_UNCORE_MN_H__
#define __HISI_UNCORE_MN_H__

#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon MN RAW event types.
 */
enum armv8_hisi_mn_event_types {
	HISI_HWEVENT_MN_EO_BARR_REQ	= 0x316,
	HISI_HWEVENT_MN_EC_BARR_REQ	= 0x317,
	HISI_HWEVENT_MN_DVM_OP_REQ	= 0x318,
	HISI_HWEVENT_MN_DVM_SYNC_REQ	= 0x319,
	HISI_HWEVENT_MN_READ_REQ	= 0x31A,
	HISI_HWEVENT_MN_WRITE_REQ	= 0x31B,
	HISI_HWEVENT_MN_COPYBK_REQ	= 0x31C,
	HISI_HWEVENT_MN_OTHER_REQ	= 0x31D,
	HISI_HWEVENT_MN_EVENT_MAX	= 0x31E,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_mn_counters {
	HISI_IDX_MN_COUNTER0	= 0x0,
	HISI_IDX_MN_COUNTER_MAX	= 0x4,
};

#define HISI_MAX_CFG_MN_CNTR	0x04

struct hisi_mn_hwcfg {
	u32 evtype_reg0_off;
	u32 counter_reg0_off;
	u32 event_ctrl_reg_off;
	u32 event_enable;
	u32 module_id;
	u32 bank_cfgen;
};

struct hisi_mn_data {
	struct device_node *djtag_node;
	DECLARE_BITMAP(hisi_mn_event_used_mask,
				HISI_MAX_CFG_MN_CNTR);
	struct hisi_mn_hwcfg mn_hwcfg;
};

int hisi_mn_get_event_idx(struct hisi_pmu *);
void hisi_clear_mn_event_idx(struct hisi_pmu *, int);
void hisi_set_mn_evtype(struct hisi_pmu *, int, u32);
u32 hisi_write_mn_counter(struct hisi_pmu *, int, u32);
u64 hisi_mn_event_update(struct perf_event *,
				struct hw_perf_event *, int);
void hisi_disable_mn_counter(struct hisi_pmu *, int);
void hisi_enable_mn_counter(struct hisi_pmu *, int);

#endif /* __HISI_UNCORE_MN_H__ */
