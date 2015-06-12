/* Copyright (c) 2014 Linaro Ltd.
 * Copyright (c) 2014 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/mbi.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/clk.h>
#include <linux/circ_buf.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>


#define BITS_TX_EN			BIT(2)
#define BITS_RX_EN			BIT(1)

#define MDIO_SINGLE_CMD			0x00
#define MDIO_READ_DATA			0x0c
#define MDIO_WRITE_DATA			0x08
#define MDIO_CTRL			0x03cc
#define MDIO_RDATA_STATUS		0x10

#define MDIO_START			BIT(14)
#define MDIO_R_VALID			BIT(0)
#define MDIO_READ			(BIT(11) | MDIO_START)
#define MDIO_WRITE			(BIT(10) | MDIO_START)
#define MDIO_ST_CLAUSE_22		BIT(12)

#define SGMII_SPEED_1000		0x2c
#define SGMII_SPEED_100			0x2f
#define SGMII_SPEED_10			0x2d
#define MII_SPEED_100			0x0f
#define MII_SPEED_10			0x0d
#define GMAC_SPEED_1000			0x08
#define GMAC_SPEED_100			0x07
#define GMAC_SPEED_10			0x06
#define GMAC_FULL_DUPLEX		0x00

#define HIP05_TX_VLD			BIT(6)

#define RX_BQ_INT_THRESHOLD		0x01
#define TX_RQ_INT_THRESHOLD		0x01
#define RX_BQ_IN_TIMEOUT		0x10000
#define TX_RQ_IN_TIMEOUT		0x50000

#define MAC_MAX_FRAME_SIZE		1600
#define DESC_SIZE			32
#define RX_DESC_NUM			1000
#define TX_DESC_NUM			1000
#define BUFFER_SIZE			4000

#define RCB_CFG_BD_NUM			0x9000
#define RCB_CFG_PKTLINE			0x9050

#define RCB_COM_CFG_ENDIAN		0x00

#define RCB_CFG_RX_RING_BASEADDR_L	0x00
#define RCB_CFG_RX_RING_BASEADDR_H	0x04
#define RCB_CFG_RX_RING_BD_NUM		0x08
#define RCB_CFG_RX_RING_BD_LEN		0x0C
#define RCB_CFG_RX_RING_PKTLINE		0x10
#define RCB_CFG_RX_RING_TAIL		0x18
#define RCB_CFG_RX_RING_HEAD		0x1C
#define RCB_CFG_RX_RING_FBDNUM		0x20
#define RCB_CFG_TX_RING_BASEADDR_L	0x40
#define RCB_CFG_TX_RING_BASEADDR_H	0x44
#define RCB_CFG_TX_RING_BD_NUM		0x48
#define RCB_CFG_TX_RING_BD_LEN		0x4C
#define RCB_CFG_TX_RING_PKTLINE		0x50
#define RCB_RING_TX_RING_TAIL		0x58
#define RCB_RING_TX_RING_HEAD		0x5C
#define RCB_RING_ENABLE			0x7C

#define RCB_CFG_SYS_FSH_REG		0x0C
#define RCB_CFG_INIT_FLAG		0x10
#define RCB_INTMASK_RXWL		0xA0
#define RCB_RING_INTSTS_RX_RING		0xA4
#define RCB_INTMASK_TXWL		0xAC
#define RCB_RING_INTSTS_TX_RING		0xB0

#define GE_DUPLEX_TYPE			0x08
#define GE_PORT_MODE			0x40
#define GE_PORT_EN			0x44
#define GE_MODE_CHANGE_EN		0x1B4
#define GE_STATION_ADDR_LOW_2		0x210
#define GE_STATION_ADDR_HIGH_2		0x214

#define HIP05_GET_DESC(R, i, type)	(&(((struct type *)((R).desc))[i]))
#define HIP05_RX_DESC(R, i)		HIP05_GET_DESC(R, i, hip05_desc)
#define HIP05_TX_DESC(R, i)		HIP05_GET_DESC(R, i, hip05_desc)

#define HIP05_NEED_TO_REFILL(R)		((R->next_to_use > R->next_to_clean) ?\
					(R->next_to_use - R->next_to_clean) :\
					(R->next_to_use + RX_DESC_NUM - R->next_to_clean))

#define HIP05_MAX_REFILL_FRAME		64

#define HIP05_PPE_COMMON_OFFSET		0x70000
#define HIP05_RCB_COMMON_OFFSET		0x80000
#define HIP05_GE_OFFSET			0x1000
#define HIP05_RCB_RING_OFFSET		0x10000

#define HIP05_TX_DESC_STATUS_RA		BIT(8)
#define HIP05_TX_DESC_STATUS_RI		BIT(9)
#define HIP05_TX_DESC_STATUS_L4CS	BIT(10)
#define HIP05_TX_DESC_STATUS_L3CS	BIT(11)
#define HIP05_TX_DESC_STATUS_FE		BIT(12)
#define HIP05_TX_DESC_STATUS_VLD	BIT(13)
#define HIP05_TX_DESC_BUFNUM_SHIFT	8
#define HIP05_TX_DESC_PKTLEN_SHIFT	16

#define HIP05_RX_DESC_STATUS_FE		BIT(25)
#define HIP05_RX_DESC_STATUS_FRAG	BIT(26)
#define HIP05_RX_DESC_STATUS_VLD	BIT(27)
#define HIP05_RX_DESC_STATUS_L2E	BIT(28)
#define HIP05_RX_DESC_STATUS_L3E	BIT(29)
#define HIP05_RX_DESC_STATUS_L4E	BIT(30)
#define HIP05_RX_DESC_STATUS_DROP	BIT(31)
#define HIP05_RX_DESC_BUFNUM_SHIFT	8

#define HIP05_RX_DESC_LEN_MASK		0xffff

#define RX_INTS_MASK			BIT(0)
#define TX_INTS_MASK			BIT(0)

#define HIP05_PPE0_RESET_REQ		BIT(0)
#define HIP05_PPE1_RESET_REQ		BIT(1)
#define HIP05_PPE2_RESET_REQ		BIT(2)
#define HIP05_PPE3_RESET_REQ		BIT(3)
#define HIP05_PPE4_RESET_REQ		BIT(4)
#define HIP05_PPE5_RESET_REQ		BIT(5)
#define HIP05_PPE6_RESET_REQ		BIT(6)
#define HIP05_PPE7_RESET_REQ		BIT(7)

#define HIP05_RCB1_RESET_REQ		BIT(8)
#define HIP05_RCB2_RESET_REQ		BIT(9)

#define HIP05_PPE0_RESET_DREQ		BIT(0)
#define HIP05_PPE1_RESET_DREQ		BIT(1)
#define HIP05_PPE2_RESET_DREQ		BIT(2)
#define HIP05_PPE3_RESET_DREQ		BIT(3)
#define HIP05_PPE4_RESET_DREQ		BIT(4)
#define HIP05_PPE5_RESET_DREQ		BIT(5)
#define HIP05_PPE6_RESET_DREQ		BIT(6)
#define HIP05_PPE7_RESET_DREQ		BIT(7)

#define HIP05_PPE_CFG_XGE_MODE		0x80

#define HIP05_RCB1_RESET_DREQ		BIT(8)
#define HIP05_RCB2_RESET_DREQ		BIT(9)

#define TIMER_COUNTER	10

#define MAX_QUEUE_NUMS	256
#define MAX_TX_QUEUES MAX_QUEUE_NUMS
#define MAX_RX_QUEUES MAX_QUEUE_NUMS

#define HIP05_MODE_10M			0x00
#define HIP05_MODE_100M			0x01
#define HIP05_MODE_1000M		0x02
#define HIP05_MODE_NAME_OFFSET		0x06

#define HIP05_PPE_CFG_MODE_XGE		0x01
#define HIP05_PPE_CFG_MODE_GE		0x00

struct hip05_desc {
	__le64	addr;
	union {
		struct {
			__le32  cmd_type_len;
			__le32	status;
			__le32	reserved_3[4];
		} tx;
		struct {
			__le32	status;
			__le32	pkt_len;
			__le32	cmd_type;
			__le32	reserved_2[3];
		} rx;
	};
};

struct hip05_buffer_info {
	dma_addr_t dma;
	struct sk_buff *skb;
	u16 length;
};

struct hip05_ring {
	struct net_device	*dev;
	struct hip05_desc	*desc;
	struct hip05_buffer_info *buff_info;
	dma_addr_t		desc_phys;
	u16	desc_count;
	u16	next_to_use;
	u16	next_to_clean;
	u32	size;
	u8	index;
};

struct hip05_priv {
	struct net_device *netdev;
	struct device *dev;
	void __iomem *rcb_common_base;
	void __iomem *rcb_ring_base;
	void __iomem *gmac_reset_base;

	struct hip05_ring *tx_ring ____cacheline_aligned_in_smp;
	struct hip05_ring *rx_ring;

	struct phy_device *phy;
	struct device_node *phy_node;
	phy_interface_t	phy_mode;
	unsigned int ppe_cfg_xge_mode;	/* 1: xge, 0: ge */

	unsigned int speed;
	unsigned int duplex;

	int rx_irq;
	int tx_irq;

	unsigned int rx_desc_num;
	unsigned int tx_desc_num;

	unsigned int rx_buf_size;
	unsigned int tx_buf_size;

	struct regmap *ppe_map;
	struct regmap *ge_map;
	struct napi_struct napi;
	struct work_struct tx_timeout_task;
};
static void hip05_port_enable(struct hip05_priv *priv);
static void hip05_port_disable(struct hip05_priv *priv);

static const char *hip05_mode_name(int mode)
{
	static const char * const names[] = {
		[HIP05_MODE_10M] = "10M",
		[HIP05_MODE_100M] = "100M",
		[HIP05_MODE_1000M] = "1000M",
	};

	int val = mode - HIP05_MODE_NAME_OFFSET;

	if (val < HIP05_MODE_10M || val > HIP05_MODE_1000M)
		return "unknown";

	return names[val];
}

static void hip05_config_port(struct net_device *dev, u32 speed, u32 duplex)
{
	struct hip05_priv *priv = netdev_priv(dev);
	u32 val;

	priv->speed = speed;
	priv->duplex = duplex;

	switch (priv->phy_mode) {
	case PHY_INTERFACE_MODE_SGMII:
		if (speed == SPEED_1000)
			val = GMAC_SPEED_1000;
		else if (speed == SPEED_100)
			val = GMAC_SPEED_100;
		else
			val = GMAC_SPEED_10;
		break;
	case PHY_INTERFACE_MODE_MII:
		if (speed == SPEED_100)
			val = GMAC_SPEED_100;
		else
			val = GMAC_SPEED_10;
		break;
	default:
		netdev_warn(dev, "not supported mode\n");
		val = GMAC_SPEED_10;
		break;
	}

	netdev_info(dev, "Set GE to Port Speed: %s, Duplex: %s\n",
		    hip05_mode_name(val), duplex ? "FULL" : "HALF");
	regmap_write(priv->ge_map, GE_PORT_MODE, val);
	regmap_write(priv->ge_map, GE_DUPLEX_TYPE, duplex);
	regmap_write(priv->ge_map, GE_MODE_CHANGE_EN, 1);
}

static int hip05_rcb_ready(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	u32	count = 0, val = 0;

	while (count < TIMER_COUNTER) {
		mdelay(10);
		val = readl_relaxed(priv->rcb_common_base + RCB_CFG_INIT_FLAG);
		if (val) {
			writel_relaxed(1, priv->rcb_common_base + RCB_CFG_SYS_FSH_REG);
			return 0;
		}
		count++;
	}

	return -1;
}

static void hip05_set_desc_addr(struct hip05_priv *priv)
{
	struct hip05_ring *tx_ring, *rx_ring;

	tx_ring = priv->tx_ring;
	rx_ring = priv->rx_ring;

	writel_relaxed((u32)rx_ring->desc_phys,
		       priv->rcb_ring_base + RCB_CFG_RX_RING_BASEADDR_L);
	writel_relaxed(rx_ring->desc_phys >> 32,
		       priv->rcb_ring_base + RCB_CFG_RX_RING_BASEADDR_H);
	writel_relaxed((u32)tx_ring->desc_phys,
		       priv->rcb_ring_base + RCB_CFG_TX_RING_BASEADDR_L);
	writel_relaxed(tx_ring->desc_phys >> 32,
		       priv->rcb_ring_base + RCB_CFG_TX_RING_BASEADDR_H);
}

static int hip05_alloc_ring(struct net_device *dev, struct device *d)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct hip05_ring *rx_ring, *tx_ring;
	struct sk_buff	*skb;
	dma_addr_t	dma;
	struct hip05_desc *rx_desc;
	struct hip05_buffer_info *buffer_info;
	int	pos, i, desc_len = sizeof(struct hip05_desc);

	tx_ring = kzalloc(sizeof(*tx_ring), GFP_KERNEL);
	if (!tx_ring)
		goto out;

	rx_ring = kzalloc(sizeof(*rx_ring), GFP_KERNEL);
	if (!rx_ring)
		goto free_tx_ring;

	priv->rx_ring = rx_ring;
	priv->tx_ring = tx_ring;
	rx_ring->desc_count = priv->rx_desc_num;
	tx_ring->desc_count = priv->tx_desc_num;

	tx_ring->desc = dma_alloc_coherent(d, desc_len * tx_ring->desc_count,
					   &tx_ring->desc_phys, GFP_ATOMIC);

	if (!tx_ring->desc)
		goto free_rx_ring;

	rx_ring->desc = dma_alloc_coherent(d, desc_len * rx_ring->desc_count,
					   &rx_ring->desc_phys, GFP_ATOMIC);

	if (!rx_ring->desc)
		goto free_tx_desc;

	tx_ring->buff_info = kcalloc(tx_ring->desc_count, sizeof(*buffer_info),
				     GFP_KERNEL);
	if (!tx_ring->buff_info)
		goto free_rx_desc;

	rx_ring->buff_info = kcalloc(rx_ring->desc_count, sizeof(*buffer_info),
				     GFP_KERNEL);
	if (!rx_ring->buff_info)
		goto free_tx_buff_info;

	for (i = 0; i < rx_ring->desc_count; i++) {
		skb = netdev_alloc_skb(dev, priv->rx_buf_size);
		if (!skb)
			goto free_rx_buff;

		dma = dma_map_single(d, skb->data,
				     priv->rx_buf_size, DMA_FROM_DEVICE);
		if (dma_mapping_error(d, dma)) {
			dev_kfree_skb_any(skb);
			goto free_rx_buff;
		}
		buffer_info = &rx_ring->buff_info[i];
		buffer_info->dma = dma;
		buffer_info->skb = skb;
		rx_desc = HIP05_RX_DESC(*rx_ring, i);
		rx_desc->addr = cpu_to_le64(buffer_info->dma);
	}

	return 0;

free_rx_buff:
	for (pos = 0; pos < i; pos++) {
		rx_desc = HIP05_RX_DESC(*rx_ring, i);
		buffer_info = &rx_ring->buff_info[i];
		if (buffer_info->dma)
			dma_unmap_single(d, buffer_info->dma,
					 priv->rx_buf_size, DMA_FROM_DEVICE);
		dev_kfree_skb_any(buffer_info->skb);
	}
	kfree(rx_ring->buff_info);
free_tx_buff_info:
	kfree(tx_ring->buff_info);
free_rx_desc:
	dma_free_coherent(d, sizeof(struct hip05_desc) * tx_ring->desc_count,
			  tx_ring->desc, tx_ring->desc_phys);
free_tx_desc:
	dma_free_coherent(d, sizeof(struct hip05_desc) * tx_ring->desc_count,
			  tx_ring->desc, tx_ring->desc_phys);
free_rx_ring:
	kfree(rx_ring);
free_tx_ring:
	kfree(tx_ring);
out:
	return -ENOMEM;
}

static void hip05_free_ring(struct net_device *dev, struct device *d)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct hip05_ring *rx_ring, *tx_ring;
	struct hip05_buffer_info *buffer_info;
	struct sk_buff *skb;
	int i;

	rx_ring = priv->rx_ring;
	tx_ring = priv->tx_ring;
	for (i = 0; i < rx_ring->desc_count; i++) {
		buffer_info = &rx_ring->buff_info[i];
		skb = buffer_info->skb;
		if (buffer_info->dma)
			dma_unmap_single(d, buffer_info->dma,
					 priv->rx_buf_size, DMA_FROM_DEVICE);
		if (skb)
			dev_kfree_skb_any(skb);
	}

	for (i = 0; i < tx_ring->desc_count; i++) {
		buffer_info = &tx_ring->buff_info[i];
		skb = buffer_info->skb;
		if (buffer_info->dma)
			dma_unmap_single(d, buffer_info->dma,
					 priv->tx_buf_size, DMA_TO_DEVICE);
		if (skb)
			dev_kfree_skb_any(skb);
	}

	kfree(rx_ring->buff_info);
	kfree(tx_ring->buff_info);

	dma_free_coherent(d, sizeof(struct hip05_desc) * tx_ring->desc_count,
			  tx_ring->desc, tx_ring->desc_phys);
	dma_free_coherent(d, sizeof(struct hip05_desc) * rx_ring->desc_count,
			  rx_ring->desc, rx_ring->desc_phys);
	kfree(priv->tx_ring);
	kfree(priv->rx_ring);
}

static void hip05_ring_enable(struct hip05_priv *priv)
{
	writel_relaxed(0x01, priv->rcb_ring_base + RCB_RING_ENABLE);
}

static void hip05_ring_disable(struct hip05_priv *priv)
{
	writel_relaxed(0x00, priv->rcb_ring_base + RCB_RING_ENABLE);
}

static void hip05_hw_init(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);

	regmap_write(priv->ppe_map, HIP05_PPE_CFG_XGE_MODE, priv->ppe_cfg_xge_mode);

	writel_relaxed(1, priv->rcb_common_base + RCB_COM_CFG_ENDIAN);

	writel_relaxed(priv->rx_desc_num,
		       priv->rcb_common_base + RCB_CFG_BD_NUM);

	writel_relaxed(0, priv->rcb_ring_base + RCB_CFG_RX_RING_BD_NUM);
	writel_relaxed(0, priv->rcb_ring_base + RCB_CFG_TX_RING_BD_NUM);

	writel_relaxed(1, priv->rcb_common_base + RCB_CFG_PKTLINE);
	writel_relaxed(0x0, priv->rcb_ring_base + RCB_CFG_RX_RING_PKTLINE);
	writel_relaxed(0x0, priv->rcb_ring_base + RCB_CFG_TX_RING_PKTLINE);

	writel_relaxed(0x03, priv->rcb_ring_base + RCB_CFG_RX_RING_BD_LEN);
	writel_relaxed(0x03, priv->rcb_ring_base + RCB_CFG_TX_RING_BD_LEN);

	hip05_set_desc_addr(priv);
}

static void hip05_irq_enable(struct hip05_priv *priv)
{
	writel_relaxed(0, priv->rcb_ring_base + RCB_INTMASK_RXWL);
	writel_relaxed(0, priv->rcb_ring_base + RCB_INTMASK_TXWL);
}

static void hip05_irq_disable(struct hip05_priv *priv)
{
	writel_relaxed(1, priv->rcb_ring_base + RCB_INTMASK_RXWL);
	writel_relaxed(1, priv->rcb_ring_base + RCB_INTMASK_TXWL);
}

static void hip05_port_enable(struct hip05_priv *priv)
{
	unsigned int val = BITS_RX_EN | BITS_TX_EN;

	regmap_write(priv->ge_map, GE_PORT_EN, val);
}

static void hip05_port_disable(struct hip05_priv *priv)
{
	unsigned int val = 0;

	regmap_write(priv->ge_map, GE_PORT_EN, val);
}

static void hip05_hw_set_mac_addr(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	unsigned char *mac = dev->dev_addr;
	u32 val;

	val = mac[1] | (mac[0] << 8);
	regmap_write(priv->ge_map, GE_STATION_ADDR_HIGH_2, val);

	val = mac[5] | (mac[4] << 8) | (mac[3] << 16) | (mac[2] << 24);
	regmap_write(priv->ge_map, GE_STATION_ADDR_LOW_2, val);
}

static int hip05_net_set_mac_address(struct net_device *dev, void *p)
{
	int ret;

	ret = eth_mac_addr(dev, p);
	if (!ret)
		hip05_hw_set_mac_addr(dev);

	return ret;
}

static void hip05_adjust_link(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct phy_device *phy = priv->phy;

	if ((priv->speed != phy->speed) || (priv->duplex != phy->duplex)) {
		hip05_config_port(dev, phy->speed, phy->duplex);
		phy_print_status(phy);
	}
}

static void hip05_rx_refill(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	struct hip05_desc *desc;
	struct hip05_ring *rx_ring = priv->rx_ring;
	struct hip05_buffer_info *buff_info;
	dma_addr_t dma;
	u32 i;


	for (i = rx_ring->next_to_clean; i != rx_ring->next_to_use;) {
		buff_info = &rx_ring->buff_info[i];
		skb = buff_info->skb;
		if (unlikely(skb)) {
			netdev_err(dev, "inconsistent rx_skb for refill\n");
			break;
		}
		desc = HIP05_RX_DESC(*rx_ring, i);
		skb = netdev_alloc_skb(dev, priv->rx_buf_size);
		if (!skb) {
			netdev_err(dev, "no memory to alloc skb\n");
			break;
		}
		dma = dma_map_single(priv->dev, skb->data,
				     priv->rx_buf_size, DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dev, dma)) {
			netdev_err(dev, "dma mapping error\n");
			dev_kfree_skb_any(skb);
			break;
		}
		buff_info->dma = dma;
		buff_info->skb = skb;
		desc->addr = cpu_to_le64(buff_info->dma);
		if (++i == rx_ring->desc_count)
			i = 0;
	}
	rx_ring->next_to_clean = i;
}

static int hip05_rx(struct net_device *dev, int limit)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	struct hip05_desc *desc;
	struct hip05_ring *rx_ring = priv->rx_ring;
	struct hip05_buffer_info *buff_info;
	dma_addr_t dma;
	u32 num, pos, count = 0, len;

	num = readl_relaxed(priv->rcb_ring_base + RCB_CFG_RX_RING_FBDNUM);
	if (num > limit)
		num = limit;

	pos = rx_ring->next_to_use;
	while (num) {
		buff_info = &rx_ring->buff_info[pos];
		skb = buff_info->skb;
		if (unlikely(!skb)) {
			netdev_err(dev, "inconsistent rx_skb, %d, %d, %d\n",
					pos, rx_ring->next_to_clean,
					rx_ring->next_to_use);
			break;
		}
		/* ensure get updated desc */
		rmb();
		buff_info->skb = NULL;
		desc = HIP05_RX_DESC(*rx_ring, pos);
		len = le32_to_cpu(desc->rx.pkt_len) & HIP05_RX_DESC_LEN_MASK;

		dma = buff_info->dma;
		dma_unmap_single(priv->dev, dma, priv->rx_buf_size,
				 DMA_FROM_DEVICE);

		if (len > MAC_MAX_FRAME_SIZE) {
			netdev_err(dev, "rcv len err, len = %d\n", skb->len);
			dev->stats.rx_errors++;
			dev->stats.rx_length_errors++;
			dev_kfree_skb_any(skb);
			goto next;
		}

		if (!(le32_to_cpu(desc->rx.status) & HIP05_RX_DESC_STATUS_VLD)) {
			netdev_warn(dev, "no valid bd\n");
			goto next;
		}

		if (le32_to_cpu(desc->rx.status) & HIP05_RX_DESC_STATUS_DROP) {
			netdev_warn(dev, "drop the packege, queue have %d package\n", num);
			goto next;
		}

		if (le32_to_cpu(desc->rx.status) & HIP05_RX_DESC_STATUS_L2E) {
			netdev_warn(dev, "l2e error\n");
			goto next;
		}
		skb_put(skb, len);

		skb->protocol = eth_type_trans(skb, dev);
		napi_gro_receive(&priv->napi, skb);
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += skb->len;
		dev->last_rx = jiffies;

next:
		writel_relaxed(1, priv->rcb_ring_base + RCB_CFG_RX_RING_HEAD);
		num = readl_relaxed(priv->rcb_ring_base + RCB_CFG_RX_RING_FBDNUM);
		if (++pos == rx_ring->desc_count)
			pos = 0;
		if (++count >= limit)
			break;

		if (HIP05_NEED_TO_REFILL(priv->rx_ring) > HIP05_MAX_REFILL_FRAME)
			hip05_rx_refill(dev);
		
	}

	rx_ring->next_to_use = pos;
	return count;
}

static void hip05_xmit_reclaim(struct net_device *dev)
{
	struct sk_buff *skb;
	struct hip05_priv *priv = netdev_priv(dev);
	struct hip05_ring *tx_ring = priv->tx_ring;
	struct hip05_buffer_info *buff_info;
	unsigned int bytes_compl = 0, pkts_compl = 0;
	u32 start, end, i;

	netif_tx_lock(dev);

	start = tx_ring->next_to_clean;
	end = readl_relaxed(priv->rcb_ring_base + RCB_RING_TX_RING_HEAD);
	rmb();
	for (i = start; i != end; ) {
		buff_info = &tx_ring->buff_info[i];
		skb = buff_info->skb;
		if (unlikely(!skb)) {
			netdev_err(dev, "inconsistent tx_skb, start is %d, end is %d, next to use %d\n",
				   start, end, tx_ring->next_to_use);
			break;
		}
		buff_info->skb = NULL;
		pkts_compl++;
		bytes_compl += buff_info->length;
		dma_unmap_single(priv->dev, buff_info->dma,
				 skb->len, DMA_TO_DEVICE);
		dev_consume_skb_any(skb);
		if (++i == tx_ring->desc_count)
			i = 0;
	}

	tx_ring->next_to_clean = i;
	netif_tx_unlock(dev);

	if (pkts_compl)
		netdev_completed_queue(dev, pkts_compl, bytes_compl);

	if (unlikely(netif_queue_stopped(priv->netdev)) && pkts_compl)
		netif_wake_queue(priv->netdev);
}

static int hip05_poll(struct napi_struct *napi, int budget)
{
	struct hip05_priv *priv = container_of(napi,
				struct hip05_priv, napi);
	struct net_device *dev = priv->netdev;
	int work_done = 0, task = budget, num = 0;

	hip05_xmit_reclaim(dev);
	num = hip05_rx(dev, task);
	work_done += num;

	if (work_done < budget) {
		napi_complete(napi);
		hip05_irq_enable(priv);
	}

	return work_done;
}

static irqreturn_t hip05_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct hip05_priv *priv = netdev_priv(dev);

	hip05_irq_disable(priv);
	napi_schedule(&priv->napi);
	return IRQ_HANDLED;
}

static int hip05_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	struct hip05_desc *desc;
	struct hip05_ring *tx_ring = priv->tx_ring;
	struct hip05_buffer_info *tx_buf;
	int buf_num = 1;

	tx_buf = &tx_ring->buff_info[tx_ring->next_to_use];
	tx_buf->skb = skb;
	tx_buf->length = skb->len;
	tx_buf->dma = dma_map_single(priv->dev, skb->data, skb->len,
				     DMA_TO_DEVICE);
	if (dma_mapping_error(priv->dev, tx_buf->dma)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	desc = HIP05_TX_DESC(*tx_ring, tx_ring->next_to_use);
	desc->addr = cpu_to_le64(tx_buf->dma);
	desc->tx.cmd_type_len = cpu_to_le32(tx_buf->length << HIP05_TX_DESC_PKTLEN_SHIFT);
	desc->tx.cmd_type_len |= cpu_to_le32(buf_num << HIP05_TX_DESC_BUFNUM_SHIFT);
	desc->tx.status = cpu_to_le32(HIP05_TX_DESC_STATUS_VLD |
				      HIP05_TX_DESC_STATUS_FE |
				      HIP05_TX_DESC_STATUS_RA);

	wmb();

	dev->trans_start = jiffies;
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;
	netdev_sent_queue(dev, skb->len);

	writel_relaxed(1, priv->rcb_ring_base + RCB_RING_TX_RING_TAIL);

	if (++tx_ring->next_to_use == tx_ring->desc_count)
		tx_ring->next_to_use = 0;

	return NETDEV_TX_OK;
}

static int hip05_net_open(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);

	priv->phy = of_phy_connect(dev, priv->phy_node,
				   &hip05_adjust_link, 0,
				   priv->phy_mode);
	if (!priv->phy)
		return -ENODEV;

	priv->phy->advertising = priv->phy->supported;

	hip05_ring_enable(priv);
	netdev_reset_queue(dev);
	netif_start_queue(dev);
	napi_enable(&priv->napi);

	hip05_port_enable(priv);
	enable_irq(priv->rx_irq);
	enable_irq(priv->tx_irq);
	hip05_irq_enable(priv);
	phy_start(priv->phy);

	return 0;
}

static int hip05_net_close(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);

	if (priv->phy) {
		phy_stop(priv->phy);
		phy_disconnect(priv->phy);
	}
	hip05_port_disable(priv);
	hip05_irq_disable(priv);
	disable_irq(priv->rx_irq);
	disable_irq(priv->tx_irq);
	napi_disable(&priv->napi);
	netif_stop_queue(dev);
	hip05_ring_disable(priv);

	return 0;
}

static void hip05_tx_timeout_task(struct work_struct *work)
{
	struct hip05_priv *priv;

	priv = container_of(work, struct hip05_priv, tx_timeout_task);
	hip05_net_close(priv->netdev);
	hip05_net_open(priv->netdev);
}

static void hip05_net_timeout(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);

	schedule_work(&priv->tx_timeout_task);
}

static const struct net_device_ops hip05_netdev_ops = {
	.ndo_open		= hip05_net_open,
	.ndo_stop		= hip05_net_close,
	.ndo_start_xmit		= hip05_net_xmit,
	.ndo_tx_timeout		= hip05_net_timeout,
	.ndo_set_mac_address	= hip05_net_set_mac_address,
};

static int hip05_get_settings(struct net_device *net_dev,
			      struct ethtool_cmd *cmd)
{
	struct hip05_priv *priv = netdev_priv(net_dev);

	if (!priv->phy)
		return -ENODEV;

	return phy_ethtool_gset(priv->phy, cmd);
}

static int hip05_set_settings(struct net_device *net_dev,
			      struct ethtool_cmd *cmd)
{
	struct hip05_priv *priv = netdev_priv(net_dev);

	if (!priv->phy)
		return -ENODEV;

	return phy_ethtool_sset(priv->phy, cmd);
}

static struct ethtool_ops hip05_ethtools_ops = {
	.get_link		= ethtool_op_get_link,
	.get_settings		= hip05_get_settings,
	.set_settings		= hip05_set_settings,
};

static void hip05_reset_ppe(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	u32	val;

	val = HIP05_PPE6_RESET_REQ | HIP05_PPE6_RESET_REQ |
	      HIP05_RCB1_RESET_REQ | HIP05_RCB2_RESET_REQ;

	writel_relaxed(val, priv->gmac_reset_base + 0xa48);

	mdelay(10);
	val = HIP05_PPE6_RESET_DREQ | HIP05_PPE6_RESET_DREQ |
	      HIP05_RCB1_RESET_DREQ | HIP05_RCB2_RESET_DREQ;

	writel_relaxed(val, priv->gmac_reset_base + 0xa4c);
}

static int hip05_ring_init(struct net_device *dev)
{
	struct hip05_priv *priv = netdev_priv(dev);
	int ret;
	static bool need_reset = true;

	if (need_reset) {
		hip05_reset_ppe(dev);
		need_reset = false;
	}

	priv->rx_desc_num = RX_DESC_NUM;
	priv->tx_desc_num = TX_DESC_NUM;
	priv->rx_buf_size = BUFFER_SIZE;
	priv->tx_buf_size = BUFFER_SIZE;

	ret = hip05_alloc_ring(dev, priv->dev);
	if (ret)
		return -ENOMEM;

	return 0;
}

static int hip05_dev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct of_phandle_args arg;
	struct net_device *ndev;
	struct hip05_priv *priv;
	struct resource *res;
	const char *mac_addr;
	int virq;
	int ret;

	ndev = alloc_netdev(sizeof(struct hip05_priv), "gmac%d",
			    NET_NAME_UNKNOWN, ether_setup);
	if (!ndev)
		return -ENOMEM;

	platform_set_drvdata(pdev, ndev);

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
		netdev_warn(ndev, "could not set mask and coherent 64 bit\n");

	priv = netdev_priv(ndev);
	priv->dev = dev;
	priv->netdev = ndev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	priv->rcb_common_base = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(priv->rcb_common_base)) {
		ret = PTR_ERR(priv->rcb_common_base);
		goto out_free_netdev;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	priv->gmac_reset_base = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(priv->gmac_reset_base)) {
		ret = PTR_ERR((void *)priv->gmac_reset_base);
		goto out_free_netdev;
	}

	priv->rcb_ring_base = priv->rcb_common_base + HIP05_RCB_RING_OFFSET;

	ret = of_parse_phandle_with_fixed_args(node, "ppe-handle", 1, 0, &arg);
	if (ret < 0) {
		dev_warn(dev, "no ppe-handle\n");
		goto out_free_netdev;
	}

	if (arg.args[0] == HIP05_PPE_CFG_MODE_XGE)
		priv->ppe_cfg_xge_mode = HIP05_PPE_CFG_MODE_XGE;
	else
		priv->ppe_cfg_xge_mode = HIP05_PPE_CFG_MODE_GE;

	priv->ppe_map = syscon_node_to_regmap(arg.np);
	if (IS_ERR(priv->ppe_map)) {
		dev_warn(dev, "no syscon hisilicon, hip05-ppe\n");
		ret = PTR_ERR(priv->ppe_map);
		goto out_free_netdev;
	}

	ret = of_parse_phandle_with_fixed_args(node, "ge-handle", 2, 0, &arg);
	if (ret < 0) {
		dev_warn(dev, "no ge-handle\n");
		goto out_free_netdev;
	}

	priv->speed = arg.args[0];
	priv->duplex = arg.args[1];

	priv->ge_map = syscon_node_to_regmap(arg.np);
	if (IS_ERR(priv->ge_map)) {
		dev_warn(dev, "no syscon hisilicon, hip05-ge\n");
		ret = PTR_ERR(priv->ge_map);
		goto out_free_netdev;
	}

	priv->phy_mode = of_get_phy_mode(node);
	if (priv->phy_mode < 0) {
		dev_err(dev, "not find phy-mode\n");
		ret = -EINVAL;
		goto out_free_netdev;
	}

	virq = mbi_parse_irqs(dev, NULL);
	if (virq > 0) {
		priv->tx_irq = virq;
		priv->rx_irq = virq + 1;
		dev_info(dev, "MBI enabled\n");
	} else {
		dev_info(dev, "Use the legacy interrupts.\n");

		priv->rx_irq = platform_get_irq(pdev, 0);
		if (priv->rx_irq <= 0) {
			dev_err(dev, "No rx irq resource\n");
			ret = -EINVAL;
			goto out_phy_node;
		}

		priv->tx_irq = platform_get_irq(pdev, 1);
		if (priv->tx_irq <= 0) {
			dev_err(dev, "No irq resource\n");
			ret = -EINVAL;
			goto out_phy_node;
		}
	}

	ret = request_irq(priv->rx_irq, hip05_interrupt,
			  IRQF_SHARED, pdev->name, ndev);
	if (ret) {
		dev_err(dev, "devm_request_irq rx ailed\n");
		goto out_phy_node;
	}
	disable_irq(priv->rx_irq);
	ret = request_irq(priv->tx_irq, hip05_interrupt,
			  IRQF_SHARED, pdev->name, ndev);
	if (ret) {
		dev_err(dev, "devm_request_irq tx failed\n");
		goto out_phy_node;
	}
	disable_irq(priv->tx_irq);

	priv->phy_node = of_parse_phandle(node, "phy-handle", 0);
	if (!priv->phy_node) {
		dev_err(dev, "could not find phy-handle\n");
		ret = -EINVAL;
		goto out_phy_node;
	}

	mac_addr = of_get_mac_address(node);
	if (mac_addr)
		ether_addr_copy(ndev->dev_addr, mac_addr);
	if (!is_valid_ether_addr(ndev->dev_addr)) {
		eth_hw_addr_random(ndev);
		hip05_hw_set_mac_addr(ndev);
		dev_warn(dev, "using random MAC address %pM\n",
			    ndev->dev_addr);
	}

	INIT_WORK(&priv->tx_timeout_task, hip05_tx_timeout_task);
	ndev->watchdog_timeo = 6 * HZ;
	ndev->priv_flags |= IFF_UNICAST_FLT;
	ndev->netdev_ops = &hip05_netdev_ops;
	ndev->ethtool_ops = &hip05_ethtools_ops;
	SET_NETDEV_DEV(ndev, dev);

	ret = hip05_ring_init(ndev);
	if (ret)
		goto out_phy_node;

	hip05_hw_init(ndev);

	ret = hip05_rcb_ready(ndev);
	if (ret)
		goto out_phy_node;

	netif_napi_add(ndev, &priv->napi, hip05_poll, NAPI_POLL_WEIGHT);
	ret = register_netdev(priv->netdev);
	if (ret) {
		dev_err(dev, "register_netdev failed!");
		goto out_destroy_queue;
	}

	return ret;

out_destroy_queue:
	netif_napi_del(&priv->napi);
	hip05_free_ring(ndev, dev);
out_phy_node:
	of_node_put(priv->phy_node);
out_free_netdev:
	free_netdev(ndev);

	return ret;
}

static int hip05_dev_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct hip05_priv *priv = netdev_priv(ndev);

	netif_napi_del(&priv->napi);
	unregister_netdev(ndev);

	free_irq(priv->tx_irq, ndev);
	free_irq(priv->rx_irq, ndev);

	mbi_free_irqs(&pdev->dev,  priv->tx_irq, 2);

	hip05_free_ring(ndev, priv->dev);
	of_node_put(priv->phy_node);
	cancel_work_sync(&priv->tx_timeout_task);
	free_netdev(ndev);

	return 0;
}

static const struct of_device_id hip05_of_match[] = {
	{.compatible = "hisilicon, hip05-mac",},
	{},
};

MODULE_DEVICE_TABLE(of, hip05_of_match);

static struct platform_driver hip05_dev_driver = {
	.driver = {
		.name = "hip05-mac",
		.of_match_table = hip05_of_match,
	},
	.probe = hip05_dev_probe,
	.remove = hip05_dev_remove,
};

module_platform_driver(hip05_dev_driver);

MODULE_DESCRIPTION("HISILICON HIX5HD2 Ethernet driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:hip05-gmac");
