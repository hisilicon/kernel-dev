/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2015 Huawei Technologies Limited
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

#include <linux/bitmap.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/perf_event.h>
#include <linux/slab.h>
#include <asm/cputype.h>
#include <asm/irq.h>
#include <asm/irq_regs.h>
#include <asm/pmu.h>
#include "perf_event_hip05.h"
#include "djtag.h"


/* Temporary change for SoC Die information.
 * This need to be removed when read from dtb file
 * is implemented.
*/

/* Define the appropriate Board type here */
#define ONEP1
/*#define TWOP1 */
/*#define TWOP2 */

#ifdef ONEP1
#define NUM_DIE_INFO 1
hisi_die_info hisi_die_info_table[] = {
	{0x0, "TotemC", HISI_DIE_TYPE_TOTEM, 0x80010000, 0x0, 0x2,
		{
			{"LLC", HISI_LLC_MAX_EVENTS, HISI_LLC_BANK_MODULE_ID,
				{
					{ HISI_LLC_BANK0_CFGEN },
					{ HISI_LLC_BANK1_CFGEN },
					{ HISI_LLC_BANK2_CFGEN },
					{ HISI_LLC_BANK3_CFGEN }
				}
			},
			{"MN", HISI_MN_MAX_EVENTS, 0x01,
				{
					{0x0}
				}
			}
		}
	}
};

#else
#ifdef TWOP1
#define NUM_DIE_INFO 2
hisi_die_info hisi_die_info_table[] = {
	{ 0x0, "TotemA", HISI_DIE_TYPE_TOTEM, 0x40010000, 0x0, 0x3,
		{
			{ "LLC", HISI_LLC_MAX_EVENTS, HISI_LLC_BANK_MODULE_ID,
				{
					{ HISI_LLC_BANK0_CFGEN },
					{ HISI_LLC_BANK1_CFGEN },
					{ HISI_LLC_BANK2_CFGEN },
					{ HISI_LLC_BANK3_CFGEN }
				}
			},
			{ "MN", HISI_MN_MAX_EVENTS, 0x01,
				{
					{ 0x0 }
				}
			}
		}
	},
	{ 0x0, "TotemB", HISI_DIE_TYPE_TOTEM, 0x60010000, 0x0, 0x3,
		{
			{ "LLC", HISI_LLC_MAX_EVENTS, HISI_LLC_BANK_MODULE_ID,
				{
					{ HISI_LLC_BANK0_CFGEN },
					{ HISI_LLC_BANK1_CFGEN },
					{ HISI_LLC_BANK2_CFGEN },
					{ HISI_LLC_BANK3_CFGEN }
				}
			},
			{ "MN", HISI_MN_MAX_EVENTS, 0x01,
				{
					{ 0x0 }
				}
			}
		}
	}
}
#else
#ifdef TWOP2

#endif

#endif

#endif

hisi_soc_hwc_info hisi_soc_hwc_info_table = {
	NUM_DIE_INFO, &hisi_die_info_table[0], 0, 0, 0,
	{
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 }
	}
};

/* Bitmap for the supported hardware event counter index */
DECLARE_BITMAP(hisi_llc_event_used_mask, HISI_ARMV8_MAX_CFG_LLC_EVENTS);
DECLARE_BITMAP(hisi_mn_event_used_mask, HISI_ARMV8_MAX_CFG_MN_EVENTS);
DECLARE_BITMAP(hisi_ddr_event_used_mask, HISI_ARMV8_MAX_CFG_DDR_EVENTS);

/* Read hardware counter and update the Perf counter statistics */
u64 hisi_armv8_pmustub_event_update(struct perf_event *event,
				 struct hw_perf_event *hwc, int idx) {
	struct arm_pmu *armpmu = to_arm_pmu(event->pmu);
	u64 delta, prev_raw_count, new_raw_count = 0;
	struct hisi_hwc_data_info *phisi_hwc_data = hwc->perf_event_data;
	u32 num_banks = phisi_hwc_data->num_banks;
	u32 num_modules;
	u32 die_idx;
	u32 mod_idx;
	u32 cfg_en;
	u64 djtag_address;
	int i, j, k = 0;

	/* Identify Event type and read appropriate hardware counter
	 * and sum the values */
	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		num_modules = hisi_soc_hwc_info_table.num_llc;
		for (i = 0; i < num_modules; i++) {
			/* Now find the LLC module die index */
			die_idx = hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
			mod_idx = hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

			/* Find the djtag address of the Die */
			djtag_address =
			hisi_die_info_table[die_idx].djtag_ioremap_address;

			/* Find the no of LLC modules and set for each */
			num_banks =
			hisi_die_info_table[die_idx].hw_mod_info[mod_idx].num_banks;

			for (j = 0; j < num_banks; j++, k++) {
				cfg_en =
				 hisi_die_info_table[die_idx].hw_mod_info[mod_idx].bank[j].cfg_en;
again:
				prev_raw_count =
				local64_read(
					&phisi_hwc_data->hwc_prev_counters[k].prev_count);
				new_raw_count =
				 hisi_read_llc_counter(idx, djtag_address, cfg_en);

				if (local64_cmpxchg(
					&phisi_hwc_data->hwc_prev_counters[k].prev_count,
						prev_raw_count, new_raw_count) !=
									 prev_raw_count)
					goto again;

				delta = (new_raw_count - prev_raw_count) &
							 armpmu->max_period;
				/*pr_info("delta is %llu\n", delta);*/

				local64_add(delta, &event->count);
				local64_sub(delta, &hwc->period_left);
			}
		}
	}
	else if (ARMV8_HISI_IDX_MN_COUNTER0 <= idx &&
			idx <= ARMV8_HISI_IDX_MN_COUNTER_MAX) {
		/*pr_info("Counter index is for MN..\n");*/
		/*hisi_set_mn_evtype(idx, val);*/
	}

	return new_raw_count;
}

#if 0
void hisi_armv8_pmustub_enable_counting(void)
{
	u32 value;
	u32 cfg_en;
	int i;

	/* Set event_bus_en bit of LLC_AUCTRL for the first event counting */
	value = __bitmap_weight(hisi_llc_event_used_mask,
				 HISI_ARMV8_MAX_CFG_LLC_EVENTS);
	if (0 == value) {
		/* Set the event_bus_en bit in LLC AUCNTRL to enable counting
		 * for all LLC banks */
		for (i = 0; i < 4; i++) {
			switch (i) {
			case 0:
				cfg_en = HISI_LLC_BANK0_CFGEN;
				break;
			case 1:
				cfg_en = HISI_LLC_BANK1_CFGEN;
				break;
			case 2:
				cfg_en = HISI_LLC_BANK2_CFGEN;
				break;
			case 3:
				cfg_en = HISI_LLC_BANK3_CFGEN;
				break;
			}

			hisi_djtag_readreg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					HISI_DJTAG_BASE_ADDRESS, &value);

			value |= 0x1000000;
			hisi_djtag_writereg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					value,
					HISI_DJTAG_BASE_ADDRESS);
		}
	}
}
#endif

int hisi_armv8_pmustub_clear_event_idx(int idx)
{
	int ret = -1;
	void *bitmap_addr;
	int counter_idx;
	/*u32 value;*/
	/*u32 reg_offset;*/

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <=	ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		bitmap_addr = hisi_llc_event_used_mask;
		counter_idx = ARMV8_HISI_IDX_LLC_COUNTER0;
	}
	else if (ARMV8_HISI_IDX_MN_COUNTER0 <= idx &&
			 idx <=	ARMV8_HISI_IDX_MN_COUNTER_MAX) {
		bitmap_addr = hisi_mn_event_used_mask;
		counter_idx = ARMV8_HISI_IDX_MN_COUNTER0;
	} else
		return ret;

	__clear_bit(idx - counter_idx, bitmap_addr);

#if 0
	if ((event_idx - ARMV8_HISI_IDX_LLC_COUNTER0) <= 3)
		reg_offset = HISI_LLC_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_LLC_EVENT_TYPE1_REG_OFF;

	/* Clear the event in LLC_EVENT_TYPE0 Register */
	ret = hisi_djtag_readreg(HISI_LLC_BANK0_MODULE_ID,
					HISI_LLC_BANK0_CFGEN,
					reg_offset,
					HISI_DJTAG_BASE_ADDRESS,
							 &value);
	if (0 < ret)
		pr_info("djtag_read failed!!\n");

	value &= ~(0xff << (8 * (event_idx - ARMV8_HISI_IDX_LLC_COUNTER0)));

	value |= (0xff << (8 * (event_idx - ARMV8_HISI_IDX_LLC_COUNTER0)));

	ret = hisi_djtag_writereg(HISI_LLC_BANK0_MODULE_ID, /* ModuleID  */
					HISI_LLC_BANK0_CFGEN, /* Cfg_enable */
					reg_offset, /* Register Offset */
					value,
					HISI_DJTAG_BASE_ADDRESS);
	/* Clear event_bus_en bit of LLC_AUCTRL if no event counting in
	 * progress */
	value = __bitmap_weight(hisi_llc_event_used_mask,
				 HISI_ARMV8_MAX_CFG_LLC_EVENTS);
	if (0 == value) {
		ret = hisi_djtag_readreg(HISI_LLC_BANK0_MODULE_ID,
				HISI_LLC_BANK0_CFGEN, /* Cfg_enable */
				HISI_LLC_AUCTRL_REG_OFF, /* Register Offset */
				HISI_DJTAG_BASE_ADDRESS, &value);

		value &= ~(HISI_LLC_AUCTRL_EVENT_BUS_EN);
		ret = hisi_djtag_writereg(HISI_LLC_BANK0_MODULE_ID,
				HISI_LLC_BANK0_CFGEN, /* Cfg_enable */
				HISI_LLC_AUCTRL_REG_OFF, /* Register Offset */
				value,
				HISI_DJTAG_BASE_ADDRESS);
	}
#endif
	return ret;
}

int hisi_armv8_pmustub_get_event_idx(unsigned long evtype)
{
	int event_idx = -1;

	/* If event type is LLC events */
	if (evtype >= ARMV8_HISI_PERFCTR_LLC_READ_ALLOCATE &&
			evtype <= ARMV8_HISI_PERFCTR__DGRAM_1B_ECC) {
		event_idx = find_first_zero_bit(hisi_llc_event_used_mask,
				HISI_ARMV8_MAX_CFG_LLC_EVENTS);

		if (event_idx == HISI_ARMV8_MAX_CFG_LLC_EVENTS)
			return -EAGAIN;

		__set_bit(event_idx, hisi_llc_event_used_mask);
	}
	/* If event type is for MN */
	else if (evtype >= ARMV8_HISI_PERFCTR_MN_EO_BARR_REQ &&
			evtype < ARMV8_HISI_PERFCTR_EVENT_MAX) {
	}

	return event_idx;
}

void hisi_armv8_pmustub_init(void)
{
	int i, j;
	int llc_idx = 0;
	int mn_idx = 0;
	int ddr_idx = 0;

	/* Init All event used masks */
	memset(hisi_llc_event_used_mask, 0, sizeof(hisi_llc_event_used_mask));
	memset(hisi_mn_event_used_mask, 0, sizeof(hisi_mn_event_used_mask));
	memset(hisi_ddr_event_used_mask, 0, sizeof(hisi_ddr_event_used_mask));

	for (i = 0; i < hisi_soc_hwc_info_table.num_dies; i++) {
		/* ioremap all Djtag addresses */
		hisi_die_info_table[i].djtag_ioremap_address =
			(u64)ioremap_nocache(
				hisi_die_info_table[i].djtag_base_address,
					0x10000);
		if (!hisi_die_info_table[i].djtag_ioremap_address) {
			pr_info("Djtag ioremap failed!\n");
			return;
		}

		/* Find the no of LLC, MN, DDR modules and update indexes
		 * to quickly index the Module information for access */
		for (j = 0; j < hisi_die_info_table[i].num_hw_modules; j++) {
			if (!strcmp(hisi_die_info_table[i].hw_mod_info[j].name,
								"LLC")) {
				hisi_soc_hwc_info_table.llc_idxs[llc_idx].die_idx = i;
				hisi_soc_hwc_info_table.llc_idxs[llc_idx].mod_idx = j;
				hisi_soc_hwc_info_table.num_llc++;
				llc_idx++;
			}
			else if (!strcmp(hisi_die_info_table[i].hw_mod_info[j].name,
						"MN")) {
				hisi_soc_hwc_info_table.mn_idxs[mn_idx].die_idx = i;
				hisi_soc_hwc_info_table.mn_idxs[mn_idx].mod_idx = j;
				hisi_soc_hwc_info_table.num_mn++;
				mn_idx++;
			}
			else if (!strcmp(hisi_die_info_table[i].hw_mod_info[j].name,
						"DDR")) {
				hisi_soc_hwc_info_table.ddr_idxs[ddr_idx].die_idx = i;
				hisi_soc_hwc_info_table.ddr_idxs[ddr_idx].mod_idx = i;
				hisi_soc_hwc_info_table.num_ddr++;
				ddr_idx++;
			}
		}
	}

/* Enable counting during Init to avoid stop counting */
/* FIXME: comment out currently due to some complexities */
/*	hisi_armv8_pmustub_enable_counting(); */
}

int hisi_pmustub_enable_intens(int idx)
{
	return 0;
}

int hisi_pmustub_disable_intens(int idx)
{
	return 0;
}

/* Create variables to store previous count based on no of
*  banks and Dies */
int hisi_init_llc_hw_perf_event(struct hw_perf_event *hwc)
{
	u32 num_llc_modules = hisi_soc_hwc_info_table.num_llc;
	struct hisi_hwc_data_info *phisi_hwc_data_info = NULL;
	u32 llc_die_idx;
	u32 llc_mod_idx;
	u32 num_banks = 0;
	int i;

        /* Find the no of LLC modules and total no of banks */
	for (i = 0; i < num_llc_modules; i++) {
		/* Now find the LLC module die index */
		llc_die_idx = hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
		llc_mod_idx = hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

		num_banks +=
		hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].num_banks;
	}

	/* Create event counter local variables for each bank to
	 * store the previous counter value */
	phisi_hwc_data_info = (struct hisi_hwc_data_info *)kzalloc(
					sizeof(struct hisi_hwc_data_info),
								 GFP_ATOMIC);
	if (unlikely(!phisi_hwc_data_info)) {
		pr_err("Alloc failed for hisi hwc die data!.\n");
		return -ENOMEM;
	}

	phisi_hwc_data_info->num_banks = num_banks;

	phisi_hwc_data_info->hwc_prev_counters =
		(struct hisi_hwc_prev_counter *)kzalloc(num_banks *
				sizeof(struct hisi_hwc_prev_counter), GFP_ATOMIC);
	if (unlikely(!phisi_hwc_data_info)) {
		pr_err("Alloc failed for hisi hwc die bank data!.\n");
		kfree(phisi_hwc_data_info);
		return -ENOMEM;
	}

	hwc->perf_event_data = phisi_hwc_data_info;

	return 0;
}

void hisi_set_llc_evtype(int idx, u32 val)
{
	u32 num_llc_modules = hisi_soc_hwc_info_table.num_llc;
	u32 llc_die_idx;
	u32 llc_mod_idx;
	u32 reg_offset;
	u32 num_banks;
	u32 value = 0;
	u32 cfg_en;
	u64 djtag_address;
	int i, j;

	/* Select the appropriate Event select register */
	if ((idx - ARMV8_HISI_IDX_LLC_COUNTER0) <= 3)
		reg_offset = HISI_LLC_EVENT_TYPE0_REG_OFF;
	else
		reg_offset = HISI_LLC_EVENT_TYPE1_REG_OFF;

	/* Value to write to event type register */
	val = (val - ARMV8_HISI_PERFCTR_LLC_READ_ALLOCATE) <<
			(8 * (idx - ARMV8_HISI_IDX_LLC_COUNTER0));

	/* Find the no of LLC modules and set for each */
	for (i = 0; i < num_llc_modules; i++) {
		/* Now find the LLC module die index */
		llc_die_idx = hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
		llc_mod_idx = hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

		/* Find the djtag address of the Die */
		djtag_address =
		 hisi_die_info_table[llc_die_idx].djtag_ioremap_address;

		num_banks =
		 hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].num_banks;

		/* Set the event in LLC_EVENT_TYPEx Register
		 * for all LLC banks */
		for (j = 0; j < num_banks; j++) {
			cfg_en =
			 hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].bank[j].cfg_en;

			hisi_djtag_readreg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					reg_offset,
					djtag_address, &value);

			value &= ~(0xff <<
				(8 * (idx - ARMV8_HISI_IDX_LLC_COUNTER0)));
			value |= val;

			hisi_djtag_writereg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					reg_offset,
					value,
					djtag_address);
		}
	}
}

void hisi_set_mn_evtype(int idx, u32 val) {
	return;
}

void hisi_pmustub_write_evtype(int idx, u32 val) {
	val &= HISI_ARMV8_EVTYPE_EVENT;

	/* Select event based on Counter Module */
	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		hisi_set_llc_evtype(idx, val);
	}
	else if (ARMV8_HISI_IDX_MN_COUNTER0 <= idx &&
		 idx <= ARMV8_HISI_IDX_MN_COUNTER_MAX) {
		hisi_set_mn_evtype(idx, val);
	}
}

inline int armv8_hisi_counter_valid(int idx) {
	return (idx >= ARMV8_HISI_IDX_LLC_COUNTER0 &&
			idx < ARMV8_HISI_IDX_COUNTER_MAX);
}

int hisi_enable_llc_counter(int idx)
{
	u32 num_llc_modules = hisi_soc_hwc_info_table.num_llc;
	u32 llc_die_idx;
	u32 llc_mod_idx;
	u32 num_banks;
	u32 value = 0;
	u32 cfg_en;
	u64 djtag_address;
	int i, j;
	int ret = 0;

	/* Find the no of LLC modules and set for each */
	for (i = 0; i < num_llc_modules; i++) {
		/* Now find the LLC module die index */
		llc_die_idx = hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
		llc_mod_idx = hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

		/* Find the djtag address of the Die */
		djtag_address =
			hisi_die_info_table[llc_die_idx].djtag_ioremap_address;

		num_banks =
			hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].num_banks;

		/* Set the event_bus_en bit in LLC AUCNTRL to enable counting
		 * for all LLC banks */
		for (j = 0; j < num_banks; j++) {
			cfg_en =
			hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].bank[j].cfg_en;

			ret = hisi_djtag_readreg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					djtag_address, &value);

			value |= HISI_LLC_AUCTRL_EVENT_BUS_EN;
			ret = hisi_djtag_writereg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					value,
					djtag_address);
		}
	}

	return ret;
}

int hisi_pmustub_enable_counter(int idx)
{
	int ret = 0;

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX)
		ret = hisi_enable_llc_counter(idx);

/* FIXME: Comment out currently, we disable counting
 * globally by LLC_AUCTRL as just writing 0 to counter
 * to stop counting has some complexities */
	/*ret = hisi_pmustub_write_counter(idx, 0);*/

	return ret;
}

void hisi_disable_llc_counter(int idx)
{
	u32 num_llc_modules = hisi_soc_hwc_info_table.num_llc;
	u32 llc_die_idx;
	u32 llc_mod_idx;
	u32 num_banks;
	u32 value = 0;
	u32 cfg_en;
	u64 djtag_address;
	int i, j;

	/* Find the no of LLC modules and set for each */
	for (i = 0; i < num_llc_modules; i++) {
		/* Now find the LLC module die index */
		llc_die_idx = hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
		llc_mod_idx = hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

		/* Find the djtag address of the Die */
		djtag_address =
		 hisi_die_info_table[llc_die_idx].djtag_ioremap_address;

		num_banks =
		 hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].num_banks;

		/* Clear the event_bus_en bit in LLC AUCNTRL if no other event counting
		 * for all LLC banks */
		for (j = 0; j < num_banks; j++) {
			cfg_en =
			 hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].bank[j].cfg_en;

			hisi_djtag_readreg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					djtag_address, &value);

			value &= ~(HISI_LLC_AUCTRL_EVENT_BUS_EN);
			hisi_djtag_writereg(HISI_LLC_BANK_MODULE_ID,
					cfg_en,
					HISI_LLC_AUCTRL_REG_OFF,
					value,
					djtag_address);
		}
	}
}

void hisi_pmustub_disable_counter(int idx) {

	/* Read the current counter value and set it for use in event_update.
	 * LLC_AUCTRL event_bus_en will not be cleared here.
	 * It will be kept ON if atleast 1 event counting is active. */
	/*	value = hisi_pmustub_read_counter(idx);*/
	/*	local64_set(&hwc->new_count, value);*/

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
		 idx <=	ARMV8_HISI_IDX_LLC_COUNTER_MAX)
		hisi_disable_llc_counter(idx);
}

u32 hisi_read_llc_counter(int idx, u64 djtag_address, u32 cfg_en)
{
	u32 reg_offset = 0;
	u32 value;

	reg_offset = HISI_LLC_COUNTER0_REG_OFF +
		((idx - ARMV8_HISI_IDX_LLC_COUNTER0) * 4);

	hisi_djtag_readreg(HISI_LLC_BANK_MODULE_ID, /* ModuleID  */
			cfg_en, /* Cfg_enable */
			reg_offset, /* Register Offset */
			djtag_address, &value);

	return value;
}

#if 0
u32 hisi_pmustub_read_counter(int idx, int i)
{
	int ret = 0;
	u32 value = 0;

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx && idx <=
			ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		hisi_read_llc_counters(idx, &value);
	}

	return ret;
}
#endif

int hisi_pmustub_write_counter(int idx, u32 value)
{
	u32 num_banks;
	u32 num_llc_modules = hisi_soc_hwc_info_table.num_llc;
	u32 llc_die_idx;
	u32 llc_mod_idx;
	u32 cfg_en;
	u32 reg_offset = 0;
	u64 djtag_address;
	int i, j, k = 0;
	int ret;

	reg_offset = HISI_LLC_COUNTER0_REG_OFF +
		((idx - ARMV8_HISI_IDX_LLC_COUNTER0) * 4);

	if (ARMV8_HISI_IDX_LLC_COUNTER0 <= idx &&
			idx <= ARMV8_HISI_IDX_LLC_COUNTER_MAX) {
		for (i = 0; i < num_llc_modules; i++) {
			/* Now find the LLC module die index */
			llc_die_idx =
				hisi_soc_hwc_info_table.llc_idxs[i].die_idx;
			llc_mod_idx =
				hisi_soc_hwc_info_table.llc_idxs[i].mod_idx;

			/* Find the djtag address of the Die */
			djtag_address =
				hisi_die_info_table[llc_die_idx].djtag_ioremap_address;

			/* Find the no of LLC modules and set for each */
			num_banks =
				hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].num_banks;

			for (j = 0; j < num_banks; j++, k++) {
				cfg_en =
					hisi_die_info_table[llc_die_idx].hw_mod_info[llc_mod_idx].bank[j].cfg_en;
				ret =
					hisi_djtag_writereg(HISI_LLC_BANK_MODULE_ID,
							cfg_en,
							reg_offset,
							value,
							djtag_address);
			}
		}
	}
	else if (ARMV8_HISI_IDX_MN_COUNTER0 <= idx &&
			idx <= ARMV8_HISI_IDX_MN_COUNTER_MAX) {

	}

	return value;
}
