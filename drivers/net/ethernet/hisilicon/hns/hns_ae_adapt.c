/*
 * Copyright (c) 2014-2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "hns_dsaf_ppe.h"
#include "hns_dsaf_rcb.h"
#include "hns_dsaf_mac.h"
#include "hns_dsaf_main.h"
#include "hnae.h"

#define AE_NAME_PORT_ID_IDX 6
#define ETH_STATIC_REG	 1
#define ETH_DUMP_REG	 5
#define ETH_GSTRING_LEN	32

static inline struct hns_mac_cb *hns_get_mac_cb(struct hnae_handle *handle)
{
	struct  hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);

	return port_cb->mac_cb;
}

static inline int hns_ae_get_port_idx(const char *ops)
{
	return ops[0] - '0';
}

static inline struct dsaf_device *hns_ae_get_dsaf_dev(struct hnae_ae_dev *dev)
{
	return container_of(dev, struct dsaf_device, ae_dev);
}

static struct hns_ppe_cb *hns_get_ppe_cb(struct hnae_handle *handle)
{
	int ppe_index;
	int ppe_common_index;
	struct ppe_common_cb *ppe_comm;
	struct  hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);

	if (port_cb->index < DSAF_SERVICE_PORT_NUM_PER_DSAF) {
		ppe_index = port_cb->index;
		ppe_common_index = 0;
	} else {
		ppe_index = 0;
		ppe_common_index =
			port_cb->index - DSAF_SERVICE_PORT_NUM_PER_DSAF + 1;
	}
	ppe_comm = port_cb->dsaf_dev->ppe_common[ppe_common_index];
	return &ppe_comm->ppe_cb[ppe_index];
}

static struct ring_pair_cb *hns_ae_get_base_ring_pair(
	struct dsaf_device *dsaf_dev, int port)
{
	int common_idx = hns_dsaf_get_comm_idx_by_port(port);
	struct ring_pair_cb *ring_pair;
	struct rcb_common_cb *rcb_comm = dsaf_dev->rcb_common[common_idx];
	u16 queue_num = rcb_comm->max_q_per_vf;

	if (HNS_DSAF_COMM_SERVICE_NW_IDX == common_idx)
		ring_pair = &rcb_comm->ring_pair_cb[port * queue_num];
	else
		ring_pair = &rcb_comm->ring_pair_cb[0];

	return ring_pair;
}

static int hns_ae_get_q_num_per_port(struct dsaf_device *dsaf_dev, int port)
{
	int common_idx = hns_dsaf_get_comm_idx_by_port(port);

	return dsaf_dev->rcb_common[common_idx]->max_q_per_vf;
}

struct hnae_handle *hns_ae_get_handle(struct hnae_ae_dev *dev,
				      const char *opts)
{
	int idx;
	int q_num;
	int i;
	struct dsaf_device *dsaf_dev;
	struct hnae_handle *ae_handle;
	struct ring_pair_cb *ring_pair_cb;
	struct hnae_port_cb *port_cb;

	dsaf_dev = hns_ae_get_dsaf_dev(dev);

	idx = hns_ae_get_port_idx(opts);

	ring_pair_cb = hns_ae_get_base_ring_pair(dsaf_dev, idx);
	q_num = hns_ae_get_q_num_per_port(dsaf_dev, idx);

	port_cb = kzalloc(sizeof(*port_cb) + q_num*sizeof(struct hnae_queue *),
			  GFP_KERNEL);
	if (unlikely(!port_cb)) {
		ae_handle = ERR_PTR(-ENOMEM);
		goto handle_err;
	}
	ae_handle = &port_cb->ae_handle;
	/* ae_handle Init  */
	ae_handle->owner_dev = dsaf_dev->dev;
	ae_handle->dev = dev;
	ae_handle->q_num = q_num;

	ae_handle->qs = (struct hnae_queue **)(&ae_handle->qs + 1);
	for (i = 0; i < q_num; i++) {
		ae_handle->qs[i] = &ring_pair_cb->q;

		ae_handle->qs[i]->rx_ring.q     = ae_handle->qs[i];
		ae_handle->qs[i]->tx_ring.q     = ae_handle->qs[i];

		if (idx < DSAF_SERVICE_PORT_NUM_PER_DSAF)
			ring_pair_cb->port_id_in_dsa = idx;
		else
			ring_pair_cb->port_id_in_dsa = 0;
		ring_pair_cb++;
	}

	port_cb->dsaf_dev = dsaf_dev;
	port_cb->index = idx;
	port_cb->mac_cb	 = &dsaf_dev->mac_cb[idx];

	ae_handle->phy_if = port_cb->mac_cb->phy_if;
	ae_handle->phy_node = port_cb->mac_cb->phy_node;
	ae_handle->if_support = port_cb->mac_cb->if_support;
	ae_handle->port_type = port_cb->mac_cb->mac_type;

handle_err:
	return ae_handle;
}

static void hns_ae_put_handle(struct hnae_handle *handle)
{
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);
	unsigned long flags;

	spin_lock_irqsave(&port_cb->lock, flags);

	port_cb->mac_cb	 = NULL;

	spin_unlock_irqrestore(&port_cb->lock, flags);

	kfree(port_cb);
}

static void hns_ae_ring_enable_all(struct hnae_handle *handle, int val)
{
	int q_num = handle->q_num;
	int i;

	for (i = 0; i < q_num; i++)
		hns_rcb_ring_enable_hw(handle->qs[i], val);
}

static void hns_ae_init_queue(struct hnae_queue *q)
{
	struct ring_pair_cb *ring =
		container_of(q, struct ring_pair_cb, q);

	hns_rcb_init_hw(ring);
}

static void hns_ae_fini_queue(struct hnae_queue *q)
{
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(q->handle);

	if (port_cb->mac_cb->mac_type == HNAE_PORT_SERVICE) {
		hns_rcb_reset_ring_hw(q);
	} else {
		hns_mac_reset(port_cb->mac_cb);
		hns_ppe_reset(port_cb->dsaf_dev);
	}
}

static struct net_device *hns_get_netdev(struct hnae_handle *handle)
{
	return container_of(handle->owner_dev, struct net_device, dev);
}

static int hns_ae_set_mac_address(struct hnae_handle *handle, void *p)
{
	int ret;
	struct net_device *netdev = hns_get_netdev(handle);
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	if (!p || !is_valid_ether_addr((const u8 *)p)) {
		dev_err(&netdev->dev, "is not valid ether addr !\n");
		return -EADDRNOTAVAIL;
	}

	ret = hns_mac_change_addr(mac_cb, p);
	if (ret != 0) {
		dev_err(&netdev->dev, "set_mac_address fail, ret=%d!\n", ret);
		return ret;
	}

	return 0;
}

static int hns_ae_set_multicast_one(struct hnae_handle *handle, void *addr)
{
	int ret;
	u8 *mac_addr = (u8 *)addr;
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	assert(mac_cb);

	ret = hns_mac_set_multi(mac_cb, mac_cb->mac_id, mac_addr, ENABLE);
	if (ret) {
		dev_err(handle->dev->dev,
			"mac add mul_mac:%pM port%d  fail, ret = %#x!\n",
			mac_addr, mac_cb->mac_id, ret);
		return ret;
	}

	ret = hns_mac_set_multi(mac_cb, DSAF_BASE_INNER_PORT_NUM,
				mac_addr, ENABLE);
	if (ret)
		dev_err(handle->dev->dev,
			"mac add mul_mac:%pM port%d  fail, ret = %#x!\n",
			mac_addr, DSAF_BASE_INNER_PORT_NUM, ret);

	return ret;
}

static int hns_ae_set_mtu(struct hnae_handle *handle, int new_mtu)
{
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	return hns_mac_set_mtu(mac_cb, new_mtu);
}

static int hns_ae_start(struct hnae_handle *handle)
{
	int ret;
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	ret = hns_mac_vm_config_bc_en(mac_cb, 0, ENABLE);
	if (ret)
		return ret;

	hns_ae_ring_enable_all(handle, 1);
	msleep(100);

	hns_mac_start(mac_cb);

	return 0;
}

void hns_ae_stop(struct hnae_handle *handle)
{
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	hns_mac_stop(mac_cb);

	usleep_range(10000, 20000);

	hns_ae_ring_enable_all(handle, 0);
	msleep(100);

	(void)hns_mac_vm_config_bc_en(mac_cb, 0, DISABLE);
}

static void hns_ae_reset(struct hnae_handle *handle)
{
	int i;
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);

	if (port_cb->mac_cb->mac_type == HNAE_PORT_SERVICE) {
		for (i = 0; i < handle->q_num; i++)
			hns_rcb_reset_ring_hw(handle->qs[i]);
	} else {
		hns_mac_reset(port_cb->mac_cb);
		hns_ppe_reset(port_cb->dsaf_dev);
	}
}

void hns_ae_toggle_ring_irq(struct hnae_ring *ring, u32 mask)
{
	u32 flag;

	if (is_tx_ring(ring))
		flag = RCB_INT_FLAG_TX;
	else
		flag = RCB_INT_FLAG_RX;

	hns_rcb_int_clr_hw(ring->q, flag);
	hns_rcb_int_ctrl_hw(ring->q, flag, mask);
}

static void hns_ae_toggle_queue_status(struct hnae_queue *queue, u32 val)
{
	hns_rcb_start(queue, val);
}

static int hns_ae_get_link_status(struct hnae_handle *handle)
{
	u32 link_status;
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	hns_mac_get_link_status(mac_cb, &link_status);

	return !!link_status;
}

static int hns_ae_get_mac_info(struct hnae_handle *handle,
			       u8 *auto_neg, u16 *speed, u8 *duplex)
{
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	return hns_mac_get_port_info(mac_cb, auto_neg, speed, duplex);
}

static int hns_ae_set_mac_info(struct hnae_handle *handle,
			       u8 auto_neg, u16 speed, u8 duplex)
{
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	return hns_mac_set_port_info(mac_cb, auto_neg, speed, duplex);
}

static void hns_ae_adjust_link(struct hnae_handle *handle, int speed,
			       int duplex)
{
	struct hns_mac_cb *mac_cb = hns_get_mac_cb(handle);

	hns_mac_adjust_link(mac_cb, speed, duplex);
}

static void hns_ae_get_ring_bdnum_limit(struct hnae_queue *queue,
					u32 *uplimit, u32 *lowlimit)
{
	*uplimit = HNS_RCB_RING_MAX_PENDING_BD;
	*lowlimit = HNS_RCB_RING_MIN_PENDING_BD;
}

static void hns_ae_get_pauseparam(struct hnae_handle *handle,
				  u32 *auto_neg, u32 *rx_en, u32 *tx_en)
{
	assert(handle);

	hns_mac_get_autoneg(hns_get_mac_cb(handle), auto_neg);

	hns_mac_get_pauseparam(hns_get_mac_cb(handle), rx_en, tx_en);
}

static int hns_ae_set_autoneg(struct hnae_handle *handle, u8 enable)
{
	assert(handle);

	return hns_mac_set_autoneg(hns_get_mac_cb(handle), enable);
}

static int hns_ae_get_autoneg(struct hnae_handle *handle)
{
	u32     auto_neg;

	assert(handle);

	hns_mac_get_autoneg(hns_get_mac_cb(handle), &auto_neg);

	return auto_neg;
}

static void hns_ae_set_pauseparam(struct hnae_handle *handle,
				  u32 rx_en, u32 tx_en)
{
	hns_mac_set_pauseparam(hns_get_mac_cb(handle), rx_en, tx_en);
}

static void hns_ae_get_coalesce_usecs(struct hnae_handle *handle,
				      u32 *tx_usecs, u32 *rx_usecs)
{
	int port;

	port = hns_ae_get_port_idx(handle->ae_opts);

	*tx_usecs = hns_rcb_get_coalesce_usecs(
		hns_ae_get_dsaf_dev(handle->dev),
		hns_dsaf_get_comm_idx_by_port(port));
	*rx_usecs = hns_rcb_get_coalesce_usecs(
		hns_ae_get_dsaf_dev(handle->dev),
		hns_dsaf_get_comm_idx_by_port(port));
}

static void hns_ae_get_rx_max_coalesced_frames(struct hnae_handle *handle,
					       u32 *tx_frames, u32 *rx_frames)
{
	int port;

	assert(handle);

	port = hns_ae_get_port_idx(handle->ae_opts);

	*tx_frames = hns_rcb_get_coalesced_frames(
		hns_ae_get_dsaf_dev(handle->dev), port);
	*rx_frames = hns_rcb_get_coalesced_frames(
		hns_ae_get_dsaf_dev(handle->dev), port);
}

static void hns_ae_set_coalesce_usecs(struct hnae_handle *handle,
				      u32 timeout)
{
	int port;

	assert(handle);

	port = hns_ae_get_port_idx(handle->ae_opts);

	hns_rcb_set_coalesce_usecs(hns_ae_get_dsaf_dev(handle->dev),
				   port, timeout);
}

static int  hns_ae_set_coalesce_frames(struct hnae_handle *handle,
				       u32 coalesce_frames)
{
	int port;
	int ret;

	assert(handle);

	port = hns_ae_get_port_idx(handle->ae_opts);

	ret = hns_rcb_set_coalesced_frames(hns_ae_get_dsaf_dev(handle->dev),
					   port, coalesce_frames);
	return ret;
}

void hns_ae_get_max_ringnum(struct hnae_handle *handle, u32 *max_ringnum)
{
	assert(handle);

	*max_ringnum = hns_rcb_get_max_ringnum(
		hns_ae_get_dsaf_dev(handle->dev));
}

void hns_ae_get_ringnum(struct hnae_handle *handle, u32 *ringnum)
{
	int port;

	assert(handle);

	port = hns_ae_get_port_idx(handle->ae_opts);

	*ringnum = hns_rcb_get_common_ringnum(
		hns_ae_get_dsaf_dev(handle->dev),
		hns_dsaf_get_comm_idx_by_port(port));
}

void hns_ae_update_stats(struct hnae_handle *handle,
			 struct net_device_stats *net_stats)
{
	int port;
	int idx;
	struct dsaf_device *dsaf_dev;
	struct hns_mac_cb *mac_cb;
	struct hns_ppe_cb *ppe_cb;
	struct hnae_queue *queue;
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);
	u64 tx_bytes = 0, rx_bytes = 0, tx_packets = 0, rx_packets = 0;
	u64 rx_errors = 0, tx_errors = 0, tx_dropped = 0;
	u64 rx_missed_errors = 0;

	dsaf_dev = hns_ae_get_dsaf_dev(handle->dev);
	if (!dsaf_dev)
		return;
	port = port_cb->index;
	ppe_cb = hns_get_ppe_cb(handle);
	mac_cb = hns_get_mac_cb(handle);

	for (idx = 0; idx < handle->q_num; idx++) {
		queue = handle->qs[idx];
		hns_rcb_update_stats(queue);

		tx_bytes += queue->tx_ring.stats.bytes.tx_bytes;
		tx_packets += queue->tx_ring.stats.pkts.tx_pkts;
		rx_bytes += queue->tx_ring.stats.bytes.rx_bytes;
		rx_packets += queue->rx_ring.stats.pkts.rx_pkts;

		rx_errors += queue->rx_ring.stats.err_pkt_len
				+ queue->rx_ring.stats.csum_err;
	}

	hns_ppe_update_stats(ppe_cb);
	rx_missed_errors = ppe_cb->hw_stats.rx_drop_no_buf;
	rx_errors += ppe_cb->hw_stats.rx_drop_no_buf
		+ ppe_cb->hw_stats.rx_err_fifo_full;
	tx_errors += ppe_cb->hw_stats.tx_err_checksum
		+ ppe_cb->hw_stats.tx_err_fifo_empty;

	hns_dsaf_update_stats(dsaf_dev, port);
	/* for port upline direction, i.e., rx. */
	rx_missed_errors += dsaf_dev->hw_stats[port].bp_drop;
	rx_errors += dsaf_dev->hw_stats[port].bp_drop;
	rx_errors += dsaf_dev->hw_stats[port].pad_drop;
	rx_errors += dsaf_dev->hw_stats[port].crc_false;

	/* for port downline direction, i.e., tx. */
	port = port + DSAF_PPE_INODE_BASE;
	hns_dsaf_update_stats(dsaf_dev, port);
	tx_dropped += dsaf_dev->hw_stats[port].bp_drop;
	tx_dropped += dsaf_dev->hw_stats[port].pad_drop;
	tx_dropped += dsaf_dev->hw_stats[port].crc_false;
	tx_dropped += dsaf_dev->hw_stats[port].rslt_drop;
	tx_dropped += dsaf_dev->hw_stats[port].vlan_drop;
	tx_dropped += dsaf_dev->hw_stats[port].stp_drop;

	hns_mac_update_stats(mac_cb);
	rx_errors += mac_cb->hw_stats.rx_fcs_err
		+ mac_cb->hw_stats.rx_align_err
		+ mac_cb->hw_stats.rx_fifo_overrun_err
		+ mac_cb->hw_stats.rx_len_err;
	tx_errors += mac_cb->hw_stats.tx_bad_pkts
		+ mac_cb->hw_stats.tx_fragment_err
		+ mac_cb->hw_stats.tx_jabber_err
		+ mac_cb->hw_stats.tx_underrun_err
		+ mac_cb->hw_stats.tx_crc_err;
	tx_dropped += mac_cb->hw_stats.tx_fragment_err
		+ mac_cb->hw_stats.tx_jabber_err
		+ mac_cb->hw_stats.tx_underrun_err
		+ mac_cb->hw_stats.tx_crc_err;

	net_stats->tx_bytes = tx_bytes;
	net_stats->tx_packets = tx_packets;
	net_stats->rx_bytes = rx_bytes;
	net_stats->rx_packets = rx_packets;
	net_stats->rx_errors = rx_errors;
	net_stats->tx_errors = tx_errors;
	net_stats->tx_dropped = tx_dropped;
	net_stats->rx_missed_errors = rx_missed_errors;
	net_stats->rx_crc_errors = mac_cb->hw_stats.rx_fcs_err;
	net_stats->rx_frame_errors = mac_cb->hw_stats.rx_align_err;
	net_stats->rx_fifo_errors = mac_cb->hw_stats.rx_fifo_overrun_err;
	net_stats->rx_length_errors = mac_cb->hw_stats.rx_len_err;
	net_stats->multicast = mac_cb->hw_stats.rx_mc_pkts;
}

void hns_ae_get_stats(struct hnae_handle *handle, u64 *data)
{
	int port;
	int idx;
	struct hns_mac_cb *mac_cb;
	struct hns_ppe_cb *ppe_cb;
	u64 *p = data;
	struct  hnae_port_cb *port_cb;

	if (!handle || !data) {
		dev_err(handle->owner_dev, "NULL handle or data pointer!\n");
		return;
	}

	port_cb = hns_ae_get_port_cb(handle);
	port = port_cb->index;
	mac_cb = hns_get_mac_cb(handle);
	ppe_cb = hns_get_ppe_cb(handle);

	for (idx = 0; idx < handle->q_num; idx++) {
		hns_rcb_get_stats(handle->qs[idx], p);
		p += hns_rcb_get_ring_sset_count((int)ETH_SS_STATS);
	}

	hns_ppe_get_stats(ppe_cb, p);
	p += hns_ppe_get_sset_count((int)ETH_SS_STATS);

	hns_mac_get_stats(mac_cb, p);
	p += hns_mac_get_sset_count(mac_cb, (int)ETH_SS_STATS);

	if (mac_cb->mac_type == HNAE_PORT_SERVICE)
		hns_dsaf_get_stats(port_cb->dsaf_dev, p, port_cb->index);

}

void hns_ae_get_strings(struct hnae_handle *handle,
			u32 stringset, u8 *data)
{
	int port;
	int idx;
	struct hns_mac_cb *mac_cb;
	struct dsaf_device *dsaf_dev;
	struct hns_ppe_cb *ppe_cb;
	u8 *p = data;
	struct	hnae_port_cb *port_cb;

	assert(handle);

	port_cb = hns_ae_get_port_cb(handle);
	port = port_cb->index;
	dsaf_dev = hns_ae_get_dsaf_dev(handle->dev);
	mac_cb = hns_get_mac_cb(handle);
	ppe_cb = hns_get_ppe_cb(handle);

	for (idx = 0; idx < handle->q_num; idx++) {
		hns_rcb_get_strings(stringset, p, idx);
		p += ETH_GSTRING_LEN * hns_rcb_get_ring_sset_count(stringset);
	}

	hns_ppe_get_strings(ppe_cb, stringset, p);
	p += ETH_GSTRING_LEN * hns_ppe_get_sset_count(stringset);

	hns_mac_get_strings(mac_cb, stringset, p);
	p += ETH_GSTRING_LEN * hns_mac_get_sset_count(mac_cb, stringset);

	if (mac_cb->mac_type == HNAE_PORT_SERVICE)
		hns_dsaf_get_strings(stringset, p, port);
}

int hns_ae_get_sset_count(struct hnae_handle *handle, int stringset)
{
	u32 sset_count = 0;
	struct hns_mac_cb *mac_cb;

	assert(handle);

	mac_cb = hns_get_mac_cb(handle);

	sset_count += hns_rcb_get_ring_sset_count(stringset) * handle->q_num;
	sset_count += hns_ppe_get_sset_count(stringset);
	sset_count += hns_mac_get_sset_count(mac_cb, stringset);

	if (mac_cb->mac_type == HNAE_PORT_SERVICE)
		sset_count += hns_dsaf_get_sset_count(stringset);

	return sset_count;
}

static int hns_ae_config_loopback(struct hnae_handle *handle,
				  enum hnae_loop loop, int en)
{
	int ret;
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);

	switch (loop) {
	case MAC_INTERNALLOOP_SERDES:
		ret = hns_mac_config_sds_loopback(port_cb->mac_cb, en);
		break;
	case MAC_INTERNALLOOP_MAC:
		ret = hns_mac_config_mac_loopback(port_cb->mac_cb, loop, en);
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

void hns_ae_update_led_status(struct hnae_handle *handle)
{
	struct hns_mac_cb *mac_cb;

	assert(handle);
	mac_cb = hns_get_mac_cb(handle);
	if (!mac_cb->cpld_vaddr)
		return;
	hns_set_led_opt(mac_cb);
}

int hns_ae_cpld_set_led_id(struct hnae_handle *handle,
			   enum hnae_led_state status)
{
	struct hns_mac_cb *mac_cb;

	assert(handle);

	mac_cb = hns_get_mac_cb(handle);

	return hns_cpld_led_set_id(mac_cb, status);
}

void hns_ae_get_regs(struct hnae_handle *handle, void *data)
{
	u32 *p = data;
	u32 rcb_com_idx;
	int i;
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);
	struct hns_ppe_cb *ppe_cb = hns_get_ppe_cb(handle);

	hns_ppe_get_regs(ppe_cb, p);
	p += hns_ppe_get_regs_count();

	rcb_com_idx = hns_dsaf_get_comm_idx_by_port(port_cb->index);
	hns_rcb_get_common_regs(port_cb->dsaf_dev->rcb_common[rcb_com_idx], p);
	p += hns_rcb_get_common_regs_count();

	for (i = 0; i < handle->q_num; i++) {
		hns_rcb_get_ring_regs(handle->qs[i], p);
		p += hns_rcb_get_ring_regs_count();
	}

	hns_mac_get_regs(port_cb->mac_cb, p);
	p += hns_mac_get_regs_count(port_cb->mac_cb);

	if (port_cb->mac_cb->mac_type == HNAE_PORT_SERVICE)
		hns_dsaf_get_regs(port_cb->dsaf_dev, port_cb->index, p);
}

int hns_ae_get_regs_len(struct hnae_handle *handle)
{
	u32 total_num;
	struct hnae_port_cb *port_cb = hns_ae_get_port_cb(handle);

	total_num = hns_ppe_get_regs_count();
	total_num += hns_rcb_get_common_regs_count();
	total_num += hns_rcb_get_ring_regs_count() * handle->q_num;
	total_num += hns_mac_get_regs_count(port_cb->mac_cb);

	if (port_cb->mac_cb->mac_type == HNAE_PORT_SERVICE)
		total_num += hns_dsaf_get_regs_count();

	return total_num;
}

static struct hnae_ae_ops hns_dsaf_ops = {
	.get_handle = hns_ae_get_handle,
	.put_handle = hns_ae_put_handle,
	.init_queue = hns_ae_init_queue,
	.fini_queue = hns_ae_fini_queue,
	.start = hns_ae_start,
	.stop = hns_ae_stop,
	.reset = hns_ae_reset,
	.toggle_ring_irq = hns_ae_toggle_ring_irq,
	.toggle_queue_status = hns_ae_toggle_queue_status,
	.get_status = hns_ae_get_link_status,
	.get_info = hns_ae_get_mac_info,
	.set_info = hns_ae_set_mac_info,
	.adjust_link = hns_ae_adjust_link,
	.set_loopback = hns_ae_config_loopback,
	.get_ring_bdnum_limit = hns_ae_get_ring_bdnum_limit,
	.get_pauseparam = hns_ae_get_pauseparam,
	.set_autoneg = hns_ae_set_autoneg,
	.get_autoneg = hns_ae_get_autoneg,
	.set_pauseparam = hns_ae_set_pauseparam,
	.get_coalesce_usecs = hns_ae_get_coalesce_usecs,
	.get_rx_max_coalesced_frames = hns_ae_get_rx_max_coalesced_frames,
	.set_coalesce_usecs = hns_ae_set_coalesce_usecs,
	.set_coalesce_frames = hns_ae_set_coalesce_frames,
	.get_max_ringnum = hns_ae_get_max_ringnum,
	.get_ringnum = hns_ae_get_ringnum,
	.set_mac_addr = hns_ae_set_mac_address,
	.set_mc_addr = hns_ae_set_multicast_one,
	.set_mtu = hns_ae_set_mtu,
	.update_stats = hns_ae_update_stats,
	.get_stats = hns_ae_get_stats,
	.get_strings = hns_ae_get_strings,
	.get_sset_count = hns_ae_get_sset_count,
	.update_led_status = hns_ae_update_led_status,
	.set_led_id = hns_ae_cpld_set_led_id,
	.get_regs = hns_ae_get_regs,
	.get_regs_len = hns_ae_get_regs_len
};

int hns_dsaf_ae_init(struct dsaf_device *dsaf_dev)
{
	struct hnae_ae_dev *ae_dev = &dsaf_dev->ae_dev;

	ae_dev->ops = &hns_dsaf_ops;
	ae_dev->dev = dsaf_dev->dev;

	return hnae_ae_register(ae_dev, THIS_MODULE);
}

void hns_dsaf_ae_uninit(struct dsaf_device *dsaf_dev)
{
	hnae_ae_unregister(&dsaf_dev->ae_dev);
}
