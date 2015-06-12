/*******************************************************************************

  Hisilicon dsa fabric driver
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

#ifndef _HRD_DSAF_MAIN_H
#define _HRD_DSAF_MAIN_H

/* #include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_log.h"
#include "iware_mac_main.h"

/****************************************************************************
** @author c00316045                   **
** @detail add for dsaf debug , for three are no
** ic_disable_msi(int virq) and int ic_enable_msi(int hwirq, int *virq)
** in kernel hulk3.19
**
****************************************/
#define DSAF_NONE_IRQ      0

/****************************************/

#define DSAF_MOD_VERSION "iWareV2R2C00B981SP1"

#define DSAF_MOD_NAME "DSAF"
#define DSAF_DRV_NAME "Hi-DSAF"
#define DSAF_DEV0_NAME "DSAF 0"
#define DSAF_DEV1_NAME "DSAF 1"
#define DSAF_DEV2_NAME "DSAF 2"
#define DSAF_DEV3_NAME "DSAF 3"

#define DSAF_BASE_INNER_PORT_NUM 127  /* mac tbl qid*/

#define DSAF_MAX_REAL_PORT_ID_PER_CHIP 5  /*max index phy-port per dsaf*/
#define DSAF_MAX_PORT_NUM_PER_CHIP 7  /*max 6 phy port and 1 vir port*/
#define DSAF_MAX_CHIP_NUM 2  /*max 2 chips */
#define DSAF_MAX_PORT_NUM (DSAF_MAX_CHIP_NUM * DSAF_MAX_PORT_NUM_PER_CHIP)

#define DSAF_IRQ_NUM 18
#define DSAF_PPE_INODE_BASE 6
#define DSAF_INODE_NUM		18

/* dsaf mode define */
enum dsaf_mode {
	DSAF_MODE_INVALID = 0,			/**< Invalid dsaf mode */
	DSAF_MODE_ENABLE_FIX,			/**< en DSAF-mode, fixed to queue*/
	DSAF_MODE_ENABLE_0VM,			/**< en DSAF-mode, support 0 VM */
	DSAF_MODE_ENABLE_8VM,			/**< en DSAF-mode, support 8 VM */
	DSAF_MODE_ENABLE_16VM,			/**< en DSAF-mode, support 16 VM */
	DSAF_MODE_ENABLE_32VM,			/**< en DSAF-mode, support 32 VM */
	DSAF_MODE_ENABLE_128VM,		/**< en DSAF-mode, support 128 VM */
	DSAF_MODE_ENABLE,				/**< befpre is enable DSAF mode*/
	DSAF_MODE_DISABLE_FIX,			/**< non-dasf, fixed to queue*/
	DSAF_MODE_DISABLE_2PORT_8VM,	/**< non-dasf, 2port 8VM */
	DSAF_MODE_DISABLE_2PORT_16VM,	/**< non-dasf, 2port 16VM */
	DSAF_MODE_DISABLE_2PORT_64VM,	/**< non-dasf, 2port 64VM */
	DSAF_MODE_DISABLE_6PORT_0VM,	/**< non-dasf, 6port 0VM */
	DSAF_MODE_DISABLE_6PORT_2VM,	/**< non-dasf, 6port 2VM */
	DSAF_MODE_DISABLE_6PORT_4VM,	/**< non-dasf, 6port 4VM */
	DSAF_MODE_DISABLE_6PORT_16VM,	/**< non-dasf, 6port 16VM */
	DSAF_MODE_MAX					/**< the last one, use as the num */
};

/* dsaf Vlan mode */
enum dsaf_vlan_mode {
	DSAF_VLAN_MODE_KEEP = 0, /**<donnot modify Vlan tag */
	DSAF_VLAN_MODE_ADD, /**<add Vlan tag */
	DSAF_VLAN_MODE_REMPVE,		/**<del Vlan tag */
	DSAF_VLAN_MODE_MAX
};

#define DSAF_DEST_PORT_NUM 256	/* DSAF max port num */
#define DSAF_WORD_BIT_CNT 32  /* the num bit of word */

/*mac entry, mc or uc entry*/
struct dsaf_drv_mac_single_dest_entry {
	/* mac addr, match the entry*/
	u8 addr[MAC_NUM_OCTETS_PER_ADDR];
	u16 in_vlan_id; /* value of VlanId */
	#if 0/*TBD*/
	enum dsaf_vlan_mode vlan_mode;
	u16 out_vlan_id; /* if vlan_mode is vld, this is vlan in vlan-tag  */
	u8 qos;/* if vlan_mode add vld, qos in vlan-tag  */
	#else
	/* the vld input port num, dsaf-mode fix 0,
		non-dasf is the entry whitch port vld*/
	u8 in_port_num;
	#endif
	u8 port_num; /*output port num*/
	u8 rsv[6];
};

/*only mc entry*/
struct dsaf_drv_mac_multi_dest_entry {
	/* mac addr, match the entry*/
	u8 addr[MAC_NUM_OCTETS_PER_ADDR];
	u16 in_vlan_id;
	/* this mac addr output port,
		bit0-bit5 means Port0-Port5(1bit is vld)**/
	u32 port_mask[DSAF_DEST_PORT_NUM/DSAF_WORD_BIT_CNT];
	#if 0/*TBD*/
	enum dsaf_vlan_mode vlan_mode;
	u16 out_vlan_id; /* if vlan_mode is vld, this is vlan in vlan-tag  */
	u8 qos;/* if vlan_mode add vld, qos in vlan-tag  */
	#else
	/* the vld input port num, dsaf-mode fix 0,
		non-dasf is the entry whitch port vld*/
	u8 in_port_num;
	u8 rsv[7];
	#endif
};

struct dsaf_inode_hw_stats {
	u64 pad_drop;
	u64 sbm_drop;
	u64 crc_false;
	u64 bp_drop;
	u64 rslt_drop;
	u64 local_addr_false;
	u64 vlan_drop;
	u64 stp_drop;
};

#define DSAF_NOF_TC_PER_PORT 8 /*max is 8 tcs */

/* Dsaf device struct define ,and mac ->  dsaf */
struct dsaf_device {
	struct device *dev;

	void *priv;
	void __iomem *vaddr; /* maped vir addr */
	s32 base_irq; /*the first irq-id */

	int virq[DSAF_IRQ_NUM];
	/*array : mac device struct pointers, ctl macs in dsaf, */
	struct mac_device *mac_dev[MAC_MAX_PORT_NUM_PER_CHIP];

	enum  dsaf_mode dsaf_mode;	 /* dsaf mode  */
	u8 chip_id;/* Chip ID, single is 0 , muilt is 0-3*/

	struct dsaf_inode_hw_stats inode_hw_stats[DSAF_INODE_NUM];

	/* Dsaf drv init */
	int (*dsaf_init)(struct dsaf_device *dsaf_dev);
	/* Dsaf drv free */
	int (*dsaf_free)(struct dsaf_device *dsaf_dev);

	/* Mac learn ctl, if is 0 Mac dis-leanrn; else learn */
	int (*ctrl_mac_learning)(struct dsaf_device *dsaf_dev, u8 enable);
	/* cfg whether promiscuous, 0 is disable, 1 is enable*/
	int (*set_promiscuous)(struct dsaf_device *dsaf_dev, u8 enable);

	/* Mac uc entry cfg, if none add one, else exist modify*/
	int (*set_mac_uc_entry)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_single_dest_entry *mac_entry);
	/* Mac mc entry cfg, if none add one, else exist just add a port*/
	int (*add_mac_mc_port)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_single_dest_entry *mac_entry);
	/* Mac mc entry cfg, if none add one, else exist modify*/
	int (*set_mac_mc_entry)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_multi_dest_entry *mac_entry);
	/* del matched entry*/
	int (*del_mac_entry)(struct dsaf_device *dsaf_dev, u16 vlan_id,
		u8 in_port_num, u8 *addr);
	/* del mac entry,
	if the port is the only one in this entry then del the entry,
	else just del one port*/
	int (*del_mac_mc_port)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_single_dest_entry *mac_entry);

	int (*get_mac_uc_entry)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_single_dest_entry *mac_entry);
	int (*get_mac_mc_entry)(struct dsaf_device *dsaf_dev,
		struct dsaf_drv_mac_multi_dest_entry *mac_entry);

	/* get mac entry by index, used for mulit entrys*/
	int (*get_mac_entry_by_index)(struct dsaf_device *dsaf_dev,
		u16 entry_index,
		struct dsaf_drv_mac_multi_dest_entry *mac_entry);

	/**fixe dsaf all mac port work mode,
		the mode is mac cfg, but dsaf neen it*/
	int (*fix_mac_mode)(struct dsaf_device *dsaf_dev, u8 port_id);

	/* DSAF int handle function*/
	void (*dsaf_int_proc)(struct dsaf_device *dsaf_dev, u32 irq_num);

	int (*set_port_def_vlan)(struct dsaf_device *dsaf_dev,
		u8 port_id, u32 port_def_vlan);
	int (*get_port_def_vlan)(struct dsaf_device *dsaf_dev,
		u8 port_id, u16 *port_def_vlan);

	/*ETHTOOL*/
	void (*get_regs)(struct dsaf_device *dsaf_dev,
		struct ethtool_regs *cmd, void *data);
	void (*get_strings)(struct dsaf_device *dsaf_dev,
		u32 stringset, u8 *data);
	/* get len of get_strings return*/
	int (*get_sset_count)(struct dsaf_device *dsaf_dev, u32 stringset);
	void (*get_ethtool_stats)(struct dsaf_device *dsaf_dev,
		struct ethtool_stats *cmd, u64 *data);

};

/* this is for dsaf_ioctl()***/
enum net_loop_mode {
	HRD_NET_LOOP_NONE = 0,
	HRD_NET_INTERNALLOOP_MAC,	   /* MAC inner loop */
	HRD_NET_INTERNALLOOP_SGMII,    /* SGMII inner loop */
	HRD_NET_INTERNALLOOP_SERDES,   /* serdes inner loop */
	HRD_NET_INTERNALLOOP_PHY,	   /* PHY inner loop */
	HRD_NET_EXTERNALLOOP_MAC,	   /* MAC outer loop */
	HRD_NET_EXTERNALLOOP_SGMII,    /* SGMII outer loop */
	HRD_NET_EXTERNALLOOP_SERDES,   /* serdes outer loop */
	HRD_NET_EXTERNALLOOP_PHY	   /* PHY outer loop */

};

/**< Number of octets (8-bit bytes) in an ethernet address */
#define HRD_NET_NUM_OCTETS_PER_ADDRESS MAC_NUM_OCTETS_PER_ADDR
/*typedef u8 HRD_NET_ADDR_T[HRD_NET_NUM_OCTETS_PER_ADDRESS];*/

enum net_speed {
	HRD_NET_SPEED_10	 = 10, /**< 10 Mbps */
	HRD_NET_SPEED_100	 = 100, /**< 100 Mbps */
	HRD_NET_SPEED_1000	 = 1000, /**< 1000 Mbps = 1 Gbps */
	HRD_NET_SPEED_10000  = 10000 /**< 10000 Mbps = 10 Gbps */
};

struct dsaf_mac_tbl_entry {
	/*mac_address : if has this entry, return error*/
	u8 mac_address[HRD_NET_NUM_OCTETS_PER_ADDRESS];
	u16 vlan_id;
	u32 port_msk; /*bit0-bit5 value is Port0-Port5*/
};

struct dsaf_mutil_mac_tbl_entry {
	u32 begin_id; /*q begin index, 0-511 */
	u32 ety_num; /*q num*/
	struct dsaf_mac_tbl_entry *mac_ety; /*begin pointer*/
	u32 ret_num; /**/
};

struct dsaf_port_attr {
	u32 port_id;
	u32 auto_neg;
	u32 duplex;

	enum net_speed speed;
	enum net_loop_mode loop_mode;

	/*def VlanId, if none-vlanid, use this */
	u16 vlan_id;

	/*VlanTag modefy mode 0: donnot modify;
	1: add (DSA port type is assess);
	2:del (DSA port type is trunk) */
	u16 vlan_tag_mode;

};

struct dsaf_port_enable_ctrl {
	u32 port_id; /*port = 0-5 */
	u32 enable;
};

struct dsaf_port_statistics_ctrl {
	u32 port_id; /*port = 0-5 */
	u32 ulResetCnt; /*clr statis, 0 is no, or yes*/
	struct mac_statistics stStatistics;
};

struct dsaf_port_fc_mode {
	u32 port_id; /*port = 0-5 */
	u32 fc_mode; /*pause mode 0: disenable;
			1: enable normal pause; 2: enable pfc*/
};

#define HRD_DSAF_NOF_TC_PER_PORT 8
struct dsaf_port_ets_mode {
	u32 port_id; /*port = 0-5 */
	/*if is 0 this TC is SP*/
	u32 weigth_arry[HRD_DSAF_NOF_TC_PER_PORT];
};

/*dasf MAGIC*/
#define DSAF_IOC_MAGIC	 ('D'<<24|'S'<<16|'A'<<8|'F')
/*
CTRL_MAC_LEARNING*/
#define DSAF_CMD_CTRL_MAC_LEARNING\
	_IOW(DSAF_IOC_MAGIC, 0, u32)
/*
ADD_MAC_ENTRY*/
#define DSAF_CMD_ADD_MAC_ENTRY\
	_IOW(DSAF_IOC_MAGIC, 1, struct dsaf_mac_tbl_entry)
/*
DEL_MAC_ENTRY*/
#define DSAF_CMD_DEL_MAC_ENTRY\
	_IOW(DSAF_IOC_MAGIC, 2, struct dsaf_mac_tbl_entry)

/*
MODIFY_MAC_ENTRY */
#define DSAF_CMD_MODIFY_MAC_ENTRY\
	_IOW(DSAF_IOC_MAGIC, 3, struct dsaf_mac_tbl_entry)
/*
SET_MAC_AGE_TIME*/
#define DSAF_CMD_SET_MAC_AGE_TIME\
	_IOW(DSAF_IOC_MAGIC, 4, u32)
/*
GET_MAC_AGE_TIME*/
#define DSAF_CMD_GET_MAC_AGE_TIME\
	_IOR(DSAF_IOC_MAGIC, 5, u32*)
/*
GET_MAC_ENTYR*/
#define DSAF_CMD_GET_MAC_ENTYR\
	_IOR(DSAF_IOC_MAGIC, 6, struct dsaf_mac_tbl_entry)
/*
SET_PORT_NEG*/
#define DSAF_CMD_SET_PORT_NEG\
	_IOW(DSAF_IOC_MAGIC, 7, struct dsaf_port_attr)
/*
GET_PORT_NEG*/
#define DSAF_CMD_GET_PORT_NEG\
	_IOR(DSAF_IOC_MAGIC, 8, struct dsaf_port_attr)
/*
SET_PORT_ENABLE*/
#define DSAF_CMD_SET_PORT_ENABLE\
	_IOW(DSAF_IOC_MAGIC, 9, struct dsaf_port_enable_ctrl)
/*
GET_PORT_ENABLE*/
#define DSAF_CMD_GET_PORT_ENABLE\
	_IOR(DSAF_IOC_MAGIC, 10, struct dsaf_port_enable_ctrl)
/*
GET_PORT_STATISTICS*/
#define DSAF_CMD_GET_PORT_STATISTICS\
	_IOR(DSAF_IOC_MAGIC, 11, struct dsaf_port_statistics_ctrl)
/*
SET_PORT_FC_MODE*/
#define DSAF_CMD_SET_PORT_FC_MODE\
	_IOW(DSAF_IOC_MAGIC, 12, struct dsaf_port_fc_mode)
/*
GET_PORT_FC_MODE*/
#define DSAF_CMD_GET_PORT_FC_MODE\
	_IOR(DSAF_IOC_MAGIC, 13, struct dsaf_port_fc_mode)
/*
SET_PORT_ETS_MODE*/
#define DSAF_CMD_SET_PORT_ETS_MODE\
	_IOW(DSAF_IOC_MAGIC, 14, struct dsaf_port_ets_mode)
/*
GET_PORT_ETS_MODE*/
#define DSAF_CMD_GET_PORT_ETS_MODE\
	_IOR(DSAF_IOC_MAGIC, 15, struct dsaf_port_ets_mode)
/*
GET_MUTIL_MAC_ENTYR*/
#define DSAF_CMD_GET_MUTIL_MAC_ENTYR\
	_IOR(DSAF_IOC_MAGIC, 16, struct dsaf_mutil_mac_tbl_entry)


static inline void *dsaf_dev_priv(const struct dsaf_device *dsaf_dev)
{
	return (void *)((u64)dsaf_dev + sizeof(*dsaf_dev));
}

extern struct dsaf_device *dsaf_register_mac_dev(
	struct mac_device *mac_dev);
extern int dsaf_unregister_mac_dev(struct mac_device *mac_dev);
extern struct mac_device *dsaf_get_mac_dev(u8 dsaf_id, u8 port_id);
extern int dsaf_get_work_mode(u8 dsaf_id, enum	dsaf_mode *mode);
extern int dsaf_add_mac_entry(u8 dsaf_id, u16 vlan_id, char *mac_addr,
	u32 port_mask);
extern void dsaf_update_stats(struct dsaf_device *dsaf_dev, u32 inode_num);

extern int dsaf_config(struct dsaf_device *dsaf_dev);

/* Dsaf prvi data size*/
extern size_t g_dsaf_sizeof_priv;

#endif /* _HRD_DSAF_MAIN_H */



