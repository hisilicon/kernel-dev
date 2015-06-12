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

#ifndef _IWARE_MAC_MAIN_H
#define _IWARE_MAC_MAIN_H

/* #include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_log.h"
#include "linux/phy.h"
#include <linux/kernel.h>

struct dsaf_device;

#define HILINK_MOD_VERSION "iWareV2R2C00B981SP1"

#define MAC_GMAC_SUPPORTED \
	(SUPPORTED_10baseT_Half \
	| SUPPORTED_10baseT_Full \
	| SUPPORTED_100baseT_Half \
	| SUPPORTED_100baseT_Full \
	| SUPPORTED_Autoneg)

#define MAC_DEFAULT_MTU 1518
#define MAC_DEFAULT_PAUSE_TIME 0xff

#define MAC_DRV_NAME "Hi-MAC"

/*max vlan id */
#define MAC_MAX_VLAN_ID 0xfff

#define MAC_MAX_PORT_NUM_PER_CHIP 8
#define MAC_MAX_CHIP_NUM 2
#define MAC_MAX_PORT_NUM (MAC_MAX_CHIP_NUM *MAC_MAX_PORT_NUM_PER_CHIP)
#define MAC_TYPE_NUM 2
#define MAC_GMAC_IDX 0
#define MAC_XGMAC_IDX 1
#define MAC_CPLD_IDX 4
#define MAC_SERDES_IDX 3
#define MAC_SYSCTL_IDX 2

#define MAC_MAX_VM_NUM 128

#define CPLD_LED_ENABLE 	1
#define CPLD_LED_DISABLE 	0

#define MAC_IRQ_NUM 1
#define MAC_GE_XGE_IRQ_OFFSET 6
#define INTERFACE_MODE_NUM	11

/* #define iware_WORD_SIZE 64*/

#define SPF_CTL_INV_ADDR (-1)
#define SPF_CTL_ADDR_NUM (2)
#define MAC_LINK_NONE (0)
#define MAC_LINK_PHY (1ULL << 0)
#define MAC_LINK_FIXED (1ULL << 1)
#define MAC_LINK_SPF (1ULL << 2)
#define MAC_LINK_CPLD (1ULL << 3)

static inline u32 iware_get_high_bit(u32 num)
{
	u32 ulIndex = 0;

	asm("CLZ %0, %1" : "=r"(ulIndex) : "r"(ulIndex), "r"(num));
	return (32 - ulIndex);
}

/* check mac addr broadcast */
#define MAC_IS_BROADCAST(p)	((*(p) == 0xff) && (*((p)+1) == 0xff) \
		&& (*((p)+2) == 0xff) &&  (*((p)+3) == 0xff)  \
		&& (*((p)+4) == 0xff) && (*((p)+5) == 0xff))

/* check mac addr is 01-00-5e-xx-xx-xx*/
#define MAC_IS_L3_MULTICAST(p) ((*((p)+0) == 0x01) \
			&& (*((p)+1) == 0x00)   \
			&& (*((p)+2) == 0x5e))

/*check the mac addr is 0 in all bit*/
#define MAC_IS_ALL_ZEROS(p)   ((*(p) == 0) && (*((p)+1) == 0)   \
	&& (*((p)+2) == 0) && (*((p)+3) == 0)	 \
	&& (*((p)+4) == 0) && (*((p)+5) == 0))

/*check mac addr multicast*/
#define MAC_IS_MULTICAST(p)	((*((u8 *)((p)+0)) & 0x01)?(1):(0))

/**< Number of octets (8-bit bytes) in an ethernet address */
#define MAC_NUM_OCTETS_PER_ADDR 6

struct mac_priv {
	void *mac;
};


/* net speed */
enum mac_speed {
	MAC_SPEED_10	 = 10,	   /**< 10 Mbps */
	MAC_SPEED_100	= 100,	  /**< 100 Mbps */
	MAC_SPEED_1000   = 1000,	 /**< 1000 Mbps = 1 Gbps */
	MAC_SPEED_10000  = 10000	 /**< 10000 Mbps = 10 Gbps */
};


/*mac interface keyword	*/
enum mac_intf {
	MAC_IF_NONE  = 0x00000000,   /**< inteface not invalid */
	MAC_IF_MII   = 0x00010000,   /**< MII interface */
	MAC_IF_RMII  = 0x00020000,   /**< RMII interface */
	MAC_IF_SMII  = 0x00030000,   /**< SMII interface */
	MAC_IF_GMII  = 0x00040000,   /**< GMII interface */
	MAC_IF_RGMII = 0x00050000,   /**< RGMII interface */
	MAC_IF_TBI   = 0x00060000,   /**< TBI interface */
	MAC_IF_RTBI  = 0x00070000,   /**< RTBI interface */
	MAC_IF_SGMII = 0x00080000,   /**< SGMII interface */
	MAC_IF_XGMII = 0x00090000,   /**< XGMII interface */
	MAC_IF_QSGMII = 0x000a0000	/**< QSGMII interface */
};

/* Interface Mode definitions */
enum mac_phy_interface {
	MAC_PHY_INTERFACE_MODE_NA,
	MAC_PHY_INTERFACE_MODE_MII,
	MAC_PHY_INTERFACE_MODE_GMII,
	MAC_PHY_INTERFACE_MODE_SGMII,
	MAC_PHY_INTERFACE_MODE_TBI,
	MAC_PHY_INTERFACE_MODE_RMII,
	MAC_PHY_INTERFACE_MODE_RGMII,
	MAC_PHY_INTERFACE_MODE_RGMII_ID,
	MAC_PHY_INTERFACE_MODE_RGMII_RXID,
	MAC_PHY_INTERFACE_MODE_RGMII_TXID,
	MAC_PHY_INTERFACE_MODE_RTBI,
	MAC_PHY_INTERFACE_MODE_SMII,
	MAC_PHY_INTERFACE_MODE_XGMII,
	MAC_PHY_INTERFACE_MODE_MOCA,
	MAC_PHY_INTERFACE_MODE_QSGMII,
	MAC_PHY_INTERFACE_MODE_MAX,
};


/*mac mode */
enum mac_mode {
	/**< Invalid Ethernet mode */
	MAC_MODE_INVALID	 = 0,
	/**<	10 Mbps MII   */
	MAC_MODE_MII_10	  = (MAC_IF_MII   | MAC_SPEED_10),
	/**<   100 Mbps MII   */
	MAC_MODE_MII_100	 = (MAC_IF_MII   | MAC_SPEED_100),
	/**<	10 Mbps RMII  */
	MAC_MODE_RMII_10	 = (MAC_IF_RMII  | MAC_SPEED_10),
	/**<   100 Mbps RMII  */
	MAC_MODE_RMII_100	= (MAC_IF_RMII  | MAC_SPEED_100),
	/**<	10 Mbps SMII  */
	MAC_MODE_SMII_10	 = (MAC_IF_SMII  | MAC_SPEED_10),
	/**<   100 Mbps SMII  */
	MAC_MODE_SMII_100	= (MAC_IF_SMII  | MAC_SPEED_100),
	/**<  1000 Mbps GMII  */
	MAC_MODE_GMII_1000   = (MAC_IF_GMII  | MAC_SPEED_1000),
	/**<	10 Mbps RGMII */
	MAC_MODE_RGMII_10	= (MAC_IF_RGMII | MAC_SPEED_10),
	/**<   100 Mbps RGMII */
	MAC_MODE_RGMII_100   = (MAC_IF_RGMII | MAC_SPEED_100),
	/**<  1000 Mbps RGMII */
	MAC_MODE_RGMII_1000  = (MAC_IF_RGMII | MAC_SPEED_1000),
	/**<  1000 Mbps TBI   */
	MAC_MODE_TBI_1000	= (MAC_IF_TBI   | MAC_SPEED_1000),
	/**<  1000 Mbps RTBI  */
	MAC_MODE_RTBI_1000   = (MAC_IF_RTBI  | MAC_SPEED_1000),
	/**<	10 Mbps SGMII */
	MAC_MODE_SGMII_10	= (MAC_IF_SGMII | MAC_SPEED_10),
	/**<   100 Mbps SGMII */
	MAC_MODE_SGMII_100   = (MAC_IF_SGMII | MAC_SPEED_100),
	/**<  1000 Mbps SGMII */
	MAC_MODE_SGMII_1000  = (MAC_IF_SGMII | MAC_SPEED_1000),
	/**< 10000 Mbps XGMII */
	MAC_MODE_XGMII_10000 = (MAC_IF_XGMII | MAC_SPEED_10000),
	/**<  1000 Mbps QSGMII */
	MAC_MODE_QSGMII_1000 = (MAC_IF_QSGMII | MAC_SPEED_1000)
};

/*mac loop mode*/
enum mac_loop_mode {
	MAC_LOOP_NONE = 0,
	MAC_INTERNALLOOP_MAC,
	MAC_INTERNALLOOP_SGMII,
	MAC_INTERNALLOOP_SERDES,
	MAC_INTERNALLOOP_PHY,
	MAC_EXTERNALLOOP_MAC,
	MAC_EXTERNALLOOP_SGMII,
	MAC_EXTERNALLOOP_SERDES,
	MAC_EXTERNALLOOP_PHY
};

/*mac communicate mode*/
enum mac_commom_mode {
	MAC_COMM_MODE_NONE	  = 0, /**< No transmit/receive communication */
	MAC_COMM_MODE_RX		= 1, /**< Only receive communication */
	MAC_COMM_MODE_TX		= 2, /**< Only transmit communication */
	MAC_COMM_MODE_RX_AND_TX = 3  /**< Both transmit and receive communication */
};

/*mac exceprion define */
enum mac_exceptions {
	MAC_EXCE_NONE,
};


/*mac statistics */
struct mac_statistics {
/* RMON */
	u64  stat_pkts64; /* r-10G tr-DT 64 byte frame counter */
	u64  stat_pkts65to127; /* r-10G 65 to 127 byte frame counter */
	u64  stat_pkts128to255; /* r-10G 128 to 255 byte frame counter */
	u64  stat_pkts256to511; /*r-10G 256 to 511 byte frame counter */
	u64  stat_pkts512to1023;/* r-10G 512 to 1023 byte frame counter */
	u64  stat_pkts1024to1518; /* r-10G 1024 to 1518 byte frame counter */
	u64  stat_pkts1519to1522; /* r-10G 1519 to 1522 byte good frame count */
	/* Total number of packets that were less than 64 octets
				long with a wrong CRC.*/
	u64  stat_fragments;
	/* Total number of packets longer than valid maximum length octets */
	u64  stat_jabbers;
	/* number of dropped packets due to internal errors of
				the MAC Client. */
	u64  stat_drop_events;
	/* Incremented when frames of correct length but with
				CRC error are received.*/
	u64  stat_crc_align_errors;
	/* Total number of packets that were less than 64 octets
				long with a good CRC.*/
	u64  stat_undersize_pkts;
	u64  stat_oversize_pkts;  /**< T,B.D*/
/* Pause */
	u64  stat_rx_pause;		   /**< Pause MAC Control received */
	u64  stat_tx_pause;		   /**< Pause MAC Control sent */

/*mIB II */
	u64  in_octets;		/*Total number of byte received. */
	u64  in_pkts;		/**< Total number of packets received.*/
	u64  in_mcast_pkts;	/*Total number of multicast frame received*/
	u64  in_bcast_Pkts;	/* Total number of broadcast frame received */
		/* Frames received, but discarded due to problems
			within the MAC RX. */
	u64  in_discards;
	u64  in_errors;		/* Number of frames received with error:
					- FIFO Overflow Error
					- CRC Error
					- Frame Too Long Error
					- Alignment Error*/
	u64  out_octets; /*Total number of byte sent. */
	u64  out_pkts;	/**< Total number of packets sent .*/
	u64  out_mcast_pkts; /* Total number of multicast frame sent */
	u64  out_bcast_pkts; /* Total number of multicast frame sent */
	/* Frames received, but discarded due to problems within
				the MAC TX N/A!.*/
	u64  out_discards;
	u64  out_errors;	/*Number of frames transmitted with error:
							- FIFO Overflow Error
							- FIFO Underflow Error
							 - Other */
};

/*callback function for irq*/
typedef void (*mac_exception_cb_t)(void * app,
		enum mac_exceptions exception_no);

/*mac para struct ,mac get param from nic or dsaf when initialize*/
struct mac_params {
	char addr[MAC_NUM_OCTETS_PER_ADDR];
	void *vaddr; /*virtual address*/
	struct device *dev;
	u8 mac_id;
	u8 chip_id;
	/**< Ethernet operation mode (MAC-PHY interface and speed) */
	enum mac_mode mac_mode;
	mac_exception_cb_t event_cb; /**< MDIO Events Callback Routine*/
	mac_exception_cb_t exception_cb;  /**< Exception Callback Routine*/
	void * app_cb; /**< A handle to an application layer object; This handle
				will be passed by the driver upon calling the above callbacks */
};

struct mac_info {

	u16 speed; /* The forced speed (lower bits) in
				 *mbps. Please use
				 * ethtool_cmd_speed()/_set() to
				 * access it */
	u8 duplex;		/* Duplex, half or full */
	u8 auto_neg;	/* Enable or disable autonegotiation */
	enum mac_loop_mode loop_mode;
	u8 tx_pause_en;
	u8 tx_pause_time;
	u8 rx_pause_en;
	u8 pad_and_crc_en;
	u8 promiscuous_en;
	u8 port_en;	 /*port enable*/
};

struct mac_entry_idx {
	u8 addr[MAC_NUM_OCTETS_PER_ADDR];
	u16 vlan_id:12;
	u16 valid:1;
	u16 qos:3;
};

/*when mac address change,roce change it's config */
typedef int (*mac_change_addr_cb_t)(void *dev, u8 port_id, char *addr);

/*when mac mtu change,roce change it's config */
typedef int (*mac_change_mtu_cb_t)(void *dev, u8 port_id, u32 new_mtu);

struct mac_hw_stats {
	u64 rx_good_pkts;	/* only for xgmac */
	u64 rx_good_bytes;
	u64 rx_total_pkts;	/* only for xgmac */
	u64 rx_total_bytes;	/* only for xgmac */
	u64 rx_bad_bytes;	/* only for gmac */
	u64 rx_uc_pkts;
	u64 rx_mc_pkts;
	u64 rx_bc_pkts;
	u64 rx_fragment_err;	/* only for xgmac */
	u64 rx_undersize;	/* only for xgmac */
	u64 rx_under_min;
	u64 rx_minto64;		/* only for gmac */
	u64 rx_64bytes;
	u64 rx_65to127;
	u64 rx_128to255;
	u64 rx_256to511;
	u64 rx_512to1023;
	u64 rx_1024to1518;
	u64 rx_1519tomax;
	u64 rx_1519tomax_good;	/* only for xgmac */
	u64 rx_oversize;
	u64 rx_jabber_err;
	u64 rx_fcs_err;
	u64 rx_vlan_pkts;	/* only for gmac */
	u64 rx_data_err;	/* only for gmac */
	u64 rx_align_err;	/* only for gmac */
	u64 rx_long_err;	/* only for gmac */
	u64 rx_pfc_tc0;
	u64 rx_pfc_tc1;		/* only for xgmac */
	u64 rx_pfc_tc2;		/* only for xgmac */
	u64 rx_pfc_tc3;		/* only for xgmac */
	u64 rx_pfc_tc4;		/* only for xgmac */
	u64 rx_pfc_tc5;		/* only for xgmac */
	u64 rx_pfc_tc6;		/* only for xgmac */
	u64 rx_pfc_tc7;		/* only for xgmac */
	u64 rx_unknown_ctrl;
	u64 rx_filter_pkts;	/* only for gmac */
	u64 rx_filter_bytes;	/* only for gmac */
	u64 rx_fifo_overrun_err;/* only for gmac */
	u64 rx_len_err;		/* only for gmac */
	u64 rx_comma_err;	/* only for gmac */
	u64 rx_symbol_err;	/* only for xgmac */
	u64 tx_good_to_sw;	/* only for xgmac */
	u64 tx_bad_to_sw;	/* only for xgmac */
	u64 rx_1731_pkts;	/* only for xgmac */

	u64 tx_good_bytes;
	u64 tx_good_pkts;	/* only for xgmac */
	u64 tx_total_bytes;	/* only for xgmac */
	u64 tx_total_pkts;	/* only for xgmac */
	u64 tx_bad_bytes;	/* only for gmac */
	u64 tx_bad_pkts;	/* only for xgmac */
	u64 tx_uc_pkts;
	u64 tx_mc_pkts;
	u64 tx_bc_pkts;
	u64 tx_undersize;	/* only for xgmac */
	u64 tx_fragment_err;	/* only for xgmac */
	u64 tx_under_min_pkts;	/* only for gmac */
	u64 tx_64bytes;
	u64 tx_65to127;
	u64 tx_128to255;
	u64 tx_256to511;
	u64 tx_512to1023;
	u64 tx_1024to1518;
	u64 tx_1519tomax;
	u64 tx_1519tomax_good;	/* only for xgmac */
	u64 tx_oversize;	/* only for xgmac */
	u64 tx_jabber_err;
	u64 tx_underrun_err;	/* only for gmac */
	u64 tx_vlan;		/* only for gmac */
	u64 tx_crc_err;		/* only for gmac */
	u64 tx_pfc_tc0;
	u64 tx_pfc_tc1; 	/* only for xgmac */
	u64 tx_pfc_tc2; 	/* only for xgmac */
	u64 tx_pfc_tc3; 	/* only for xgmac */
	u64 tx_pfc_tc4; 	/* only for xgmac */
	u64 tx_pfc_tc5; 	/* only for xgmac */
	u64 tx_pfc_tc6; 	/* only for xgmac */
	u64 tx_pfc_tc7; 	/* only for xgmac */
	u64 tx_ctrl;		/* only for xgmac */
	u64 tx_1731_pkts;	/* only for xgmac */
	u64 tx_1588_pkts;	/* only for xgmac */
	u64 rx_good_from_sw;	/* only for xgmac */
	u64 rx_bad_from_sw;	/* only for xgmac */
};

struct mac_device {
	struct device *dev;
	void *priv;
	void __iomem *vaddr;
	void __iomem *cpld_vaddr;
	void __iomem *sys_ctl_vaddr;
	void __iomem *serdes_vaddr;
	u8 sfp_prsnt;
	u8 cpld_led_value;
	u16 phy_led_value;
	int txpkt_for_led;
	int rxpkt_for_led;
	int cpld_led_enable;
	s32 base_irq;
	s32 irq_num;
	struct mac_entry_idx addr_entry_idx[MAC_MAX_VM_NUM];
	u16 addr_entry_num;
	u8 mac_id;
	u8 global_mac_id;
	u8 chip_id;
	u8 promisc;
	enum mac_phy_interface phy_if;
	u32 if_support;
	u8 link;
	u8 half_duplex;
	u16 speed;
	u16 max_speed;
	u16 max_frm;
	u16 tx_pause_frm_time;
	enum mac_loop_mode loop_mode;
	struct phy_device	*phy_dev;

	#define MAC_PHY_BUS_ID_SIZE (MII_BUS_ID_SIZE + 3)
	u8 phy_bus_id[MAC_PHY_BUS_ID_SIZE];
	struct device_node *phy_node;

	struct mac_hw_stats hw_stats;

	struct list_head mc_addr_list;
	mac_change_addr_cb_t change_addr_cb;
	mac_change_mtu_cb_t change_mtu_cb;
	void * roce_dev;
	struct dsaf_device *dsaf_dev;
	char spf_ctl_addr[SPF_CTL_ADDR_NUM];
	/*bit0:phy, bit1:fixed-link, bit2 spf, bit3:cpld**/
	u32 link_features;

	int (*init_phy)(struct net_device *net_dev);
	int (*init)(struct mac_device *mac_dev);
	int (*start)(struct mac_device *mac_dev);
	int (*stop)(struct mac_device *mac_dev);
	int (*reset)(struct mac_device *mac_dev);
	int (*change_promisc)(struct mac_device *mac_dev);
	int (*change_addr)(struct mac_device *mac_dev, char *addr);
	int (*get_addr)(struct mac_device *mac_dev, char *addr);
	int (*set_multi)(struct mac_device *mac_dev,
			u32 queue, const u8 addr[MAC_NUM_OCTETS_PER_ADDR], u8 en);
	int (*uninit)(struct mac_device *mac_dev);
	int (*ptp_enable)(struct mac_device *mac_dev);
	int (*ptp_disable)(struct mac_device *mac_dev);
	int (*add_mac)(struct mac_device *mac_dev, u32 vfn, char *mac);
	int (*del_mac)(struct mac_device *mac_dev, u32 vfn, char *mac);
	int (*set_vf_mac)(struct mac_device *mac_dev, u32 queue, char *mac);
	int (*set_vf_vlan)(struct mac_device *mac_dev, u32 queue,
		u16 vlan, u8 qos);
	int (*get_vf_mac)(struct mac_device *mac_dev, u32 queue, char *mac);
	int (*get_vf_vlan)(struct mac_device *mac_dev, u32 queue,
		u16 *vlan, u8 *qos);
	int (*change_mtu)(struct mac_device *mac_dev, u32 new_mtu);
	int (*adjust_link)(struct mac_device *mac_dev, u16 speed,
		u8 full_duplex);
	int (*set_an_mode)(struct mac_device *mac_dev, u8 enable);
	int (*config_loopback)(struct mac_device *mac_dev,
		enum mac_loop_mode loop_mode);
	int (*pausefrm_cfg)(struct mac_device *mac_dev, u8 rx_en, u8 tx_en);
	int (*get_info)(struct mac_device *mac_dev, struct mac_info *mac_info);
	enum mac_mode (*get_mac_mode)(struct mac_device *mac_dev);
	int (*queue_config_bc_en)(struct mac_device *mac_dev,
		u32 queue, u8 en);

	int (*phy_reset)(struct mac_device *mac_dev);
	int (*autoneg_stat)(struct mac_device *mac_dev, u32 *enable);
	int (*set_pause_enable)(struct mac_device *mac_dev,
		u32 rx_pause_en, u32 tx_pause_en);
	int (*get_pause_enable)(struct mac_device *mac_dev,
		u32 *rx_pause_en, u32 *tx_pause_en);
	int (*get_link_status)(struct mac_device *mac_dev, u32 *link_stat);
	void (*get_regs)(struct mac_device *mac_dev,
		struct ethtool_regs *cmd, void *data);
	void (*get_strings)(struct mac_device *mac_dev, u32 stringset, u8 *data);
	int (*get_sset_count)(struct mac_device *mac_dev, u32 stringset);
	void (*get_ethtool_stats)(struct mac_device *mac_dev,
		struct ethtool_stats *cmd, u64 *data);
	int (*get_dump_regs)(struct mac_device *mac_dev);
	int (*sfp_led_opt)(struct mac_device *mac_dev);
	void (*led_reset)(struct mac_device *mac_dev);
	void (*led_set_id)(struct mac_device *mac_dev, int status);
	void (*get_total_txrx_pkts)(struct mac_device *mac_dev,
		int *tx_pkts, int *rx_pkts);
	int (*phy_set_led_id)(struct mac_device *mac_dev, int value);
	int (*sfp_open)(struct mac_device *mac_dev);
	int (*sfp_close)(struct mac_device *mac_dev);
	int (*get_sfp_prsnt)(struct mac_device *mac_dev);
};

struct mac_driver {
	/*init Mac when init nic or dsaf*/
	int (*mac_init)(void *mac_drv);
	/*remove mac when remove nic or dsaf*/
	int (*mac_free)(void *mac_drv);
	/*reset mac when reset nic or dsaf,wait is the waiting time
		if it's zero;if it's zero,no wait */
	int (*mac_reset)(void *mac_drv, u32 wait);
	/*enable mac when enable nic or dsaf*/
	int (*mac_enable)(void *mac_drv,
				enum mac_commom_mode mode);
	/*disable mac when disable nic or dsaf*/
	int (*mac_disable)(void *mac_drv,
				enum mac_commom_mode mode);
	/*get mac statistic and display the data*/
	int (*mac_get_statistics)(struct mac_device *mac_dev,
				struct mac_statistics *statistics,
				u8 reset_count);
	/* config mac address*/
	int (*mac_set_mac_addr)(void *mac_drv,	char *mac_addr);
	/*adjust mac mode of port,include speed and duplex*/
	int (*mac_adjust_link)(void *mac_drv,
				enum mac_speed speed,
				u32 full_duplex);
	/* config autoegotaite mode of port*/
	int (*mac_set_an_mode)(void *mac_drv, u8 enable);
	/* config loopbank mode */
	int (*mac_config_loopback)(void *mac_drv,
	enum mac_loop_mode loop_mode, u8 enable);
	/* config mtu*/
	int (*mac_config_max_frame_length)
				(void *mac_drv, u16 newval);
	/*config PAD and CRC enable */
	int (*mac_config_pad_and_crc)
				(void *mac_drv, u8 newval);
	/* config duplex mode*/
	int (*mac_config_half_duplex)
				(void *mac_drv, u8 newval);
	/* config 1588 enable */
	int (*mac_config_1588_time_stamp)
				(void *mac_drv, u8 enable);
	/*config tx pause time,if pause_time is zero,disable tx pause enable*/
	int (*mac_set_tx_auto_pause_frames)
				(void *mac_drv, u16 pause_time);
	/*config rx pause enable*/
	int (*mac_set_rx_ignore_pause_frames)(void *mac_drv,
			u32 enable);
	/* config rx mode for promiscuous*/
	int (*mac_set_promiscuous)(void *mac_drv, u8 enable);
	/* get mac id */
	int (*mac_get_id)(void *mac_drv, u8 *mac_id);
	/* get MTU*/
	u16 (*mac_get_max_frame_length)(void *mac_drv);
	/* write phy reg by mdio */
	int (*mac_mdio_write_phy_reg)(void *mac_drv, u8 phy_addr,
				u8 page, u32 reg, u16 data);
	/* read phy reg by mdio */
	int (*mac_mdio_read_phy_reg)(void *mac_drv, u8 phy_addr,
				u8 page, u32 reg, u16 *data);
	int (*mac_dump_regs)(void *mac_drv);
	int (*mac_pausefrm_cfg)(void *mac_drv, u32 uwrxpauseen,
		u32 uwtxpauseen);

	int (*autoneg_stat)(void *mac_drv , u32 *enable);
	int (*set_pause_enable)(void *mac_drv, u32 uwrxpauseen,
		u32 uwtxpauseen);
	int (*get_pause_enable)(void *mac_drv, u32 *uwrxpauseen,
		u32 *uwtxpauseen);
	int (*get_link_status)(void *mac_drv, u32 *link_stat);
	/* get the imporant regs*/
	void (*get_regs)(void *mac_drv, struct ethtool_regs *cmd,
					void *data);
	/* get strings name for ethtool statistic */
	void (*get_strings)(void *mac_drv, u32 stringset, u8 *data);
	/* get the number of strings*/
	int (*get_sset_count)(void *mac_drv, u32 stringset);

	/* get the statistic by ethtools*/
	void (*get_ethtool_stats)(struct mac_device *mac_dev,
				struct ethtool_stats *cmd, u64 *data);
	void (*get_total_txrx_pkts)(void *mac_drv,
		u32 *tx_pkts, u32 *rx_pkts);

	/* get mac information */
	int (*get_info)(void *mac_drv, struct mac_info *mac_info);

	int (*mac_ioctl)(void *mac_drv, int cmd, u32 offset, u32 *value);

	void (*update_stats)(struct mac_device *mac_dev);
	void (*clean_stats)(struct mac_device *mac_dev);

	enum mac_mode mac_mode;
	u8 mac_id;
	u8 chip_id;
	void __iomem *vaddr;
	unsigned int mac_en_flg;/*you'd better don't enable mac twice*/
	unsigned int virt_dev_num;
	struct device *dev;
};


#define MAC_MAKE_MODE(interface, speed) (enum mac_mode)((interface) | (speed))
#define MAC_INTERFACE_FROM_MODE(mode) (enum mac_intf)((mode) & 0xFFFF0000)
#define MAC_SPEED_FROM_MODE(mode) (enum mac_speed)((mode) & 0x0000FFFF)
#define MAC_STATS_FIELD_OFF(field) (offsetof(struct mac_hw_stats,field))
#define MAC_STATS_READ(p, offset) *((u64*)((u64)(p) + (offset)))

static inline void *mac_dev_priv(const struct mac_device *mac_dev)
{
	return (void *)((u64)mac_dev + sizeof(*mac_dev));
}
extern int mac_register_roce_cb(struct mac_device *mac_dev,
	void * dev, mac_change_addr_cb_t change_addr_cb,
	mac_change_mtu_cb_t change_mtu_cb);
extern void mac_update_stats(struct mac_device *mac_dev);
extern void mac_clean_stats(struct mac_device *mac_dev);

extern void * gmac_config(struct mac_params *mac_param);
extern void * xgmac_config(struct mac_params *mac_param);
extern void mac_setup_gmac(struct mac_device *mac_dev);
extern void mac_setup_xgmac(struct mac_device *mac_dev);

#define cpld_reg_write(addr, data)\
    (writeb((data), (__iomem unsigned char *)(addr)))
#define cpld_reg_read(addr)\
    (readb((__iomem unsigned char *)(addr)))
#define serdes_reg_write(addr, data)\
    writel((data), (__iomem unsigned int *)(addr))
#define serdes_reg_read(addr)\
    (readl((__iomem unsigned int *)(addr)))

#endif /* _IWARE_MAC_MAIN_H */

