/************************************************************************

  Hisilicon MAC-LED driver
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

#ifndef _IWARE_LED_HAL_H
#define _IWARE_LED_HAL_H

#include "iware_mac_main.h"
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>

/*serdes offset**/
#define MAC_HILINK3_REG (0x180)
#define MAC_HILINK4_REG (0x190)
#define HILINK4_LANE0_CTLEDFE_REG 0x000BFFCCULL
#define HILINK4_LANE1_CTLEDFE_REG 0x000BFFBCULL
#define HILINK4_LANE2_CTLEDFE_REG 0x000BFFACULL
#define HILINK4_LANE3_CTLEDFE_REG 0x000BFF9CULL
#define HILINK4_LANE0_STATE_REG 0x000BFFD4ULL
#define HILINK4_LANE1_STATE_REG 0x000BFFC4ULL
#define HILINK4_LANE2_STATE_REG 0x000BFFB4ULL
#define HILINK4_LANE3_STATE_REG 0x000BFFA4ULL
#define HILINK_RESET_TIMOUT 10000/**1000000*/

#define PCA9555_REG0 0
#define PCA9555_REG1 1
#define PCA9555_REG2 2
#define PCA9555_REG3 3
#define PCA9555_REG4 4
#define PCA9555_REG5 5
#define PCA9555_REG6 6
#define PCA9555_REG7 7


/* LED modes */
#define NIC_LED_LINK_UP       (0x0)
#define NIC_LED_LINK_10G      (0x1)
#define NICE_LED_MAC          (0x2)
#define NIC_LED_FILTER        (0x3)
#define NIC_LED_LINK_ACTIVE   (0x4)
#define NIC_LED_LINK_1G       (0x5)
#define NIC_LED_ON            (0xE)
#define NIC_LED_OFF           (0xF)

#define NIC_LED_ON_VALUE      0x20
#define NIC_LED_OFF_VALUE     0x0



/*for 88e1543 phy*/
#define LD_FORCE_ON			(9)
#define LD_FORCE_OFF		(8)
#define LED_FORCE_OFF_MOD		   (0x8)
#define LED_FORCE_ON_MOD			(0x9)
#define LED_ACT_DEF_MOD		 (0x1)
#define LED_SPEED_DEF_MOD		   (0x7)
#define LED_SPEED_OFFSET		(0)
#define LED_ACT_OFFSET		  (12)
#define PHY_PAGE_REG			(22)
#define PHY_PAGE_LED			(3)
#define PHY_PAGE_RESET			(0)
#define PHY_NEG_EN			(0x1000)
#define PHY_NEG_RESTART			(0x200)
#define PHY_AUTO_REG			(0)
#define PHY_PAGE_COPPER	 (0)
/*LED[3:0] Function Control Register (4bits per led)
Page 3, Register 16**/
#define LED_FCR				 (16)
/*LED[3:0] Polarity Control Register (4bits per led)
Page 3, Register 17*/
#define LED_PCR				 (17)

#define CPLD_DEFAULT_VALUE	0x3

/*for 10G clpd reg**/
#define MAC_LED_PORT0 (0X200)
#define MAC_LED_PORT1 (0X201)
#define MAC_LED_PORT2 (0X202)
#define MAC_LED_PORT3 (0X203)
/*define opt value**/
#define DRV_LED_TEST_ON 0x0
#define DRV_PORT_LED_RESTORE 0x1
#define DRV_LED_TEST_OFF 0x2
#define DRV_PORT_LED_SPEED_HIGH 0x3
#define DRV_PORT_LED_SPEED_LOW 0x4
/*
SFP optical module / SFP+
PRSNT_REG - LED_REG = 0XC
*/
#define MAC_SFP_PORT_OFFSET (0XC)


#define MAC_LED_OFF 0
#define MAC_LED_BLUE_ON 1 /* max high speed*/
#define MAC_LED_BLUE_ING 2 /* max high speed working*/
#define MAC_LED_GREEN_ON 3 /* non high speed*/
#define MAC_LED_GREEN_ING 4 /* non high speed working*/
#define MAC_LED_RED_ON 5 /* error */
#define MAC_LED_RED_ING 6 /* error checking*/

union mac_led_port {
	struct {
		unsigned int led_xgep0_speed:2; /*[1..0]  */
		unsigned int led_xgep0_link:1; /*[2]  */
		unsigned int led_xgep0_alarm:1; /*[3]  */
		unsigned int led_xgep0_data:1; /*[4]  */
		unsigned int led_xgep0_anchor:1; /*[5]  */
		unsigned int led_xgep0_spare:2; /*[7..6]  */
	} bits;
	unsigned char u8;
};


union mac_sfp_port {
	struct {
		unsigned int ctl_sfp0_prsnt:1; /*[0]  */
		unsigned int ctl_sfp0_rx_los:1; /*[1]  */
		unsigned int ctl_sfp0_tx_fault:1;/*[2]  */
		unsigned int ctl_sfp0_tx_dis:1; /*[3]  */
		unsigned int ctl_sfp0_spare:4; /*[7..4]*/
	} bits;
	unsigned char u8;
};

union mac_half_u8 {
	struct {
		unsigned int low_4bits:4; /*[3..0]*/
		unsigned int hig_4bits:4; /*[7..4]*/
	} bits;
	unsigned char u8;
};

/*CPLD reg = <0x0 0x98000000 0x0 0x1000>;*/

/*#define cpld_reg_write(addr, data)\
    (writeb((data), (__iomem unsigned char *)(addr)))
#define cpld_reg_read(addr)\
    (readb((__iomem unsigned char *)(addr)))*/


void sfp_led_set(struct mac_device * mac_dev, int port, int link_status, \
		u16 speed, int data);
void __iomem *mac_get_base_addr(struct device_node *np, int index);
void sfp_led_reset(struct mac_device * mac_dev, int port);
enum mac_phy_interface mac_get_sds_mode(void __iomem *sys_ctl_vaddr, int dev_id);
int phy_led_set(struct mac_device * mac_dev, int value);
int mac_sfp_open(struct mac_device *mac_dev);
int mac_sfp_close(struct mac_device *mac_dev);
int mac_sfp_prsnt(struct mac_device *mac_dev);


#endif /* _IWARE_LED_HAL_H */

