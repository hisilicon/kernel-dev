/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe48wNwMqH78/VlG0EabmlVRGy7L0B9wpcvm+tA3mUPGYFmwaj5pLXn5cu1vZb6uX4JJc5C
L8d0ofLTs6sRNwbLjhAb77sy+WTs1Pin2G/SdjbncCrFaCprD+AyojFYSQeg9Q==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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

/* #include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_error.h"
#include "iware_log.h"
#include "iware_dsaf_main.h"
#include "iware_mac_main.h"
#include "iware_nic_main.h"
#include "iware_mac_hal.h"
#include "iware_led_hal.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>

#define MAC_EN_FLAG_V	   0xada0328

const enum mac_mode g_mac_mode_100[] = {
	[MAC_PHY_INTERFACE_MODE_MII]	= MAC_MODE_MII_100,
	[MAC_PHY_INTERFACE_MODE_RMII]   = MAC_MODE_RMII_100
};

static const enum mac_mode g_mac_mode_1000[] = {
	[MAC_PHY_INTERFACE_MODE_GMII]   = MAC_MODE_GMII_1000,
	[MAC_PHY_INTERFACE_MODE_SGMII]  = MAC_MODE_SGMII_1000,
	[MAC_PHY_INTERFACE_MODE_TBI]	= MAC_MODE_TBI_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII]  = MAC_MODE_RGMII_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_ID]   = MAC_MODE_RGMII_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_RXID] = MAC_MODE_RGMII_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_TXID] = MAC_MODE_RGMII_1000,
	[MAC_PHY_INTERFACE_MODE_RTBI]   = MAC_MODE_RTBI_1000
};

/*max vm by dsaf*/
static const u8 g_mac_max_vm[] = {
	[DSAF_MODE_INVALID] = 0,
	[DSAF_MODE_ENABLE_FIX] = 1,
	[DSAF_MODE_ENABLE_0VM] = 1,
	[DSAF_MODE_ENABLE_8VM] = 8,
	[DSAF_MODE_ENABLE_16VM] = 16,
	[DSAF_MODE_ENABLE_32VM] = 32,
	[DSAF_MODE_ENABLE_128VM] = 128,
	[DSAF_MODE_ENABLE] = 0,
	[DSAF_MODE_DISABLE_FIX] = 1,
	[DSAF_MODE_DISABLE_2PORT_8VM] = 8,
	[DSAF_MODE_DISABLE_2PORT_16VM] = 16,
	[DSAF_MODE_DISABLE_2PORT_64VM] = 64,
	[DSAF_MODE_DISABLE_6PORT_0VM] = 1,
	[DSAF_MODE_DISABLE_6PORT_2VM] = 2,
	[DSAF_MODE_DISABLE_6PORT_4VM] = 4,
	[DSAF_MODE_DISABLE_6PORT_16VM] = 16
};

static enum mac_mode mac_dev_to_enet_interface
			(const struct mac_device *mac_dev)
{
	switch (mac_dev->max_speed) {
	case MAC_SPEED_100:
		return g_mac_mode_100[mac_dev->phy_if];
	case MAC_SPEED_1000:
		return g_mac_mode_1000[mac_dev->phy_if];
	case MAC_SPEED_10000:
		return MAC_MODE_XGMII_10000;
	default:
		return MAC_MODE_MII_100;
	}
}

/**
 *mac_exception - exception for mac
 *@in_mac_dev: mac device
 *@exception: mac exception
 */
static void mac_exception(void * in_mac_dev,
			enum mac_exceptions exception)
{
	struct mac_device *mac_dev = (struct mac_device *)in_mac_dev;

	log_dbg(mac_dev->dev, "mac_init config mac faild,mac%d dsaf%d!\n",
		mac_dev->mac_id, mac_dev->chip_id);
}

/**
 *mac_virturl_free - free mac device
 *@in_mac_dev: mac device
 *@exception: mac exception
 */
static int mac_virturl_free(void * mac_drv)
{
	osal_kfree(mac_drv);
	return 0;
}

/**
 *mac_virturl_config - config virturl mac param
 *@mac_param: mac params
 * return : mac device
 */
static void * mac_virturl_config(struct mac_params *mac_param)
{
	struct mac_driver *mac_drv = NULL;

	mac_drv = (struct mac_driver *)osal_kmalloc(sizeof(*mac_drv),
					GFP_KERNEL);
	if (NULL == mac_drv) {
		log_err(mac_param->dev,
			"mac_virturl_config osal_kmalloc faild!\n");
		return NULL;
	}

	memset(mac_drv, 0, sizeof(struct mac_driver));
	mac_drv->mac_free = mac_virturl_free;
	mac_drv->chip_id = mac_param->chip_id;
	mac_drv->mac_id = mac_param->mac_id;
	mac_drv->mac_mode = mac_param->mac_mode;
	mac_drv->vaddr = mac_param->vaddr;
	mac_drv->dev = mac_param->dev;

	return (void *)mac_drv;
}

static int mac_start(struct mac_device *mac_dev)
{
	int ret;
	struct phy_device *phy_dev = mac_dev->phy_dev;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_drv = (struct mac_driver *)(priv->mac);

	/*modified for virt */
	if (MAC_EN_FLAG_V == mac_drv->mac_en_flg) {
		log_warn(mac_dev->dev, "Enable mac device again, just warnning\n");

		/*plus 1 when the virtual mac has been enabled */
		mac_drv->virt_dev_num += 1;
		return 0;
	}

	log_dbg(mac_dev->dev, "addr %#llx dsaf%d mac%d\n",
		(u64)mac_drv->vaddr, mac_drv->chip_id, mac_drv->mac_id);

	if (NULL != mac_drv->mac_enable) {
		ret = mac_drv->mac_enable(priv->mac, MAC_COMM_MODE_RX_AND_TX);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_start mac_enable faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
		mac_drv->mac_en_flg = MAC_EN_FLAG_V;
	}

	if (phy_dev)
		phy_start(phy_dev);

	return 0;
}

static int mac_stop(struct mac_device *mac_dev)
{
	int ret = 0;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (mac_dev->phy_dev) {
	    phy_stop(mac_dev->phy_dev);
	    /*disconnect phy, for nic can up again*/
	    phy_disconnect(mac_dev->phy_dev);
	    mac_dev->phy_dev = NULL;
	}

	/*modified for virtualization */
	if (mac_ctrl_drv->virt_dev_num > 0) {
		mac_ctrl_drv->virt_dev_num -= 1;
		if (mac_ctrl_drv->virt_dev_num > 0)
			return 0;
	}

	if (NULL != mac_ctrl_drv->mac_disable) {
		ret = mac_ctrl_drv->mac_disable(priv->mac,
			MAC_COMM_MODE_RX_AND_TX);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_stop mac_disable faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	mac_ctrl_drv->mac_en_flg = 0;
	mac_dev->link = 0;

	return 0;
}

/**
 *mac_queue_config_bc_en - set broadcast rx&tx enable
 *@mac_dev: mac device
 *@queue: queue number
 *@en:enable
 */
static int mac_port_config_bc_en(struct mac_device *mac_dev,
		u32 port_num, u16 vlan_id, u8 en)
{
	int ret = 0;
	struct dsaf_device *dsaf_dev = mac_dev->dsaf_dev;
	u8 addr[MAC_NUM_OCTETS_PER_ADDR]
		= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct dsaf_drv_mac_single_dest_entry mac_entry;

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->add_mac_mc_port)
		&& (NULL != dsaf_dev->add_mac_mc_port)) {
		memcpy(mac_entry.addr, addr, sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = vlan_id;
		mac_entry.in_port_num = mac_dev->mac_id;
		mac_entry.port_num = port_num;

		if (DISABLE == en) {
			ret = dsaf_dev->del_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"del_mac_mc_port faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		} else {
			ret = dsaf_dev->add_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"add_mac_mc_port faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		}
	}

	return 0;
}



/**
 *mac_set_multi - set multi mac address
 *@mac_dev: mac device
 */
static int  mac_set_multi(struct mac_device *mac_dev,
			u32 port_num, const u8 addr[MAC_NUM_OCTETS_PER_ADDR], u8 en)
{
	int ret = 0;
	struct dsaf_device *dsaf_dev = mac_dev->dsaf_dev;
	struct dsaf_drv_mac_single_dest_entry mac_entry;

	if ((NULL != dsaf_dev)
		&& (NULL != dsaf_dev->add_mac_mc_port)
		&& (NULL != addr)) {
		memcpy(mac_entry.addr, addr, sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = 0;/*vlan_id;*/
		mac_entry.in_port_num = mac_dev->mac_id;
		mac_entry.port_num = port_num;

		if (DISABLE == en) {
			ret = dsaf_dev->del_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"del_mac_mc_port faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		} else {
			ret = dsaf_dev->add_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"add_mac_mc_port faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		}
	}

	return 0;
}


static int mac_reset(struct mac_device *mac_dev)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_drv = NULL;

	log_dbg(mac_dev->dev, "enter mac_init_config!\n");
	priv = mac_dev_priv(mac_dev);
	mac_drv = (struct mac_driver *)(priv->mac);

	ret = mac_drv->mac_init(priv->mac);
	if (ret) {
		log_err(mac_dev->dev,
			"mac_init mac_init faild,mac%d dsaf%d ret = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		return ret;
	}

	if (NULL != mac_drv->mac_config_max_frame_length) {
		ret = mac_drv->mac_config_max_frame_length(priv->mac,
			mac_dev->max_frm);
		if (ret) {
			log_err(mac_dev->dev,
				"config mtu faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	if (NULL != mac_drv->mac_set_tx_auto_pause_frames) {
		ret = mac_drv->mac_set_tx_auto_pause_frames(priv->mac,
			mac_dev->tx_pause_frm_time);
		if (ret) {
			log_err(mac_dev->dev,
				"set pause frames faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	if (NULL != mac_drv->mac_set_an_mode) {
		ret = mac_drv->mac_set_an_mode(priv->mac, 1);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_set_an_mode faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	if (NULL != mac_drv->mac_adjust_link) {
		ret = mac_drv->mac_adjust_link(priv->mac,
			(enum mac_speed)mac_dev->speed, !mac_dev->half_duplex);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_adjust_link faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	return 0;
}


static int mac_init(struct mac_device *mac_dev)
{
	int ret = -1;
	int ret_tmp = -1;
	struct mac_priv *priv = NULL;
	struct mac_params param;
	struct mac_driver *mac_ctrl_drv = NULL;

	log_dbg(mac_dev->dev, "enter mac_init!\n");
	priv = mac_dev_priv(mac_dev);

	param.vaddr = mac_dev->vaddr;
	param.mac_mode = mac_dev_to_enet_interface(mac_dev);
	memcpy(param.addr, mac_dev->addr_entry_idx[0].addr,
				MAC_NUM_OCTETS_PER_ADDR);
	param.mac_id = mac_dev->mac_id;
	param.chip_id = mac_dev->chip_id;

	param.exception_cb = mac_exception;
	param.event_cb = mac_exception;
	param.app_cb = mac_dev;
	param.dev = mac_dev->dev;

	log_dbg(mac_dev->dev, "chip%d mac%d vaddr = %#llx param.vaddr=%#llx\n",
		mac_dev->chip_id, mac_dev->mac_id,
		(u64)mac_dev->vaddr, (u64)param.vaddr);

	/*init mac struct,need consider dsaf  TBD */
	if ((MAC_MAX_PORT_NUM_PER_CHIP) != mac_dev->mac_id) {
		log_dbg(mac_dev->dev, "postion0\n");
		if (MAC_SPEED_FROM_MODE(param.mac_mode) < MAC_SPEED_10000)
			mac_ctrl_drv = (struct mac_driver *)gmac_config(&param);
		else
			mac_ctrl_drv = (struct mac_driver *)xgmac_config(&param);

		if (NULL == mac_ctrl_drv) {
			ret = HRD_COMMON_ERR_NULL_POINTER;
			log_err(mac_dev->dev,
				"mac_init config mac faild,mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
			return ret;
		}
	} else {
		mac_ctrl_drv = mac_virturl_config(&param);
		if (NULL == mac_ctrl_drv) {
			ret = HRD_COMMON_ERR_NULL_POINTER;
			log_err(mac_dev->dev,
				"mac_init config mac faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	priv->mac = (void *)mac_ctrl_drv;

	ret = mac_reset(mac_dev);
	if (ret) {
		log_err(mac_dev->dev,
			"mac_init mac_reset faild,mac%d dsaf%d ret = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		goto free_mac_drv;
	}

	ret = mac_port_config_bc_en(mac_dev, mac_dev->mac_id, 0, ENABLE);
	if (ret) {
		log_err(mac_dev->dev,
			"port config bc en faild, mac%d dsaf%d ret = %d!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		goto free_mac_drv;
	}

	if ((NULL != mac_dev->dsaf_dev)
		&& (mac_dev->dsaf_dev->dsaf_mode < DSAF_MODE_ENABLE)
		&& ((DSAF_MAX_PORT_NUM_PER_CHIP-1) != mac_dev->mac_id)) {
		ret = mac_start(mac_dev);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_start faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			goto free_mac_drv;
		}
	}

	log_dbg(mac_dev->dev,
		"Dsaf%d Mac%d Mac mode %s\n", mac_dev->chip_id, mac_dev->mac_id,
		((mac_dev_to_enet_interface(mac_dev) != MAC_MODE_XGMII_10000) ?
		"GMAC" : "XGMAC"));

	return 0;

free_mac_drv:
	if (NULL != mac_ctrl_drv->mac_free) {
		ret_tmp = mac_ctrl_drv->mac_free(priv->mac);
		if (ret_tmp) {
			log_err(mac_dev->dev,
				"mac_free faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret_tmp);
		}
	}
	return ret;
}

/**
 *mac_change_promisc - change mac to promisc
 *@mac_dev: mac device
 */
static int mac_change_promisc(struct mac_device *mac_dev)
{
	int ret = 0;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_ctrl_drv = (struct mac_driver *)(priv->mac);
	struct dsaf_device *dsaf_dev = mac_dev->dsaf_dev;
	u8 promisc = !mac_dev->promisc;

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->set_promiscuous)) {
		ret = dsaf_dev->set_promiscuous(dsaf_dev, promisc);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_set_mac_addr faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	if (NULL != mac_ctrl_drv->mac_set_promiscuous) {
		ret = mac_ctrl_drv->mac_set_promiscuous(priv->mac, promisc);
		if (ret) {
			log_err(mac_dev->dev,
				"set_promiscuous faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	mac_dev->promisc = promisc;

	return 0;
}

/**
 *mac_get_inner_port_num - get mac table inner port number
 *@mac_dev: mac device
 *@vmid: vm id
 *@port_num:port number
 *
 */
static int mac_get_inner_port_num(struct mac_device *mac_dev,
	u8 vmid, u8 *port_num)
{
	u8 tmp_port = 0;

	if (mac_dev->dsaf_dev->dsaf_mode <= DSAF_MODE_ENABLE) {
		if ((MAC_MAX_PORT_NUM_PER_CHIP-0) != mac_dev->mac_id) {
			log_err(mac_dev->dev, "input invalid, mac%d dsaf%d vmid%d !\n",
				mac_dev->mac_id, mac_dev->chip_id, vmid);
			return HRD_COMMON_ERR_INPUT_INVALID;
		}
	} else if (mac_dev->dsaf_dev->dsaf_mode < DSAF_MODE_MAX) {
		if ((MAC_MAX_PORT_NUM_PER_CHIP-0) <= mac_dev->mac_id) {
			log_err(mac_dev->dev, "input invalid,mac%d dsaf%d vmid%d!\n",
				mac_dev->mac_id, mac_dev->chip_id, vmid);
			return HRD_COMMON_ERR_INPUT_INVALID;
		}
	}
	else {
		log_err(mac_dev->dev, "dsaf mode invalid,mac%d dsaf%d !\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return HRD_COMMON_ERR_UNKNOW_MODE;
	}

	if (vmid >= g_mac_max_vm[mac_dev->dsaf_dev->dsaf_mode]) {
		log_err(mac_dev->dev, "input invalid, mac%d dsaf%d vmid%d !\n",
			mac_dev->mac_id, mac_dev->chip_id, vmid);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	switch (mac_dev->dsaf_dev->dsaf_mode) {
	case DSAF_MODE_ENABLE_FIX:
		tmp_port = 0;
		break;
	case DSAF_MODE_DISABLE_FIX:
		tmp_port = 0;
		break;
	case DSAF_MODE_ENABLE_0VM:
	case DSAF_MODE_ENABLE_8VM:
	case DSAF_MODE_ENABLE_16VM:
	case DSAF_MODE_ENABLE_32VM:
	case DSAF_MODE_ENABLE_128VM:
	case DSAF_MODE_DISABLE_2PORT_8VM:
	case DSAF_MODE_DISABLE_2PORT_16VM:
	case DSAF_MODE_DISABLE_2PORT_64VM:
	case DSAF_MODE_DISABLE_6PORT_0VM:
	case DSAF_MODE_DISABLE_6PORT_2VM:
	case DSAF_MODE_DISABLE_6PORT_4VM:
	case DSAF_MODE_DISABLE_6PORT_16VM:
		tmp_port = vmid;
		break;
	default:
		log_err(mac_dev->dev, "dsaf mode invalid,mac%d dsaf%d !\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return HRD_COMMON_ERR_UNKNOW_MODE;
	}
	tmp_port += DSAF_BASE_INNER_PORT_NUM;

	log_dbg(mac_dev->dev,
		"mac_get_inner_port_num mac%d dsaf%d port_num%d !\n",
		mac_dev->mac_id, mac_dev->chip_id, tmp_port);
	*port_num = tmp_port;

	return 0;
}


/**
 *mac_get_inner_port_num - change vf mac address
 *@mac_dev: mac device
 *@queue: queue number
 *@addr:mac address
 */
static int mac_change_vf_addr(struct mac_device *mac_dev,
			u32 queue, char *addr)
{
	int ret;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_ctrl_drv = (struct mac_driver *)(priv->mac);
	struct dsaf_device *dsaf_dev = mac_dev->dsaf_dev;
	u8 physical_port_id = 0;
	u8 vmid = (u8)queue;
	struct dsaf_drv_mac_single_dest_entry mac_entry;
	struct mac_entry_idx *old_entry;

	old_entry = &(mac_dev->addr_entry_idx[queue]);
	if ((NULL != dsaf_dev)
		&& (NULL != dsaf_dev->del_mac_entry)
		&& (NULL != dsaf_dev->set_mac_uc_entry)) {
		memcpy(mac_entry.addr, addr, sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = old_entry->vlan_id;
		mac_entry.in_port_num = mac_dev->mac_id;
		ret = mac_get_inner_port_num(mac_dev, vmid,
				&mac_entry.port_num);
		if (ret) {
			log_err(mac_dev->dev,
				"get_inner_port_num faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}

		if ((old_entry->valid != 0) &&
			(memcmp(old_entry->addr, addr, sizeof(mac_entry.addr)) != 0)) {
			ret = dsaf_dev->del_mac_entry(dsaf_dev,
				old_entry->vlan_id,
				mac_dev->mac_id, old_entry->addr);
			if (ret) {
				log_err(mac_dev->dev,
					"mac_change_addr faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		}

		ret = dsaf_dev->set_mac_uc_entry(dsaf_dev, &mac_entry);
		if (ret) {
			log_err(mac_dev->dev,
				"set_mac_uc_entry faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	if (NULL != mac_ctrl_drv->mac_set_mac_addr) {
		ret = mac_ctrl_drv->mac_set_mac_addr(priv->mac, addr);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_set_mac_addr faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}

	if ((0 == vmid) && (NULL != mac_dev->change_addr_cb)) {
		physical_port_id
			= (mac_dev->mac_id == (DSAF_MAX_PORT_NUM_PER_CHIP-1)
				? 0 : mac_dev->mac_id);
		ret = mac_dev->change_addr_cb(mac_dev->roce_dev, physical_port_id,
			addr);
		if (ret) {
			log_err(mac_dev->dev,
				"change_addr_cb faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	memcpy(old_entry->addr, addr, sizeof(old_entry->addr));
	old_entry->valid = 1;
	return 0;
}

/**
 *mac_queue_config_bc_en - set broadcast rx&tx enable
 *@mac_dev: mac device
 *@queue: queue number
 *@en:enable
 */
static int mac_queue_config_bc_en(struct mac_device *mac_dev,
			u32 queue, u8 en)
{
	int ret;
	struct dsaf_device *dsaf_dev = mac_dev->dsaf_dev;
	u8 vmid = (u8)queue;
	u8 port_num;
	u8 addr[MAC_NUM_OCTETS_PER_ADDR]
		= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct mac_entry_idx *uc_mac_entry = NULL;
	struct dsaf_drv_mac_single_dest_entry mac_entry;

	uc_mac_entry = &(mac_dev->addr_entry_idx[vmid]);

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->add_mac_mc_port)
		&& (NULL != dsaf_dev->add_mac_mc_port)) {
		memcpy(mac_entry.addr, addr, sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = uc_mac_entry->vlan_id;
		mac_entry.in_port_num = mac_dev->mac_id;
		ret = mac_get_inner_port_num(mac_dev, vmid, &port_num);
		if (ret) {
			log_err(mac_dev->dev,
			"mac_get_inner_port_num faild,mac%d dsaf%d ret=%#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
		mac_entry.port_num = port_num;

		if (DISABLE == en) {
			ret = dsaf_dev->del_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"set_mac_mc_entry faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		} else {
			ret = dsaf_dev->add_mac_mc_port(dsaf_dev, &mac_entry);
			if (ret) {
				log_err(mac_dev->dev,
					"set_mac_mc_entry faild,mac%d dsaf%d ret = %#x!\n",
					mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			}
		}
	}

	return 0;
}



/**
 *mac_get_vf_mac - get mac address
 *@mac_dev: mac device
 *@queue: queue number
 *@mac:mac address
 */
int mac_get_vf_mac(struct mac_device *mac_dev, u32 queue, char *mac)
{
	struct mac_entry_idx *old_mac_entry = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_dev is NULL, mac dsaf!\n");
		return -EINVAL;
	}
	if (queue < MAC_MAX_VM_NUM)
		old_mac_entry = &(mac_dev->addr_entry_idx[queue]);
	else {
		log_err(mac_dev->dev,
			"vf queue is too large, mac%d dsaf%d queue = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, queue);
		  return -EINVAL;
	}

	if (NULL != old_mac_entry->addr)
		memcpy(mac, old_mac_entry->addr, sizeof(old_mac_entry->addr));
	else {
		log_err(mac_dev->dev, "vf mac is NULL.\n");
	}

	return 0;
}

static int mac_change_addr(struct mac_device *mac_dev, char *addr)
{
	int ret;
	u8 vmid = 0;

	ret = mac_change_vf_addr(mac_dev, vmid, addr);
	if (ret) {
		log_err(mac_dev->dev,
			"mac_change_vf_addr faild,mac%d dsaf%d ret = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		return ret;
	}

	return 0;
}

static int mac_get_addr(struct mac_device *mac_dev, char *addr)
{
	struct mac_entry_idx *mac_entry = NULL;

	mac_entry = &(mac_dev->addr_entry_idx[0]);
	memcpy(addr, mac_entry->addr, MAC_NUM_OCTETS_PER_ADDR);
	return 0;
}

/**
 *mac_adjust_link - adjust net work mode by the phy stat or new param
 *@net_dev: net device
 */
void mac_adjust_link(struct net_device *net_dev)
{
	int ret;
	struct nic_device *priv = NULL;
	struct mac_device *mac_dev = NULL;
	struct mac_priv *mac_priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;
	struct phy_device *phy_dev = NULL;

	priv = netdev_priv(net_dev);
	mac_dev = priv->mac_dev;
	phy_dev = mac_dev->phy_dev;
	mac_priv = mac_dev_priv(mac_dev);
	mac_ctrl_drv = (struct mac_driver *)(mac_priv->mac);

	mac_dev->speed = phy_dev->speed;
	mac_dev->half_duplex = !phy_dev->duplex;
	mac_ctrl_drv->mac_mode = mac_dev_to_enet_interface(mac_dev);

	if (NULL != mac_ctrl_drv->mac_adjust_link) {
		ret = mac_ctrl_drv->mac_adjust_link(mac_priv->mac,
			(enum mac_speed)phy_dev->speed, phy_dev->duplex);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_adjust_link faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
		}
	}
}

/**
 *mac_init_gmac_phy - init phy of gmac ,bingding the phy to mac
 *@net_dev: net device
 */
static int mac_init_gmac_phy(struct net_device *net_dev)
{
	int ret;
	struct nic_device *priv = NULL;
	struct mac_device *mac_dev = NULL;
	struct phy_device *phy_dev = NULL;
	struct device *dev = NULL;

	priv = netdev_priv(net_dev);
	mac_dev = priv->mac_dev;

	dev = (struct device *)mac_dev->dev;
	if (mac_dev->phy_node == NULL) {
		if (of_phy_is_fixed_link(dev->of_node)) {
			ret = of_phy_register_fixed_link(dev->of_node);
			if (ret) {
				log_err(dev,
					"of_phy_register_fixed_link faild,mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
				return ret;
			}
			mac_dev->phy_node = dev->of_node;
		} else {
			log_err(dev, "has not phy and fixed_link, mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
			return 0;/*-EINVAL;*/
		}

		phy_dev = of_phy_attach(net_dev, mac_dev->phy_node,
					0, (phy_interface_t)mac_dev->phy_if);

		if (unlikely(phy_dev == NULL) || IS_ERR(phy_dev)) {
			log_err(dev,
				"phy_attach faild, mac%d dsaf%d phy %s!\n",
				mac_dev->mac_id, mac_dev->chip_id, mac_dev->phy_bus_id);
			return phy_dev == NULL ? -ENODEV : PTR_ERR(phy_dev);
		}

	} else {
		phy_dev = of_phy_connect(net_dev, mac_dev->phy_node, mac_adjust_link,
					0, (phy_interface_t)mac_dev->phy_if);
		if (unlikely(phy_dev == NULL) || IS_ERR(phy_dev)) {
			log_err(dev,
			"gmac_init_phy phy_connect faild,mac%d dsaf%d phy %s!\n",
			mac_dev->mac_id, mac_dev->chip_id, mac_dev->phy_bus_id);
			return phy_dev == NULL ? -ENODEV : PTR_ERR(phy_dev);
		}
	}

	phy_dev->supported &= priv->mac_dev->if_support;
	phy_dev->advertising = phy_dev->supported;
	mac_dev->phy_dev = phy_dev;
	log_dbg(dev, "mac%d mac_dev=%#llx, phy_dev=%#llx\n",
		mac_dev->global_mac_id, (u64)mac_dev, (u64)phy_dev);

	return 0;
}

int mac_init_xgmac_phy(struct net_device *net_dev)
{
	struct nic_device *priv = NULL;
	struct mac_device *mac_dev = NULL;
	struct phy_device *phy_dev = NULL;

	priv = netdev_priv(net_dev);
	mac_dev = priv->mac_dev;

	log_dbg(mac_dev->dev,
		"[mac_init_xgmac_phy]mac_dev=%p,mac_dev->phy_node=%p\n",
		mac_dev, (mac_dev ? mac_dev->phy_node : 0));

    	/*fixed-link donot support XG, so return success**/
	if (mac_dev->phy_node == NULL)
		return 0;

	phy_dev = of_phy_attach(net_dev, mac_dev->phy_node, 0,
			(phy_interface_t)mac_dev->phy_if);

	if (unlikely(phy_dev == NULL) || IS_ERR(phy_dev)) {
		log_err(mac_dev->dev,
			"phy_attach faild, mac%d dsaf%d phy %s!\n",
			mac_dev->mac_id, mac_dev->chip_id, mac_dev->phy_bus_id);
		return phy_dev == NULL ? -ENODEV : PTR_ERR(phy_dev);
	}

	phy_dev->supported &= priv->mac_dev->if_support;
	phy_dev->advertising = phy_dev->supported;
	mac_dev->phy_dev = phy_dev;

	return 0;
}

static int mac_change_mtu(struct mac_device *mac_dev, u32 new_mtu)
{
	int ret = 0;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_drv = (struct mac_driver *)(priv->mac);

	mac_dev->max_frm = new_mtu;

	if (NULL != mac_drv->mac_config_max_frame_length) {
		ret = mac_drv->mac_config_max_frame_length(priv->mac, new_mtu);
		if (ret) {
			log_err(mac_dev->dev,
				"change_mtu faild, mac%d dsaf%d mtu[%d] ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, new_mtu, ret);
			return ret;
		}
	}
	return 0;
}

static int mac_uninit(struct mac_device *mac_dev)
{
	int ret = 0;
	int tmp_ret = 0;
	const struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_drv->mac_disable) {
		ret = mac_drv->mac_disable(priv->mac, MAC_COMM_MODE_RX_AND_TX);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_disable faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
		}
	}
	if (NULL != mac_drv->mac_free) {
		tmp_ret = mac_drv->mac_free(priv->mac);
		if (tmp_ret) {
			log_err(mac_dev->dev,
				"mac_free faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, tmp_ret);
		}
	}

	return ((ret == 0) ? tmp_ret : ret);
}



/**
 *mac_show_statistics - show statistics of MAC about rx and tx packets
 *@net_dev: net device
 */
int mac_show_statistics(struct mac_device *mac_dev)
{
	int ret;
	struct mac_priv *priv = mac_dev_priv(mac_dev);
	struct mac_driver *mac_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_drv->mac_get_statistics) {
		ret = mac_drv->mac_get_statistics(mac_dev, NULL, 0);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_get_statistics	faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	return 0;
}
EXPORT_SYMBOL(mac_show_statistics);


/**
 *mac_set_vf_vlan - set address of vf
 *@net_dev: net device
 *@queue:  vf queue
 *@vlan  :   vf lan
 *@qos   :   vf addr
 */
static int mac_set_vf_vlan(struct mac_device *mac_dev,
		u32 queue, u16 vlan, u8 qos)
{
	int ret;
	struct dsaf_device *dsaf_dev = NULL;
	u8 vmid = (u8)queue;
	struct mac_entry_idx *old_mac_entry = NULL;
	struct dsaf_drv_mac_single_dest_entry mac_entry;

	if (NULL == mac_dev) {
		pr_err("set_vf_vlan faild,mac_dev is NULL!\n");
	 	return -EINVAL;
	}

	dsaf_dev = mac_dev->dsaf_dev;

	if (queue < MAC_MAX_VM_NUM)
		old_mac_entry = &(mac_dev->addr_entry_idx[queue]);
	else {
		log_err(mac_dev->dev,
			"vf queue is too large, mac%d dsaf%d queue = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, queue);
		 return -EINVAL;
	}

	if ((NULL != dsaf_dev)
		&& (NULL != dsaf_dev->set_mac_uc_entry)) {
		memcpy(mac_entry.addr, old_mac_entry->addr,
			sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = vlan;
		mac_entry.in_port_num = mac_dev->mac_id;

		ret = mac_get_inner_port_num(mac_dev, vmid,
			&mac_entry.port_num);

		if (ret) {
			log_err(mac_dev->dev,
				"get_inner_port_num faild,mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}

		ret = dsaf_dev->set_mac_uc_entry(dsaf_dev, &mac_entry);
		if (ret) {
			log_err(mac_dev->dev,
				"set_mac_uc_entry faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
	}
	old_mac_entry->vlan_id = vlan;
	old_mac_entry->valid = 1;

	return 0;
}

static int mac_get_vf_vlan(struct mac_device *mac_dev,
		u32 queue, u16 *vlan, u8 *qos)
{
	struct mac_entry_idx *old_mac_entry = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_get_vf_vlan faild, mac_dev is NULL!\n");
		return -EINVAL;
	}

	if (queue < MAC_MAX_VM_NUM)
		old_mac_entry = &(mac_dev->addr_entry_idx[queue]);
	else {
		log_err(mac_dev->dev,
			"vf queue is too large, mac%d dsaf%d queue = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, queue);
		  return -EINVAL;
	}

	*vlan = old_mac_entry->vlan_id;
	*qos = old_mac_entry->qos;

	return 0;
}

/**
 *mac_add_mac - add mac address into dsaf table,can't add the same address twice
 *@net_dev: net device
 *@vfn  :   vf lan
 *@mac   : mac address
 * return status
 */
static int mac_add_mac(struct mac_device *mac_dev,
				u32 vfn, char *mac)
{
	struct dsaf_device *dsaf_dev = NULL;
	int ret;
	struct mac_entry_idx *old_mac_entry = NULL;
	struct dsaf_drv_mac_single_dest_entry mac_entry;

	if (NULL == mac_dev) {
		pr_err("mac_add_mac faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	dsaf_dev = mac_dev->dsaf_dev;

	if (vfn < MAC_MAX_VM_NUM)
		old_mac_entry = &(mac_dev->addr_entry_idx[vfn]);
	else {
		log_err(mac_dev->dev,
			"vf queue is too large, mac%d dsaf%d queue = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, vfn);
		  return -EINVAL;
	}

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->set_mac_uc_entry)) {
		memcpy(mac_entry.addr, mac, MAC_NUM_OCTETS_PER_ADDR);
		mac_entry.in_vlan_id = old_mac_entry->vlan_id;
		mac_entry.in_port_num = mac_dev->mac_id;
		(void)mac_get_inner_port_num(mac_dev, vfn, &mac_entry.port_num);
		ret = dsaf_dev->set_mac_uc_entry(dsaf_dev, &mac_entry);
		if (ret) {
			log_err(mac_dev->dev,
				"set_mac_uc_entry faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
		if (0 == mac_dev->addr_entry_idx[0].valid) {
			memcpy(mac_dev->addr_entry_idx[0].addr, mac,
				MAC_NUM_OCTETS_PER_ADDR);
			mac_dev->addr_entry_idx[0].valid = 1;
		}
	}

	return 0;
}

/**
 *mac_del_mac - delete mac address into dsaf table,can't delete the same address twice
 *@net_dev: net device
 *@vfn  :   vf lan
 *@mac   : mac address
 * return status
 */
static int mac_del_mac(struct mac_device *mac_dev,
				u32 vfn, char *mac)
{
	struct mac_entry_idx *old_mac = NULL;
	struct dsaf_device *dsaf_dev = NULL;
	u32 ret = 0;

	if (NULL == mac_dev) {
		pr_err("mac_del_mac faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	dsaf_dev = mac_dev->dsaf_dev;

	if (vfn < MAC_MAX_VM_NUM)
		old_mac = &(mac_dev->addr_entry_idx[vfn]);
	else {
		log_err(mac_dev->dev,
			"vf queue is too large, mac%d dsaf%d queue = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, vfn);
		  return -EINVAL;
	}

	if (NULL != dsaf_dev) {
		ret = dsaf_dev->del_mac_entry(dsaf_dev, old_mac->vlan_id,
				mac_dev->mac_id, old_mac->addr);
		if (ret) {
			log_err(mac_dev->dev,
				"set_mac_addr faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
				return ret;
			   }
		if (memcmp(old_mac->addr, mac, sizeof(old_mac->addr)) == 0)
			old_mac->valid = 0;
	}

	return 0;
}

/**
 *mac_adjust_link_dsaf - adjust mac link mode by the new speed and duplex
 *@net_dev: net device
 *@speed  :  speed
 *@full_duplex   :duplex mode
 * return status
 */
static int mac_adjust_link_dsaf(struct mac_device *mac_dev,
	u16 speed, u8 full_duplex)
{
	int ret = 0;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_adjust_link_dsaf faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	priv = mac_dev_priv(mac_dev);
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);
	if (NULL == mac_ctrl_drv->mac_adjust_link) {
		log_err(mac_dev->dev,
			"mac_adjust_link NULL, mac%d dsaf%d ret = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		return -EINVAL;
	}

	ret = mac_ctrl_drv->mac_adjust_link(priv->mac, (enum mac_speed)speed, full_duplex);
	if (ret) {
		log_err(mac_dev->dev,
			"mac_adjust_link faild, mac%d dsaf%d ret = %#x!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		return ret;
	}

	return 0;
}

/**
 *mac_set_an_mode - set auto-negoation enable of mac
 *@net_dev: net device
 *@enable : negoation enable of mac
 * return status
 */
static int mac_set_an_mode(struct mac_device *mac_dev, u8 enable)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_set_an_mode faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	priv = mac_dev_priv(mac_dev);
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->mac_set_an_mode) {
		ret = mac_ctrl_drv->mac_set_an_mode(priv->mac, enable);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_set_an_mode faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
		return ret;
		}
	} else {
		log_err(mac_dev->dev,
			"set_an_mode faild, mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return -ENOTSUPP;
	}
	return 0;
}


int mac_serdes_reg_write(struct mac_device *mac_dev,u8 macro_Id,
	u32 reg_offset , u32 high_bit,u32 low_bit,u32 val )
{
	u32 reg_cfg_max_val;
	u32 orign_reg_val;
	u32 final_val;
	u32 mask;
	u32 add;
	u64 reg_addr = 0;

	const u64 macro_offset[MAC_HILINK_MAX] = {
	    0ULL,
	    HILINK3_TO_HILINK4_OFFSET,
	};


	if(macro_Id >= MAC_HILINK_MAX){
		log_err(mac_dev->dev,"macro_Id(0x%x) is error!\n",macro_Id);
		return -EINVAL;
	}
	if (!mac_dev->serdes_vaddr) {
		log_err(mac_dev->dev,"serdes_addr is NULL!\n");
		return -EINVAL;
	}
	reg_addr = (u64)mac_dev->serdes_vaddr + macro_offset[macro_Id] +
		reg_offset;

	if(high_bit < low_bit){
		high_bit ^= low_bit;
		low_bit  ^= high_bit;
		high_bit ^= low_bit;
	}
	reg_cfg_max_val = (0x1 << (high_bit - low_bit+1)) -1;

	if ( val > reg_cfg_max_val ){
		log_err(mac_dev->dev,"val(0x%x) is error!\n",val);
		return -EINVAL;
	}

	orign_reg_val  = serdes_reg_read(reg_addr);

	mask =(~ (reg_cfg_max_val<< low_bit))& 0xffff;
	orign_reg_val &= mask;
	add = val << low_bit;
	final_val      = orign_reg_val + add;

	serdes_reg_write(reg_addr,final_val);

	return 0;
}

int mac_config_serdes_loopback(struct mac_device *mac_dev,u8 en)
{
	u8 macro = 0;
	u8 lane = 0;
	int ret = 0;
	const u8 lane_id[]={
		0,	/* mac 0 -> lane 0 */
		1,	/* mac 1 -> lane 1 */
		2,	/* mac 2 -> lane 2 */
		3,	/* mac 3 -> lane 3 */
		2,	/* mac 4 -> lane 2 */
		3,	/* mac 5 -> lane 3 */
		0,	/* mac 6 -> lane 0 */
		1 	/* mac 7 -> lane 1 */
	};
	macro   = mac_dev->mac_id <= 3 ? MAC_HILINK4 : MAC_HILINK3;
	lane 	= lane_id[mac_dev->mac_id];

	ret = mac_serdes_reg_write(mac_dev,macro,RX_CSR(lane,0),10,10,
		(u32)(!!en));

	msleep(100);

	return ret;

}

int mac_config_ge_phy_loopback(struct mac_device *mac_dev,u8 en)
{

	u16 val = 0;

	if (NULL == mac_dev->phy_dev) {
		log_err(mac_dev->dev,
		  "mac_led_opt mac_id=%d, phy_dev is null !\n",
		  mac_dev->mac_id);
		return -EINVAL;
	}
	if (NULL == mac_dev->phy_dev->bus) {
		log_err(mac_dev->dev,
		  "mac_phy_in_loop mac_id=%d,phy_dev->bus is null !\n",
		  mac_dev->mac_id);
		return -EINVAL;
	}

	if(en){

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, PHY_PAGE_REG, 2);
		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 21, 0x1046);/* speed : 1000M */

		/* Force Master */
		(void)mdiobus_write(mac_dev->phy_dev->bus,
	    		mac_dev->phy_dev->addr, 9, 0x1F00);
		/* Soft-reset */
		(void)mdiobus_write(mac_dev->phy_dev->bus,
	    		mac_dev->phy_dev->addr, 0, 0x9140);

		/* If autoneg disabled, two soft-reset operations are required */
		(void)mdiobus_write(mac_dev->phy_dev->bus,
	    		mac_dev->phy_dev->addr, 0, 0x9140);

		(void)mdiobus_write(mac_dev->phy_dev->bus,
	    		mac_dev->phy_dev->addr, 22, 0xFA);

		/* Default is 0x0400 */
		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 1, 0x418);

		/* Force 1000M Link ¡§C Default is 0x0200 */
		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 7, 0x20C);

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 22, 0);

		/* Enable MAC loop-back */
		val = (u16)mdiobus_read(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, COPPER_CONTROL_REG);
		val |= PHY_LOOP_BACK;
		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, COPPER_CONTROL_REG, val);

		msleep(100);

	}else{
		(void)mdiobus_write(mac_dev->phy_dev->bus,
	    		mac_dev->phy_dev->addr, 22, 0xFA);

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 1, 0x400);

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 7, 0x200);

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, 22, 0);


		val = (u16)mdiobus_read(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, COPPER_CONTROL_REG);

		val &= ~PHY_LOOP_BACK;

		(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, COPPER_CONTROL_REG, val);

	}


	return 0;
}

int mac_config_xge_phy_loopback(struct mac_device *mac_dev,u8 enable)
{
	/* TBD */
	#if 0
	u32 slice = 0;
	u32 line_host = XGE_PHY_LINE_LB;
	u32 near_far = XGE_PHY_NEAR_LB;
	u32 enable_ctl = enable;

	if(mac_dev->mac_id > 3){
		log_err(mac_dev->dev,
			"mac_config_xge_phy_loopback faild, mac%d dsaf%d\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return -EINVAL;
	}

	slice = mac_dev->mac_id;

	return phy_loopback_set(slice,line_host,near_far,enable_ctl);
	#endif
	return 0;
}

int mac_config_phy_loopback(struct mac_device *mac_dev,u8 en)
{

	if(MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if)
		return mac_config_xge_phy_loopback(mac_dev,en);
	else if(mac_dev->link_features & MAC_LINK_PHY)
		return mac_config_ge_phy_loopback(mac_dev,en);
	else
		return 0;
}

static int mac_config_loopback(struct mac_device *mac_dev,
	enum mac_loop_mode loop_mode)
{
	int ret = 0;
	int temp_ret = 0;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_config_loopback faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	priv = mac_dev_priv(mac_dev);
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	switch (loop_mode) {
	case MAC_LOOP_NONE:
		temp_ret = mac_ctrl_drv->mac_config_loopback(priv->mac,
			MAC_LOOP_NONE, DISABLE);
		if (temp_ret) {
			log_err(mac_dev->dev,
				"config_loopback faild, mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			ret = -EINVAL;
		 }

		temp_ret = mac_config_serdes_loopback(mac_dev, DISABLE);
		if (temp_ret) {
			log_err(mac_dev->dev,
				"mac_config_serdes_loopback(MAC_LOOP_NONE) faild"
				"mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			ret = -EINVAL;
		}

		temp_ret = mac_config_phy_loopback(mac_dev, DISABLE);
		if (temp_ret) {
			log_err(mac_dev->dev,
				"mac_config_phy_loopback(MAC_LOOP_NONE) faild"
				"mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, temp_ret);
			ret = -EINVAL;
		}
		return ret;

	case MAC_INTERNALLOOP_SERDES:
		return mac_config_serdes_loopback(mac_dev, ENABLE);
	case MAC_INTERNALLOOP_PHY:
		return mac_config_phy_loopback(mac_dev, ENABLE);
	default:
		mac_dev->loop_mode = loop_mode;
		ret = mac_ctrl_drv->mac_config_loopback(priv->mac, loop_mode,
			ENABLE);
		if (ret) {
			log_err(mac_dev->dev,
				"config_loopback faild,	mac%d dsaf%d ret = %#x!\n",
				mac_dev->mac_id, mac_dev->chip_id, ret);
			return ret;
		}
		break;
	}

	return 0;
}


/**
 *mac_get_info - get mac infomation
 *@net_dev: net device
 *@mac_info : mac information
 * return status
 */
static int mac_hal_get_info(struct mac_device *mac_dev,
		struct mac_info *mac_info)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_get_info faild, mac_dev is NULL!\n");
		return -EINVAL;
	}

	priv = mac_dev_priv(mac_dev);
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	ret = mac_ctrl_drv->get_info(priv->mac, mac_info);
	if (ret) {
		log_err(mac_dev->dev,
			"get_info faild, mac%d dsaf%d ret = %d!\n",
			mac_dev->mac_id, mac_dev->chip_id, ret);
		return ret;
	}
	mac_info->loop_mode = mac_dev->loop_mode;

	return 0;
}

int mac_phy_reset(struct mac_device *mac_dev)
{
	u16 auto_neg = 0;

	if (NULL == mac_dev) {
		pr_err("mac_phy_reset faild, mac_dev is NULL!\n");
		return -EINVAL;
	}

	if (NULL == mac_dev->phy_dev) {
		log_err(mac_dev->dev, "phy_dev is null!\n");
		return -EINVAL;
	}

	(void)mdiobus_write(mac_dev->phy_dev->bus, mac_dev->phy_dev->addr,
	 	PHY_PAGE_REG, PHY_PAGE_RESET);
	auto_neg = mdiobus_read(mac_dev->phy_dev->bus, mac_dev->phy_dev->addr,
	 	PHY_AUTO_REG);
	if(PHY_NEG_EN & auto_neg)
		auto_neg |= PHY_NEG_RESTART;
	(void)mdiobus_write(mac_dev->phy_dev->bus,
			mac_dev->phy_dev->addr, PHY_AUTO_REG, auto_neg);

	return 0;
}

/**
 *mac_autoneg_stat - get autoneg enable status
 *@net_dev: net device
 *@enable : mac autoneg enable status
 * return status
 */
static int mac_autoneg_stat(struct mac_device *mac_dev, u32 *enable)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_autoneg_stat faild,	mac_dev is NULL!\n");
		return -EINVAL;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL,mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
		  return -EINVAL;
	 }
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->autoneg_stat) {
		ret = mac_ctrl_drv->autoneg_stat(mac_ctrl_drv, enable);
		if (ret) {
			log_err(mac_dev->dev, "autoneg_stat fail,mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
			return ret;
		}
	}

	return 0;
}

static int mac_get_pause_enable(struct mac_device *mac_dev,
			u32 *rx_pause_en, u32 *tx_pause_en)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("get_pause_enable faild, mac_dev is NULL!\n");
		return -EINVAL;
	 }

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL,mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return -EINVAL;
	 }
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_pause_enable) {
		ret = mac_ctrl_drv->get_pause_enable(mac_ctrl_drv,
			rx_pause_en, tx_pause_en);
		if (ret) {
			log_err(mac_dev->dev, "get_pause_enable fail,mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
			return ret;
		}
	}

	return 0;
}


static int mac_set_pause_enable(struct mac_device *mac_dev,
			u32 rx_pause_en, u32 tx_pause_en)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_set_pause_enable faild,	mac_dev is NULL!\n");
		return -EINVAL;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL,mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return -EINVAL;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->mac_pausefrm_cfg) {
		ret = mac_ctrl_drv->mac_pausefrm_cfg(mac_ctrl_drv,
			rx_pause_en, tx_pause_en);
		if (ret) {
			log_err(mac_dev->dev,
				"mac_pausefrm_cfg fail,mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
			return ret;
		}
	}
	return 0;
}

/**
 *mac_get_link_status - get mac status--link up or link down
 *@net_dev: net device
 *@enable : mac autoneg enable status
 * return status
 */
static int mac_get_link_status(struct mac_device *mac_dev,
			u32 *link_stat)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;
	struct phy_device *phy_dev = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_set_pause_enable faild, mac_dev is NULL!\n");
		return -EINVAL;
	}
	phy_dev = mac_dev->phy_dev;

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL,mac%d dsaf%d!\n",
			mac_dev->mac_id, mac_dev->chip_id);
		return -EINVAL;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_link_status) {
		ret = mac_ctrl_drv->get_link_status(mac_ctrl_drv, link_stat);
		if (ret) {
			log_err(mac_dev->dev, "get_link_status fail,mac%d dsaf%d!\n",
				mac_dev->mac_id, mac_dev->chip_id);
			return ret;
		}
	}

    /* updata for phy info */
	if (phy_dev) {
		if (!genphy_update_link(phy_dev))
		    *link_stat = *link_stat && phy_dev->link;
	}

	return 0;
}

/**
 *mac_get_regs - get important regs of mac about debug,using in ethtool
 *@net_dev: net device
 *@enable : mac autoneg enable status
 * return status
 */
static void mac_get_regs(struct mac_device *mac_dev,
			struct ethtool_regs *cmd, void *data)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("set_pause_enable faild, mac_dev is NULL!\n");
		return;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_regs)
		mac_ctrl_drv->get_regs(mac_ctrl_drv, cmd, data);
}

/**
 *mac_get_sset_count - get the number important regs of mac about debug,using in ethtool
 *@net_dev: net device
 *@stringset : typr of string name, 0-selftest 1-stats 5- dump
 * return the number of regs
 */
static int mac_get_sset_count(struct mac_device *mac_dev, u32 stringset)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_drv = NULL;
	int mac_reg_num = 0;

	if (NULL == mac_dev) {
		pr_err("mac_set_pause_enable faild, mac_dev is NULL!\n");
		return -EINVAL;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return -EINVAL;
	}
	mac_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_drv->get_sset_count)
		mac_reg_num = mac_drv->get_sset_count(mac_drv, stringset);

	return mac_reg_num;
}


/**
 *mac_get_strings - get the name of  important regs about debug,using in ethtool
 *@net_dev: net device
 *@stringset : typr of string name, 0-selftest 1-stats 5- dump
 * return status
 */
static void mac_get_strings(struct mac_device *mac_dev,
			u32 stringset, u8 *data)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_set_pause_enable faild , mac_dev is NULL!\n");
		return;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_strings)
		mac_ctrl_drv->get_strings(mac_ctrl_drv, stringset, data);
}


/**
 *mac_get_ethtool_stats - get statistics of mac about debug,using in ethtool
 *@net_dev: net device
 *@cmd : ethtool_stats
 * return status
 */
static void mac_get_ethtool_stats(struct mac_device *mac_dev,
		struct ethtool_stats *cmd, u64 *data)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_set_pause_enable faild , mac_dev is NULL!\n");
		return;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_ethtool_stats)
		mac_ctrl_drv->get_ethtool_stats(mac_dev, cmd, data);

}

/**
 *mac_get_dump_regs - dump regs,using in ethtool
 *@net_dev: net device
 *@cmd : ethtool_stats
 * return status
 */
static int mac_get_dump_regs(struct mac_device *mac_dev)
{
	int ret;
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_get_dump_regs faild, mac_dev is NULL!\n");
		return -EINVAL;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return -EINVAL;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->mac_dump_regs) {
		ret = mac_ctrl_drv->mac_dump_regs(mac_ctrl_drv);
		if (ret) {
			log_err(mac_dev->dev, "mac_dump_regs fail!\n");
			return ret;
		}
	}

	return 0;
}


/**
 *mac_get_mode - get mac's working mode
 *@net_dev: net device
 * return mac's working mode
 */
enum mac_mode mac_get_mode(struct mac_device *mac_dev)
{
	return mac_dev_to_enet_interface(mac_dev);
}

/**
 *mac_get_mode - get mac's working mode
 *@net_dev: net device
 * return mac's working mode
 */
int mac_sfp_led_opt(struct mac_device *mac_dev)
{
	int port = 0;
	int nic_data = 0, tx_pkts = 0, rx_pkts = 0;


	port = mac_dev->mac_id + mac_dev->chip_id * MAC_MAX_PORT_NUM_PER_CHIP;

	if(NULL != mac_dev->get_total_txrx_pkts)
		mac_dev->get_total_txrx_pkts(mac_dev, &tx_pkts, &rx_pkts);

	if((tx_pkts - mac_dev->txpkt_for_led) || (rx_pkts-mac_dev->rxpkt_for_led))
		 nic_data = 1;
	else
		 nic_data = 0;

	mac_dev->txpkt_for_led = tx_pkts;
	mac_dev->rxpkt_for_led = rx_pkts;

	sfp_led_set(mac_dev, port, (int)mac_dev->link,  mac_dev->speed, nic_data);


	return 0;
}

/**
 *mac_sfp_led_reset - reset led value
 *@net_dev: net device
 * return mac's working mode
 */
void mac_sfp_led_reset(struct mac_device *mac_dev)
{
	int port = 0;

	port = mac_dev->mac_id + mac_dev->chip_id * MAC_MAX_PORT_NUM_PER_CHIP;

	sfp_led_reset(mac_dev, port);
}


/**
 *mac_get_mode - get mac's working mode
 *@net_dev: net device
 * return mac's working mode
 */
void mac_get_total_pkts(struct mac_device *mac_dev, int *tx_pkts,
					int *rx_pkts)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_get_dump_regs faild, mac_dev is NULL!\n");
		return ;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return ;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->get_total_txrx_pkts)
		mac_ctrl_drv->get_total_txrx_pkts(mac_ctrl_drv, (u32 *)tx_pkts, (u32 *)rx_pkts);
	if(MAC_SPEED_1000 == mac_dev->max_speed) {
		*tx_pkts += mac_dev->txpkt_for_led;
		*rx_pkts += mac_dev->rxpkt_for_led;
	}
}

/**
 *mac_led_set_id - set led identifiaton on or off
 *@net_dev: net device
 * return
 */
void mac_led_set_id(struct mac_device *mac_dev, int status)
{
	if (NULL == mac_dev) {
		pr_err("mac_get_dump_regs faild, mac_dev is NULL!\n");
		return ;
	}


	switch(status) {
	case NIC_LED_ON:
		cpld_reg_write(mac_dev->cpld_vaddr, NIC_LED_ON_VALUE);
		break;
	case NIC_LED_OFF:
		cpld_reg_write(mac_dev->cpld_vaddr, NIC_LED_OFF_VALUE);
		break;
	}
}

void mac_update_stats(struct mac_device *mac_dev)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_update_stats faild, mac_dev is NULL!\n");
		return ;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return ;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->update_stats)
		mac_ctrl_drv->update_stats(mac_dev);
}
EXPORT_SYMBOL(mac_update_stats);

void mac_clean_stats(struct mac_device *mac_dev)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *mac_ctrl_drv = NULL;

	if (NULL == mac_dev) {
		pr_err("mac_clean_stats faild, mac_dev is NULL!\n");
		return ;
	}

	priv = mac_dev_priv(mac_dev);
	if (NULL == priv) {
		log_err(mac_dev->dev, "priv is NULL\n");
		return ;
	}
	mac_ctrl_drv = (struct mac_driver *)(priv->mac);

	if (NULL != mac_ctrl_drv->clean_stats)
		mac_ctrl_drv->clean_stats(mac_dev);
}
EXPORT_SYMBOL(mac_clean_stats);

/**
 *mac_setup_gmac - init gmac struct
 *@mac_dev: mac device
 */
void mac_setup_gmac(struct mac_device *mac_dev)
{
	mac_dev->init_phy = mac_init_gmac_phy;
	mac_dev->init = mac_init;
	mac_dev->start = mac_start;
	mac_dev->stop = mac_stop;
	mac_dev->reset = mac_reset;
	mac_dev->change_promisc = mac_change_promisc;
	mac_dev->change_addr = mac_change_addr;
	mac_dev->get_addr = mac_get_addr;
	mac_dev->set_multi = mac_set_multi;
	mac_dev->uninit = mac_uninit;
	mac_dev->ptp_enable = NULL;
	mac_dev->ptp_disable = NULL;
	mac_dev->set_vf_mac = mac_change_vf_addr;
	mac_dev->change_mtu = mac_change_mtu;
	mac_dev->queue_config_bc_en = mac_queue_config_bc_en;

	mac_dev->get_vf_mac = mac_get_vf_mac;
	mac_dev->set_vf_vlan = mac_set_vf_vlan;
	mac_dev->get_vf_vlan = mac_get_vf_vlan;
	mac_dev->add_mac = mac_add_mac;
	mac_dev->del_mac = mac_del_mac;
	mac_dev->adjust_link = mac_adjust_link_dsaf;
	mac_dev->set_an_mode = mac_set_an_mode;
	mac_dev->config_loopback = mac_config_loopback;
	mac_dev->get_info = mac_hal_get_info;
	mac_dev->phy_reset = mac_phy_reset;
	mac_dev->autoneg_stat = mac_autoneg_stat;
	mac_dev->get_pause_enable = mac_get_pause_enable;
	mac_dev->set_pause_enable = mac_set_pause_enable;
	mac_dev->get_link_status = mac_get_link_status;
	mac_dev->get_regs = mac_get_regs;
	mac_dev->get_sset_count = mac_get_sset_count;
	mac_dev->get_strings = mac_get_strings;
	mac_dev->get_ethtool_stats = mac_get_ethtool_stats;
	mac_dev->get_dump_regs = mac_get_dump_regs;
	mac_dev->get_mac_mode = mac_get_mode;
	mac_dev->sfp_led_opt = mac_sfp_led_opt;
	mac_dev->sfp_open = NULL;
	mac_dev->sfp_close = NULL;
	mac_dev->get_sfp_prsnt = NULL;
	mac_dev->led_reset = mac_sfp_led_reset;
	mac_dev->get_total_txrx_pkts = mac_get_total_pkts;
	mac_dev->led_set_id = mac_led_set_id;
	mac_dev->phy_set_led_id = phy_led_set;

}

/**
 *mac_setup_xgmac - init gmac struct
 *@mac_dev: mac device
 */
void mac_setup_xgmac(struct mac_device *mac_dev)
{
	mac_dev->init_phy = mac_init_xgmac_phy;
	mac_dev->init = mac_init;
	mac_dev->start = mac_start;
	mac_dev->stop = mac_stop;
	mac_dev->change_promisc = mac_change_promisc;
	mac_dev->change_addr = mac_change_addr;
	mac_dev->get_addr = mac_get_addr;
	mac_dev->set_multi = mac_set_multi;
	mac_dev->uninit = mac_uninit;
	mac_dev->ptp_enable = NULL;
	mac_dev->ptp_disable = NULL;
	mac_dev->set_vf_mac = mac_change_vf_addr;
	mac_dev->get_vf_mac = mac_get_vf_mac;
	mac_dev->set_vf_vlan = mac_set_vf_vlan;
	mac_dev->get_vf_vlan = mac_get_vf_vlan;
	mac_dev->change_mtu = mac_change_mtu;

	mac_dev->queue_config_bc_en = mac_queue_config_bc_en;

	mac_dev->add_mac = mac_add_mac;
	mac_dev->del_mac = mac_del_mac;
	mac_dev->adjust_link = NULL;
	mac_dev->set_an_mode = NULL;
	mac_dev->config_loopback = mac_config_loopback;
	mac_dev->get_info = mac_hal_get_info;
	mac_dev->phy_reset = NULL;
	mac_dev->get_pause_enable = mac_get_pause_enable;
	mac_dev->set_pause_enable = mac_set_pause_enable;
	mac_dev->get_link_status = mac_get_link_status;
	mac_dev->get_regs = mac_get_regs;
	mac_dev->get_sset_count = mac_get_sset_count;
	mac_dev->get_strings = mac_get_strings;
	mac_dev->get_ethtool_stats = mac_get_ethtool_stats;
	mac_dev->get_dump_regs = mac_get_dump_regs;
	mac_dev->get_mac_mode = mac_get_mode;
	mac_dev->sfp_led_opt = mac_sfp_led_opt;
	mac_dev->sfp_open = mac_sfp_open;
	mac_dev->sfp_close = mac_sfp_close;
	mac_dev->get_sfp_prsnt = mac_sfp_prsnt;
	mac_dev->led_reset = mac_sfp_led_reset;
	mac_dev->get_total_txrx_pkts = mac_get_total_pkts;
	mac_dev->led_set_id = mac_led_set_id;

}






