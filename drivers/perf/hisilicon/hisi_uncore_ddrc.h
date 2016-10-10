/*
 * HiSilicon SoC DDRC Hardware event counters support
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
#ifndef __HISI_UNCORE_DDRC_H__
#define __HISI_UNCORE_DDRC_H__

#include "hisi_uncore_pmu.h"

/*
 * ARMv8 HiSilicon DDRC event types.
 */
enum armv8_hisi_ddrc_event_types {
	HISI_HWEVENT_DDRC_FLUX_WR		= 0x0,
	HISI_HWEVENT_DDRC_FLUX_RD		= 0x01,
	HISI_HWEVENT_DDRC_FLUX_WCMD		= 0x02,
	HISI_HWEVENT_DDRC_FLUX_RCMD		= 0x03,
	HISI_HWEVENT_DDRC_FLUXID_WD		= 0x04,
	HISI_HWEVENT_DDRC_FLUXID_RD		= 0x05,
	HISI_HWEVENT_DDRC_FLUXID_WCMD		= 0x06,
	HISI_HWEVENT_DDRC_FLUXID_RCMD		= 0x07,
	HISI_HWEVENT_DDRC_WLAT_CNT0		= 0x08,
	HISI_HWEVENT_DDRC_RLAT_CNT0		= 0x09,
	HISI_HWEVENT_DDRC_WLAT_CNT1		= 0x0A,
	HISI_HWEVENT_DDRC_RLAT_CNT1		= 0x0B,
	HISI_HWEVENT_DDRC_INHERE_RLAT_CNT	= 0x0C,
	HISI_HWEVENT_DDRC_MAX_EVENT,
};

#define HISI_DDRC_CTRL_PERF_REG_OFF		0x010
#define HISI_DDRC_CFG_PERF_REG_OFF		0x270
#define HISI_DDRC_FLUX_WR_REG_OFF		0x380
#define HISI_DDRC_FLUX_RD_REG_OFF		0x384
#define HISI_DDRC_FLUX_WCMD_REG_OFF		0x388
#define HISI_DDRC_FLUX_RCMD_REG_OFF		0x38C
#define HISI_DDRC_FLUXID_WR_REG_OFF		0x390
#define HISI_DDRC_FLUXID_RD_REG_OFF		0x394
#define HISI_DDRC_FLUXID_WCMD_REG_OFF		0x398
#define HISI_DDRC_FLUXID_RCMD_REG_OFF		0x39C
#define HISI_DDRC_FLUX_WLATCNT0_REG_OFF		0x3A0
#define HISI_DDRC_FLUX_RLAT_CNT0_REG_OFF	0x3A4
#define HISI_DDRC_FLUX_WLATCNT1_REG_OFF		0x3A8
#define HISI_DDRC_FLUX_RLAT_CNT1_REG_OFF	0x3AC

struct hisi_ddrc_hwcfg {
	u32 channel_id;
};

struct hisi_ddrc_data {
	void __iomem *regs_base;
	DECLARE_BITMAP(hisi_ddrc_event_used_mask,
				HISI_HWEVENT_DDRC_MAX_EVENT);
	struct hisi_ddrc_hwcfg ddrc_hwcfg;
};

#endif /* __HISI_UNCORE_DDRC_H__ */
