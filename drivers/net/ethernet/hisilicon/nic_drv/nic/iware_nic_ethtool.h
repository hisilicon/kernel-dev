/*******************************************************************************

  Hisilicon network interface controller ethool driver
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

*******************************************************************************/

#ifndef _HRD_NIC_ETHTOOL_H
#define _HRD_NIC_ETHTOOL_H
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <asm/cacheflush.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ethtool.h>


#define ETH_STATIC_REG	 1
#define ETH_DUMP_REG	   5

extern const struct ethtool_ops g_nic_ethtools;

#define MAX_RING_PAIR_NUM 129
#define NIC_MAX_RING_NUM   16

#define NIC_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define NIC_GET_REG_LEN	360	/* TBD */

/* LED modes */
#define NIC_LED_LINK_UP       0x0
#define NIC_LED_LINK_10G      0x1
#define NICE_LED_MAC           0x2
#define NIC_LED_FILTER        0x3
#define NIC_LED_LINK_ACTIVE   0x4
#define NIC_LED_LINK_1G       0x5
#define NIC_LED_ON            0xE
#define NIC_LED_OFF           0xF

#define PHY_PAGE_REG			(22)
#define PHY_PAGE_LED			(3)
#define LD_FORCE_ON		(9)
#define LD_FORCE_OFF		(8)
#define PHY_PAGE_COPPER	 (0)
#define LED_FCR				 (16)
#define LED_PCR				 (17)


#define NIC_LB_TEST_PKT_NUM_PER_CYCLE  1

#define NIC_LB_TEST_RING_ID 0
#define NIC_LB_TEST_FRAME_SIZE 128

/* nic loopback test err  */
enum nic_lb_err{
	NIC_LB_TEST_NO_MEM_ERR	= 1,
	NIC_LB_TEST_TX_CNT_ERR,
	NIC_LB_TEST_RX_CNT_ERR
};


#endif
