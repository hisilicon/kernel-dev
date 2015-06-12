/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd														 */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfZKUcb5gddZ8H7IjOdefl+HMIQWXPWghpVC+mNZZc8XkFh9OLc4cPhEVPLHd5sZnrxfP
KqxZNQj4YL/8LmI05+N5OariN4VbvmXKFv5RlOy5W+D57hMBsyvEg8A8jvFUKA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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

/* #include "hrd_module.h"
 #include "hrd_typedef.h" */
#include "iware_error.h"
#include "iware_log.h"
#include "iware_dsaf_main.h"
#include "iware_nic_ethtool.h"
#include "iware_nic_sysfs.h"
#include "iware_nic_main.h"

#include <linux/cdev.h>		/*struct cdev */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <asm/cacheflush.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/fs.h>		/*struct file_operations */
#include <linux/ethtool.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/rtnetlink.h>

/**
 * nic_rx_napi_poll - NAPI Rx polling callback
 * @napi: structure for representing this polling device。
 * @budget: how many packets driver is allowed to clean。
 *
 * Return budget
 */
int nic_rx_napi_poll(struct napi_struct *napi, int budget)
{
	int ret;
	struct nic_rx_ring *rx_ring
		= container_of(napi, struct nic_rx_ring, napi);
	struct nic_ring_pair *ring
		= container_of(rx_ring, struct nic_ring_pair, rx_ring);
	bool clean_complete = true;

	ret = rcb_rx_poll(ring, budget);
	if (ret < 0)
		return budget;

	clean_complete = (ret < budget);

	/* If all work not completed, return budget and keep polling */
	if (!clean_complete)
		return budget;

	/* all work done, exit the polling mode */
	napi_complete(napi);

	log_dbg(ring->dev, "napi_complete end ring_idx=%d\n",
		ring->rcb_dev.index);

	rcb_irq_enable(ring, RCB_INT_FLAG_RX);

	/* for hardware bug fixed */
	if (rcb_get_rx_ring_vld_bd_num(ring) > 0) {
		rcb_irq_disable(ring, RCB_INT_FLAG_RX);
		napi_schedule(napi);
	}

	return 0;
}

/**
 * nic_rx_napi_poll - NAPI Rx polling callback
 * @napi: structure for representing this polling device。
 * @budget: how many packets driver is allowed to clean。
 * Return budget
 */
int nic_tx_napi_poll(struct napi_struct *napi, int budget)
{
	struct nic_tx_ring *tx_ring
		= container_of(napi, struct nic_tx_ring, napi);
	struct nic_ring_pair *ring
		= container_of(tx_ring, struct nic_ring_pair, tx_ring);
	bool clean_complete = true;

	log_dbg(ring->dev, "func begin\n");

	clean_complete = !!rcb_clean_tx_irq(ring);

	if (!clean_complete)
		return budget;

	/* all work done, exit the polling mode */
	napi_complete(napi);

	log_dbg(ring->dev, "napi_complete finish\n");

	rcb_irq_enable(ring, RCB_INT_FLAG_TX);

	/* for hardware bug fixed */
	if (rcb_get_tx_ring_head(ring) != tx_ring->next_to_clean) {
		rcb_irq_disable(ring, RCB_INT_FLAG_TX);
		napi_schedule(napi);
	}

	return 0;
}

/**
 * nic_set_mac_address - set mac address
 * @netdev: net device。
 * @p: mac address。
 *
 * Return 0 on success, else on failure
 */
int nic_set_mac_address(struct net_device *netdev, void *p)
{
	int ret;
	struct nic_device *nic_dev = NULL;
	struct sockaddr *addr = (struct sockaddr *)p;
	char mac_addr[MAC_NUM_OCTETS_PER_ADDR];

	log_dbg(&netdev->dev, "func begin\n");

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(&netdev->dev, "nic_device is NULL!\n");
		return -ENODEV;
	}

	if (!is_valid_ether_addr((u8*)addr->sa_data)) {
		log_err(&netdev->dev, "is not valid ether addr !\n");
		return -EADDRNOTAVAIL;
	}
	memcpy(mac_addr, addr->sa_data, MAC_NUM_OCTETS_PER_ADDR);

	ret = nic_dev->ops.set_mac_address(nic_dev, mac_addr);
	if (ret != 0) {
		log_err(&netdev->dev, "set_mac_address fail, ret=%d!\n", ret);
		return ret;
	}
	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);

	return 0;
}

/**
 * nic_set_multicast_list - set mutl mac address
 * @netdev: net device。
 * @p: mac address。
 *
 * return void
 */
void nic_set_multicast_list(struct net_device *dev)
{
	struct nic_device *nic_dev = NULL;
	struct netdev_hw_addr *ha = NULL;

	log_dbg(&dev->dev, "nic set multicast list begin\n");

	nic_dev = netdev_priv(dev);
	if (NULL == nic_dev) {
		log_err(&dev->dev, "set multicast list get priv NULL!\n");
		return;
	}

	if (nic_dev->ops.set_multicast_one) {
		netdev_for_each_mc_addr(ha, dev)
			nic_dev->ops.set_multicast_one(nic_dev, ha->addr);
	}
}


/**
 * nic_start_xmit - send packet
 * @skb: skb info。
 * @netdev: net device。
 *
 * Return 0 on success, negative on failure
 */
netdev_tx_t nic_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct nic_device *nic_dev = netdev_priv(netdev);
	struct nic_ring_pair *tx_ring = NULL;

	log_dbg(&netdev->dev, "func begin\n");

	tx_ring = nic_dev->ring[skb->queue_mapping];

	return rcb_pkt_send(skb, tx_ring);

}

/**
 * nic_led_timer - action for set LED
 * @nic_dev: nic_dev device。
 *
 */
void nic_led_timer(struct nic_device *nic_dev)
{
	struct mac_device *mac_dev = NULL;

	mac_dev = nic_dev->mac_dev;
	if(NULL != mac_dev->sfp_led_opt)
		(void)mac_dev->sfp_led_opt( mac_dev);
}

void nic_update_stats(struct net_device *netdev)
{
	u32 idx = 0;
	u32 port;
	u64 tx_bytes = 0;
	u64 rx_bytes = 0;
	u64 tx_pkts = 0;
	u64 rx_pkts = 0;
	u64 rx_err = 0;
	u64 tx_err = 0;
	u64 rx_drop = 0;
	u64 tx_drop = 0;
	struct nic_device *nic_dev = netdev_priv(netdev);
	struct dsaf_device *dsaf_dev = NULL;

	if (nic_dev->flag & NIC_FLAG_STOP_STAT)
		return;
	for (idx=0; idx<nic_dev->max_q_per_vf; idx++) {
		rcb_update_stats(nic_dev->ring[idx]);

		tx_bytes += nic_dev->ring[idx]->tx_ring.tx_bytes;
		tx_pkts += nic_dev->ring[idx]->tx_ring.tx_pkts;
		rx_bytes += nic_dev->ring[idx]->rx_ring.rx_bytes;
		rx_pkts += nic_dev->ring[idx]->rx_ring.rx_pkts;

		/* tx_drop += nic_dev->ring[idx]->tx_ring.tx_err_cnt; */
		rx_err += nic_dev->ring[idx]->rx_ring.rx_stats.err_pkt_len
			+ nic_dev->ring[idx]->rx_ring.rx_stats.csum_err;
	}

	ppe_update_stats(nic_dev->ppe_device);
	netdev->stats.rx_missed_errors
		/*= nic_dev->ppe_device->hw_stats.rx_drop_no_bd*/
		= nic_dev->ppe_device->hw_stats.rx_drop_no_buf;
	/*rx_err += nic_dev->ppe_device->hw_stats.rx_drop_no_bd*/
	rx_err += nic_dev->ppe_device->hw_stats.rx_drop_no_buf
		+  nic_dev->ppe_device->hw_stats.rx_err_fifo_full;
	tx_err = nic_dev->ppe_device->hw_stats.tx_err_checksum
		+ nic_dev->ppe_device->hw_stats.tx_err_fifo_empty;

	dsaf_dev = nic_dev->mac_dev->dsaf_dev;
	if (dsaf_dev != NULL) {
		port = nic_dev->index;
		dsaf_update_stats(dsaf_dev, port);
		netdev->stats.rx_missed_errors
			+= dsaf_dev->inode_hw_stats[port].bp_drop;
		rx_err += dsaf_dev->inode_hw_stats[port].bp_drop;
		rx_err += dsaf_dev->inode_hw_stats[port].pad_drop;
		/*rx_drop += dsaf_dev->inode_hw_stats[port].sbm_drop;*/
		rx_err += dsaf_dev->inode_hw_stats[port].crc_false;
		/* rx_drop += dsaf_dev->inode_hw_stats[port].rslt_drop; */
		/* rx_drop += dsaf_dev->inode_hw_stats[port].local_addr_false; */
		/* rx_drop += dsaf_dev->inode_hw_stats[port].vlan_drop;
		rx_drop += dsaf_dev->inode_hw_stats[port].stp_drop; */

		port = port + DSAF_PPE_INODE_BASE;
		dsaf_update_stats(dsaf_dev, port);
		tx_drop += dsaf_dev->inode_hw_stats[port].bp_drop;
		tx_drop += dsaf_dev->inode_hw_stats[port].pad_drop;
		/*tx_drop += dsaf_dev->inode_hw_stats[port].sbm_drop;*/
		tx_drop += dsaf_dev->inode_hw_stats[port].crc_false;
		tx_drop += dsaf_dev->inode_hw_stats[port].rslt_drop;
		/* tx_drop += dsaf_dev->inode_hw_stats[port].local_addr_false; */
		tx_drop += dsaf_dev->inode_hw_stats[port].vlan_drop;
		tx_drop += dsaf_dev->inode_hw_stats[port].stp_drop;
	}
	mac_update_stats(nic_dev->mac_dev);
	rx_err += nic_dev->mac_dev->hw_stats.rx_fcs_err
		+ nic_dev->mac_dev->hw_stats.rx_align_err
		+ nic_dev->mac_dev->hw_stats.rx_fifo_overrun_err
		+ nic_dev->mac_dev->hw_stats.rx_len_err;
	/* rx_drop += nic_dev->mac_dev->hw_stats.rx_filter_pkts; */
	tx_err += nic_dev->mac_dev->hw_stats.tx_bad_pkts
		+ nic_dev->mac_dev->hw_stats.tx_fragment_err
		+ nic_dev->mac_dev->hw_stats.tx_jabber_err
		+ nic_dev->mac_dev->hw_stats.tx_underrun_err
		+ nic_dev->mac_dev->hw_stats.tx_crc_err;
	tx_drop += nic_dev->mac_dev->hw_stats.tx_fragment_err
		+ nic_dev->mac_dev->hw_stats.tx_jabber_err
		+ nic_dev->mac_dev->hw_stats.tx_underrun_err
		+ nic_dev->mac_dev->hw_stats.tx_crc_err;

	netdev->stats.rx_bytes = rx_bytes;
	netdev->stats.rx_packets = rx_pkts;
	netdev->stats.tx_bytes = tx_bytes;
	netdev->stats.tx_packets = tx_pkts;

	netdev->stats.rx_crc_errors = nic_dev->mac_dev->hw_stats.rx_fcs_err;
	netdev->stats.rx_frame_errors = nic_dev->mac_dev->hw_stats.rx_align_err;
	netdev->stats.rx_fifo_errors
		= nic_dev->mac_dev->hw_stats.rx_fifo_overrun_err;
	netdev->stats.rx_length_errors = nic_dev->mac_dev->hw_stats.rx_len_err;
	netdev->stats.multicast = nic_dev->mac_dev->hw_stats.rx_mc_pkts
		+ nic_dev->ppe_device->hw_stats.rx_multicast;
	netdev->stats.rx_errors = rx_err;
	netdev->stats.rx_dropped = rx_drop;
	netdev->stats.tx_errors = tx_err;
	netdev->stats.tx_dropped = tx_drop;
}

/**
 * nic_update_link_status - update link status
 * @netdev: net device。
 *
 */
void nic_update_link_status(struct net_device *netdev)
{
	struct nic_device *nic_dev = netdev_priv(netdev);
	struct mac_device *mac_dev = nic_dev->mac_dev;
	u32 link_status = 0;
	u32 old_status = mac_dev->link;

	/* If we're already down, removing or resetting, do nothing */
	if (test_bit(NIC_STATE_DOWN, &nic_dev->state) ||
	    test_bit(NIC_STATE_REMOVING, &nic_dev->state) ||
	    test_bit(NIC_STATE_RESETTING, &nic_dev->state))
		return;

	if (mac_dev->get_link_status) {
		(void)mac_dev->get_link_status(mac_dev, &link_status);

		if (old_status != (u8)link_status) {
			log_info(&netdev->dev,"NIC%d Link is %s\n",
				mac_dev->mac_id, link_status?"Up":"Down");
		}

		mac_dev->link = (u8)link_status;
	}
        if (mac_dev->link) {
		if (old_status != (u8)link_status)
			netif_tx_wake_all_queues(nic_dev->netdev);

		/* only continue if link was previously down */
		if (netif_carrier_ok(netdev))
			return;

		netif_carrier_on(netdev);

	} else {
		/* only continue if link was up previously */
		if (!netif_carrier_ok(netdev))
			return;

		netif_carrier_off(netdev);
	}
}

static void nic_reset_subtask(struct net_device *netdev)
{
	struct nic_device *nic_dev = netdev_priv(netdev);

	if (!(nic_dev->flag & NIC_FLAG_RESET))
		return;

	nic_dev->flag &= ~NIC_FLAG_RESET;

	/* If we're already down, removing or resetting, just bail */
	if (test_bit(NIC_STATE_DOWN, &nic_dev->state) ||
	    test_bit(NIC_STATE_REMOVING, &nic_dev->state) ||
	    test_bit(NIC_STATE_RESETTING, &nic_dev->state))
		return;

	//nic_dump(nic_dev);
	log_err(&nic_dev->netdev->dev, "Reset NIC\n");
	nic_dev->tx_timeout_count++;

	rtnl_lock();
	nic_reinit(netdev);
	rtnl_unlock();
}

static struct rtnl_link_stats64 *nic_get_stats64(struct net_device *netdev,
	struct rtnl_link_stats64 *stats)
{
	u32 idx = 0;
	u64 tx_bytes = 0;
	u64 rx_bytes = 0;
	u64 tx_pkts = 0;
	u64 rx_pkts = 0;
	struct nic_device *nic_dev = netdev_priv(netdev);

	for (idx=0; idx<nic_dev->max_q_per_vf; idx++) {
		tx_bytes += nic_dev->ring[idx]->tx_ring.tx_bytes;
		tx_pkts += nic_dev->ring[idx]->tx_ring.tx_pkts;
		rx_bytes += nic_dev->ring[idx]->rx_ring.rx_bytes;
		rx_pkts += nic_dev->ring[idx]->rx_ring.rx_pkts;
	}
	stats->tx_bytes= tx_bytes;
	stats->tx_packets = tx_pkts;
	stats->rx_bytes= rx_bytes;
	stats->rx_packets = rx_pkts;

	stats->rx_errors = netdev->stats.rx_errors;
	stats->multicast = netdev->stats.multicast;
	stats->rx_length_errors = netdev->stats.rx_length_errors;
	stats->rx_crc_errors = netdev->stats.rx_crc_errors;
	stats->rx_missed_errors = netdev->stats.rx_missed_errors;

	stats->tx_errors = netdev->stats.tx_errors;
	stats->rx_dropped = netdev->stats.rx_dropped;
	stats->tx_dropped = netdev->stats.tx_dropped;
	stats->collisions = netdev->stats.collisions;
	stats->rx_over_errors = netdev->stats.rx_over_errors;
	stats->rx_frame_errors = netdev->stats.rx_frame_errors;
	stats->rx_fifo_errors = netdev->stats.rx_fifo_errors;
	stats->tx_aborted_errors = netdev->stats.tx_aborted_errors;
	stats->tx_carrier_errors = netdev->stats.tx_carrier_errors;
	stats->tx_fifo_errors = netdev->stats.tx_fifo_errors;
	stats->tx_heartbeat_errors = netdev->stats.tx_heartbeat_errors;
	stats->tx_window_errors = netdev->stats.tx_window_errors;
	stats->rx_compressed = netdev->stats.rx_compressed;
	stats->tx_compressed = netdev->stats.tx_compressed;

	return stats;
}

static struct net_device_stats* nic_get_stats(struct net_device *netdev)
{
	u32 idx = 0;
	u64 tx_bytes = 0;
	u64 rx_bytes = 0;
	u64 tx_pkts = 0;
	u64 rx_pkts = 0;
	struct nic_device *nic_dev = netdev_priv(netdev);

	for (idx=0; idx<nic_dev->max_q_per_vf; idx++) {
		tx_bytes += nic_dev->ring[idx]->tx_ring.tx_bytes;
		tx_pkts += nic_dev->ring[idx]->tx_ring.tx_pkts;
		rx_bytes += nic_dev->ring[idx]->rx_ring.rx_bytes;
		rx_pkts += nic_dev->ring[idx]->rx_ring.rx_pkts;
	}
	netdev->stats.tx_bytes= tx_bytes;
	netdev->stats.tx_packets = tx_pkts;
	netdev->stats.rx_bytes= rx_bytes;
	netdev->stats.rx_packets = rx_pkts;

	return &netdev->stats;
}

int nic_clean_stats(struct net_device *netdev, struct rtnl_link_stats64 *stats)
{
	u32 idx = 0;
	struct nic_device *nic_dev = NULL;
	struct dsaf_inode_hw_stats *inode_stats;

	if (netdev == NULL) {
		pr_err("input invalid.\n");
		return -EINVAL;
	}

	nic_dev = netdev_priv(netdev);
	nic_dev->flag &= ~NIC_FLAG_STOP_STAT;
	nic_update_stats(netdev);

	if (stats != NULL) {
		stats->tx_bytes= netdev->stats.tx_bytes;
		stats->tx_packets = netdev->stats.tx_packets;
		stats->rx_bytes= netdev->stats.rx_bytes;
		stats->rx_packets = netdev->stats.rx_packets;

		stats->tx_dropped = netdev->stats.tx_dropped;
		stats->rx_errors = netdev->stats.rx_errors;
		stats->multicast = netdev->stats.multicast;
		stats->rx_length_errors = netdev->stats.rx_length_errors;
		stats->rx_crc_errors = netdev->stats.rx_crc_errors;
		stats->rx_missed_errors = netdev->stats.rx_missed_errors;

		stats->tx_errors = netdev->stats.tx_errors;
		stats->rx_dropped = netdev->stats.rx_dropped;
		stats->tx_dropped = netdev->stats.tx_dropped;
		stats->collisions = netdev->stats.collisions;
		stats->rx_over_errors = netdev->stats.rx_over_errors;
		stats->rx_frame_errors = netdev->stats.rx_frame_errors;
		stats->rx_fifo_errors = netdev->stats.rx_fifo_errors;
		stats->tx_aborted_errors = netdev->stats.tx_aborted_errors;
		stats->tx_carrier_errors = netdev->stats.tx_carrier_errors;
		stats->tx_fifo_errors = netdev->stats.tx_fifo_errors;
		stats->tx_heartbeat_errors = netdev->stats.tx_heartbeat_errors;
		stats->tx_window_errors = netdev->stats.tx_window_errors;
		stats->rx_compressed = netdev->stats.rx_compressed;
		stats->tx_compressed = netdev->stats.tx_compressed;
	}

	/*nic_dev = netdev_priv(netdev);*/

	memset(&nic_dev->mac_dev->hw_stats, 0, sizeof(struct mac_hw_stats));
	mac_clean_stats(nic_dev->mac_dev);
	memset(&nic_dev->ppe_device->hw_stats, 0, sizeof(struct ppe_hw_stats));
	for (idx=0; idx<nic_dev->max_q_per_vf; idx++) {
		memset(&nic_dev->ring[idx]->hw_stats, 0,
			sizeof(struct nic_ring_hw_stats));
		nic_dev->ring[idx]->tx_ring.tx_bytes = 0;
		nic_dev->ring[idx]->tx_ring.tx_pkts = 0;
		nic_dev->ring[idx]->rx_ring.rx_bytes = 0;
		nic_dev->ring[idx]->rx_ring.rx_pkts = 0;
		nic_dev->ring[idx]->tx_ring.tx_err_cnt = 0;
		memset(&nic_dev->ring[idx]->rx_ring.rx_stats, 0,
			sizeof(struct nic_rx_queue_stats));
	}

	if (nic_dev->mac_dev->dsaf_dev != NULL) {
		idx = nic_dev->index;
		inode_stats = &nic_dev->mac_dev->dsaf_dev->inode_hw_stats[idx];
		memset(inode_stats, 0, sizeof(struct dsaf_inode_hw_stats));

		idx += DSAF_PPE_INODE_BASE;
		inode_stats = &nic_dev->mac_dev->dsaf_dev->inode_hw_stats[idx];
		memset(inode_stats, 0, sizeof(struct dsaf_inode_hw_stats));
	}

	memset(&netdev->stats, 0, sizeof(struct net_device_stats));

	return 0;
}
EXPORT_SYMBOL(nic_clean_stats);

int nic_set_stats(struct net_device *netdev, struct rtnl_link_stats64 *storage)
{
	int ret;
	struct nic_device *nic_dev = NULL;

	if ((storage == NULL) || (netdev == NULL)) {
		pr_err( "input invalid.\n");
		return -EINVAL;
	}

	ret = nic_clean_stats(netdev, NULL);
	if (ret)
		return ret;

	nic_dev = netdev_priv(netdev);

	nic_dev->flag |= NIC_FLAG_STOP_STAT;
	nic_dev->ring[0]->tx_ring.tx_pkts = storage->tx_packets;
	nic_dev->ring[0]->tx_ring.tx_bytes = storage->tx_bytes;
	nic_dev->ring[0]->rx_ring.rx_pkts = storage->rx_packets;
	nic_dev->ring[0]->rx_ring.rx_bytes = storage->rx_bytes;

	netdev->stats.rx_packets = storage->rx_packets;
	netdev->stats.tx_packets = storage->tx_packets;
	netdev->stats.rx_bytes = storage->rx_bytes;
	netdev->stats.tx_bytes = storage->tx_bytes;
	netdev->stats.rx_errors = storage->rx_errors;

	netdev->stats.tx_errors = storage->tx_errors;
	netdev->stats.rx_dropped = storage->rx_dropped;
	netdev->stats.tx_dropped = storage->tx_dropped;
	netdev->stats.multicast = storage->multicast;
	netdev->stats.collisions = storage->collisions;
	netdev->stats.rx_length_errors = storage->rx_length_errors;
	netdev->stats.rx_over_errors = storage->rx_over_errors;
	netdev->stats.rx_crc_errors = storage->rx_crc_errors;
	netdev->stats.rx_frame_errors = storage->rx_frame_errors;
	netdev->stats.rx_fifo_errors = storage->rx_fifo_errors;
	netdev->stats.rx_missed_errors = storage->rx_missed_errors;
	netdev->stats.tx_aborted_errors = storage->tx_aborted_errors;
	netdev->stats.tx_carrier_errors = storage->tx_carrier_errors;
	netdev->stats.tx_fifo_errors = storage->tx_fifo_errors;
	netdev->stats.tx_heartbeat_errors = storage->tx_heartbeat_errors;
	netdev->stats.tx_window_errors = storage->tx_window_errors;
	netdev->stats.rx_compressed = storage->rx_compressed;
	netdev->stats.tx_compressed = storage->tx_compressed;

	return 0;
}
EXPORT_SYMBOL(nic_set_stats);

static void nic_task_schedule(struct nic_device *nic_dev)
{
	if (!test_bit(NIC_STATE_DOWN, &nic_dev->state) &&
	    !test_bit(NIC_STATE_REMOVING, &nic_dev->state) &&
	    !test_and_set_bit(NIC_STATE_SERVICE_SCHED, &nic_dev->state))
		(void)schedule_work(&nic_dev->service_task);
}

/**
 * nic_service_timer - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/
static void nic_service_timer(unsigned long data)
{
	struct nic_device *nic_dev = (struct nic_device*)data;

	/*struct nic_device *nic_dev
		= (struct nic_device *)data;*/




	//(void)schedule_delayed_work(&nic_dev->hb_timer, msecs_to_jiffies(1000));

	/* Reset the timer */
	(void)mod_timer(&nic_dev->service_timer, HZ + jiffies);

	nic_task_schedule(nic_dev);
}

static void nic_service_event_complete(struct nic_device *nic_dev)
{
	BUG_ON(!test_bit(NIC_STATE_SERVICE_SCHED, &nic_dev->state));

	smp_mb__before_atomic();
	clear_bit(NIC_STATE_SERVICE_SCHED, &nic_dev->state);
}

/**
 * nic_service_task - manages and runs subtasks
 * @work: pointer to work_struct containing our data
 **/
static void nic_service_task(struct work_struct *work)
{
	struct nic_device *nic_dev
		= container_of(work, struct nic_device, service_task);

	nic_update_link_status(nic_dev->netdev);

	if(NULL != nic_dev->mac_dev->cpld_vaddr
		&& CPLD_LED_ENABLE == nic_dev->mac_dev->cpld_led_enable)
		nic_led_timer(nic_dev);

	if(nic_dev && nic_dev->mac_dev &&
		nic_dev->mac_dev->cpld_vaddr &&
		nic_dev->mac_dev->get_sfp_prsnt)
		(void)nic_dev->mac_dev->get_sfp_prsnt(nic_dev->mac_dev);

	nic_update_stats(nic_dev->netdev);



	nic_reset_subtask(nic_dev->netdev);

	nic_service_event_complete(nic_dev);
}

/**
 * nic_set_led_func - open or close led function
 * @netdev: net device。
 * @value : 0 - open led function, 1 - close led function
 * Return 0 on success, negative on failure
 */

int nic_set_led_enable(struct net_device *netdev, int value)
{
	struct nic_device *nic_dev = netdev_priv(netdev);

	if(CPLD_LED_DISABLE == value)
		nic_dev->mac_dev->cpld_led_enable = CPLD_LED_DISABLE;
	else if (CPLD_LED_ENABLE == value)
		nic_dev->mac_dev->cpld_led_enable = CPLD_LED_ENABLE;
	else {
		log_err(&netdev->dev, "nic_set_led_func error param!\n");
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(nic_set_led_enable);



/**
 * nic_open - open netdev
 * @netdev: net device。
 *
 * Return 0 on success, negative on failure
 */
int nic_up(struct net_device *netdev)
{
	int ret;
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;
	int ring_idx = 0;
	int ring_fail_idx = 0;
	struct sockaddr sock_addr;
	char mac_addr[MAC_NUM_OCTETS_PER_ADDR];

	nic_dev = netdev_priv(netdev);

	for (ring_idx = 0; ring_idx < nic_dev->ring_pair_num; ring_idx++) {
		ret = rcb_ring_enable(nic_dev->ring[ring_idx]);
		if (ret < 0) {
			log_err(&netdev->dev,
				"rcb_ring_enable fail, ring_id=%d, ret=%d!\n",
				nic_dev->ring[ring_idx]->rcb_dev.index, ret);
			goto ring_enable_fail;
		}
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

	memcpy(sock_addr.sa_data, netdev->dev_addr, netdev->addr_len);
	ret = nic_set_mac_address(netdev, &sock_addr);
	if (ret < 0) {
		log_err(&netdev->dev,
			"chip_id=%d nic_idx=%d nic_set_mac_address faild!\n",
			nic_dev->chip_id, nic_dev->index);
		goto set_mac_addr_fail;
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

	netif_tx_wake_all_queues(nic_dev->netdev);

	clear_bit(NIC_STATE_DOWN, &nic_dev->state);

	(void)mod_timer(&nic_dev->service_timer, jiffies);

	return 0;

mac_start_fail:
	if (mac_dev->del_mac != NULL) {
		memcpy(mac_addr, netdev->dev_addr, MAC_NUM_OCTETS_PER_ADDR);
		ret = mac_dev->del_mac(mac_dev, 0, mac_addr);
		if (ret < 0)
			log_err(&netdev->dev,
				"del_mac fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

set_mac_addr_fail:
	if (mac_dev->queue_config_bc_en != NULL) {
		ret = mac_dev->queue_config_bc_en(mac_dev, 0, DISABLE);
		if (ret < 0)
			log_err(&netdev->dev,
				"queue_config_bc_en fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

ring_enable_fail:
	for (ring_fail_idx = 0; ring_fail_idx < ring_idx; ring_fail_idx++)
		rcb_ring_disable(nic_dev->ring[ring_fail_idx]);

	set_bit(NIC_STATE_DOWN, &nic_dev->state);

	return ret;

}

/**
 * nic_open - open netdev
 * @netdev: net device。
 *
 * Return 0 on success, negative on failure
 */
int nic_open(struct net_device *netdev)
{
	int ret;
	struct nic_device *nic_dev = NULL;

	log_info(&netdev->dev, "func begin!\n");

	nic_dev = netdev_priv(netdev);
	if (test_bit(NIC_STATE_TESTING, &nic_dev->state))
		return -EBUSY;

	/* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);

	(void)nic_clean_stats(netdev, NULL);

	ret = netif_set_real_num_tx_queues(netdev, nic_dev->max_q_per_vf);
	if (ret) {
		log_err(&netdev->dev,
			"netif_set_real_num_tx_queues fail, ret=%d!\n", ret);
		return ret;
	}

	ret = netif_set_real_num_rx_queues(netdev, nic_dev->max_q_per_vf);
	if (ret) {
		log_err(&netdev->dev,
			"netif_set_real_num_rx_queues fail, ret=%d!\n", ret);
		return ret;
	}

	ret = nic_up(netdev);
	if (ret) {
		log_err(&netdev->dev, "nic_up fail, ret=%d!\n", ret);
		return ret;
	}
	//INIT_DELAYED_WORK(&nic_dev->hb_timer, nic_service_timer);
	//schedule_delayed_work(&nic_dev->hb_timer, msecs_to_jiffies(1000));

	return 0;
}

static void nic_down(struct net_device *netdev)
{
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;
	char mac_addr[MAC_NUM_OCTETS_PER_ADDR];

	int ret;

	nic_dev = netdev_priv(netdev);
	mac_dev = nic_dev->mac_dev;

	if (test_and_set_bit(NIC_STATE_DOWN, &nic_dev->state))
		return;

	/* disable receives */
	if(mac_dev->stop(mac_dev))
		log_warn(&netdev->dev, "stop mac%d fail!\n", nic_dev->gidx);

	usleep_range(10000, 20000);

	netif_tx_stop_all_queues(netdev);

	/* call carrier off first to avoid false dev_watchdog timeouts */
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);

	rcb_ring_disable_all(nic_dev);

	if (mac_dev->del_mac != NULL) {
		memcpy(mac_addr, netdev->dev_addr, MAC_NUM_OCTETS_PER_ADDR);
		ret = mac_dev->del_mac(mac_dev, 0, mac_addr);
		if (ret < 0)
			log_err(&netdev->dev,
				"del_mac fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

	if (mac_dev->queue_config_bc_en != NULL) {
		ret = mac_dev->queue_config_bc_en(mac_dev, 0, DISABLE);
		if (ret < 0)
			log_err(&netdev->dev,
				"queue_config_bc_en fail, mac_id=%d, ret=%d!\n",
				mac_dev->global_mac_id, ret);
	}

	(void)del_timer_sync(&nic_dev->service_timer);
}

/**
 * nic_stop - stop netdev
 * @netdev: net device。
 *
 * Return 0 on success, negative on failure
 */
int nic_stop(struct net_device *netdev)
{
	int ret;
	struct mac_device *mac_dev = NULL;
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(netdev);

	mac_dev = nic_dev->mac_dev;

	nic_down(netdev);

	if(NULL != mac_dev->led_reset && 0 != mac_dev->cpld_vaddr)
			mac_dev->led_reset(mac_dev);

	if (mac_dev->sfp_close) {
		ret = mac_dev->sfp_close(mac_dev);
		if (ret)
			log_err(&netdev->dev,
				"mac close sfp fail, ret = %#x!\n", ret);
	}

	return 0;
}

/**
 * nic_reset - reset all sub module and reinit it, but not enable
 * @netdev: net device。
 */
void nic_reset(struct net_device *netdev)
{
	int idx;
	struct nic_device *nic_dev = NULL;
	struct nic_ring_pair * ring = NULL;
	struct mac_device *mac_dev = NULL;

	nic_dev = netdev_priv(netdev);
	mac_dev = nic_dev->mac_dev;

	while (test_and_set_bit(NIC_STATE_RESETTING, &nic_dev->state))
		usleep_range(1000, 2000);

	if(mac_dev->stop(mac_dev))
		log_warn(&netdev->dev, "stop mac%d fail!\n", nic_dev->gidx);

	for (idx=0; idx<nic_dev->ring_pair_num; idx++) {
		ring = nic_dev->ring[idx];
		ring->rcb_dev.ops.disable(ring);
	}

	msleep(100);
	if (mac_dev->dsaf_dev)
		for (idx = 0; idx < nic_dev->ring_pair_num; idx++) {
			ring = nic_dev->ring[idx];
			ring->rcb_dev.ops.reset_ring(ring);
		}

	/* only reset mac when dsaf_dev is null */
	if (!mac_dev->dsaf_dev)
		if(mac_dev->reset(mac_dev))
			log_warn(&netdev->dev, "reset mac%d fail!\n",
				nic_dev->gidx);
	ppe_reset(nic_dev->ppe_device);

	clear_bit(NIC_STATE_RESETTING, &nic_dev->state);
}

EXPORT_SYMBOL(nic_reset);
/**
 * nic_reinit - reset all sub module, reinit and enable it
 * @netdev: net device。
 */
void nic_reinit(struct net_device *netdev)
{
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(netdev);

	nic_dev->netdev->trans_start = jiffies;
	while (test_and_set_bit(NIC_STATE_REINITING, &nic_dev->state))
		usleep_range(1000, 2000);

	nic_down(netdev);
	nic_reset(netdev);
	(void)nic_up(netdev);

	clear_bit(NIC_STATE_REINITING, &nic_dev->state);
}


#ifdef CONFIG_NET_POLL_CONTROLLER
/**
 * nic_poll_controller - netdev poll controller
 * @netdev: net device。
 */
void nic_poll_controller(struct net_device *dev)
{
	/* TBD */
#if 0
	unsigned long ulFlags = 0;

	if (NULL == dev)
		return;

	local_irq_save(ulFlags);
	netdrv_interrupt(0, dev);
	local_irq_restore(ulFlags);
#endif
}
#endif

/**
 * nic_timeout - nic timeout
 * @netdev: net device。
 */
void nic_timeout(struct net_device *netdev)
{
	struct nic_device *nic_dev = netdev_priv(netdev);

	if (!test_bit(NIC_STATE_DOWN, &nic_dev->state)) {
		//nic_dev->flag |= NIC_FLAG_RESET;
		//log_warn(&netdev->dev, "reset due to tx timeout\n");
		//nic_task_schedule(nic_dev);
	}
}

/**
 * nic_change_mtu - change mtu
 * @netdev: net device。
 * @new_mtu: mtu
 *
 * Return 0 on success, negative on failure
 */
int nic_change_mtu(struct net_device *netdev, int new_mtu)
{
	int ret;
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(netdev);

	ret = nic_dev->ops.change_mtu(nic_dev, new_mtu);
	if (ret < 0) {
		log_err(&netdev->dev, "change_mtu fail, ret=%d!\n", ret);
		return ret;
	}
	netdev->mtu = new_mtu;

	if (netif_running(netdev))
		nic_reinit(netdev);

	return 0;
}

/**
 * nic_suspend - netdev suspend
 * @pdev: platform device。
 * @state: power manage message
 *
 * Return 0 on success, negative on failure
 */
int nic_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* TBD */

	return 0;
}

/**
 * nic_resume - netdev resume
 * @pdev: platform device。
 *
 * Return 0 on success, negative on failure
 */
int nic_resume(struct platform_device *pdev)
{
/* TBD */
	return 0;
}

int nic_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
	default:
		return -EINVAL;
	}

}

/**
 * nic_show_sw_stat_by_port - show software status
 * @netdev: net device。
 */
void nic_show_sw_stat_by_port(struct net_device *netdev)
{
	struct nic_device *nic_dev = NULL;

	nic_dev = netdev_priv(netdev);

	osal_printf("**********************nic info**************************");
	osal_printf("**********************\r\n");
	osal_printf("max_vfn		   :%#18x  max_q_per_vf	  :%#18x\r\n",
		    nic_dev->max_vfn, nic_dev->max_q_per_vf);
	osal_printf("tx_timeout_count  :%#18x  hw_csum_rx_error  :%#18llx\r\n",
		    nic_dev->tx_timeout_count, nic_dev->hw_csum_rx_error);
	osal_printf
	    ("hw_rx_no_dma_res  :%#18llx  rsc_total_count   :%#18llx\r\n",
	     nic_dev->hw_rx_no_dma_resources, nic_dev->rsc_total_count);
	osal_printf
	    ("rsc_total_flush   :%#18llx  state			 :%#18lx\r\n",
	     nic_dev->rsc_total_flush, nic_dev->state);
	osal_printf
	    ("ring_pair_num	 :%#18x  index			 :%#18x\r\n",
	     nic_dev->ring_pair_num, nic_dev->index);
	osal_printf
	    ("chip_id		   :%#18x  gidx			  :%#18x\r\n",
	     nic_dev->chip_id, nic_dev->gidx);
	osal_printf("desc_cnt		  :%#18x  coalesced_frames  :%#18x\r\n",
		    nic_dev->desc_cnt, nic_dev->coalesced_frames);
	osal_printf("time_out		  :%#18x  global_virq	   :%#18x\r\n",
		    nic_dev->time_out, nic_dev->global_virq);
	osal_printf
	    ("mtu               :%#18x  change_mtu        :%#18llx\r\n",
	     nic_dev->mtu, (u64) nic_dev->ops.change_mtu);
	osal_printf("set_mac_address   :%#18llx\r\n",
		    (u64) nic_dev->ops.set_mac_address);
}

const struct net_device_ops g_nic_netdev_ops = {
	.ndo_open = nic_open,
	.ndo_stop = nic_stop,
	.ndo_start_xmit = nic_start_xmit,
	.ndo_tx_timeout = nic_timeout,
	.ndo_set_mac_address = nic_set_mac_address,
	.ndo_change_mtu = nic_change_mtu,
	.ndo_do_ioctl = nic_ioctl,
	.ndo_get_stats64 = nic_get_stats64,
	.ndo_get_stats = nic_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = nic_poll_controller,
#endif

	.ndo_set_rx_mode	= nic_set_multicast_list,

};

/**
 * nic_get_chip_id - get chip id
 * @pdev: platform device。
 *
 * Return chip id. if can't read from device tree, then return 0
 */
u32 nic_get_chip_id(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	u32 chip_id = 0;
	int ret;

	np = of_get_parent(pdev->dev.of_node);
	if (np == NULL)
		return 0;

	ret = of_property_read_u32(np, "chip-id", &chip_id);
	if (ret < 0)
		return 0;

	return chip_id;
}

/**
 * nic_bind_mac - bind mac to netdev
 * @pdev: platform device。
 *
 * Return 0 on success, negative on failure
 */
static int nic_bind_mac(struct platform_device *pdev)
{
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct device_node *np = NULL;
	struct platform_device *tmp_dev = NULL;

	netdev = (struct net_device *)platform_get_drvdata(pdev);
	nic_dev = (struct nic_device *)netdev_priv(netdev);

	np = of_parse_phandle(pdev->dev.of_node, "mac-handle", 0);
	if (np == NULL) {
		log_err(&pdev->dev, "of_parse_phandle mac-handle fail!\r\n");
		return -EINVAL;
	}

	tmp_dev = of_find_device_by_node(np);
	if (np == NULL) {
		log_err(&pdev->dev,
			"of_find_device_by_node mac-handle fail!\r\n");
		return -EINVAL;
	}

	nic_dev->mac_dev =
	    (struct mac_device *) (dev_get_drvdata(get_device(&tmp_dev->dev)));

	return 0;
}

/**
 * nic_probe - probe netdev
 * @pdev: platform device。
 *
 * Return 0 on success, negative on failure
 */
int nic_probe(struct platform_device *pdev)
{
	struct nic_device *nic_dev = NULL;
	struct net_device *netdev = NULL;
	int ret;
	u32 dev_id;
	u32 chip_id = 0;
	enum dsaf_mode dsaf_mode = DSAF_MODE_INVALID;
	const void *mac_addr_temp = NULL;

	log_info(&pdev->dev, "nic_probe begin\n");

	/* Init network device */
	/**Modefied for hulk3.19 by CHJ,add NET_NAME_UNKNOWN*/
	netdev = alloc_netdev_mq(sizeof(struct nic_device), "HiNIC%d",NET_NAME_UNKNOWN,ether_setup,
				 NIC_MAX_Q_PER_VF);
	if (NULL == netdev) {
		log_err(&pdev->dev, "alloc_netdev_mq failed!\n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(netdev, &pdev->dev);
	platform_set_drvdata(pdev, netdev);

	nic_dev = (struct nic_device *)netdev_priv(netdev);
	memset(nic_dev, 0, sizeof(*nic_dev));
	nic_dev->netdev = netdev;
	nic_dev->dev = &pdev->dev;

	chip_id = nic_get_chip_id(pdev);
	if (of_device_is_compatible(pdev->dev.of_node,
		"hisilicon,soc-dsa-nic")) {
		ret = dsaf_get_work_mode(chip_id, &dsaf_mode);
		if (ret) {
			log_err(&pdev->dev,
				"dsaf_get_work_mode fail, ret = %d!\r\n", ret);
			ret = -EINVAL;
			goto get_work_mode_fail;
		}
	} else
		dsaf_mode = DSAF_MODE_DISABLE_FIX;

	log_info(&pdev->dev, "dsaf_mode = %d\n", dsaf_mode);

	if (dsaf_mode <= DSAF_MODE_ENABLE) {
		/* in switch mode, get mac 0 from dsa fabric */
		nic_dev->mac_dev = dsaf_get_mac_dev(chip_id, 0);
		if (NULL == nic_dev->mac_dev) {
			log_err(&pdev->dev,
				"dsaf_get_mac_dev fail, mac_dev is null!\r\n");
			ret = -EINVAL;
			goto get_work_mode_fail;
		}
		ret =
		    of_property_read_u32(pdev->dev.of_node, "nic-id", &dev_id);
		if (ret) {
			log_err(&pdev->dev,
				"of_property_read_u32 fail, ret = %d!\r\n",
				ret);
			goto get_work_mode_fail;
		}
	} else {
		ret = nic_bind_mac(pdev);
		if (ret) {
			log_err(&pdev->dev, "nic_bind_mac fail, ret = %d!\r\n",
				ret);
			goto get_work_mode_fail;
		}
		dev_id = nic_dev->mac_dev->mac_id;
	}
	nic_dev->index = dev_id;
	nic_dev->gidx = dev_id + (NIC_MAX_NUM_PER_CHIP * chip_id) ;
	netdev->dev_id = nic_dev->gidx;

	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64ULL)))
		log_dbg(&pdev->dev, "set mask to 64bit\n");
	else
		log_dbg(&pdev->dev, "set mask to 32bit\n");

	ret = ppe_init(pdev, dsaf_mode);
	if (ret) {
		log_err(&pdev->dev, "ppe_init fail, nic_idx=%#x ret=%d!\r\n",
			dev_id, ret);
		goto get_work_mode_fail;
	}

	ret = rcb_init(pdev, dsaf_mode);
	if (ret) {
		log_err(&pdev->dev, "rcb_init fail, nic_idx=%#x ret=%d!\r\n",
			dev_id, ret);
		goto rcb_init_fail;
	}

	nic_set_ops(&nic_dev->ops);

	/* The NETDRV-specific entries in the device structure. */
	netdev->netdev_ops = &g_nic_netdev_ops;
	netdev->ethtool_ops = &g_nic_ethtools;
	netdev->watchdog_timeo = NIC_TX_TIMEOUT;
	netdev->features |=
		NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM;
	netdev->features |= NETIF_F_SG | NETIF_F_GSO | NETIF_F_GRO;

	netdev->vlan_features |=
		NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM;
	netdev->vlan_features |= NETIF_F_SG | NETIF_F_GSO | NETIF_F_GRO;

	setup_timer(&nic_dev->service_timer, nic_service_timer,
		    (unsigned long)nic_dev);

	INIT_WORK(&nic_dev->service_task, nic_service_task);
	set_bit(NIC_STATE_SERVICE_INITED, &nic_dev->state);
	clear_bit(NIC_STATE_SERVICE_SCHED, &nic_dev->state);




	mac_addr_temp = of_get_mac_address(pdev->dev.of_node);
	if (NULL != mac_addr_temp) {
		log_info(&pdev->dev,
			"node_mac_addr[%2x-%2x-%2x-%2x-%2x-%2x]\n",
			((u8 *)mac_addr_temp)[0], ((u8 *)mac_addr_temp)[1],
			((u8 *)mac_addr_temp)[2], ((u8 *)mac_addr_temp)[3],
			((u8 *)mac_addr_temp)[4], ((u8 *)mac_addr_temp)[5]);
		memcpy(netdev->dev_addr, mac_addr_temp,
			netdev->addr_len);
	} else {
		eth_hw_addr_random(netdev);
		log_warn(&netdev->dev,
			"using random MAC address %pM\n",
			netdev->dev_addr);
	}

	ret = nic_change_mtu(netdev, NIC_DEFAULT_MTU);
	if (ret) {
		log_err(&pdev->dev,
			"nic_change_mtu fail, nic_idx=%#x ret=%d!\n", dev_id,
			ret);
		goto set_mac_addr_fail;
	}

	/* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);

	ret = nic_add_sysfs(&pdev->dev);
	if (ret) {
		log_err(&pdev->dev,
			"nic_add_sysfs fail, nic_idx=%#x ret=%d!\r\n", dev_id,
			ret);
		goto set_mac_addr_fail;
	}

	log_dbg(&pdev->dev, "chip_id=%d nic_idx=%d end\n",
		nic_dev->chip_id, nic_dev->index);

	return 0;

set_mac_addr_fail:
	unregister_netdev(netdev);

/*reg_netdev_fail:*/
	rcb_uninit(pdev);

rcb_init_fail:
	ppe_uninit(pdev);

get_work_mode_fail:
	platform_set_drvdata(pdev, NULL);
	free_netdev(netdev);

	return ret;
}

/**
 * nic_remove - bind mac to netdev
 * @pdev: platform device。
 *
 * Return 0 on success, negative on failure
 */
int nic_remove(struct platform_device *pdev)
{
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;

	netdev = platform_get_drvdata(pdev);
	nic_dev = netdev_priv(netdev);

	log_dbg(&pdev->dev, "chip_id=%d, nic_idx=%d!\n",
		nic_dev->chip_id, nic_dev->index);

	set_bit(NIC_STATE_REMOVING, &nic_dev->state);
	(void)cancel_work_sync(&nic_dev->service_task);

	mac_dev = nic_dev->mac_dev;

	if(NULL != mac_dev->led_reset
		&& 0 != mac_dev->cpld_vaddr)
			mac_dev->led_reset(mac_dev);

	nic_del_sysfs(&pdev->dev);
	unregister_netdev(netdev);
	rcb_uninit(pdev);
	ppe_uninit(pdev);
	platform_set_drvdata(pdev, NULL);
	free_netdev(netdev);

	return 0;
}

static const struct of_device_id g_nic_match[] = {
	{.compatible = "hisilicon,soc-dsa-nic"},
	{.compatible = "hisilicon,soc-single-nic"},
	{}
};

MODULE_DEVICE_TABLE(of, g_nic_match);

static struct platform_driver g_nic_driver = {
	.probe = nic_probe,
	.remove = nic_remove,
	.suspend = nic_suspend,
	.resume = nic_resume,
	.driver = {
		   .name = DRIVER_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = g_nic_match,
		   },
};

/**
 *nic_register_alldev -  register all net dev
 *retuen status
 */
static int nic_register_alldev(void)
{
	struct device_node *np = NULL;
	struct platform_device *pdev = NULL;
	struct net_device *netdev = NULL;
	int i, ret;

    for (i = 0; i < (int)(ARRAY_SIZE(g_nic_match) - 1); i++) {
        for_each_compatible_node(np, NULL,
            g_nic_match[i].compatible) {
            pdev = of_find_device_by_node(np);
            if (NULL == pdev) {
                pr_err("of_find_device_by_node fail!\r\n");
                return -ENODEV;
            }
            netdev = platform_get_drvdata(pdev);
            ret = register_netdev(netdev);
            if (ret) {
                log_err(&pdev->dev,
                    "register_netdev fail, nic_idx=%#x ret=%d!\r\n",
                    netdev->dev_id, ret);
                return -ENODEV;
            }
            log_info(&pdev->dev,
                "register_netdev dev_id=%#x!\r\n",
                netdev->dev_id);
        }
    }

	return 0;
}

/**
 * nic_module_init - init module
 *
 * Return 0 on success, negative on failure
 */
int __init nic_module_init(void)
{
	int ret;

	pr_info("v%s begin\n", NIC_MOD_VERSION);

	ret = ppe_commom_init();
	if (ret) {
		pr_err("ppe_commom_init fail, ret=%d!\r\n", ret);
		return ret;
	}

	ret = rcb_commom_init();
	if (ret) {
		pr_err("rcb_commom_init fail, ret=%d!\r\n", ret);
		goto rcb_commom_init_fail;
	}

	ret = platform_driver_register(&g_nic_driver);
	if (ret) {
		pr_err("platform_driver_register fail, ret = %d\n", ret);
		goto platform_driver_register_fail;
	}

	ret = rcb_commom_init_commit();
	if (ret) {
		pr_err("rcb_commom_init_commit fail, ret = %d\n", ret);
		goto rcb_commom_init_commit_fail;
	}

	ret = nic_register_alldev();
	if (ret) {
		pr_err("nic_register_alldev fail, ret = %d\n", ret);
		goto nic_register_alldev_fail;
	}

	pr_info("func end\n");

	return 0;

nic_register_alldev_fail:
rcb_commom_init_commit_fail:
	platform_driver_unregister(&g_nic_driver);

platform_driver_register_fail:
	rcb_commom_uninit();

rcb_commom_init_fail:
	ppe_commom_uninit();

		return ret;


}

/**
 * nic_module_exit - module exit
 */
void __exit nic_module_exit(void)
{
	pr_debug("v%s begin\n", NIC_MOD_VERSION);

	platform_driver_unregister(&g_nic_driver);

	rcb_commom_uninit();

	ppe_commom_uninit();

	pr_debug("end\n");
}

module_init(nic_module_init);
module_exit(nic_module_exit);
MODULE_DESCRIPTION("Hisilicon PV660 nic driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_VERSION(NIC_MOD_VERSION);
