/*********************************************************************

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

**********************************************************************/

#ifndef _IWARE_MDIO_MAIN_H
#define _IWARE_MDIO_MAIN_H

#include "iware_log.h"
#include <linux/phy.h>
#include <linux/mutex.h>
#include <linux/spinlock_types.h>

#define MDIO_MOD_VERSION "iWareV2R2C00B981"
#define MDIO_DRV_NAME "Hi-MDIO"

#define DSAF_MAX_CHIP_NUM 2

#define MDIO_CTL_DEV_ADDR(x)	(x & 0x1f)
#define MDIO_CTL_PORT_ADDR(x)	((x & 0x1f) << 5)

struct mdio_device;

struct mdio_ops {
	int (*write_phy_reg)(struct mdio_device *mdio_dev, u8 phy_addr,
		u8 is_c45, u8 page, u16 reg, u16 data);
	int (*read_phy_reg)(struct mdio_device *mdio_dev, u8 phy_addr,
		u8 is_c45, u8 page, u16 reg, u16 *data);
	int (*reset)(struct mdio_device *mdio_dev);
};

struct mdio_device {
	struct device *dev;
	void *vbase;		/* mdio reg base address */
	void *sys_vbase;		/* mdio reg base address */
	u8 phy_class[PHY_MAX_ADDR];
	u8 index;
	u8 chip_id;
	u8 gidx;		/* global index */
	struct mutex mdio_lock;
	struct mdio_ops ops;
};

void mdio_set_ops(struct mdio_ops *ops);

#endif				/* _IWARE_MDIO_MAIN_H */

