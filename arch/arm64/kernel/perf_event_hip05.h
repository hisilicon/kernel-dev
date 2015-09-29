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
u64 hisi_armv8_pmustub_event_update(struct perf_event *,
				struct hw_perf_event *, int);
int hisi_pmustub_enable_counter(int);
void hisi_pmustub_disable_counter(int);
int armv8_hisi_counter_valid(int);
int hisi_pmustub_write_counter(int, u32);
u32 hisi_pmustub_read_counter(int, int);
void hisi_pmustub_write_evtype(int, u32);
int hisi_pmustub_enable_intens(int);
int hisi_pmustub_disable_intens(int);
int hisi_armv8_pmustub_get_event_idx(unsigned long);
int hisi_armv8_pmustub_clear_event_idx(int);
void hisi_armv8_pmustub_enable_counting(void);
void hisi_armv8_pmustub_init(void);
void hisi_set_llc_evtype(int, u32);
u32 hisi_read_llc_counter(int, u64, u32);
int hisi_init_llc_hw_perf_event(struct hw_perf_event *);

/*
 * ARMv8 HiSilicon LLC RAW event types.
 */
enum armv8_hisi_llc_event_types {
	ARMV8_HISI_PERFCTR_LLC_READ_ALLOCATE			= 0x300,
	ARMV8_HISI_PERFCTR_LLC_WRITE_ALLOCATE			= 0x301,
	ARMV8_HISI_PERFCTR_LLC_READ_NOALLOCATE			= 0x302,
	ARMV8_HISI_PERFCTR_LLC_WRITE_NOALLOCATE			= 0x303,
	ARMV8_HISI_PERFCTR_LLC_READ_HIT				= 0x304,
	ARMV8_HISI_PERFCTR_LLC_WRITE_HIT			= 0x305,
	ARMV8_HISI_PERFCTR_CMO_REQUEST				= 0x306,
	ARMV8_HISI_PERFCTR_COPYBACK_REQ				= 0x307,
	ARMV8_HISI_PERFCTR_HCCS_SNOOP_REQ			= 0x308,
	ARMV8_HISI_PERFCTR_SMMU_REQ				= 0x309,
	ARMV8_HISI_PERFCTR_EXCL_SUCCESS				= 0x30A,
	ARMV8_HISI_PERFCTR_EXCL_FAIL				= 0x30B,
	ARMV8_HISI_PERFCTR_CACHELINE_OFLOW			= 0x30C,
	ARMV8_HISI_PERFCTR_RECV_ERR				= 0x30D,
	ARMV8_HISI_PERFCTR_RECV_PREFETCH			= 0x30E,
	ARMV8_HISI_PERFCTR_RETRY_REQ				= 0x30F,
	ARMV8_HISI_PERFCTR_DGRAM_2B_ECC				= 0x310,
	ARMV8_HISI_PERFCTR_TGRAM_2B_ECC				= 0x311,
	ARMV8_HISI_PERFCTR_SPECULATE_SNOOP			= 0x312,
	ARMV8_HISI_PERFCTR_SPECULATE_SNOOP_SUCCESS		= 0x313,
	ARMV8_HISI_PERFCTR_TGRAM_1B_ECC				= 0x314,
	ARMV8_HISI_PERFCTR__DGRAM_1B_ECC			= 0x315,
};

/*
 * ARMv8 HiSilicon MN RAW event types.
 */
enum armv8_hisi_mn_event_types {
	ARMV8_HISI_PERFCTR_MN_EO_BARR_REQ			= 0x316,
	ARMV8_HISI_PERFCTR_MN_EC_BARR_REQ			= 0x317,
	ARMV8_HISI_PERFCTR_MN_DV_MOP_REQ			= 0x318,
	ARMV8_HISI_PERFCTR_MN_READ_REQ				= 0x319,
	ARMV8_HISI_PERFCTR_MN_WRITE_REQ				= 0x31A,
	ARMV8_HISI_PERFCTR_MN_COPYBK_REQ			= 0x31B,
	ARMV8_HISI_PERFCTR_MN_OTHER_REQ				= 0x31C,
	ARMV8_HISI_PERFCTR_MN_RETRY_REQ				= 0x31D,
	ARMV8_HISI_PERFCTR_EVENT_MAX,
};

enum armv8_hisi_llc_counters {
	ARMV8_HISI_IDX_LLC_COUNTER0 = 0x30,
	ARMV8_HISI_IDX_LLC_COUNTER1,
	ARMV8_HISI_IDX_LLC_COUNTER2,
	ARMV8_HISI_IDX_LLC_COUNTER3,
	ARMV8_HISI_IDX_LLC_COUNTER4,
	ARMV8_HISI_IDX_LLC_COUNTER5,
	ARMV8_HISI_IDX_LLC_COUNTER6,
	ARMV8_HISI_IDX_LLC_COUNTER_MAX,
	ARMV8_HISI_IDX_MN_COUNTER0,
	ARMV8_HISI_IDX_MN_COUNTER1,
	ARMV8_HISI_IDX_MN_COUNTER2,
	ARMV8_HISI_IDX_MN_COUNTER_MAX,
	ARMV8_HISI_IDX_COUNTER_MAX,
};

enum hisi_die_type {
	HISI_DIE_TYPE_TOTEM,
	HISI_DIE_TYPE_NIMBUS,
};

#define HISI_LLC_BANK_MODULE_ID 0x04

#define HISI_LLC_BANK0_CFGEN  0x02
#define HISI_LLC_BANK1_CFGEN  0x04
#define HISI_LLC_BANK2_CFGEN  0x01
#define HISI_LLC_BANK3_CFGEN  0x08

#define HISI_DJTAG_BASE_ADDRESS 0x80010000
#define HISI_DJTAG_TOTEMA_BASE_ADDRESS 0x40010000
#define HISI_DJTAG_TOTEMB_BASE_ADDRESS 0x60010000

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

#define HISI_LLC_MAX_EVENTS 22

#define HISI_MN_MAX_EVENTS 9

#define	HISI_ARMV8_EVTYPE_EVENT	0x3ff
#define HISI_ARMV8_MAX_CFG_LLC_EVENTS 0x08
#define HISI_ARMV8_MAX_CFG_MN_EVENTS 0x04
#define HISI_ARMV8_MAX_CFG_DDR_EVENTS 0x04
#define HISI_ARMV8_MAX_CFG_EVENTS_MASK 0xff

#define MAX_BANKS 8
#define MAX_HW_MODULES 6
#define MAX_DIE 6

typedef struct bank_info_t {
	u32 cfg_en;
} bank_info;

struct hisi_hwc_prev_counter {
	local64_t prev_count;
};

struct hisi_hwc_data_info {
	u32 num_banks;
	struct hisi_hwc_prev_counter *hwc_prev_counters;
};

typedef struct hisi_hardware_module_info_t {
	char *name;
	u32 num_events;
	u32 num_banks;
	bank_info bank[MAX_BANKS];
} hisi_hardware_module_info;

typedef struct hisi_die_info_t {
	u32 dieid;
	char *die_name;
	u32 die_type;
	u64 djtag_base_address;
	u64 djtag_ioremap_address;
	u32 num_hw_modules;
	hisi_hardware_module_info hw_mod_info[MAX_HW_MODULES];
} hisi_die_info;

typedef struct hw_mod_index_t {
	u32 die_idx;
	u32 mod_idx;
} hw_mod_index;

typedef struct hisi_soc_hwc_info_t {
	u32 num_dies;
	hisi_die_info *phisi_die_info;
	u32 num_llc;
	u32 num_mn;
	u32 num_ddr;
	hw_mod_index llc_idxs[MAX_DIE];
	hw_mod_index mn_idxs[MAX_DIE];
	hw_mod_index ddr_idxs[MAX_DIE];
} hisi_soc_hwc_info;

extern hisi_soc_hwc_info hisi_soc_hwc_info_table;
extern hisi_die_info hisi_die_info_table[];
