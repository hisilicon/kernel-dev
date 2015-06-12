/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe482hQEF4bqe2nK7raIwblHjOYUSNq6ZVT8arps0OCq7L/vqaN7BAqGF/62jYnaXZNXNr3
NUkXZvzKfPK+631BAW7sT/SyUYmwrAfbhfr6Nr7aWO7v+EmwjvcUX0cPqd+NmA==*/
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
/*
#include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_error.h"
#include "iware_log.h"
#include "osal_api.h"
#include "iware_dsaf_main.h"
#include "iware_mac_main.h"
#include "iware_led_hal.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/phy_fixed.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>

static const u16 mac_phy_to_speed[] = {
	[MAC_PHY_INTERFACE_MODE_MII] = MAC_SPEED_100,
	[MAC_PHY_INTERFACE_MODE_GMII] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_SGMII] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_TBI] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_RMII] = MAC_SPEED_100,
	[MAC_PHY_INTERFACE_MODE_RGMII] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_ID] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_RXID]	= MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_RGMII_TXID]	= MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_RTBI] = MAC_SPEED_1000,
	[MAC_PHY_INTERFACE_MODE_XGMII] = MAC_SPEED_10000
};


static const char g_phy_str[][INTERFACE_MODE_NUM] = {
	[MAC_PHY_INTERFACE_MODE_MII] = "mii",
	[MAC_PHY_INTERFACE_MODE_GMII] = "gmii",
	[MAC_PHY_INTERFACE_MODE_SGMII] = "sgmii",
	[MAC_PHY_INTERFACE_MODE_TBI] = "tbi",
	[MAC_PHY_INTERFACE_MODE_RMII] = "rmii",
	[MAC_PHY_INTERFACE_MODE_RGMII] = "rgmii",
	[MAC_PHY_INTERFACE_MODE_RGMII_ID] = "rgmii-id",
	[MAC_PHY_INTERFACE_MODE_RGMII_RXID] = "rgmii-rxid",
	[MAC_PHY_INTERFACE_MODE_RGMII_TXID] = "rgmii-txid",
	[MAC_PHY_INTERFACE_MODE_RTBI] = "rtbi",
	[MAC_PHY_INTERFACE_MODE_XGMII] = "xgmii"
};

/* initialize mac device */
static void (*const g_mac_setup_hal[MAC_TYPE_NUM])(struct
		mac_device *mac_dev) = {
	[MAC_GMAC_IDX] = mac_setup_gmac,
	[MAC_XGMAC_IDX] = mac_setup_xgmac,
};

const size_t g_mac_sizeof_priv[] = {
	[MAC_GMAC_IDX] = sizeof(struct mac_priv),
	[MAC_XGMAC_IDX] = sizeof(struct mac_priv)
};

static enum mac_phy_interface mac_str_to_phy(const char *str)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_phy_str); i++)
		if (strcmp(str, g_phy_str[i]) == 0)
			return (enum mac_phy_interface)i;

	return MAC_PHY_INTERFACE_MODE_SGMII;
}

/**
 *mac_get_chip_id - get chip id
 *@pdev: platform_device
 * Return chip id
 */
static int mac_get_chip_id(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	u32 chip_id = 0;
	int ret;

	if(NULL == pdev->dev.of_node)
		return -EINVAL;

	np = of_get_parent(pdev->dev.of_node);
	if (np == NULL)
		return -EINVAL;

	/* find chip attr,if can't be found ,it's single chip*/
	ret = of_property_read_u32(np, "chip-id", &chip_id);
	if (ret)
		return 0;


	if(chip_id >= DSAF_MAX_CHIP_NUM) {
		log_err(&pdev->dev, "chip_id error!\r\n");
		return -EINVAL;
	}
	else
		return chip_id;
}

/**
 *mac_register_roce_cb - regist roce
 *@mac_dev: mac device
 *@dev: roce device
 *@change_addr_cb: callback when addr change
 *@change_mtu_cb: callback when mtu change
 *
 * Return STATUS
 */
int mac_register_roce_cb(struct mac_device *mac_dev, void * dev,
		mac_change_addr_cb_t change_addr_cb,
		mac_change_mtu_cb_t change_mtu_cb)
{
	if (NULL == mac_dev) {
		pr_err("register_addr_change_cb faild,input invalid !\n");
		return HRD_COMMON_ERR_INPUT_INVALID;
	}
	mac_dev->change_addr_cb = change_addr_cb;
	mac_dev->change_mtu_cb = change_mtu_cb;
	mac_dev->roce_dev = dev;
	return 0;
}
EXPORT_SYMBOL(mac_register_roce_cb);

/**
 *mac_alloc_dev -  alloc mac device
 *@dev: mac device
 *@sizeof_priv: size of mac device
 * Return mac device
 */
static struct mac_device *mac_alloc_dev(struct device *dev,
	size_t sizeof_priv,	void (*setup)(struct mac_device *mac_dev))
{
	struct mac_device *mac_dev;

	mac_dev = osal_kzalloc(sizeof(*mac_dev) + sizeof_priv, GFP_KERNEL);
	if (unlikely(mac_dev == NULL))
		mac_dev = ERR_PTR(-ENOMEM);
	else {
		if (dev != NULL) {
			mac_dev->dev = dev;
			log_dbg(dev, "mac_dev->dev=%#llx,->of_node=%#llx!\n",
				(u64)mac_dev->dev, (u64)dev->of_node);

			dev_set_drvdata(dev, mac_dev);
		}
		setup(mac_dev);
	}

	return mac_dev;
}


/**
 *mac_free_dev -	free mac device
 *@mac_dev: mac device
 * Return STATUS
 */
static int mac_free_dev(struct mac_device *mac_dev)
{
	if (mac_dev->dev != NULL) {
		log_dbg(mac_dev->dev, "mac_dev->dev=%#llx!\n", (u64)mac_dev->dev);
		dev_set_drvdata(mac_dev->dev, NULL);
	}

	osal_kfree(mac_dev);

	return 0;
}

irqreturn_t mac_drv_interrupt(int irq, void *dev_id)
{
	return IRQ_NONE;
}

/**
 *mac_free_dev  - get mac information from device node
 *@mac_dev: mac device
 *@np:device node
 *@mac_mode_idx:mac mode index
 * Return STATUS
 */
static int mac_get_info(struct mac_device *mac_dev, struct device_node *np,
			u32 mac_mode_idx)
{
	int sz = 0;
	u32 sz_u32;
	u32 irq_info;
	int ret;
	struct device_node *fixed_link_node = NULL;
	const u32 *fixed_link_prop = NULL;

	log_dbg(mac_dev->dev, "enter func mac%d!\n", mac_dev->global_mac_id);

	mac_dev->link = false;
	mac_dev->half_duplex = false;
	mac_dev->speed = mac_phy_to_speed[mac_dev->phy_if];
	mac_dev->max_speed	= mac_dev->speed;
#if 1/* what decide the attribution  TBD */
	if (MAC_PHY_INTERFACE_MODE_SGMII == mac_dev->phy_if) {
		mac_dev->if_support = MAC_GMAC_SUPPORTED;
		mac_dev->if_support |= SUPPORTED_1000baseT_Full;
		mac_dev->if_support |= SUPPORTED_2500baseX_Full;
	} else if (MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if) {
		mac_dev->if_support = SUPPORTED_10000baseR_FEC;
		mac_dev->if_support |= SUPPORTED_10000baseKR_Full;
	}
#endif
	mac_dev->max_frm = MAC_DEFAULT_MTU;
	mac_dev->tx_pause_frm_time = MAC_DEFAULT_PAUSE_TIME;

	mac_dev->link_features = MAC_LINK_NONE;
	/* Get the rest of the PHY information */
	mac_dev->phy_node = of_parse_phandle(np, "phy-handle", 0);
	if (mac_dev->phy_node == NULL) {
	    if (MAC_PHY_INTERFACE_MODE_XGMII == mac_dev->phy_if)
	        snprintf((char *)mac_dev->phy_bus_id, MAC_PHY_BUS_ID_SIZE,
                PHY_ID_FMT, "X", 0);
	    else {
            /* New binding */
            fixed_link_node = of_get_child_by_name(np, "fixed-link");
            if (fixed_link_node) {
                if (!of_property_read_u32(fixed_link_node, "phy-id", &sz_u32))
                    snprintf((char *)mac_dev->phy_bus_id, MAC_PHY_BUS_ID_SIZE,
                        PHY_ID_FMT, "0", sz_u32);
            } else { /* Old binding */
                fixed_link_prop = of_get_property(np, "fixed-link", &sz);
                if (fixed_link_prop && sz == (5 * sizeof(u32)))
                    snprintf((char *)mac_dev->phy_bus_id, MAC_PHY_BUS_ID_SIZE,
                        PHY_ID_FMT, "0", fixed_link_prop[0]);
            }
    		if (!fixed_link_node && !fixed_link_prop) {
    			log_err(mac_dev->dev, "No PHY (or fixed link) found\n");
    			/**return -EINVAL;*/
    		} else
    			mac_dev->link_features |= MAC_LINK_FIXED;
	    }
	    log_info(mac_dev->dev, "phy_bus_id: %s sz=%#x\n",
			mac_dev->phy_bus_id, sz);
	} else {
		log_info(mac_dev->dev, "phy_node: %s\n", mac_dev->phy_node->name);
		mac_dev->link_features |= MAC_LINK_PHY;
	}

	ret = of_property_read_u32(np, "irq-num", &irq_info);
	if (ret) {
		log_dbg(mac_dev->dev,
			"mac(%d) of_property_read_u32 irq-num fail, ret = %d!\n",
			mac_dev->global_mac_id, ret);
		return ret;
	}
	if (MAC_XGMAC_IDX == mac_mode_idx)
		irq_info += MAC_GE_XGE_IRQ_OFFSET;

	mac_dev->base_irq = irq_info;
	mac_dev->irq_num = MAC_IRQ_NUM;

	log_dbg(mac_dev->dev, "mac%d irq_num=%d!\n",
		mac_dev->global_mac_id, irq_info);

	return 0;
}


/**
 *mac_virtual_probe  - probe virtual mac
 *@net_pdev: platform device
 * Return STATUS
 */
int mac_virtual_probe(struct platform_device *net_pdev)
{
	int ret;
	int chip_id;
	int sz;
	struct mac_device *mac_dev = NULL;
	const u32 *phy_id = NULL;
	enum mac_phy_interface phy_if = MAC_PHY_INTERFACE_MODE_XGMII;

	chip_id = mac_get_chip_id(net_pdev);
	if(chip_id < 0)
		 return -EINVAL;

	log_dbg(mac_dev->dev, "func begin chip_id%d!\n", chip_id);

	mac_dev = mac_alloc_dev(NULL, g_mac_sizeof_priv[MAC_XGMAC_IDX],
			g_mac_setup_hal[MAC_XGMAC_IDX]);
	if (IS_ERR(mac_dev)) {
		ret = PTR_ERR(mac_dev);
		log_err(&net_pdev->dev,
			"mac_alloc_dev faild,chip_id%d ret = %d!\n",
			chip_id, ret);
		return ret;
	}
	/* virtual mac id has no function£¬just for regist dsaf */
	mac_dev->dev = &net_pdev->dev;
	mac_dev->mac_id = DSAF_MAX_PORT_NUM_PER_CHIP - 1;
	mac_dev->chip_id = chip_id;
	mac_dev->global_mac_id
		= mac_dev->mac_id +	chip_id * MAC_MAX_PORT_NUM_PER_CHIP;
	mac_dev->phy_if = phy_if;
	mac_dev->link = false;
	mac_dev->half_duplex = false;
	mac_dev->speed = mac_phy_to_speed[phy_if];
	mac_dev->max_speed = mac_dev->speed;
	mac_dev->max_frm = MAC_DEFAULT_MTU;
	mac_dev->tx_pause_frm_time = MAC_DEFAULT_PAUSE_TIME;

	/* what decide the attribution  TBD */
	mac_dev->if_support = MAC_GMAC_SUPPORTED;
	mac_dev->if_support |= SUPPORTED_1000baseT_Full;
	mac_dev->if_support |= SUPPORTED_2500baseX_Full;
	mac_dev->if_support |= SUPPORTED_10000baseR_FEC;
	mac_dev->if_support |= SUPPORTED_10000baseKR_Full;

	INIT_LIST_HEAD(&mac_dev->mc_addr_list);
	phy_id = of_get_property(net_pdev->dev.of_node, "fixed-link", &sz);
	if (!phy_id || (u32)sz < sizeof(*phy_id)) {
		log_err(mac_dev->dev, "No PHY (or fixed link) found\n");
		ret = -EINVAL;
		goto get_mac_addr_fail;
	}
	snprintf((char *)mac_dev->phy_bus_id, MAC_PHY_BUS_ID_SIZE, PHY_ID_FMT,
		"0", phy_id[0]);
	log_dbg(mac_dev->dev, "phy_bus_id: %s\n", mac_dev->phy_bus_id);

	mac_dev->dsaf_dev = dsaf_register_mac_dev(mac_dev);
	if (NULL == mac_dev->dsaf_dev) {
		ret = HRD_COMMON_ERR_NULL_POINTER;
		log_err(mac_dev->dev,
			"mac_probe dsaf_register_mac_dev faild, chip%d ret = %d\n",
			mac_dev->chip_id, ret);
		goto get_mac_addr_fail;
	}

	ret = mac_dev->init(mac_dev);
	if (ret) {
		log_err(mac_dev->dev,
			"mac_probe mac_dev->init faild, chip_id%d ret = %d\n",
			chip_id, ret);
		goto mac_drv_init_fail;
	}

	return 0;

mac_drv_init_fail:
	(void)dsaf_unregister_mac_dev(mac_dev);

get_mac_addr_fail:
	(void)mac_free_dev(mac_dev);

	return ret;
}
EXPORT_SYMBOL(mac_virtual_probe);

static int mac_probe(struct platform_device *pdev)
{
	int ret;
	u32 dev_id;
	int chip_id;
	struct mac_device *mac_dev = NULL;
	u32 mac_mode_idx = 0;
	u32 res_idx;
	const char *char_prop = NULL;
	struct device_node *np = NULL;
	enum mac_phy_interface phy_if;
    void __iomem *sys_ctl_vaddr = NULL;
	u32 spf_ctl_addr_tmp[SPF_CTL_ADDR_NUM];

	np = pdev->dev.of_node;
	ret = of_property_read_u32(np, "mac-index", &dev_id);
	if (ret) {
		log_err(&pdev->dev,
			"of_property_read_u32 mac-index fail, ret = %d!\r\n", ret);
		return -EINVAL;
	}
	chip_id = mac_get_chip_id(pdev);
	if(chip_id < 0)
		 return -EINVAL;

	log_info(&pdev->dev, "enter mac_probe chip_id%d mac%d,np=%p!\n",
		chip_id, dev_id, np);

	/* Get the PHY connection type */
	char_prop
		= (const char *)of_get_property(np,	"phy-connection-type", NULL);
	if (unlikely(char_prop == NULL)) {
		log_warn(&pdev->dev,
			"chip_id%d mac%d of_get_property phy-connection-type faild",
			chip_id, dev_id);
		phy_if = MAC_PHY_INTERFACE_MODE_SGMII;
	} else
		phy_if = mac_str_to_phy(char_prop);
    /*Updata phy_if from serdes mode***/
	sys_ctl_vaddr = mac_get_base_addr(np, MAC_SYSCTL_IDX);
	if (sys_ctl_vaddr) {
        	phy_if = mac_get_sds_mode(sys_ctl_vaddr, dev_id);
   		log_info(&pdev->dev,
       		 "mac_probe chip_id%d mac%d,sys_ctl_vaddr=%p, phy_if=%d!\n",
       			 chip_id, dev_id, sys_ctl_vaddr, phy_if);
	} else
		log_err(&pdev->dev, "sys_ctl_vaddr is null!\n");

	switch (phy_if) {
	case MAC_PHY_INTERFACE_MODE_SGMII:
		mac_mode_idx = MAC_GMAC_IDX;
		break;

	case MAC_PHY_INTERFACE_MODE_XGMII:
		mac_mode_idx = MAC_XGMAC_IDX;
		break;

	default:
		log_err(&pdev->dev, "chip_id%d mac%d phy_if(%d) err!\n",
			chip_id, dev_id, phy_if);
		return -EINVAL;
	}
	log_info(&pdev->dev, "chip_id%d mac%d mac_mode_idx=%d!\n",
		chip_id, dev_id, mac_mode_idx);

	mac_dev = mac_alloc_dev(&(pdev->dev), g_mac_sizeof_priv[mac_mode_idx],
		g_mac_setup_hal[mac_mode_idx]);
	if (IS_ERR(mac_dev)) {
		ret = PTR_ERR(mac_dev);
		log_err(&pdev->dev,
			"mac_alloc_dev faild,mac%d ret = %#x!\n",	dev_id, ret);
		return ret;
	}
	mac_dev->mac_id = dev_id;
	mac_dev->chip_id = chip_id;
	mac_dev->global_mac_id = dev_id + chip_id * MAC_MAX_PORT_NUM_PER_CHIP;
	mac_dev->phy_if = phy_if;
	mac_dev->sys_ctl_vaddr = sys_ctl_vaddr;
	/*get serdes base addr***/
	mac_dev->serdes_vaddr = mac_get_base_addr(np, MAC_SERDES_IDX);
	/*init: sfp+ isnot present**/
	mac_dev->sfp_prsnt = 0;
	mac_dev->phy_dev = NULL;

	INIT_LIST_HEAD(&mac_dev->mc_addr_list);

	ret = mac_get_info(mac_dev, np, mac_mode_idx);
	if (ret) {
		log_err(&pdev->dev, "mac_get_info faild,mac%d ret = %#x!\n",
			mac_dev->global_mac_id, ret);
		goto get_info_fail;
	}
	if (of_device_is_compatible(np, "hisilicon,mac-multi"))
		res_idx = mac_mode_idx;
	else
		res_idx = 0;
	mac_dev->vaddr = of_iomap(np, res_idx);
	if (NULL == mac_dev->vaddr) {
		log_err(&pdev->dev, "mac(%d) of_iomap fail!\n",
			mac_dev->global_mac_id);
		ret = -ENOMEM;
		goto get_info_fail;
	}
	log_info(&pdev->dev, "chip%d mac%d vaddr = %#llx device=%#llx\n",
		mac_dev->chip_id, dev_id, (u64)mac_dev->vaddr, (u64)mac_dev->dev);

	/* cpld: has cpld addr and has not phy */
	if (!(mac_dev->link_features & MAC_LINK_PHY))
		mac_dev->cpld_vaddr = mac_get_base_addr(np, MAC_CPLD_IDX);
	if (mac_dev->cpld_vaddr) {
		mac_dev->cpld_led_value = cpld_reg_read(mac_dev->cpld_vaddr);
		mac_dev->link_features |= MAC_LINK_CPLD;
		mac_dev->cpld_led_enable = CPLD_LED_ENABLE;
		mac_dev->txpkt_for_led = 0;
		mac_dev->rxpkt_for_led = 0;
	}
	ret = of_property_read_u32_array(np,
		"sfp-ctl-addr", spf_ctl_addr_tmp, SPF_CTL_ADDR_NUM);
	if (ret) {
		mac_dev->spf_ctl_addr[0] = (char)SPF_CTL_INV_ADDR;
		mac_dev->spf_ctl_addr[1] = (char)SPF_CTL_INV_ADDR;
	} else {
		mac_dev->spf_ctl_addr[0] = (char)(spf_ctl_addr_tmp[0] >> 1);
		mac_dev->spf_ctl_addr[1] = (char)(spf_ctl_addr_tmp[1] >> 1);
	}
	log_info(&pdev->dev,
		"mac%d spf addr: 0x%x-0x%x, 0x%x-0x%x, link_features=0x%x!\n",
		dev_id, spf_ctl_addr_tmp[0], spf_ctl_addr_tmp[1],
		mac_dev->spf_ctl_addr[0], mac_dev->spf_ctl_addr[1],
		mac_dev->link_features);

	/*MAC0-5 get dasf device,mac6-7 needn't get dsaf device*/
	if (of_device_is_compatible(np, "hisilicon,mac-multi")) {
		mac_dev->dsaf_dev = dsaf_register_mac_dev(mac_dev);
		if (NULL == mac_dev->dsaf_dev) {
			ret = HRD_COMMON_ERR_NULL_POINTER;
			log_err(&pdev->dev,
				"register_mac_dev faild, chip%d mac%d ret = %#x\n",
				mac_dev->chip_id, dev_id, ret);
			goto reg_mac_fail;
		}
	} else
		mac_dev->dsaf_dev = NULL;

	ret = mac_dev->init(mac_dev);
	if (ret) {
		log_err(&pdev->dev, "mac_dev->init faild, mac%d ret = %#x\n",
			dev_id, ret);
		goto mac_init_fail;
	}

	return 0;

mac_init_fail:
	if (of_device_is_compatible(np, "hisilicon,mac-multi"))
		(void)dsaf_unregister_mac_dev(mac_dev);
reg_mac_fail:
	iounmap(mac_dev->cpld_vaddr);
	iounmap(mac_dev->vaddr);

get_info_fail:
	(void)mac_free_dev(mac_dev);

	return ret;
}

/**
 *mac_virtual_probe  - remove mac
 *@net_pdev: platform device
 * Return STATUS
 */
static int mac_remove(struct platform_device *dev)
{
	int ret;
	int tmp_ret = 0;
	struct mac_device *mac_dev = NULL;

	log_dbg(&dev->dev, "func begin!\n");

	mac_dev = (struct mac_device *)platform_get_drvdata(dev);
	log_dbg(&dev->dev, "mac_dev->dev=%#llx!\n", (u64)mac_dev->dev);
	ret = mac_dev->uninit(mac_dev);

	/* cancel regist mac device */
	if (mac_dev->dsaf_dev)
	    tmp_ret = dsaf_unregister_mac_dev(mac_dev);
	iounmap((void __iomem *)mac_dev->vaddr);
	if (mac_dev->serdes_vaddr)
		iounmap((void __iomem *)mac_dev->serdes_vaddr);
	if (mac_dev->sys_ctl_vaddr)
		iounmap((void __iomem *)mac_dev->sys_ctl_vaddr);
	if (mac_dev->cpld_vaddr)
		iounmap((void __iomem *)mac_dev->cpld_vaddr);
	(void)mac_free_dev(mac_dev);
	return (0 == ret ? tmp_ret : ret);
}

static const struct of_device_id g_mac_match[] = {
	{.compatible = "hisilicon,mac"},
	{.compatible = "hisilicon,mac-ge"},
	{.compatible = "hisilicon,mac-multi"},
	{}
};

static struct platform_driver g_mac_driver = {
	.probe = mac_probe,
	.remove	= mac_remove,
	.driver		= {
		.name	= MAC_DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = g_mac_match,
	},
};

static int __init mac_module_init(void)
{
	int ret;

	ret = platform_driver_register(&g_mac_driver);
	if (0 != ret)
		return ret;
	return 0;
}
static void __exit mac_module_exit(void)
{
	platform_driver_unregister(&g_mac_driver);
}

module_init(mac_module_init);
module_exit(mac_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("MAC driver");
MODULE_VERSION(HILINK_MOD_VERSION);

