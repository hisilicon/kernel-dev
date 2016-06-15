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
#ifndef __HISI_UNCORE_LLC_H__
#define __HISI_UNCORE_LLC_H__

#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon LLC RAW event types.
 */
enum armv8_hisi_llc_event_types {
	HISI_HWEVENT_LLC_READ_ALLOCATE				= 0x300,
	HISI_HWEVENT_LLC_WRITE_ALLOCATE				= 0x301,
	HISI_HWEVENT_LLC_READ_NOALLOCATE			= 0x302,
	HISI_HWEVENT_LLC_WRITE_NOALLOCATE			= 0x303,
	HISI_HWEVENT_LLC_READ_HIT				= 0x304,
	HISI_HWEVENT_LLC_WRITE_HIT				= 0x305,
	HISI_HWEVENT_LLC_CMO_REQUEST				= 0x306,
	HISI_HWEVENT_LLC_COPYBACK_REQ				= 0x307,
	HISI_HWEVENT_LLC_HCCS_SNOOP_REQ				= 0x308,
	HISI_HWEVENT_LLC_SMMU_REQ				= 0x309,
	HISI_HWEVENT_LLC_EXCL_SUCCESS				= 0x30A,
	HISI_HWEVENT_LLC_EXCL_FAIL				= 0x30B,
	HISI_HWEVENT_LLC_CACHELINE_OFLOW			= 0x30C,
	HISI_HWEVENT_LLC_RECV_ERR				= 0x30D,
	HISI_HWEVENT_LLC_RECV_PREFETCH				= 0x30E,
	HISI_HWEVENT_LLC_RETRY_REQ				= 0x30F,
	HISI_HWEVENT_LLC_DGRAM_2B_ECC				= 0x310,
	HISI_HWEVENT_LLC_TGRAM_2B_ECC				= 0x311,
	HISI_HWEVENT_LLC_SPECULATE_SNOOP			= 0x312,
	HISI_HWEVENT_LLC_SPECULATE_SNOOP_SUCCESS		= 0x313,
	HISI_HWEVENT_LLC_TGRAM_1B_ECC				= 0x314,
	HISI_HWEVENT_LLC_DGRAM_1B_ECC				= 0x315,
	HISI_HWEVENT_EVENT_MAX,
};

/*
 * ARMv8 HiSilicon Hardware counter Index.
 */
enum armv8_hisi_llc_counters {
	ARMV8_HISI_IDX_LLC_COUNTER0			= 0x0,
	ARMV8_HISI_IDX_LLC_COUNTER_MAX			= 0x7,
};

#define HISI_ARMV8_MAX_CFG_LLC_CNTR	0x08

#define HISI_LLC_MODULE_ID	0x04

#define HISI_LLC_BANK0_CFGEN  0x02
#define HISI_LLC_BANK1_CFGEN  0x04
#define HISI_LLC_BANK2_CFGEN  0x01
#define HISI_LLC_BANK3_CFGEN  0x08

#define HISI_LLC_AUCTRL_REG_OFF 0x04
#define HISI_LLC_AUCTRL_EVENT_BUS_EN 0x1000000

#define HISI_LLC_EVENT_TYPE0_REG_OFF 0x140
#define HISI_LLC_EVENT_TYPE1_REG_OFF 0x144

#define HISI_LLC_COUNTER0_REG_OFF 0x170
#define HISI_LLC_COUNTER1_REG_OFF 0x174
#define HISI_LLC_COUNTER2_REG_OFF 0x178
#define HISI_LLC_COUNTER3_REG_OFF 0x17C
#define HISI_LLC_COUNTER4_REG_OFF 0x180
#define HISI_LLC_COUNTER5_REG_OFF 0x184
#define HISI_LLC_COUNTER6_REG_OFF 0x188
#define HISI_LLC_COUNTER7_REG_OFF 0x18C

#define HISI_LLC_BANK_INTM 0x100
#define HISI_LLC_BANK_RINT 0x104
#define HISI_LLC_BANK_INTS 0x108
#define HISI_LLC_BANK_INTC 0x10C

#define HISI_LLC_MAX_EVENTS 22

#define NUM_LLC_BANKS 4

struct hisi_hwc_prev_counter {
	local64_t prev_count;
};

struct hisi_llc_hwc_data_info {
	u32 num_banks;
	struct hisi_hwc_prev_counter *hwc_prev_counters;
};

typedef struct bank_info_t {
/*	struct list_head entry;  To implement as list later */
	u32 cfg_en;
	int irq;
	/* Overflow counter for each cnt idx */
	atomic_t cntr_ovflw[HISI_ARMV8_MAX_CFG_LLC_CNTR];
} llc_bank_info;

typedef struct hisi_llc_data_t {
	struct device_node *djtag_node;
	DECLARE_BITMAP(hisi_llc_event_used_mask,
				HISI_ARMV8_MAX_CFG_LLC_CNTR);
	u32 num_banks;
	llc_bank_info bank[MAX_BANKS];
	/*	   struct list_head bank_list; To implement as list later */
} hisi_llc_data;

extern struct hisi_pmu *hisi_uncore_llc;

int hisi_llc_get_event_idx(struct hisi_hwmod_unit *);
void hisi_clear_llc_event_idx(struct hisi_hwmod_unit *,
							int);
void hisi_set_llc_evtype(hisi_llc_data *, int, u32);
u32 hisi_read_llc_counter(int, struct device_node *, int);
u32 hisi_write_llc_counter(hisi_llc_data *, int, u32);
int hisi_init_llc_hw_perf_event(struct hisi_pmu *, struct hw_perf_event *);
irqreturn_t hisi_llc_event_handle_irq(int, void *);
u64 hisi_llc_event_update(struct perf_event *,
					struct hw_perf_event *, int);
void hisi_disable_llc_counter(hisi_llc_data *, int);
int hisi_enable_llc_counter(hisi_llc_data *, int);

#endif /* __HISI_UNCORE_LLC_H__ */
