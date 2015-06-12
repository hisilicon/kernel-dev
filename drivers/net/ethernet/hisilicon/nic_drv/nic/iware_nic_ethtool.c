/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd														 */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfS7wnisvw3AzUxl5XUKKWC2cO3yv9GJw9DrL41Ssu8U5Ffg8H6snGBXmw2b4u0Dr7/W2
v56uKMXlBDS23gELA4dOQ5R1FsLObpLvKsKql/XppAzX42M/AIT+oF5Spwx3gQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <asm/cacheflush.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ethtool.h>
/*
#include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_error.h"
#include "iware_log.h"
#include "osal_api.h"
#include "iware_mac_main.h"
#include "iware_rcb_main.h"
#include "iware_nic_main.h"
#include "iware_nic_ethtool.h"


static const char nic_gstrings_test[][ETH_GSTRING_LEN] = {
	"Mac    Loopback test",
	"Serdes Loopback test",
	"Phy    Loopback test"
};

/**
 * nic_get_settings - get netdev setting
 * @dev: net device
 * @cmd: ethtool command
 *
 * Return 0 on success, negative on failure
 */
int nic_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct mac_info mac_info;
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct phy_device *phy_dev = NULL;
	u32 link_stat = 0;
	int ret;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return -ENODEV;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return -ENODEV;
	}
	phy_dev = mac_dev->phy_dev;


	if (NULL != mac_dev->get_link_status) {
		ret = mac_dev->get_link_status(mac_dev, &link_stat);
		if (ret) {
			log_err(&dev->dev, "get_link_status err!\n");
			return -ENODEV;
		}
	}

	link_stat = link_stat && mac_dev->link;


	if (NULL != mac_dev->get_info) {
		ret = mac_dev->get_info(mac_dev, &mac_info);
		if (ret) {
			log_err(&dev->dev,
				"nic_get_settings get_info error !\n");
			return ret;
		}

		cmd->autoneg = mac_info.auto_neg;

		if(link_stat) {
			ethtool_cmd_speed_set(cmd, mac_info.speed);
			cmd->duplex = mac_info.duplex;
		} else {
			ethtool_cmd_speed_set(cmd, (u32)SPEED_UNKNOWN);
			cmd->duplex = DUPLEX_UNKNOWN;
		}

		if (mac_info.auto_neg) {
			cmd->supported |= SUPPORTED_Autoneg;
			cmd->advertising |= ADVERTISED_Autoneg;
			cmd->autoneg = AUTONEG_ENABLE;

		} else
			cmd->autoneg = AUTONEG_DISABLE;
	} else {
		ethtool_cmd_speed_set(cmd, (u32)SPEED_UNKNOWN);
		cmd->duplex = DUPLEX_UNKNOWN;
	}

	/* updata from phy info */
	if (phy_dev) {
	cmd->supported = phy_dev->supported;

	cmd->advertising = phy_dev->advertising;
	cmd->lp_advertising = phy_dev->lp_advertising;

			if(link_stat) {
				ethtool_cmd_speed_set(cmd, phy_dev->speed);
			    cmd->duplex = phy_dev->duplex;
			} else {
				ethtool_cmd_speed_set(cmd, (u32)SPEED_UNKNOWN);
				cmd->duplex = DUPLEX_UNKNOWN;
			}

	cmd->phy_address = phy_dev->addr;
	cmd->transceiver = phy_is_internal(phy_dev) ?
		XCVR_INTERNAL : XCVR_EXTERNAL;
	cmd->autoneg = phy_dev->autoneg;
    }

	if (MAC_PHY_INTERFACE_MODE_SGMII == mac_dev->phy_if) {
		cmd->supported |= MAC_GMAC_SUPPORTED;
		cmd->supported |= SUPPORTED_1000baseT_Full;
		cmd->supported |= SUPPORTED_1000baseT_Half;
		cmd->supported |= SUPPORTED_2500baseX_Full;
		cmd->supported |= SUPPORTED_FIBRE;
		cmd->supported |= SUPPORTED_TP;
	} else if (MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if) {
		cmd->supported |= SUPPORTED_10000baseR_FEC;
		cmd->supported |= SUPPORTED_10000baseKR_Full;
		cmd->supported |= SUPPORTED_FIBRE;
	}
	if (MAC_PHY_INTERFACE_MODE_SGMII == mac_dev->phy_if) {
		cmd->advertising |= ADVERTISED_1000baseT_Full;
		cmd->advertising |= ADVERTISED_1000baseT_Half;
	} else if (MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if)
		cmd->advertising |= ADVERTISED_10000baseKR_Full;

	cmd->transceiver = XCVR_EXTERNAL;
	cmd->mdio_support = (ETH_MDIO_SUPPORTS_C45 | ETH_MDIO_SUPPORTS_C22);

    /* updata port type */
    if (MAC_PHY_INTERFACE_MODE_SGMII == mac_dev->phy_if)
       cmd->port = PORT_TP;
    else if (MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if)
       cmd->port = PORT_FIBRE;

	return 0;
}

/**
 * nic_set_settings - set netdev setting
 * @dev: net device
 * @cmd: ethtool command
 *
 * Return 0 on success, negative on failure
 */
int nic_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	int ret;
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct phy_device *phy_dev = NULL;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "\n nic_device is NULL!\n");
		return -EINVAL;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "\n mac_dev is NULL!\n");
		return -EINVAL;
	}

	phy_dev = mac_dev->phy_dev;
	if((MAC_SPEED_1000 == cmd->speed) && (false == cmd->duplex)) {
		log_err(&dev->dev, "\n not surport 1000M half mode!\n");
		return -EINVAL;
	}
	if(MAC_SPEED_10000 == cmd->speed) {
		log_err(&dev->dev, "\n 10GE mode not surport setting!\n");
		return -EINVAL;
	}
	if (phy_dev) {
		(void) phy_ethtool_sset(phy_dev, cmd);
		return 0;
	}
	if (NULL != mac_dev->set_an_mode) {
		ret = mac_dev->set_an_mode(mac_dev, cmd->autoneg);
		if (ret) {
			log_err(&dev->dev, "\n ethtool set autoneg error!\n");
			return ret;
		}
	}
	if((AUTONEG_ENABLE == cmd->autoneg)
		&& ((cmd->speed !=mac_dev->speed)
		|| (cmd->duplex != (!mac_dev->half_duplex)))) {
		log_err(&dev->dev,
		"\n autoneg enable mode not surport speed seting and duplex setting!\n");
		return -EINVAL;
	}
	if (NULL != mac_dev->adjust_link) {
		ret = mac_dev->adjust_link(mac_dev, cmd->speed, cmd->duplex);
		if (ret) {
			log_err(&dev->dev,
				"\n ethtool set speed and duplex error!\n");
			return ret;
		}
		mac_dev->speed = ethtool_cmd_speed(cmd);
		mac_dev->half_duplex = !cmd->duplex;
	} else
		log_err(&dev->dev, "\n not surport setting!\n");


	/* updata for phy info */

	return 0;
}

/**
 * nic_get_drvinfo - get net driver info
 * @dev: net device
 * @drvinfo: driver info
 */
void nic_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return;
	}

    if (strlen(NIC_MOD_VERSION) < 32) /* TBD */
	strncpy(drvinfo->version, NIC_MOD_VERSION,
		strlen(NIC_MOD_VERSION) + 1);
    else if (strlen(NIC_MOD_VERSION) >= 32) {
	strncpy(drvinfo->version, NIC_MOD_VERSION, 31);
	drvinfo->version[31] = '\0';
    }

	if (strlen(DRIVER_NAME) < 32)
		strncpy(drvinfo->driver, DRIVER_NAME, strlen(DRIVER_NAME) + 1);
	else if (strlen(DRIVER_NAME) >= 32) {
		strncpy(drvinfo->driver, DRIVER_NAME, 31);
		drvinfo->driver[31] = '\0';
	}

	/*what is fw_version   TBD */
	snprintf(drvinfo->fw_version, strlen(DRIVER_NAME) + 1, "%s",
		 DRIVER_NAME);
}

/**
 * nic_get_regs - get net device register
 * @dev: net device
 * @cmd: ethtool cmd
 * @date: register data
 */
void nic_get_regs(struct net_device *dev, struct ethtool_regs *cmd, void *data)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	struct dsaf_device *dsaf_dev = NULL;
	struct nic_ring_pair *ring = NULL;
	struct ppe_device *ppe_device = NULL;
	u32 rcb_num = 0;
	u32 ppe_num = 0;
	u32 dsaf_num = 0;
	u32 *p = data;

	cmd->version = (1 << 24);

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return;
	}

	rcb_common = nic_dev->rcb_common;
	if (NULL == rcb_common) {
		log_err(&dev->dev, "rcb_common is NULL!\n");
		return;
	}
	dsaf_dev = mac_dev->dsaf_dev;
	if (NULL == dsaf_dev) {
		log_err(&dev->dev, "dsaf_dev is NULL!\n");
		return;
	}

	ring = nic_dev->ring[0];
	if ((NULL != ring) && (NULL != ring->rcb_dev.ops.get_sset_count))
		rcb_num = ring->rcb_dev.ops.get_sset_count(ring, ETH_DUMP_REG);

	ppe_device = nic_dev->ppe_device;
	if (NULL != ppe_device->ops.get_sset_count)
		ppe_num = ppe_device->ops.get_sset_count(ppe_device, ETH_DUMP_REG);

	if (NULL != dsaf_dev->get_sset_count)
		dsaf_num = dsaf_dev->get_sset_count(dsaf_dev, ETH_DUMP_REG);


	/* get rcb register */
	if (NULL != rcb_common->ops.get_regs) {
		rcb_common->ops.get_regs(rcb_common, p);
		p += rcb_num;
	}

	ppe_device = nic_dev->ppe_device;
	if (NULL != ppe_device->ops.get_regs) {
		ppe_device->ops.get_regs(ppe_device, cmd, p);
		p += ppe_num;
	}

	/* get dsaf register TBD */
	if (NULL != dsaf_dev->get_regs) {
		dsaf_dev->get_regs(dsaf_dev, cmd, p);
		p += dsaf_num;
	}
	if (NULL != mac_dev->get_regs) {
		mac_dev->get_regs(mac_dev, cmd, p);
	}
}

/**
 * ethtool_op_get_link - get link status
 * @dev: net device
 *
 * Return 0 on success, negative on failure, others link status
 */
u32 nic_get_link(struct net_device *dev)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	u32 link_stat = 0;
	int ret;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return 0;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return 0;
	}
	if (NULL != mac_dev->get_link_status) {
		ret = mac_dev->get_link_status(mac_dev, &link_stat);
		if (ret) {
			log_err(&dev->dev, "get_link_status err!\n");
			return 0;
		}
	}
	return link_stat;
}

/**
 * nic_nway_reset - nway reset
 * @dev: net device
 *
 * Return 0 on success, negative on failure
 */
int nic_nway_reset(struct net_device *netdev)
{
	int ret;
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;

	if (netif_running(netdev))
		nic_reinit(netdev);

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(&netdev->dev, "nic_device is NULL!\n");
		return -ENODEV;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&netdev->dev, "mac_dev is NULL!\n");
		return -ENODEV;
	}
	/* check phy info TBD */
	if (NULL != mac_dev->phy_reset) {
		ret = mac_dev->phy_reset(mac_dev);
		if (ret) {
			log_err(&netdev->dev, "phy_reset error!\n");
			return -ENODEV;
		}
		log_dbg(&netdev->dev, "phy reset successfully!\n");
	} else
		log_dbg(&netdev->dev, "no phy register!\n");

	return 0;
}

/**
 * nic_get_ringparam - get ring parameter
 * @dev: net device
 * @param: ethtool parameter
 */
void nic_get_ringparam(struct net_device *dev, struct ethtool_ringparam *param)
{
	/* TBD */
	struct nic_device *nic_dev = NULL;
	struct nic_ring_pair *tx_ring = NULL;
	struct nic_ring_pair *rx_ring = NULL;

	nic_dev = netdev_priv(dev);

	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}

	tx_ring = nic_dev->ring[0];	/* TBD */
	rx_ring = nic_dev->ring[0];

	param->rx_max_pending = RCB_RING_MAX_PENDING_BD;
	param->tx_max_pending = RCB_RING_MAX_PENDING_BD;
	param->rx_mini_max_pending = NIC_MAX_RING_NUM;
	param->rx_jumbo_max_pending = RCB_RING_JUNBO_MAX_PENDING_BD;

	param->rx_mini_pending = NIC_MAX_RING_NUM;
	param->rx_jumbo_pending = NIC_MAX_RING_NUM;
	param->rx_pending = rx_ring->rx_ring.count;
	param->tx_pending = tx_ring->tx_ring.count;
}

/**
 * nic_get_pauseparam - get pause parameter
 * @dev: net device
 * @param: pause parameter
 */
void nic_get_pauseparam(struct net_device *dev,
			struct ethtool_pauseparam *param)
{
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;
	u32 auto_neg_stat = 0;
	u32 rx_pause = 0;
	u32 tx_pause = 0;
	int ret;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return;
	}
	if (NULL != mac_dev->autoneg_stat) {
		ret = mac_dev->autoneg_stat(mac_dev, &auto_neg_stat);
		if (ret) {
			log_err(&dev->dev, "get auto_neg_stat fail!\n");
			return;
		}
	}
	param->autoneg = auto_neg_stat;
	if (NULL != mac_dev->get_pause_enable) {
		ret = mac_dev->get_pause_enable(mac_dev, &rx_pause, &tx_pause);
		if (ret) {
			log_err(&dev->dev, "get auto_neg_stat fail!\n");
			return;
		}
	}

	param->rx_pause = rx_pause;
	param->tx_pause = tx_pause;
	/* param->autoneg = 0; TBD */

}

/**
 * nic_set_pauseparam - set pause parameter
 * @dev: net device
 * @param: pause parameter
 *
 * Return 0 on success, negative on failure
 */
int nic_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *param)
{
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;
	int ret = 0 ;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return -EINVAL;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return -EINVAL;
	}

	if(NULL != mac_dev->set_pause_enable)
		ret = mac_dev->set_pause_enable(mac_dev, param->rx_pause,
			param->tx_pause);
	if(0 != ret){
		log_err(&dev->dev, "set_pause_enable fail!\n");
		return -EIO;
	}

	return 0;
}


static int nic_setup_loopback(struct nic_device *nic_dev,
	enum mac_loop_mode loop_mode)
{
    struct mac_device *mac_dev = nic_dev->mac_dev;

    if(!mac_dev)
        return -ENOTSUPP;

    if(mac_dev->config_loopback)
        return mac_dev->config_loopback(mac_dev,loop_mode);
    else
        return -ENOTSUPP;
}

static int nic_setup_loopback_test(struct nic_device *nic_dev,
	enum mac_loop_mode loop_mode)
{
	int ret;
	struct mac_device *mac_dev = NULL;
	u32 ring_idx = 0;
	u32 ring_fail_idx = 0;
	struct nic_ring_pair * ring;
	struct net_device *netdev = nic_dev->netdev;

	nic_reset(nic_dev->netdev);

	for (ring_idx = 0; ring_idx < nic_dev->max_q_per_vf; ring_idx++) {
		ring = nic_dev->ring[ring_idx];
		rcb_enable_test_ring(ring);
	}

	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev || NULL == mac_dev->start) {
		log_err(&netdev->dev, "mac start fail, mac_dev = %p!\n",
			mac_dev);
		ret = -ENODEV;
		goto ring_enable_fail;
	}

	if (mac_dev->init_phy != NULL) {
		ret = mac_dev->init_phy(netdev);
		if (ret < 0) {
			log_err(&netdev->dev,
				"init_phy fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
			goto ring_enable_fail;
		}
	}


	if (mac_dev->queue_config_bc_en != NULL) {
		ret = mac_dev->queue_config_bc_en(mac_dev, 0, ENABLE);
		if (ret < 0) {
			log_err(&netdev->dev,
				"queue_config_bc_en fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
			goto ring_enable_fail;
		}
	}

	/* set loopback mode */
	ret = nic_setup_loopback(nic_dev,loop_mode);
	if (ret < 0) {
		log_err(&netdev->dev,
			"chip_id=%d nic_idx=%d nic_setup_loopback(0x%x) faild(%#x)!\n",
			nic_dev->chip_id, nic_dev->index,loop_mode,ret);
		goto set_loopback_fail;
	}


	ret = mac_dev->start(mac_dev);
	if (ret < 0) {
		log_err(&netdev->dev, "mac start fail, ret = %#x!\n", ret);
		goto mac_start_fail;
	}


	if (mac_dev->sfp_open) {
	    ret = mac_dev->sfp_open(mac_dev);
		    if (ret) {
			    log_err(&netdev->dev,
				    "mac open sfp fail, ret = %#x!\n", ret);
			    goto mac_start_fail;
		    }
	    }


	return 0;


mac_start_fail:

	if (mac_dev->queue_config_bc_en != NULL) {
		ret = mac_dev->queue_config_bc_en(mac_dev, 0, DISABLE);
		if (ret < 0)
			log_err(&netdev->dev,
				"queue_config_bc_en fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

set_loopback_fail:

		ret = nic_setup_loopback(nic_dev,MAC_LOOP_NONE);
		if (ret < 0)
			log_err(&netdev->dev,"clear loopback fail, ret=%d!\n",ret);

ring_enable_fail:
	for (ring_fail_idx = 0; ring_fail_idx < ring_idx; ring_fail_idx++){
		ring = nic_dev->ring[ring_fail_idx];
		rcb_disable_test_ring(ring);
	}


	return ret;

}

static void nic_create_lbtest_frame(struct net_device *netdev,
                                        struct sk_buff *skb,
                                        unsigned int frame_size)
{
	memset((skb->data), 0xFF, frame_size);
	frame_size &= ~1;
	memset(&skb->data[frame_size / 2], 0xAA, frame_size / 2 - 1);
	memset(&skb->data[frame_size / 2 + 10], 0xBE, 1);
	memset(&skb->data[frame_size / 2 + 12], 0xAF, 1);

}


int nic_check_lbtest_frame(struct sk_buff *skb)
{
	unsigned int frame_size = skb->len;

	frame_size &= ~1;
	if (*(skb->data + 10) == 0xFF) {
		if ((*(skb->data + frame_size / 2 + 10) == 0xBE) &&
		    (*(skb->data + frame_size / 2 + 12) == 0xAF)) {
			kfree_skb(skb);
			return 0;
		}
	}

	log_info(NULL,"frame_size = %d\n",frame_size);
	/*dump_mem(skb->data, frame_size);*/

	kfree_skb(skb);
	return -EFAULT;
}
int nic_lb_rx_process(struct napi_struct *napi, struct sk_buff *skb)
{
	(void)napi;
	return nic_check_lbtest_frame(skb);
}


int nic_clean_rx_rings(struct nic_ring_pair *ring,
                                  unsigned int size)
{
	int ret;

	(void)size;

	ret = rcb_recv_hw_ex(ring,NIC_LB_TEST_PKT_NUM_PER_CYCLE,nic_lb_rx_process);

	return (ret < 0)? 0 :ret ;
}

int nic_clean_all_rx_rings(struct nic_device *nic_dev,
                                  unsigned int size)
{
	int sum = 0;
	int ring_idx = 0;

	for(ring_idx = 0;ring_idx < nic_dev->ring_pair_num;ring_idx++)
		sum += nic_clean_rx_rings(nic_dev->ring[ring_idx],size);

	return sum;
}
void nic_clean_tx_ring(struct nic_ring_pair *ring)
{
	ring->rcb_dev.ops.clean_tx_ring(ring);
}

/**
 * nic_run_loopback_test -  run loopback test
 * @nic_dev: net device
 * @loopback_type: loopback type
 */
static int nic_run_loopback_test(struct nic_device *nic_dev,
                                        enum mac_loop_mode loop_mode)
{
	struct nic_ring_pair *ring;
	int i, j, lc, good_cnt, ret_val = 0;
	unsigned int size ;
	netdev_tx_t tx_ret_val;
	struct sk_buff *skb;

	(void)loop_mode;

	ring = nic_dev->ring[NIC_LB_TEST_RING_ID];
	size = NIC_LB_TEST_FRAME_SIZE;

	/* allocate test skb */
	skb = alloc_skb(size, GFP_KERNEL);
	if (!skb)
		return NIC_LB_TEST_NO_MEM_ERR;

	/* place data into test skb */
	nic_create_lbtest_frame(nic_dev->netdev,skb, size);
	(void)skb_put(skb, size);

	lc = 1;
	for (j = 0; j < lc; j++) {
		/* reset count of good packets */
		good_cnt = 0;

		/* place 64 packets on the transmit queue*/
		for (i = 0; i < NIC_LB_TEST_PKT_NUM_PER_CYCLE; i++) {
			(void)skb_get(skb);

			tx_ret_val = rcb_pkt_send(skb, ring);
			if (tx_ret_val == NETDEV_TX_OK)
				good_cnt++;

		}

		if (good_cnt != NIC_LB_TEST_PKT_NUM_PER_CYCLE) {
			ret_val = NIC_LB_TEST_TX_CNT_ERR;
			break;
		}

		/* allow 100 milliseconds for packets to go from Tx to Rx */
		msleep(100);


		good_cnt = nic_clean_all_rx_rings(nic_dev,size);
		if (good_cnt != NIC_LB_TEST_PKT_NUM_PER_CYCLE) {
			ret_val = NIC_LB_TEST_RX_CNT_ERR;
			break;
		}

		nic_clean_tx_ring(ring);
	}

	/* free the original skb */
	kfree_skb(skb);

	return ret_val;
}

int nic_loopback_ring_cleanup(struct nic_device *nic_dev)
{

	int ring_id;
	struct nic_ring_pair *ring = NULL;
	struct rcb_device *rcb_dev = NULL;

	for (ring_id=0; ring_id<nic_dev->ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		rcb_dev = &ring->rcb_dev;
		rcb_dev->ops.disable(ring);
	}
	msleep(100);

	for (ring_id=0; ring_id<nic_dev->ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		rcb_dev = &ring->rcb_dev;

		rcb_dev->ops.clean_tx_ring(ring);
	}

	return 0;

}


int nic_loopback_down(struct nic_device *nic_dev)
{
	int ret = 0;
	struct mac_device *mac_dev = nic_dev->mac_dev;
	struct net_device *netdev = nic_dev->netdev;


	if(mac_dev->stop(mac_dev))
		log_warn(&nic_dev->netdev->dev, "stop mac%d fail!\n", nic_dev->gidx);

	usleep_range(10000, 20000);


	(void)nic_loopback_ring_cleanup(nic_dev);


	if (mac_dev->queue_config_bc_en != NULL) {
		ret = mac_dev->queue_config_bc_en(mac_dev, 0, DISABLE);
		if (ret < 0)
			log_err(&netdev->dev,
				"queue_config_bc_en fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

	return 0;

}

void nic_loopback_cleanup(struct nic_device *nic_dev)
{
	int ret = 0;

	struct mac_device *mac_dev = nic_dev->mac_dev;
	struct net_device *netdev = nic_dev->netdev;

	(void)nic_setup_loopback(nic_dev,MAC_LOOP_NONE);

	(void)nic_loopback_down(nic_dev);

	if(NULL != mac_dev->led_reset
		&& 0 != mac_dev->cpld_vaddr)
		mac_dev->led_reset(mac_dev);

	if (mac_dev->sfp_close) {
		ret = mac_dev->sfp_close(mac_dev);
		if (ret)
			log_err(&netdev->dev,
				"mac close sfp fail, ret = %#x!\n", ret);
	}

	nic_reset(netdev);
}
/**
 * nic_loopback_test -  loopback test
 * @dev: net device
 * @data: test result
 */
static int nic_loopback_test(struct nic_device *nic_dev,
                                    enum mac_loop_mode loop_mode, u64 *data)
{

	*data = (u64)nic_setup_loopback_test(nic_dev,loop_mode);

	if (*data)
		return *data;

	*data = (u64)nic_run_loopback_test(nic_dev,loop_mode);

	nic_loopback_cleanup(nic_dev);

	return *data;
}

/**
 * nic_self_test - self test
 * @dev: net device
 * @eth_test: test cmd
 * @data: test result
 */
void nic_self_test(struct net_device *dev, struct ethtool_test *eth_test,
		   u64 *data)
{
	struct nic_device *nic_dev = netdev_priv(dev);
	bool if_running = netif_running(dev);

	set_bit(NIC_STATE_TESTING, &nic_dev->state);

	data[0] = ENOTSUPP;
	data[1] = ENOTSUPP;
	data[2] = ENOTSUPP;

	if (eth_test->flags == ETH_TEST_FL_OFFLINE) {

		if (if_running)
			(void)dev_close(dev);

		/* XGE not supported */
		if(nic_dev->mac_dev->phy_if != MAC_PHY_INTERFACE_MODE_XGMII){
			if (nic_loopback_test(nic_dev, MAC_INTERNALLOOP_MAC,
						&data[0]) != 0) {
				eth_test->flags |= ETH_TEST_FL_FAILED;
			}
		}

		if (nic_loopback_test(nic_dev, MAC_INTERNALLOOP_SERDES,
						&data[1]) != 0) {
			eth_test->flags |= ETH_TEST_FL_FAILED;
		}

		/* XGE not supported */
		if(nic_dev->mac_dev->link_features & MAC_LINK_PHY){

			if (nic_loopback_test(nic_dev, MAC_INTERNALLOOP_PHY,
							&data[2]) != 0) {
				eth_test->flags |= ETH_TEST_FL_FAILED;
			}
		}

		nic_reset(nic_dev->netdev);
		clear_bit(NIC_STATE_TESTING, &nic_dev->state);

		if (if_running)
			(void)dev_open(dev);

	}
	/* Online tests aren't run; pass by default */

	(void)msleep_interruptible(4 * 1000);
}

/**
 * nic_get_strings - get strings
 * @dev: net device
 * @stringset: string set index, 0: self test string; 1: statistics string.
 * @data: strings
 */
void nic_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct dsaf_device *dsaf_dev;
	struct nic_ring_pair *ring;
	struct ppe_device *ppe_device;
	u32 rcb_num = 0;
	u32 ppe_num = 0;
	u32 dsaf_num = 0;
	u32 i;
	char *p = (char*)data;

	if(ETH_SS_TEST == stringset){
		memcpy(data, *nic_gstrings_test,sizeof(nic_gstrings_test));
		return ;
	}

	if((ETH_SS_PRIV_FLAGS == stringset)
		||(ETH_SS_NTUPLE_FILTERS == stringset)
		||(ETH_SS_FEATURES == stringset)){
		return ;
	}

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return;
	}

	dsaf_dev = mac_dev->dsaf_dev;

	for (i = 0; i < (u32)nic_dev->ring_pair_num; i++) {
		ring = nic_dev->ring[i];
		if ((NULL != ring) && (NULL != ring->rcb_dev.ops.get_sset_count)
		    && (NULL != ring->rcb_dev.ops.get_strings)) {
			rcb_num =
			    ring->rcb_dev.ops.get_sset_count(ring, stringset);
			ring->rcb_dev.ops.get_strings(ring, stringset, (u8 *)p);
			p += rcb_num * ETH_GSTRING_LEN;
		}
	}

	ppe_device = nic_dev->ppe_device;
	if ((NULL !=ppe_device) && (NULL != ppe_device->ops.get_sset_count)
	    && (NULL != ppe_device->ops.get_strings)) {
		ppe_num = ppe_device->ops.get_sset_count(ppe_device, stringset);
		ppe_device->ops.get_strings(ppe_device, stringset, (u8 *)p);
		p += ppe_num * ETH_GSTRING_LEN;
	}

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->get_sset_count)
	    && (NULL != dsaf_dev->get_strings)) {
		dsaf_num = dsaf_dev->get_sset_count(dsaf_dev, stringset);
		dsaf_dev->get_strings(dsaf_dev, stringset, (u8 *)p);
		p += dsaf_num * ETH_GSTRING_LEN;
	}

	if ((NULL != mac_dev->get_sset_count)
		&& (NULL != mac_dev->get_strings))
	mac_dev->get_strings(mac_dev, stringset, (u8*)p);
}

/**
 * nic_get_sset_count - get string set count witch returned by nic_get_strings.
 * @dev: net device
 * @stringset: string set index, 0: self test string; 1: statistics string.
 *
 * Return string set count.
 */
int nic_get_sset_count(struct net_device *dev, int sset)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct dsaf_device *dsaf_dev;
	struct nic_ring_pair *ring;
	struct ppe_device *ppe_device;
	u32 rcb_num = 0;
	u32 ppe_num = 0;
	u32 dsaf_num = 0;
	u32 mac_num = 0;
	u32 total_num = 0;
	int i;

	if(ETH_SS_TEST == sset){
	        return (sizeof(nic_gstrings_test) / ETH_GSTRING_LEN);
	}
	if((ETH_SS_PRIV_FLAGS == sset)||(ETH_SS_NTUPLE_FILTERS == sset)
		||(ETH_SS_FEATURES == sset)){
		return -ENOTSUPP;
	}

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return 0;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return 0;
	}

	dsaf_dev = mac_dev->dsaf_dev;

	if (ETH_DUMP_REG == sset) {	/* TBD why do not use for loop */
		ring = nic_dev->ring[0];
		if (NULL != ring->rcb_dev.ops.get_sset_count)
			rcb_num = ring->rcb_dev.ops.get_sset_count(ring, sset);
	} else if (ETH_STATIC_REG == sset) {
		for (i = 0; i < nic_dev->ring_pair_num; i++) {
			ring = nic_dev->ring[i];
			if (NULL != ring->rcb_dev.ops.get_sset_count)
				rcb_num +=
				    ring->rcb_dev.ops.get_sset_count(ring,
								     sset);
		}
	}

	ppe_device = nic_dev->ppe_device;
	if (NULL != ppe_device->ops.get_sset_count)
		ppe_num = ppe_device->ops.get_sset_count(ppe_device, sset);

	/*dsaf  TBD */
	if ((dsaf_dev != NULL) && (NULL != dsaf_dev->get_sset_count))
		dsaf_num = dsaf_dev->get_sset_count(dsaf_dev, sset);

	if (NULL != mac_dev->get_sset_count)
		mac_num = mac_dev->get_sset_count(mac_dev, sset);

	total_num = rcb_num + ppe_num + dsaf_num + mac_num;

	return total_num;
}

/**
 * nic_set_phys_id - set phy identify LED.
 * @dev: net device
 * @state: LED state.
 *
 * Return 0 on success, negative on failure.
 */
int nic_set_phys_id(struct net_device *dev, enum ethtool_phys_id_state state)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return 0;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return 0;
	}

	switch (state) {
	case ETHTOOL_ID_ACTIVE:
		if((mac_dev->link_features & MAC_LINK_PHY)
			&& (NULL != mac_dev->phy_dev)) {

			(void) mdiobus_write(mac_dev->phy_dev->bus, mac_dev->phy_dev->addr,
				PHY_PAGE_REG, PHY_PAGE_LED);
			mac_dev->phy_led_value = (u16)mdiobus_read(mac_dev->phy_dev->bus,
				mac_dev->phy_dev->addr, LED_FCR);
			(void) mdiobus_write(mac_dev->phy_dev->bus,
				mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_COPPER);
			return 2;
		} else if(NULL != mac_dev->cpld_vaddr){
			mac_dev->cpld_led_value = cpld_reg_read(mac_dev->cpld_vaddr);
			return 2;
		} else {
			osal_pr("not surport ethtool -p!\n");
			return 0;
		}

	case ETHTOOL_ID_ON:
		if((mac_dev->link_features & MAC_LINK_PHY)
			&& (NULL != mac_dev->phy_dev))
			(void) mac_dev->phy_set_led_id(mac_dev, LD_FORCE_ON);
		else if(NULL != mac_dev->cpld_vaddr)
			mac_dev->led_set_id(mac_dev, NIC_LED_ON);
		break;

	case ETHTOOL_ID_OFF:
		if((mac_dev->link_features & MAC_LINK_PHY)
			&& (NULL != mac_dev->phy_dev))
			(void) mac_dev->phy_set_led_id(mac_dev, LD_FORCE_OFF);
		else if(NULL != mac_dev->cpld_vaddr)
			mac_dev->led_set_id(mac_dev, NIC_LED_OFF);
		break;

	case ETHTOOL_ID_INACTIVE:
		/* Restore LED settings */
		if((mac_dev->link_features & MAC_LINK_PHY)
			&& (NULL != mac_dev->phy_dev)) {
			(void) mdiobus_write(mac_dev->phy_dev->bus, mac_dev->phy_dev->addr,
				PHY_PAGE_REG, PHY_PAGE_LED);
			(void) mdiobus_write(mac_dev->phy_dev->bus, mac_dev->phy_dev->addr,
					LED_FCR, mac_dev->phy_led_value);
			(void) mdiobus_write(mac_dev->phy_dev->bus,
				mac_dev->phy_dev->addr, PHY_PAGE_REG, PHY_PAGE_COPPER);
		} else if(NULL != mac_dev->cpld_vaddr) {
			cpld_reg_write(mac_dev->cpld_vaddr, mac_dev->cpld_led_value);
		}
		break;
	}
	return 0;
}

/**
 * get_ethtool_stats - get detail statistics.
 * @dev: net device
 * @stats: statistics info.
 * @data: statistics data.
 */
void get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats,
		       u64 *data)
{
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;
	struct dsaf_device *dsaf_dev = NULL;
	struct nic_ring_pair *ring = NULL;
	struct ppe_device *ppe_device = NULL;
	u32 rcb_num = 0;
	u32 ppe_num = 0;
	u32 dsaf_num = 0;
	u64 *p = data;
	int i;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}
	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(&dev->dev, "mac_dev is NULL!\n");
		return;
	}

	dsaf_dev = mac_dev->dsaf_dev;

	for (i = 0; i < nic_dev->ring_pair_num; i++) {
		ring = nic_dev->ring[i];
		if ((NULL != ring->rcb_dev.ops.get_sset_count)
		    && (NULL != ring->rcb_dev.ops.get_ethtool_stats)) {
			rcb_num =
			    ring->rcb_dev.ops.get_sset_count(ring,
							     ETH_STATIC_REG);
			ring->rcb_dev.ops.get_ethtool_stats(ring, stats, p);
			p += rcb_num;
		}
	}
	ppe_device = nic_dev->ppe_device;
	if ((NULL != ppe_device->ops.get_sset_count)
	    && (NULL != ppe_device->ops.get_ethtool_stats)) {
		ppe_num = ppe_device->ops.get_sset_count(ppe_device, ETH_STATIC_REG);
		ppe_device->ops.get_ethtool_stats(ppe_device, stats, p);
		p += ppe_num;
	}

	/* dsaf  TBD */
	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->get_sset_count)
	    && (NULL != dsaf_dev->get_ethtool_stats)) {
		dsaf_num = dsaf_dev->get_sset_count(dsaf_dev, ETH_STATIC_REG);
		dsaf_dev->get_ethtool_stats(dsaf_dev, stats, p);
		p += dsaf_num;
	}

	if ((NULL != mac_dev->get_sset_count)
	    && (NULL != mac_dev->get_ethtool_stats)) {
		mac_dev->get_ethtool_stats(mac_dev, stats, p);
	}
}

/**
 * nic_get_coalesce - get coalesce info.
 * @dev: net device
 * @ec: coalesce info.
 *
 * Return 0 on success, negative on failure.
 */
int nic_get_coalesce(struct net_device *dev, struct ethtool_coalesce *ec)
{
	struct nic_device *nic_dev = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	u32 rx_coalesce_usecs = 0;
	u32 rx_max_coalesced_frames = 0;
	u32 port_idx = 0;

	nic_dev = netdev_priv(dev);

	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return -EINVAL;
	}
	rcb_common = nic_dev->rcb_common;	/* TBD */
	if ((NULL == rcb_common)
	    || (NULL == rcb_common->ops.get_rx_coalesce_usecs)
	    || (NULL == rcb_common->ops.get_rx_max_coalesced_frames)) {
		log_err(&dev->dev, "not support!\n");
		return -ENOTSUPP;
	}

	port_idx = nic_dev->index;
	(void) rcb_common->ops.get_rx_coalesce_usecs(rcb_common, port_idx, &rx_coalesce_usecs);
	(void) rcb_common->ops.get_rx_max_coalesced_frames(rcb_common, port_idx,
						    &rx_max_coalesced_frames);
	ec->rx_coalesce_usecs = rx_coalesce_usecs;
	ec->tx_coalesce_usecs = rx_coalesce_usecs;
	ec->rx_max_coalesced_frames = rx_max_coalesced_frames;
	ec->tx_max_coalesced_frames = rx_max_coalesced_frames;
	ec->use_adaptive_rx_coalesce = 1;
	ec->use_adaptive_tx_coalesce = 1;

	return 0;
}

/**
 * nic_set_coalesce - set coalesce info.
 * @dev: net device
 * @ec: coalesce info.
 *
 * Return 0 on success, negative on failure.
 */
int nic_set_coalesce(struct net_device *dev, struct ethtool_coalesce *ec)
{
	struct nic_device *nic_dev = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	u32 port_idx = 0;
	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return -EINVAL;
	}
	rcb_common = nic_dev->rcb_common;	/* TBD */
	if ((NULL == rcb_common)
	    || (NULL == rcb_common->ops.set_port_desc_cnt)
	    || (NULL == rcb_common->ops.set_port_coalesced_frames)) {
		log_err(&dev->dev, "not support!\n");
	return -ENOTSUPP;
	}
	port_idx = nic_dev->index;
	if(ec->rx_max_coalesced_frames != nic_dev->coalesced_frames &&
		ec->tx_max_coalesced_frames != nic_dev->coalesced_frames) {
		log_err(&dev->dev, "paran error:rx_frames != tx_frames\n");
		return -EINVAL;
	}
	if (rcb_common->ops.set_port_coalesced_frames != NULL) {
		if(ec->rx_max_coalesced_frames > 1 &&
			nic_dev->coalesced_frames != ec->rx_max_coalesced_frames) {
			nic_dev->coalesced_frames = ec->rx_max_coalesced_frames;
		rcb_common->ops.set_port_coalesced_frames(rcb_common, port_idx,
						nic_dev->coalesced_frames);
		} else if (ec->tx_max_coalesced_frames > 1 &&
			nic_dev->coalesced_frames != ec->tx_max_coalesced_frames) {
			nic_dev->coalesced_frames = ec->tx_max_coalesced_frames;
		rcb_common->ops.set_port_coalesced_frames(rcb_common, port_idx,
						nic_dev->coalesced_frames);
		}
	}
	if(ec->rx_coalesce_usecs != nic_dev->time_out &&
		ec->tx_coalesce_usecs != nic_dev->time_out) {
		log_err(&dev->dev, "paran error:rx_frames != tx_frames\n");
		return -EINVAL;
	}
	if (rcb_common->ops.set_port_timeout != NULL &&
		(port_idx%NIC_MAX_NUM_PER_CHIP) >= 6) {
		if(ec->rx_coalesce_usecs > 1 &&
			nic_dev->time_out != ec->rx_coalesce_usecs) {
		nic_dev->time_out = ec->rx_coalesce_usecs;
		rcb_common->ops.set_port_timeout(rcb_common, port_idx,
						 nic_dev->time_out);
		} else if (ec->tx_coalesce_usecs > 1 &&
			nic_dev->time_out != ec->tx_coalesce_usecs) {
		nic_dev->time_out = ec->tx_coalesce_usecs;
		rcb_common->ops.set_port_timeout(rcb_common, port_idx,
						 nic_dev->time_out);
		}
	} else {
		if(nic_dev->time_out != ec->rx_coalesce_usecs
			|| nic_dev->time_out != ec->tx_coalesce_usecs)
		log_err(&dev->dev, "error: not surport coalesce_usecs setting!\n");
	}

	return 0;
}

/**
 * nic_get_channels - get channel info.
 * @dev: net device
 * @ch: channel info.
 */
void nic_get_channels(struct net_device *dev, struct ethtool_channels *ch)
{
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "nic_device is NULL!\n");
		return;
	}

	/* must return max ring num per port TBD */
	ch->max_rx = MAX_RING_PAIR_NUM;
	ch->max_tx = MAX_RING_PAIR_NUM;
	ch->max_combined = MAX_RING_PAIR_NUM;
	ch->max_other = MAX_RING_PAIR_NUM;

	ch->rx_count = nic_dev->ring_pair_num;
	ch->tx_count = nic_dev->ring_pair_num;
	ch->combined_count = nic_dev->ring_pair_num;
	ch->other_count = nic_dev->ring_pair_num;
}

/**
 * nic_get_regs_len - get total register len.
 * @dev: net device
 *
 * Return total register len.
 */
static int nic_get_regs_len(struct net_device *dev)
{
	int reg_num = 0;

	reg_num = nic_get_sset_count(dev, ETH_DUMP_REG);
	if (reg_num > 0)
		return reg_num * sizeof(u32);
	else
		return reg_num;	/* error code */
}

const struct ethtool_ops g_nic_ethtools = {
	.get_settings = nic_get_settings,
	.set_settings = nic_set_settings,
	.get_drvinfo = nic_get_drvinfo,
	.get_regs = nic_get_regs,
	.nway_reset = nic_nway_reset,
	.get_link = nic_get_link,
	.get_ringparam = nic_get_ringparam,
	.get_pauseparam = nic_get_pauseparam,
	.set_pauseparam = nic_set_pauseparam,
	.self_test = nic_self_test,
	.get_strings = nic_get_strings,
	.get_sset_count = nic_get_sset_count,
	.set_phys_id = nic_set_phys_id,
	.get_ethtool_stats = get_ethtool_stats,
	.get_coalesce = nic_get_coalesce,
	.set_coalesce = nic_set_coalesce,
	.get_channels = nic_get_channels,
	.get_regs_len = nic_get_regs_len,
};
