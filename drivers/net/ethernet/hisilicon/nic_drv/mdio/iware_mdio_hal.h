/************************************************************************

  Hisilicon MDIO driver
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

#ifndef _IWARE_MDIO_HAL_H
#define _IWARE_MDIO_HAL_H

#include "iware_log.h"
#include "iware_mac_main.h"
#include "iware_mdio_main.h"

#define GMAC_MDIOBUS_PORT	(0)
#define MDIO_OK			  (0)
#define MDIO_ERROR		   (-1)

#define MDIO_ST_CLAUSE_22		   (0x01)
#define MDIO_ST_CLAUSE_45		   (0x00)

#define MDIO_C22_READ		 (0x02)
#define MDIO_C22_WRITE		(0x01)

#define MDIO_C45_READ		 (0x03)
#define MDIO_C45_READ_INCREMENT		 (0x02)
#define MDIO_C45_WRITE_DATA   (0x01)
#define MDIO_C45_WRITE_ADDR   (0x00)

#define MDIO_BASE_ADDR			  (0x403C0000)
#define MDIO_REG_ADDR_LEN		   (0x1000)
#define MDIO_PHY_GRP_LEN			(0x100)
#define MDIO_REG_LEN				(0x10)
#define MDIO_PHY_ADDR_NUM		   (5)
#define MDIO_MAX_PHY_ADDR		   (0x1F)
#define MDIO_MAX_PHY_REG_ADDR	   (0xFFFF)

#define MDIO_TIMEOUT				(1000000)
#define MDIO_DELAY_NS				(0)

#ifndef MDIO_DEVAD_NONE
#define MDIO_DEVAD_NONE		 (-1)
#endif
static inline void mdio_write_reg(struct mdio_device *mdio_drv, u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(mdio_drv->vbase);

	writel(value, reg_addr + reg);
}

#define MDIO_WRITE_REG(a, reg, value) mdio_write_reg((a), (reg), (value))

static inline u32 mdio_read_reg(struct mdio_device *mdio_drv, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(mdio_drv->vbase);
	u32 value;

	value = readl(reg_addr + reg);
	return value;
}

#define MDIO_READ_REG(a, reg) mdio_read_reg((a), (reg))

#define MDIO_SC_CLK_EN	0x338
#define MDIO_SC_CLK_DIS	0x33C
#define MDIO_SC_RESET_REQ	0xA38
#define MDIO_SC_RESET_DREQ	0xA3C
#define MDIO_SC_CTRL	0x2010
#define MDIO_SC_CLK_ST	0x531C
#define MDIO_SC_RESET_ST	0x5A1C

#define mdio_sc_reg_write(addr, data)\
    writel((data), (__iomem unsigned int *)(addr))
#define mdio_sc_reg_read(addr)\
    (readl((__iomem unsigned int *)(addr)))

enum phy_loop_mode {
	PHY_LOOP_MODE_OFF = 0,	/*no loopback */
	PHY_LOOP_MODE_SYS,
	PHY_LOOP_MODE_LINE,
	PHY_LOOP_MODE_UNKNOWN
};

enum phy_inerface {
	SGMII_COPPER = 0,
	RGMII_COPPER,
	RTBI_COPPER,
	RGMII_FIBER,
	RTBI_FIBER,
	SGMII_100BASE_FX = 5,
	RGMII_100BASE_FX,
	GBIC			/*fiber to copper */
};

#define MDIO_COMMAND_REG			(0x0)
#define MDIO_ADDR_REG				(0x4)
#define MDIO_WDATA_REG				(0x8)
#define MDIO_RDATA_REG				(0xc)
#define MDIO_STA_REG				(0x10)

union mdio_command_reg {
	struct {
		unsigned int Mdio_Devad:5;
		unsigned int Mdio_Prtad:5;
		unsigned int Mdio_Op:2;
		unsigned int Mdio_St:2;
		unsigned int Mdio_Start:1;
		unsigned int Rsv:17;
	} bits;
	/* Define an unsigned member  */
	unsigned int u32;

};

/* Define the union union mdio_addr_reg  */
union mdio_addr_reg {
	/* Define the struct bits  */
	struct {
		unsigned int Mdio_Address:16;
		unsigned int Rsv:16;
	} bits;

	/* Define an unsigned member  */
	unsigned int u32;

};

/* Define the union union mdio_wdata_reg  */

union mdio_wdata_reg {
	/* Define the struct bits  */
	struct {
		unsigned int Mdio_Wdata:16;
		unsigned int Rsv:16;
	} bits;

	/* Define an unsigned member  */
	unsigned int u32;

};

/* Define the union union mdio_radata_reg  */
union mdio_radata_reg {
	/* Define the struct bits  */
	struct {
		unsigned int Mdio_Rdata:16;
		unsigned int Rsv:16;
	} bits;

	/* Define an unsigned member  */
	unsigned int u32;

};

/* Define the union union mdio_sta_reg  */
union mdio_sta_reg {
	/* Define the struct bits  */
	struct {
		unsigned int Mdio_Sta:1;
		unsigned int Rsv:31;
	} bits;

	/* Define an unsigned member  */
	unsigned int u32;

};

#endif				/* _IWARE_MDIO_HAL_H */

