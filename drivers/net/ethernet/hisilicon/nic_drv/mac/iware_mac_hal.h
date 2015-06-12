/************************************************************************

  Hisilicon MAC driver
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:TBD

************************************************************************/

#ifndef _IWARE_MAC_HAL_H
#define _IWARE_MAC_HAL_H

/* #include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_log.h"
#include "iware_mac_main.h"
#if 0 /* TBD */
struct tagGmacDev {
	struct mac_driver mac_drv;
	void * app;
	u64 addr;
	enum mac_mode port_mode;
	mac_exception_cb_t *event;
	/**< Exception Callback Routine  */
	mac_exception_cb_t *exception;

	u8 half_duplex;
	u8 mac_id;
};
#endif
#define IS_ENET_MODE_VALID(mode) \
		(((mode) == MAC_MODE_MII_10) || \
		 ((mode) == MAC_MODE_MII_100) || \
		 ((mode) == MAC_MODE_RMII_10) || \
		 ((mode) == MAC_MODE_RMII_100) || \
		 ((mode) == MAC_MODE_SMII_10) || \
		 ((mode) == MAC_MODE_SMII_100) || \
		 ((mode) == MAC_MODE_GMII_1000) || \
		 ((mode) == MAC_MODE_RGMII_10) || \
		 ((mode) == MAC_MODE_RGMII_100) || \
		 ((mode) == MAC_MODE_RGMII_1000) || \
		 ((mode) == MAC_MODE_TBI_1000) || \
		 ((mode) == MAC_MODE_RTBI_1000) || \
		 ((mode) == MAC_MODE_SGMII_10) || \
		 ((mode) == MAC_MODE_SGMII_100) || \
		 ((mode) == MAC_MODE_SGMII_1000) || \
		 ((mode) == MAC_MODE_XGMII_10000) || \
		 ((mode) == MAC_MODE_QSGMII_1000))



#define PHY_AUTONEG_ENABLE   1
#define PHY_AUTONEG_DISABLE  0

#define MAR_PHY_MAX_ADDR 0x1F

/* Full duplex 1000Mb/s supported */
#define MARVL88E1119_MII_ANA_FD_1000		  (1 << 9)
/* Half duplex 1000Mb/s supported */
#define MARVL88E1119_MII_ANA_HD_1000		  (1 << 8)
/*mII 1000Base-T control register */
#define MII_GB_CTRL_REG		 0x09

#define MII_CTRL_REG			0x00
/*mII Status Register: ro */
#define MII_STAT_REG			0x01
#define MII_PHY_ID0_REG			0x02
#define MII_PHY_ID1_REG			0x03
#define MII_ANA_REG				0x04
#define MII_POL_CTRL_REG		0x11
#define MII_PAGE_ADD			0x16

/* Link Active */
#define MII_STAT_LA				(1 << 2)
/* Half duplex 10Mb/s supported */
#define MII_ANA_HD_10			(1 << 5)
/* Full duplex 10Mb/s supported */
#define MII_ANA_FD_10			(1 << 6)
/* Half duplex 100Mb/s supported */
#define MII_ANA_HD_100			(1 << 7)
/* Full duplex 100Mb/s supported */
#define MII_ANA_FD_100			(1 << 8)
#define MII_ANA_T4				(1 << 9)
#define MII_ANA_PAUSE			(1 << 10)
/* Asymmetric pause supported */
#define MII_ANA_ASYM_PAUSE		(1 << 11)

/** 1000Base-T Control Register */
#define MII_GB_CTRL_ADV_1000FD  (1 << 9)
#define MII_GB_CTRL_ADV_1000HD  (1 << 8)

/* Autonegotiation enable */
#define MII_CTRL_AE				(1 << 12)
/* PHY reset */
#define MII_CTRL_RESET			(1 << 15)
/* Restart Autonegotiation */

#define MII_CTRL_RAN			(1 << 9)


/* ge phy reg */
#define  COPPER_CONTROL_REG 	0
#define  PHY_LOOP_BACK 		(1<<14)
#define  PHY_AN_ENABLE 		(1<<12)
#define  PHY_AN_RESTART 	(1<<8)

/* xge phy reg */
#define XGE_PHY_LINE_LB 1
#define XGE_PHY_HOST_LB 0
#define XGE_PHY_NEAR_LB 1
#define XGE_PHY_FAR_LB  0



#define RX_CSR(lane,reg)       	(((0x4080+(reg)*0x0002+(lane)*0x0200))*2)
#define HILINK3_TO_HILINK4_OFFSET 0x80000ULL

enum serdes_for_mac{
	MAC_HILINK3, 	/* mac4-7 */
	MAC_HILINK4,	/* mac0-3 */
	MAC_HILINK_MAX
};


struct mac_stats_string {
	char desc[64];
	unsigned long offset;
};
#if 0
extern int phy_loopback_set(u32 slice,u32 line_host,
				u32 near_far,u32 enable_ctl);
#endif
#endif /* _IWARE_MAC_HAL_H */

