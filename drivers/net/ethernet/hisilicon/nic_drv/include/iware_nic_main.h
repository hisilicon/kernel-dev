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

#ifndef _NIC_MAIN_H
#define _NIC_MAIN_H
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/if_vlan.h>

/*#include "hrd_module.h"
#include "hrd_typedef.h"
#include "iwr_eth_api.h"*/
#include "iware_mac_main.h"
#include "iware_ppe_main.h"
#include "iware_rcb_main.h"

struct nic_device;

#define NIC_MOD_VERSION "iWareV2R2C00B981SP1"

#define NIC_MAX_RING_PAIR_NUM 16
#define NIC_MAX_RX_QUEUES 16
#define NIC_MAX_Q_PER_VF 16

#define NIC_MAX_NUM_PER_CHIP 8

#define NIC_DEFAULT_MTU 1500

#define NIC_TX_CLEAN_MAX_NUM 256
#define NIC_RX_CLEAN_MAX_NUM 64

#ifndef ALIGN
#define ALIGN(x, a)	 __ALIGN_MASK((x), (typeof(x)) (a) - 1)
#endif
#ifndef __ALIGN_MASK
#define __ALIGN_MASK(x, mask)   (((x) + (mask)) & ~(mask))
#endif
#ifndef PTR_ALIGN
#define PTR_ALIGN(p, a)	 ((typeof(p)) ALIGN((unsigned long) (p), (a)))
#endif

/* eth device name, such as platform device name */
#define DRIVER_NAME "Hi-NIC"

/* Transmitter timeout */
#define NIC_TX_TIMEOUT					  (5 * HZ)

#define ENABLE							  (0x1)
#define DISABLE							 (0x0)

enum nic_state {
	NIC_STATE_TESTING = 0,
	NIC_STATE_RESETTING,
	NIC_STATE_REINITING,
	NIC_STATE_DOWN,
	NIC_STATE_DISABLED,
	NIC_STATE_REMOVING,
	NIC_STATE_SERVICE_INITED,
	NIC_STATE_SERVICE_SCHED,
	NIC_STATE_MAX
};

struct nic_ops {
	int (*change_mtu)(struct nic_device *nic_dev, int new_mtu);
	int (*set_mac_address)(struct nic_device *nic_dev, char *mac_addr);
	void (*set_multicast_one)(struct nic_device *dev, const u8 *mac);
	void (*set_promiscuous_mode)(struct nic_device *dev);
};

struct eeprom_addr {
	char init_flag;
	char chip_addr;
	int i2c_id;
	unsigned int sub_addr;
};

struct nic_device {
	struct net_device *netdev;
	struct device *dev;

	u16 max_vfn;
	u16 max_q_per_vf;

	struct nic_ring_pair * ring[NIC_MAX_RING_PAIR_NUM];

	u32 tx_timeout_count;
	u64 hw_csum_rx_error;
	u64 hw_rx_no_dma_resources;
	u64 rsc_total_count;
	u64 rsc_total_flush;
	unsigned long state;
#define NIC_FLAG_RESET (u32)(1 << 0)
#define NIC_FLAG_STOP_STAT (u32)(1 << 1)
	u32 flag;

	int ring_pair_num;	/* current number of ring pair for device */

	struct nic_ring_pair *test_ring;

	spinlock_t lock;	/* during all ops except TX pckt processing */
	int index;/* index in one chip£¬nic in switch is 0-5£¬others is 6¡¢7 */
	u32 chip_id;
	int gidx;		/* index for all nic */

	struct eeprom_addr eeprom_addr;

	u16 desc_cnt;
	u16 coalesced_frames;
	u32 time_out;

	struct timer_list service_timer;
	struct work_struct service_task;
	struct delayed_work hb_timer;
	struct mac_device *mac_dev;
	struct ppe_device *ppe_device;
	struct ppe_common_dev *ppe_common;
	struct rcb_common_dev *rcb_common;
	int global_virq;
	unsigned int mtu;	/* interface MTU value          */
	struct nic_ops ops;

};

extern int nic_tx_napi_poll(struct napi_struct *napi, int budget);
extern int nic_rx_napi_poll(struct napi_struct *napi, int budget);
extern u32 nic_get_chip_id(struct platform_device *pdev);
extern void rcb_show_stat_by_port(struct device *dev);
extern void mac_show_stat_by_port(struct device *dev);
extern void nic_show_sw_stat_by_port(struct net_device *netdev);
extern void nic_set_ops(struct nic_ops *ops);
extern void nic_reset(struct net_device *netdev);
extern void nic_reinit(struct net_device *netdev);
extern int rcb_recv_hw_ex(struct nic_ring_pair * ring, int budget,
	int(* rx_process)(struct napi_struct *, struct sk_buff *));

#endif
