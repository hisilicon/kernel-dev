/************************************************************************

  Hisilicon NIC driver
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

#ifndef _HRD_RCB_MAIN_H
#define _HRD_RCB_MAIN_H
/* #include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_dsaf_main.h"

#include <linux/netdevice.h>

#include <linux/platform_device.h>

struct nic_ring_pair;
struct rcb_common_dev;
struct rcb_device;
struct nic_device;

#define RCB_COMMON_DRIVER_NAME "rcb-common"

#define RCB_COMMON_IRQ_OFFSET 4

#define RCB_COMMON_MAX_DESC_CNT 1024
#define RCB_COMMON_MIN_DESC_CNT 16

#define RCB_IRQ_TX_RX_OFFSET 1
#define RCB_IRQ_NUM_PER_QUEUE 2
#define RCB_IRQ_IDX_TX 0
#define RCB_IRQ_IDX_RX 1

#define RCB_RING_JUNBO_MAX_PENDING_BD 65534
#define RCB_RING_MAX_PENDING_BD 129
#define RCB_RING_MIN_PENDING_BD 10

#define RCB_RING_MAX_BD_PER_PKT 3

#define RCB_MUTIL_BD_SUPPORT 0
#define RCB_REG_OFFSET 0x10000
#define RCB_COMMON_REG_SIZE 0x10000
#define RCB_IRQ_OFFSET 2

#define RCB_MAX_COALESCED_FRAMES 50

#define RCB_MAX_TIME_OUT 0x5dc0

#define RCB_IRQ_NAME_LEN 12
#define RCB_ALL_RING_NUM 96

#define RCB_IRQ_NOT_INITED 0
#define RCB_IRQ_INITED 1


enum rcb_int_flag {
	RCB_INT_FLAG_TX = 0x1,
	RCB_INT_FLAG_RX = (0x1 << 1),
	RCB_INT_FLAG_ERR = (0x1 << 2),
	RCB_INT_FLAG_MAX = (0x1 << 3),	/*must bu the last element */
};

enum rcb_common_mode {
	RCB_COMMON_MODE_SINGLE = 0,
	RCB_COMMON_MODE_MULTI,
	RCB_COMMON_MODE_MAX
};

struct rcb_operation {
	int (*init)(struct nic_ring_pair *ring);
	void (*uninit)(struct nic_ring_pair *ring);
	int (*enable)(struct nic_ring_pair *ring);
	void (*disable)(struct nic_ring_pair *ring);
	void (*reset_ring)(struct nic_ring_pair *ring);
	netdev_tx_t (*pkt_send)(struct sk_buff *skb, struct nic_ring_pair *ring);
	/*ring  recieve oackets */
	int (*pkt_recv)(struct sk_buff *skb, struct nic_ring_pair *ring);
	int (*multi_pkt_recv)(struct nic_ring_pair *ring, int budget);
	bool (*clean_tx_irq)(struct nic_ring_pair *ring);
	void (*clean_tx_ring)(struct nic_ring_pair *ring);
	u32 (*get_tx_ring_head)(struct rcb_device *rcb_dev);
	u32 (*get_rx_ring_vld_bd_num)(struct rcb_device *rcb_dev);

	/* get key regs */
	void (*get_regs)(struct nic_ring_pair *ring,
			  struct ethtool_regs *cmd, void *data);

	/* get strings name for ethtool */
	void (*get_strings)(struct nic_ring_pair *ring,
			     u32 stringset, u8 *data);
	/*get strings count for ethtool */
	int (*get_sset_count)(struct nic_ring_pair *ring, u32 stringset);

	void (*get_ethtool_stats)(struct nic_ring_pair *ring,
				   struct ethtool_stats *cmd, u64 *data);

	/*coalesce timer */
	int (*set_rx_coalesce_usecs)(struct nic_ring_pair *ring, u32 usecs);
	int (*get_rx_coalesce_usecs)(struct nic_ring_pair *ring, u32 *usecs);

	/*coalesce frames */
	int (*set_rx_max_coalesced_frames)(struct nic_ring_pair *ring,
					    u32 frame_num);
	int (*get_rx_max_coalesced_frames)(struct nic_ring_pair *ring,
					    u32 *frame_num);

	/*coalesce enable */
	int (*set_use_adaptive_rx_coalesce)(struct nic_ring_pair *ring,
					     u32 enable);
	int (*get_use_adaptive_rx_coalesce)(struct nic_ring_pair *ring,
					     u32 *enable);

	/*dump regs */
	void (*dump_reg)(struct nic_ring_pair *ring);
	void (*isr_handle)(void);
	void (*int_ctrl)(struct rcb_device *rcb_dev, u32 flag,
			  u32 enable);

};

struct rcb_device {
	phys_addr_t base;
	void __iomem *vbase;

	u16 index;
	u16 buf_size;

	s32 base_irq;
	int virq[RCB_IRQ_NUM_PER_QUEUE];
	int irq_init_flag;
	char irq_name[RCB_IRQ_NUM_PER_QUEUE][RCB_IRQ_NAME_LEN];

	u8 queue_index;		/* needed for multiqueue queue management */
	u8 max_buf_num;
	u8 port_id_in_dsa;
	u8 comm_index;

	struct rcb_operation ops;

};

struct nic_rx_queue_stats {
	u64 rx_buff_err;
	u64 non_vld_descs;
	u64 err_pkt_len;
	u64 alloc_rx_page_failed;
	u64 alloc_rx_buff_failed;
	u64 csum_err;
	u64 non_eop_descs;
};

struct nic_tx_queue_stats {
	u64 restart_queue;
	u64 tx_busy;
	u64 tx_done_old;
};

struct nic_tx_buffer {

	struct sk_buff *skb;
	unsigned int bytecount;
	u16 buf_num;		/* buffer num per pkg */
	DEFINE_DMA_UNMAP_ADDR(dma);
	DEFINE_DMA_UNMAP_LEN(len);
};

struct nic_rx_buffer {
	struct sk_buff *skb;
	dma_addr_t dma;
	struct page *page;
	unsigned int page_offset;
};

struct nic_tx_ring {
	void *desc;		/*descriptor ring memory */
	struct nic_tx_buffer *tx_buffer_info;
	u16 count;		/* amount of descriptors */
	u32 numa_node;
	u16 next_to_use;
	u16 next_to_clean;
	u64 tx_pkts;
	u64 tx_bytes;
	u64 tx_err_cnt;

	struct napi_struct napi ____cacheline_aligned_in_smp;
	struct nic_tx_queue_stats tx_stats;
	/* 保证结构体按照cacheline对齐 */

};

struct nic_ring_hw_stats {
	u64 tx_pkts;
	u64 rx_pkts;
};

struct nic_rx_ring {
	void *desc;		/*descriptor ring memory */
	struct nic_rx_buffer *rx_buffer_info;
	u16 count;		/* amount of descriptors */

	u32 numa_node;
	u16 next_to_use;
	u16 next_to_clean;
	u16 next_to_alloc;

	u64 rx_pkts;
	u64 rx_bytes;
	struct napi_struct napi ____cacheline_aligned_in_smp;
	struct nic_rx_queue_stats rx_stats;

};

struct nic_ring_pair {
	struct net_device *netdev;	/* netdev ring belongs to */
	struct device *dev;	/*device for DMA mapping */
	struct rcb_device rcb_dev;
	u8 queue_index;		/* needed for multiqueue queue management */
	u8 max_buf_num;
	struct nic_ring_hw_stats hw_stats;
	struct nic_tx_ring tx_ring ____cacheline_aligned_in_smp;
	struct nic_rx_ring rx_ring ____cacheline_aligned_in_smp;
};

struct rcb_common_ops {
	int (*init)(struct rcb_common_dev *rcb_common);
	void (*uninit)(struct rcb_common_dev *rcb_common);
	void (*init_commit)(struct rcb_common_dev *rcb_common);
	void (*set_port_desc_cnt)(struct rcb_common_dev *rcb_common,
				   u32 port_idx, u32 desc_cnt);
	void (*set_port_coalesced_frames)(struct rcb_common_dev *
					   rcb_common, u32 port_idx,
					   u32 coalesced_frames);
	void (*set_port_timeout)(struct rcb_common_dev *rcb_common,
				  u32 port_idx, u32 time_out);

	/*coalesce_usecs time */
	int (*get_rx_coalesce_usecs)(struct rcb_common_dev *rcb_common,
				      u32 port_id, u32 *usecs);

	/*coalesced_frames */
	int (*get_rx_max_coalesced_frames)(struct rcb_common_dev *rcb_common,
					u32 port_id, u32 *frame_num);

	/*coalesce_usecs enable */
	int (*set_use_adaptive_rx_coalesce)(struct rcb_common_dev *
					     rcb_common, u32 enable);
	int (*get_use_adaptive_rx_coalesce)(struct rcb_common_dev *
					     rcb_common, u32 *enable);
	void (*get_regs)(struct rcb_common_dev *rcb_common, void *data);
	void (*dump_regs)(struct rcb_common_dev *rcb_common);

};

struct rcb_common_dev {
	phys_addr_t base;
	void __iomem *vbase;

	spinlock_t lock;

	enum dsaf_mode dsaf_mode;

	s32 base_irq;
	s32 irq_num;
	int virq;

	/*rcb common cent,remove rcb_common when it's zero */
	u16 ref_cnt;

	struct rcb_common_ops ops;
	u8 comm_index;

};

/*
 * FCoE requires that all Rx buffers be over 2200 bytes in length.  Since
 * this is twice the size of a half page we need to double the page order
 * for FCoE enabled Rx queues.
 */
static inline unsigned int rcb_rx_bufsz(struct nic_rx_ring *rx_ring)
{
	return container_of(rx_ring, struct nic_ring_pair, rx_ring)->rcb_dev.buf_size;
	/*return 4096;*/
}
static inline unsigned int rcb_rx_pg_order(struct nic_rx_ring *rx_ring)
{
	return 0;
}
#define rcb_rx_pg_size(_ring) (PAGE_SIZE << rcb_rx_pg_order(_ring))
#define RCB_RX_HDR_SIZE 256

extern int rcb_commom_init_commit(void);
extern int rcb_init(struct platform_device *pdev, enum dsaf_mode dsaf_mode);
extern void rcb_uninit(struct platform_device *pdev);
extern int rcb_rx_poll(struct nic_ring_pair *ring, int budget);
extern bool rcb_clean_tx_irq(struct nic_ring_pair *ring);
extern void rcb_irq_enable(struct nic_ring_pair *ring,
				u32 flag);
extern void rcb_irq_disable(struct nic_ring_pair *ring,
				u32 flag);
extern netdev_tx_t rcb_pkt_send(struct sk_buff *skb, struct nic_ring_pair *ring);
extern int rcb_ring_enable(struct nic_ring_pair *ring);
extern void rcb_ring_disable(struct nic_ring_pair *ring);
extern void rcb_ring_disable_all(struct nic_device *nic_dev);
extern int rcb_commom_init(void);
extern void rcb_commom_uninit(void);
extern u32 rcb_get_rx_ring_vld_bd_num(struct nic_ring_pair *ring);
extern u32 rcb_get_tx_ring_head(struct nic_ring_pair *ring);
extern void rcb_show_sw_stat_by_ring(struct nic_ring_pair *ring);
extern void rcb_common_show_sw_info(struct
				rcb_common_dev *rcb_commom);
extern void rcb_update_stats(struct nic_ring_pair *ring);

extern void rcb_set_ops(struct rcb_operation *ops);
extern void rcb_set_common_ops(struct rcb_common_ops *ops);
extern void rcb_reinit_ring(struct nic_ring_pair *ring);
extern void rcb_enable_test_ring(struct nic_ring_pair *ring);
extern void rcb_disable_test_ring(struct nic_ring_pair *ring);

#endif				/* _HRD_RCB_MAIN_H */
