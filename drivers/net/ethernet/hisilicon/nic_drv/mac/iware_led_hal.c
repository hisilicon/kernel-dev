/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe483nhsrd4BVmLd72511nY25AnXpzqgQ9t1e7ru3eeIo6LpM0Nnxzmq9nNca7gYCckCrYR
V7poVkbD6jZg2RWuMebablzu87C/jFfEHEiZB/IqeoyHmnFk/ZSbfcbC2kgzNw==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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

#include "iware_error.h"
#include "iware_log.h"
#include "iware_led_hal.h"
#include "iware_gmac_hal.h"
#include "iware_mac_main.h"

#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/i2c.h>



/**
 *phy_led_opt - ctl led light for phy 88ee1543
 *@mac_dev: mac device
 *@opt  :  opt num
 * return status
 */
 /*
int phy_led_opt(struct mac_device *mac_dev, int opt)
{
    int phy_reg = 0;
    int retval;

    if (NULL == mac_dev) {
        pr_err("mac_led_opt mac_dev is null!\n");
        return -EINVAL;
    }
    if (NULL == mac_dev->phy_dev) {
        log_err(mac_dev->dev,
            "mac_led_opt mac_id=%d, phy_dev is null !\n",
            mac_dev->mac_id);
        return -EINVAL;
    }
    if (NULL == mac_dev->phy_dev->bus) {
        log_err(mac_dev->dev,
            "mac_led_opt mac_id=%d,phy_dev->bus is null !\n",
            mac_dev->mac_id);
        return -EINVAL;
    }

    retval = mdiobus_write(mac_dev->phy_dev->bus,
            mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_LED);
    if (retval) {
        log_err(mac_dev->dev,
            "mdiobus_write fail mac_id=%d, ret = %d!\n",
            mac_dev->mac_id, retval);
        return retval;
    }

    switch (opt) {
    case 0x00:
        phy_reg = (LED_FORCE_ON_MOD << LED_SPEED_OFFSET)
                | (LED_FORCE_ON_MOD << LED_ACT_OFFSET);
        retval = mdiobus_write(mac_dev->phy_dev->bus,
                mac_dev->phy_dev->addr, LED_FCR, phy_reg);
        break;

    case 0x01:
        phy_reg = (LED_SPEED_DEF_MOD << LED_SPEED_OFFSET)
                | (LED_ACT_DEF_MOD << LED_ACT_OFFSET);
        retval = mdiobus_write(mac_dev->phy_dev->bus,
                mac_dev->phy_dev->addr, LED_FCR, phy_reg);
        break;

    case 0x02:
        phy_reg = (LED_FORCE_OFF_MOD << LED_SPEED_OFFSET)
                | (LED_FORCE_OFF_MOD << LED_ACT_OFFSET);
        retval = mdiobus_write(mac_dev->phy_dev->bus,
                mac_dev->phy_dev->addr, LED_FCR, phy_reg);
        break;

    case 0x03:
        phy_reg = (LED_FORCE_ON_MOD << LED_SPEED_OFFSET);
        retval = mdiobus_write(mac_dev->phy_dev->bus,
                mac_dev->phy_dev->addr, LED_FCR, phy_reg);
        break;

    case 0x04:
        phy_reg = (LED_FORCE_OFF_MOD << LED_SPEED_OFFSET);
        retval = mdiobus_write(mac_dev->phy_dev->bus,
                mac_dev->phy_dev->addr, LED_FCR, phy_reg);
        break;

    default:
        log_dbg(mac_dev->dev, "undefine led opt code !\n");
        return -EINVAL;
    }
    if (retval) {
        log_err(mac_dev->dev,
            "mdiobus_write fail mac_id=%d, ret = %d!\n",
            mac_dev->mac_id, retval);
        return retval;
    }

    retval = mdiobus_write(mac_dev->phy_dev->bus,
        mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_COPPER);
    if (retval) {
        log_err(mac_dev->dev,
            "mdiobus_write fail mac_id=%d, ret = %d!\n",
            mac_dev->mac_id, retval);
        return retval;
    }
    return 0;
}
*/

/**
 *sfp_led_opt - ctl led light for phy 88ee1543
 *@mac_dev: mac device
 *@port  :  port num: 0-3
 *@colour  :  colour vlaue: 0-2 => red green blue
 *@display  :  opt display: 0-2 => off ing on
 * return status
 */
/*
int sfp_led_opt(struct mac_device *mac_dev,
    int port, int colour, int display)
{

    const unsigned int port_regs[4] = {
        MAC_LED_PORT0,
        MAC_LED_PORT1,
        MAC_LED_PORT2,
        MAC_LED_PORT3,
    };

    union mac_led_port value;

    if (NULL == mac_dev) {
        pr_err("sfp_led_opt mac_dev is null!\n");
        return -EINVAL;
    }
    if (NULL == mac_dev->cpld_vaddr) {
        log_err(mac_dev->dev,
            "sfp_led_opt mac_id=%d, cpld_vaddr is null !\n",
            mac_dev->mac_id);
        return -EINVAL;
    }

    value.u8 = cpld_reg_read(mac_dev->cpld_vaddr + port_regs[port & 3]);

    if (0x2 == colour) {
        value.bits.led_xgep0_speed = 1;
        value.bits.led_xgep0_alarm = 0;
    } else if (0x1 == colour) {
        value.bits.led_xgep0_speed = 0;
        value.bits.led_xgep0_alarm = 0;
    } else {
        value.bits.led_xgep0_alarm = 1;
        value.bits.led_xgep0_speed = 0;
    }

    if (0x2 == display) {
        value.bits.led_xgep0_link = 1;
        value.bits.led_xgep0_anchor = 0;
        value.bits.led_xgep0_data = 0;
    } else if (0x1 == display) {
        value.bits.led_xgep0_link = 1;
        if (0x0 == colour) {
            value.bits.led_xgep0_anchor = 1;
            value.bits.led_xgep0_data = 0;
        } else {
            value.bits.led_xgep0_data = 1;
            value.bits.led_xgep0_anchor = 0;
        }
    } else
        value.u8 = 0;

    cpld_reg_write(mac_dev->cpld_vaddr + port_regs[port & 3], value.u8);

    return 0;
}
*/

/**
 *sfp_led_set - ctl led light for phy 88ee1543
 *@mac_dev: mac device
 *@port  :  port num: 0-3
 *@link_status  :  link_status
 *@speed  :  speed
 *@data    : data
 * return status
 */

void sfp_led_set(struct mac_device * mac_dev, int port, int link_status,
		u16 speed, int data)
{
	int speed_reg = 0;
	/*
	 const unsigned int port_regs[4] = {
        MAC_LED_PORT0,
        MAC_LED_PORT1,
        MAC_LED_PORT2,
        MAC_LED_PORT3,
    };
	 */

    union mac_led_port value;

    if (NULL == mac_dev) {
        pr_err("sfp_led_opt mac_dev is null!\n");
        return ;
    }
    if (NULL == mac_dev->cpld_vaddr) {
        log_err(mac_dev->dev,
            "sfp_led_opt mac_id=%d, cpld_vaddr is null !\n",
            mac_dev->mac_id);
        return ;
    }

	if(speed == MAC_SPEED_10000)
		speed_reg = 1;

    value.u8 = mac_dev->cpld_led_value;

	if(link_status) {
		if(value.bits.led_xgep0_link != link_status)
			value.bits.led_xgep0_link = link_status;

		if(value.bits.led_xgep0_speed != speed_reg)
			value.bits.led_xgep0_speed = speed_reg;

		if(value.bits.led_xgep0_data != data)
			value.bits.led_xgep0_data = data;

		cpld_reg_write(mac_dev->cpld_vaddr, value.u8);
		mac_dev->cpld_led_value = value.u8;
	} else {
		cpld_reg_write(mac_dev->cpld_vaddr, CPLD_DEFAULT_VALUE);
		mac_dev->cpld_led_value = CPLD_DEFAULT_VALUE;
	}

}

void sfp_led_reset(struct mac_device * mac_dev, int port)
{

    if (0 == mac_dev) {
        pr_err("sfp_led_opt mac_dev is null!\n");
        return ;
    }
    if (0 == mac_dev->cpld_vaddr) {
        log_err(mac_dev->dev,
            "sfp_led_opt mac_id=%d, cpld_vaddr is null !\n",
            mac_dev->mac_id);
        return ;
    }
	cpld_reg_write(mac_dev->cpld_vaddr, CPLD_DEFAULT_VALUE);
	mac_dev->cpld_led_value = CPLD_DEFAULT_VALUE;
}


/**
 *sfp_led_set - ctl led light for phy 88ee1543
 *@mac_dev: mac device
 *@port  :  port num: 0-3
 *@link_status  :  link_status
 *@speed  :  speed
 *@data    : data
 * return status
 */
void __iomem *mac_get_base_addr(struct device_node *np, int index)
{
	u64 size = 0;
	void __iomem *tmp_addr = NULL;
	void __iomem *cpld_base_addr = NULL;
	void __iomem *cpld_phy_addr = NULL;

	tmp_addr = (void __iomem *)of_get_address(np, index, &size, NULL);
	if (!tmp_addr)
		return NULL;
	cpld_phy_addr = (void __iomem *)of_translate_address(np, (const void*)tmp_addr);
	if(!cpld_phy_addr)
		return NULL;
	cpld_base_addr = (void __iomem *)ioremap((phys_addr_t)cpld_phy_addr, size);
    if (!cpld_base_addr)
        return NULL;

	return cpld_base_addr;
}

enum mac_phy_interface mac_get_sds_mode(
    void __iomem *sys_ctl_vaddr, int dev_id)
{

    u32 hilink3_mode = 0;
    u32 hilink4_mode = 0;
    enum mac_phy_interface phy_if = MAC_PHY_INTERFACE_MODE_NA;

    hilink3_mode = serdes_reg_read((u64)sys_ctl_vaddr + MAC_HILINK3_REG);
    hilink4_mode = serdes_reg_read((u64)sys_ctl_vaddr + MAC_HILINK4_REG);
    if (dev_id >= 0 && dev_id <= 3) {
        if(0 == hilink4_mode)
            phy_if = MAC_PHY_INTERFACE_MODE_SGMII;
        else
            phy_if = MAC_PHY_INTERFACE_MODE_XGMII;
    } else if(dev_id >= 4 && dev_id <= 5) {
        if(0 == hilink3_mode)
            phy_if = MAC_PHY_INTERFACE_MODE_SGMII;
        else
            phy_if = MAC_PHY_INTERFACE_MODE_XGMII;
    } else
        phy_if = MAC_PHY_INTERFACE_MODE_SGMII;

    pr_info(
        "[mac_get_sds_mode]hilink3_mode=%d, hilink4_mode=%d, dev_id=%d, phy_if=%d\n",
        hilink3_mode, hilink4_mode, dev_id, phy_if);
    return phy_if;
}

int phy_led_set(struct mac_device * mac_dev, int value)
{
	int retval = 0;
	if (NULL == mac_dev) {
		pr_err("mac_led_opt mac_dev is null!\n");
		return -EINVAL;
	}
	if (NULL == mac_dev->phy_dev) {
		log_err(mac_dev->dev,
		  "mac_led_opt mac_id=%d, phy_dev is null !\n",
		  mac_dev->mac_id);
		return -EINVAL;
	}
	if (NULL == mac_dev->phy_dev->bus) {
		log_err(mac_dev->dev,
		  "mac_led_opt mac_id=%d,phy_dev->bus is null !\n",
		  mac_dev->mac_id);
		return -EINVAL;
	}
	retval = mdiobus_write(mac_dev->phy_dev->bus,
            mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_LED);
	retval = mdiobus_write(mac_dev->phy_dev->bus,
		  mac_dev->phy_dev->addr, LED_FCR, value);
	retval = mdiobus_write(mac_dev->phy_dev->bus,
            mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_COPPER);
	if (retval) {
	        log_err(mac_dev->dev,
	            "mdiobus_write fail mac_id=%d, ret = %d!\n",
	            mac_dev->mac_id, retval);
	        return retval;
	}
	return 0;
}

/**
 * mac_do_i2c_xfer - access eeprom by i2c interface
 * @bus_num: i2c bus num
 * @chip_addr: i2c chip selete
 * @sub_addr: reg offset
 * @mode: i2c access mode
 * @buf: buf address
 * @size: buffer size
 * @sub_addr_len: sub address length
 * @type: type
 *
 * Return 0 on success, negative on failure
 */
int mac_do_i2c_xfer(int bus_num, char chip_addr, unsigned int sub_addr,
			   int mode, char *buf, unsigned int size,
			   int sub_addr_len, const char *type)
{

	int ret = 0;

	char sub_addr_buf[8] = { 0 };
	char *tmp = NULL;
	int find = 0;

	struct i2c_client *client, *next;
	struct i2c_board_info info;
	struct i2c_adapter *adap;

	adap = i2c_get_adapter(bus_num);
	if (!adap)
		return -EINVAL;

	mutex_lock_nested(&adap->userspace_clients_lock,
			  i2c_adapter_depth(adap));
	list_for_each_entry_safe(client, next, &adap->userspace_clients,
				 detected) {
		if ((typeof(chip_addr)) client->addr == chip_addr) {
			find = 1;
			break;
		}
	}
	if (!find) {
		memset(&info, 0, sizeof(struct i2c_board_info));
		if (type)
			strlcpy(info.type, type, I2C_NAME_SIZE);
		else
			strlcpy(info.type, "eeprom_mac", I2C_NAME_SIZE);
		info.addr = chip_addr;
		client = i2c_new_device(adap, &info);
		if (!client) {
			mutex_unlock(&adap->userspace_clients_lock);
			return -EINVAL;
		}
		/* Keep track of the added device */
		list_add_tail(&client->detected, &adap->userspace_clients);
	}
	mutex_unlock(&adap->userspace_clients_lock);

	if (1 == sub_addr_len)
		sub_addr_buf[0] = (char)(sub_addr);
	else if (2 == sub_addr_len) {
		sub_addr_buf[1] = (char)(sub_addr & 0xff);
		sub_addr_buf[0] = (char)(sub_addr >> 8);
	} else
		sub_addr_len = 0;

	if (!mode) {
		tmp = kmalloc(size + sub_addr_len, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;
		if (sub_addr_len)
			memcpy(&tmp[0], sub_addr_buf, sub_addr_len);
		memcpy(&tmp[sub_addr_len], buf, size);
		ret = i2c_master_send(client, tmp, size + sub_addr_len);
		ret = (ret == size + sub_addr_len) ? size : ret;
		kfree(tmp);
	} else {
		if (sub_addr_len) {
			ret =
			    i2c_master_send(client, sub_addr_buf, sub_addr_len);
			if (ret < 0)
				return ret;
		}
		ret = i2c_master_recv(client, buf, size);
	}

	return ret;
}
EXPORT_SYMBOL(mac_do_i2c_xfer);

/**
 * mac_reset_hilink_ctledfe - open sfp
 * @macro: hilink id
 * @lane: lane id
 * Return 0 on success, negative on failure
 */
int mac_reset_hilink_ctledfe(
	struct mac_device *mac_dev, u32 macro,u32 lane)
{
    /*reset used HiLink's lane CTLE/DFE,
    adjust CTLE/DFE value:
    for hilink4 line3:
    devmem 0xc22bff9c w 0xa859
    wait(1ms);
    devmem 0xc22bff9c w 0xa851
    0xC22BFFCC
    0xC22BFFbC
    0xC22BFFaC
    0xC22BFF9C*/
    const u64 ctle_dfe_regs[] = {
        HILINK4_LANE0_CTLEDFE_REG,
        HILINK4_LANE1_CTLEDFE_REG,
        HILINK4_LANE2_CTLEDFE_REG,
        HILINK4_LANE3_CTLEDFE_REG,
    };
    const u64 ctle_state_regs[] = {
        HILINK4_LANE0_STATE_REG,
        HILINK4_LANE1_STATE_REG,
        HILINK4_LANE2_STATE_REG,
        HILINK4_LANE3_STATE_REG,
    };

    unsigned int data = 0;
    unsigned int i = 0;

    if (!mac_dev) {
        pr_err("mac_release_hilink_ctledfe mac dev is null\n");
        return -EINVAL;
    }
    if (!mac_dev->serdes_vaddr) {
        log_err(mac_dev->dev,
        	"serdes_addr=%p hasnot mapped\n",
        	mac_dev->serdes_vaddr);
        return -EINVAL;
    }

    data = serdes_reg_read(
    	(u64)mac_dev->serdes_vaddr + ctle_dfe_regs[lane & 3]);
    data = data | (1ul << 3);
    serdes_reg_write(
    	((u64)mac_dev->serdes_vaddr + ctle_dfe_regs[lane & 3]), data);

    for (i = 0; i < HILINK_RESET_TIMOUT; i++) {
        udelay(100);
        data = serdes_reg_read(
        	(u64)mac_dev->serdes_vaddr + ctle_state_regs[lane & 3]);
        if (data & (1ul << 3))
            break;
    }
    if (!(data & (1ul << 3))) {
        log_err(mac_dev->dev,
        	"macro%d, lane%d ctle/dfe reset timeout(%d)!\n",
        	macro, lane, i);
        return -EBUSY;
    }

	log_info(mac_dev->dev,
			"macro%d, lane%d ctle/dfe reset cnt=%d!\n",
			macro, lane, i);

    return 0;
}
int mac_release_hilink_ctledfe(
	struct mac_device *mac_dev, u32 macro, u32 lane)
{
    /*reset used HiLink's lane CTLE/DFE,
    adjust CTLE/DFE value:
    for hilink4 line3:
    devmem 0xc22bff9c w 0xa859
    wait(1ms);
    devmem 0xc22bff9c w 0xa851
    0xC22BFFCC
    0xC22BFFbC
    0xC22BFFaC
    0xC22BFF9C*/
    const u64 ctle_dfe_regs[] = {
        HILINK4_LANE0_CTLEDFE_REG,
        HILINK4_LANE1_CTLEDFE_REG,
        HILINK4_LANE2_CTLEDFE_REG,
        HILINK4_LANE3_CTLEDFE_REG,
    };

    unsigned int data = 0;

    if (!mac_dev) {
        pr_err("mac_release_hilink_ctledfe mac dev is null\n");
        return -EINVAL;
    }
    if (!mac_dev->serdes_vaddr) {
        log_err(mac_dev->dev,
        	"serdes_addr=%p hasnot mapped\n",
        	mac_dev->serdes_vaddr);
        return -EINVAL;
    }

    data = serdes_reg_read(
    	(u64)mac_dev->serdes_vaddr + ctle_dfe_regs[lane & 3]);
    data = data & ~(1ul << 3);
    serdes_reg_write(
    	(u64)mac_dev->serdes_vaddr + ctle_dfe_regs[lane & 3], data);

    return 0;
}


/**
 * sfp_open - open sfp
 * @netdev: net device¡£
 *
 * Return 0 on success, negative on failure
 */
extern void serdes_ctle_dfe_reset(u32 macro,u32 lane);
extern void  serdes_ctle_dfe_release_reset(u32 macro,u32 lane);

static int mac_sfp_ctl(
	char pca9545_addr, char pca9555_addr, int sfp_id, int en)
{
    union mac_half_u8 value;
	unsigned int sub_addr = 0;
    int ret = 0;

	/* need not init**/
	if ((char)SPF_CTL_INV_ADDR == pca9545_addr ||
		(char)SPF_CTL_INV_ADDR == pca9555_addr) {
		pr_debug(
	    	"sfp need not open 9545_addr=0x%x, 9555_addr=0x%x!\n",
	    	pca9545_addr, pca9555_addr);
		return 0;
	}
    /*disable PCA9545*/
    value.u8 = 0;
    ret = mac_do_i2c_xfer(1,
        pca9545_addr, 0, 0,
        (char *)&value.u8, 1, 0, "PCA9545");
    if (ret < 0)
	    pr_info(
	    	"dis_9545, ret = %#x!\n", ret);

	/*this delay is for PCA9555 slc vld*/
	mdelay(10);
    /**PCA9555 configures the directions of the I/O pins*/
    if (sfp_id & 0x2)
    	sub_addr = PCA9555_REG7;
    else
    	sub_addr = PCA9555_REG6;
	ret |= mac_do_i2c_xfer(1,
        pca9555_addr, sub_addr, 0x1,
        (char *)&value.u8, 1, 1, "PCA9555");
    if (ret < 0)
    	pr_info(
    		"read pca9555 reg0x%x, ret = %#x!\n", sub_addr, ret);
    if (sfp_id & 0x1)
    	value.bits.hig_4bits = 0x7;
    else
    	value.bits.low_4bits = 0x7;
	ret |= mac_do_i2c_xfer(1,
        pca9555_addr, sub_addr, 0x0,
        (char *)&value.u8, 1, 1, "PCA9555");
    if (ret < 0)
    	pr_info(
    		"write pca9555 reg0x%x, ret = %#x!\n", sub_addr, ret);


	/**write 2 and 3 for out port, 0 is enable*/
    if (sfp_id & 0x2)
    	sub_addr = PCA9555_REG3;
    else
    	sub_addr = PCA9555_REG2;
	ret |= mac_do_i2c_xfer(1,
        pca9555_addr, sub_addr, 0x1,
        (char *)&value.u8, 1, 1, "PCA9555");
    if (ret < 0)
    	pr_info(
    		"read pca9555 reg0x%x, ret = %#x!\n", sub_addr, ret);
    if (sfp_id & 0x1)
    	value.bits.hig_4bits = (en ? 0x0 : 0x8);
    else
    	value.bits.low_4bits = (en ? 0x0 : 0x8);
	ret |= mac_do_i2c_xfer(1,
        pca9555_addr, sub_addr, 0x0,
        (char *)&value.u8, 1, 1, "PCA9555");
    if (ret < 0)
    	pr_info(
    		"write pca9555 reg0x%x, ret = %#x!\n", sub_addr, ret);

    return ret;

}

static int mac_update_sfp_prsnt(struct mac_device * mac_dev)
{
    if (!mac_dev->cpld_vaddr) {
        log_err(mac_dev->dev,
            "get_sfp_prsnt mac_id=%d, cpld_vaddr is null !\n",
            mac_dev->mac_id);
        return -1;
    }
	/* prsnt_reg : 1 not present, 0 present**/
	mac_dev->sfp_prsnt = !cpld_reg_read(
		(u64)mac_dev->cpld_vaddr + MAC_SFP_PORT_OFFSET);

	return 0;
}

int mac_sfp_prsnt(struct mac_device *mac_dev)
{
	int ret = 0;
	/*sfp_id 0, 1, 2, 3  <==> mac_id 0, 2, 1, 3*/
	static const int sfp_id_arry[] = {0, 2, 1, 3};
	int sfp_id = 0;
	unsigned char old_sfp_prsnt = 0;
	char pca9545;
	char pca9555;

	if (!mac_dev) {
		pr_err("sfp_open mac dev is null\n");
		return -1;
	}

	if ((!mac_dev->cpld_vaddr) || (!mac_dev->serdes_vaddr)) {
		log_info(mac_dev->dev,
			"mac_id%d, phy_if%d need not sfp!\n",
			mac_dev->mac_id, mac_dev->phy_if);
		return 0;
	}

	old_sfp_prsnt = mac_dev->sfp_prsnt;
    ret = mac_update_sfp_prsnt(mac_dev);
    if (ret) {
		log_err(mac_dev->dev,
			"get sfp prsnt error\n");
		return ret;
	}
	/* init sfp id**/
	sfp_id = sfp_id_arry[mac_dev->mac_id & 3];

	pca9545 = mac_dev->spf_ctl_addr[0];
	pca9555 = mac_dev->spf_ctl_addr[1];
	if (old_sfp_prsnt != mac_dev->sfp_prsnt) {
		if (mac_dev->sfp_prsnt) {
			(void)mac_sfp_ctl(pca9545, pca9555, sfp_id, 1);
			/*this delay is for hilink release reset ctle/dfe*/
			mdelay(20);
			ret = mac_release_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
			if (ret)
				return ret;
			log_info(mac_dev->dev,
				"sfp_prsnt=%d release ctledfe ok!\n",
				mac_dev->sfp_prsnt);
		} else {
			ret = mac_reset_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
			if (ret)
				return ret;
			log_info(mac_dev->dev,
				"sfp_prsnt=%d reset ctledfe ok!\n",
				mac_dev->sfp_prsnt);
			(void)mac_sfp_ctl(pca9545, pca9555, sfp_id, 0);
		}
	}

	return ret;
}

int mac_sfp_open(struct mac_device *mac_dev)
{
	int ret = 0;
	/*sfp_id 0, 1, 2, 3  <==> mac_id 0, 2, 1, 3*/
	static const int sfp_id_arry[] = {0, 2, 1, 3};
	int sfp_id = 0;
	char pca9545;
	char pca9555;

	if (!mac_dev) {
		pr_err("sfp_open mac dev is null\n");
		return -1;
	}

	if ((!mac_dev->cpld_vaddr) || (!mac_dev->serdes_vaddr)) {
		log_info(mac_dev->dev,
			"mac_id%d, phy_if%d need not sfp!\n",
			mac_dev->mac_id, mac_dev->phy_if);
		return 0;
	}
    ret = mac_update_sfp_prsnt(mac_dev);
    if (ret) {
		log_err(mac_dev->dev,
			"get sfp prsnt error\n");
		return ret;
	}
	/* init sfp id**/
	sfp_id = sfp_id_arry[mac_dev->mac_id & 3];

	pca9545 = mac_dev->spf_ctl_addr[0];
	pca9555 = mac_dev->spf_ctl_addr[1];
	if (mac_dev->sfp_prsnt) {
		ret = mac_reset_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
		if (ret)
			return ret;
		log_info(mac_dev->dev,
			"sfp_open=%d reset ctledfe ok!\n",
			mac_dev->sfp_prsnt);

		(void)mac_sfp_ctl(pca9545, pca9555, sfp_id, 1);
		/*this delay is for hilink release reset ctle/dfe*/
		mdelay(20);
		ret = mac_release_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
		if (ret)
			return ret;
		log_info(mac_dev->dev,
			"sfp_open=%d release ctledfe ok!\n",
			mac_dev->sfp_prsnt);
	} else {
		ret = mac_reset_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
		if (ret)
			return ret;
		log_info(mac_dev->dev,
			"sfp_open=%d reset ctledfe ok!\n",
			mac_dev->sfp_prsnt);
		(void)mac_sfp_ctl(pca9545, pca9555, sfp_id, 0);
	}

    return ret;
}
int mac_sfp_close(struct mac_device *mac_dev)
{
	int ret = 0;
	/*sfp_id 0, 1, 2, 3  <==> mac_id 0, 2, 1, 3*/
	static const int sfp_id_arry[] = {0, 2, 1, 3};
	int sfp_id = 0;
	char pca9545;
	char pca9555;

	if (!mac_dev) {
		pr_err("sfp_open mac dev is null\n");
		return -1;
	}
	if ((!mac_dev->cpld_vaddr) || (!mac_dev->serdes_vaddr)) {
		log_info(mac_dev->dev,
			"mac_id%d, phy_if%d need not sfp!\n",
			mac_dev->mac_id, mac_dev->phy_if);
		return 0;
	}
	/* init sfp id**/
	sfp_id = sfp_id_arry[mac_dev->mac_id & 3];

	pca9545 = mac_dev->spf_ctl_addr[0];
	pca9555 = mac_dev->spf_ctl_addr[1];
	ret = mac_reset_hilink_ctledfe(mac_dev, 4, mac_dev->mac_id);
	if (ret)
		return ret;
	log_info(mac_dev->dev,
		"sfp_close=%d reset ctledfe ok!\n",
		mac_dev->sfp_prsnt);
	(void)mac_sfp_ctl(pca9545, pca9555, sfp_id, 0);

	return ret;
}





