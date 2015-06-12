/*******************************************************************************

  Hisilicon network interface controller driver
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

#ifndef _PPE_MAIN_H
#define _PPE_MAIN_H
/* #include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_dsaf_main.h"
#include "iware_mac_main.h"
#include "iware_rcb_main.h"

#include <linux/platform_device.h>

struct ppe_device;
struct ppe_common_dev;
struct rcb_device;

#define PPE_COMMON_DRIVER_NAME "ppe-common"

#define PPE_COMMON_REG_SIZE 0x10000
#define PPE_COMMON_REG_OFFSET 0x70000
#define PPE_REG_OFFSET 0x10000
#define PPE_REG_IRQ_OFFSET 1

enum ppe_common_mode {
	PPE_COMMON_MODE_SINGLE = 0,
	PPE_COMMON_MODE_MULTI,
	PPE_COMMON_MODE_MAX
};

struct ppe_ops {
	int (*init)(struct ppe_device *ppe_dev);
	void (*uninit)(struct ppe_device *ppe_dev);
	void (*checksum_hw)(struct ppe_device *ppe_dev, u32 value);
	void (*get_regs)(struct ppe_device *ppe_dev, struct ethtool_regs *cmd,
			  void *data);
	void (*get_strings)(struct ppe_device *ppe_dev, u32 stringset,
			     u8 *data);
	int (*get_sset_count)(struct ppe_device *ppe_dev, u32 stringset);
	void (*show_stat_by_port)(struct ppe_device *ppe_dev);
	void (*get_ethtool_stats)(struct ppe_device *ppe_dev,
				   struct ethtool_stats *cmd, u64 *data);
	/*dump all regs */
	void (*dump_regs)(struct ppe_device *ppe_dev);
};

struct ppe_hw_stats {
	u64 rx_pkts_from_sw;
	u64 rx_pkts;
	u64 rx_drop_no_bd;
	u64 rx_alloc_buf_fail;
	u64 rx_alloc_buf_wait;
	u64 rx_drop_no_buf;
	u64 rx_err_fifo_full;
	u64 rx_multicast;  	/* only for stats modified */
	u64 tx_bd_form_rcb;
	u64 tx_pkts_from_rcb;
	u64 tx_pkts;
	u64 tx_err_fifo_empty;
	u64 tx_err_checksum;
};

struct ppe_device {
	struct ppe_device *next;	/* pointer to next ppe device */
	struct net_device *netdev;

	u8 index;
	u8 chip_id;

	void __iomem *vbase;
	s32 base_irq;
	s32 irq_num;
	int virq;
	enum dsaf_mode dsaf_mode;
	enum ppe_common_mode ppe_mode;

	struct ppe_hw_stats hw_stats;
	struct ppe_ops ops;
	u8 comm_index;

};

struct ppe_common_ops {
	int (*init)(struct ppe_common_dev *ppe_common);
	void (*uninit)(struct ppe_common_dev *ppe_common);
	void (*reset)(struct ppe_common_dev *ppe_common);
};

struct ppe_common_dev {
	struct device *dev;
	phys_addr_t base;
	void __iomem *vbase;

	spinlock_t lock;

	enum dsaf_mode dsaf_mode;
	enum ppe_common_mode ppe_mode;

	struct ppe_common_ops ops;

	u16 ref_cnt;	/*ppe_common cent,remove rcb_common when it's zero */
	u8 comm_index;   /*ppe_common index*/

};

extern int ppe_init(struct platform_device *pdev, enum dsaf_mode dsaf_mode);
extern void ppe_uninit(struct platform_device *pdev);
extern int ppe_commom_init(void);
extern void ppe_show_stat_by_port(struct device *dev);
extern void ppe_commom_uninit(void);
extern void ppe_show_sw_stat_by_port(struct ppe_device *ppe_device);
extern void ppe_common_show_sw_info(struct ppe_common_dev *ppe_commom);
extern void ppe_set_ops(struct ppe_ops *ops);
extern void ppe_set_common_ops(struct ppe_common_ops *ops);
extern void ppe_update_stats(struct ppe_device *ppe_dev);
extern void ppe_reset(struct ppe_device *ppe_dev);

/* TBD */
extern void rcb_show_tx_cnt(struct nic_ring_pair *ring);
extern void rcb_show_rx_cnt(struct nic_ring_pair *ring);
extern int mac_show_statistics(struct mac_device *);
extern u32 HRD_Dsaf_PpeSrstByPort(u32 ulPort, u32 ulEn);
extern u32 HRD_Dsaf_PpeComSrst(u32 ulCommId, u32 ulEn);

#endif				/* _PPE_MAIN_H */
