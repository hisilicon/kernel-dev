/*
 * Copyright (c) 2015 Linaro Ltd.
 * Copyright (c) 2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "hisi_sas.h"
#include <linux/swab.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>

/* global registers need init*/
#define DLVRY_QUEUE_ENABLE		0x0
#define IOST_BASE_ADDR_LO		0x8
#define IOST_BASE_ADDR_HI		0xc
#define ITCT_BASE_ADDR_LO		0x10
#define ITCT_BASE_ADDR_HI		0x14
#define IO_BROKEN_MSG_ADDR_LO		0x18
#define IO_BROKEN_MSG_ADDR_HI		0x1c
#define PHY_CONTEXT			0x20
#define PHY_STATE			0x24
#define PHY_PORT_NUM_MA			0x28
#define PHY_CONN_RATE			0x30
#define HGC_TRANS_TASK_CNT_LIMIT	0x38
#define AXI_AHB_CLK_CFG			0x3c
#define IO_SATA_BROKEN_MSG_ADDR_LO	0x58
#define IO_SATA_BROKEN_MSG_ADDR_HI	0x5c
#define SATA_INITI_D2H_STORE_ADDR_LO	0x60
#define SATA_INITI_D2H_STORE_ADDR_HI	0x64
#define HGC_SAS_TXFAIL_RETRY_CTRL	0x84
#define HGC_GET_ITV_TIME		0x90
#define DEVICE_MSG_WORK_MODE		0x94
#define I_T_NEXUS_LOSS_TIME		0xa0
#define BUS_INACTIVE_LIMIT_TIME		0xa8
#define REJECT_TO_OPEN_LIMIT_TIME	0xac
#define CFG_AGING_TIME			0xbc
#define HGC_DFX_CFG2			0xc0
#define CFG_1US_TIMER_TRSH		0xcc
#define HGC_INVLD_DQE_INFO		0x148
#define INT_COAL_EN			0x19c
#define OQ_INT_COAL_TIME		0x1a0
#define OQ_INT_COAL_CNT			0x1a4
#define ENT_INT_COAL_TIME		0x1a8
#define ENT_INT_COAL_CNT		0x1ac
#define OQ_INT_SRC			0x1b0
#define OQ_INT_SRC_MSK			0x1b4
#define ENT_INT_SRC1			0x1b8
#define ENT_INT_SRC2			0x1bc
#define ENT_INT_SRC3			0x1c0
#define ENT_INT_SRC_MSK1		0x1c4
#define ENT_INT_SRC_MSK2		0x1c8
#define ENT_INT_SRC_MSK3		0x1cc
#define SAS_ECC_INTR_MSK		0x1ec
#define HGC_ERR_STAT_EN			0x238
#define DLVRY_Q_0_BASE_ADDR_LO		0x260
#define DLVRY_Q_0_BASE_ADDR_HI		0x264
#define DLVRY_Q_0_DEPTH			0x268
#define DLVRY_Q_0_WR_PTR		0x26c
#define DLVRY_Q_0_RD_PTR		0x270
#define COMPL_Q_0_BASE_ADDR_LO		0x4e0
#define COMPL_Q_0_BASE_ADDR_HI		0x4e4
#define COMPL_Q_0_DEPTH			0x4e8
#define COMPL_Q_0_WR_PTR		0x4ec
#define COMPL_Q_0_RD_PTR		0x4f0

/* phy registers need init */
#define PORT_BASE			(0x2000)

#define PHY_CFG				(PORT_BASE + 0x0)
#define PHY_CFG_ENA_OFF			0
#define PHY_CFG_ENA_MSK			0x1
#define PHY_CFG_DC_OPT_OFF		2
#define PHY_CFG_DC_OPT_MSK		0x4
#define PROG_PHY_LINK_RATE		(PORT_BASE + 0x8)
#define PROG_PHY_LINK_RATE_MAX_OFF	0
#define PROG_PHY_LINK_RATE_MAX_MSK	0xff
#define PROG_PHY_LINK_RATE_OOB_OFF	8
#define PROG_PHY_LINK_RATE_OOB_MSK	0xf00
#define SL_CFG				(PORT_BASE + 0x84)
#define PHY_PCN				(PORT_BASE + 0x44)
#define SL_TOUT_CFG			(PORT_BASE + 0x8c)
#define SL_CONTROL			(PORT_BASE + 0x94)
#define SL_CONTROL_NOTIFY_ENA_OFF	0
#define SL_CONTROL_NOTIFY_ENA_MSK	0x1
#define TX_ID_DWORD0			(PORT_BASE + 0x9c)
#define TX_ID_DWORD1			(PORT_BASE + 0xa0)
#define TX_ID_DWORD2			(PORT_BASE + 0xa4)
#define TX_ID_DWORD3			(PORT_BASE + 0xa8)
#define TX_ID_DWORD4			(PORT_BASE + 0xaC)
#define TX_ID_DWORD5			(PORT_BASE + 0xb0)
#define TX_ID_DWORD6			(PORT_BASE + 0xb4)
#define RX_IDAF_DWORD0			(PORT_BASE + 0xc4)
#define RX_IDAF_DWORD1			(PORT_BASE + 0xc8)
#define RX_IDAF_DWORD2			(PORT_BASE + 0xcc)
#define RX_IDAF_DWORD3			(PORT_BASE + 0xd0)
#define RX_IDAF_DWORD4			(PORT_BASE + 0xd4)
#define RX_IDAF_DWORD5			(PORT_BASE + 0xd8)
#define RX_IDAF_DWORD6			(PORT_BASE + 0xdc)
#define RXOP_CHECK_CFG_H		(PORT_BASE + 0xfc)
#define DONE_RECEIVED_TIME		(PORT_BASE + 0x11c)
#define CHL_INT0			(PORT_BASE + 0x1b4)
#define CHL_INT0_SL_RX_BCST_ACK_MSK	0x2
#define CHL_INT0_SL_RX_BCST_ACK_OFF	1
#define CHL_INT0_SL_ENA_MSK		0x4
#define CHL_INT0_SL_ENA_OFF		2
#define CHL_INT0_NOT_RDY_MSK		0x10
#define CHL_INT0_NOT_RDY_OFF		4
#define CHL_INT1			(PORT_BASE + 0x1b8)
#define CHL_INT2			(PORT_BASE + 0x1bc)
#define CHL_INT0_MSK			(PORT_BASE + 0x1c0)
#define CHL_INT1_MSK			(PORT_BASE + 0x1c4)
#define CHL_INT2_MSK			(PORT_BASE + 0x1c8)
#define PHYCTRL_NOT_RDY_MSK		(PORT_BASE + 0x2b4)
#define PHYCTRL_PHY_ENA_MSK		(PORT_BASE + 0x2bc)
#define CHL_INT2_MSK_RX_INVLD_OFF	30
#define CHL_INT2_MSK_RX_INVLD_MSK	0x8000000

enum {
	HISI_SAS_PHY_HOTPLUG_TOUT,
	HISI_SAS_PHY_PHY_UPDOWN,
	HISI_SAS_PHY_CHNL_INT,
	HISI_SAS_PHY_INT_NR
};

#define HISI_SAS_PHY_MAX_INT_NR (HISI_SAS_PHY_INT_NR * HISI_SAS_MAX_PHYS)
#define HISI_SAS_CQ_MAX_INT_NR (HISI_SAS_MAX_QUEUES)
#define HISI_SAS_FATAL_INT_NR (2)

#define HISI_SAS_MAX_INT_NR (HISI_SAS_PHY_MAX_INT_NR + HISI_SAS_CQ_MAX_INT_NR + HISI_SAS_FATAL_INT_NR)
struct hisi_sas_cmd_hdr_dw0 {
	u32 abort_flag:2;
	u32 abort_device_type:1;
	u32 rsvd0:2;
	u32 resp_report:1;
	u32 tlr_ctrl:2;
	u32 phy_id:9;
	u32 force_phy:1;
	u32 port:4;
	u32 rsvd1:5;
	u32 priority:1;
	u32 rsvd2:1;
	u32 cmd:3;
};

struct hisi_sas_cmd_hdr_dw1 {
	u32 rsvd:4;
	u32 ssp_pass_through:1;
	u32 dir:2;
	u32 reset:1;
	u32 pir_pres:1;
	u32 enable_tlr:1;
	u32 verify_dtl:1;
	u32 frame_type:5;
	u32 device_id:16;
};

struct hisi_sas_cmd_hdr_dw2 {
	u32 cmd_frame_len:9;
	u32 leave_affil_open:1;
	u32 ncq_tag:5;
	u32 max_resp_frame_len:9;
	u32 sg_mode:2;
	u32 first_burst:1;
	u32 rsvd:5;
};

#define DIR_NO_DATA 0
#define DIR_TO_INI 1
#define DIR_TO_DEVICE 2
#define DIR_RESERVED 3

#define SATA_PROTOCOL_NONDATA		0x1
#define SATA_PROTOCOL_PIO		0x2
#define SATA_PROTOCOL_DMA		0x4
#define SATA_PROTOCOL_FPDMA		0x8
#define SATA_PROTOCOL_ATAPI		0x10

static inline u32 hisi_sas_read32(struct hisi_hba *hisi_hba, u32 off)
{
	void __iomem *regs = hisi_hba->regs + off;

	return readl(regs);
}

static inline void hisi_sas_write32(struct hisi_hba *hisi_hba, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + off;

	writel(val, regs);
}

static inline void hisi_sas_phy_write32(struct hisi_hba *hisi_hba, int phy, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	writel(val, regs);
}

static inline u32 hisi_sas_phy_read32(struct hisi_hba *hisi_hba, int phy, u32 off)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	return readl(regs);
}

static void config_phy_link_param(struct hisi_hba *hisi_hba,
					int phy,
					enum sas_linkrate linkrate)
{
	u32 rate = hisi_sas_phy_read32(hisi_hba, phy, PROG_PHY_LINK_RATE);
	u32 pcn;

	rate &= ~PROG_PHY_LINK_RATE_MAX_MSK;
	switch (linkrate) {
	case SAS_LINK_RATE_1_5_GBPS:
		rate |= SAS_LINK_RATE_1_5_GBPS << PROG_PHY_LINK_RATE_MAX_OFF;
		pcn = 0x80800000;
		break;
	case SAS_LINK_RATE_3_0_GBPS:
		rate |= SAS_LINK_RATE_3_0_GBPS << PROG_PHY_LINK_RATE_MAX_OFF;
		pcn = 0x80a00001;
		break;
	case SAS_LINK_RATE_6_0_GBPS:
		rate |= SAS_LINK_RATE_6_0_GBPS << PROG_PHY_LINK_RATE_MAX_OFF;
		pcn = 0x80a80000;
		break;
	case SAS_LINK_RATE_12_0_GBPS:
		rate |= SAS_LINK_RATE_12_0_GBPS << PROG_PHY_LINK_RATE_MAX_OFF;
		pcn = 0x80aa0001;
		break;
	default:
		dev_warn(hisi_hba->dev, "%s unsupported linkrate, %d",
			 __func__, linkrate);
		return;
	}

	hisi_sas_phy_write32(hisi_hba, phy, PROG_PHY_LINK_RATE, rate);
	hisi_sas_phy_write32(hisi_hba, phy, PHY_PCN, pcn);
}

static void config_phy_opt_mode(struct hisi_hba *hisi_hba, int phy)
{
	/* j00310691 assume not optical cable for now */
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG);
	cfg &= ~PHY_CFG_DC_OPT_MSK;
	cfg |= 1 << PHY_CFG_DC_OPT_OFF;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG, cfg);
}

static void config_id_frame(struct hisi_hba *hisi_hba, int phy)
{
	struct sas_identify_frame identify_frame;
	u32 *identify_buffer;

	memset(&identify_frame, 0, sizeof(identify_frame));
	/*dev_type is [6-4]bit, frame_type is [3-0]bit
	 *according to IT code, the byte is set to 0x10 */
	/* l00293075 I found this one */
	identify_frame.dev_type = SAS_END_DEVICE;
	identify_frame.frame_type = 0;
	/*_un1 is the second byte,the byte is set to 0x1 in IT code*/
	identify_frame._un1 = 1;
	identify_frame.initiator_bits = SAS_PROTOCOL_ALL;
	identify_frame.target_bits = SAS_PROTOCOL_NONE;
	memcpy(&identify_frame._un4_11[0], hisi_hba->sas_addr, SAS_ADDR_SIZE);
	memcpy(&identify_frame.sas_addr[0], hisi_hba->sas_addr,	SAS_ADDR_SIZE);
	identify_frame.phy_id = phy;
	identify_buffer = (u32 *)(&identify_frame);

	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD0,
			__swab32(identify_buffer[0]));
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD1,
			identify_buffer[2]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD2,
			identify_buffer[1]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD3,
			identify_buffer[4]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD4,
			identify_buffer[3]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD5,
			__swab32(identify_buffer[5]));
}

static void init_id_frame(struct hisi_hba *hisi_hba)
{
	int i;

	/*ifdef _LITTLE_ENDIAN_BITFIELD,
	*sas_identify_frame the same as the structure in IT code*/
	for (i = 0; i < hisi_hba->n_phy; i++)
		config_id_frame(hisi_hba, i);
}

static void init_reg(struct hisi_hba *hisi_hba)
{
	int i;

	/* Global registers init*/
	hisi_sas_write32(hisi_hba,
		DLVRY_QUEUE_ENABLE,
		(u32)((1ULL << hisi_hba->queue_count) - 1));
	hisi_sas_write32(hisi_hba, HGC_TRANS_TASK_CNT_LIMIT, 0x11);
	hisi_sas_write32(hisi_hba, DEVICE_MSG_WORK_MODE, 0x1);
//	hisi_sas_write32(hisi_hba, MAX_BURST_BYTES, 0);
//	hisi_sas_write32(hisi_hba, SMP_TIMEOUT_TIMER, 0);
//	hisi_sas_write32(hisi_hba, MAX_CON_TIME_LIMIT_TIME, 0);
	hisi_sas_write32(hisi_hba, HGC_SAS_TXFAIL_RETRY_CTRL, 0x211ff);
	hisi_sas_write32(hisi_hba, HGC_ERR_STAT_EN, 0x401);
	hisi_sas_write32(hisi_hba, CFG_1US_TIMER_TRSH, 0x64);
	hisi_sas_write32(hisi_hba, HGC_GET_ITV_TIME, 0x1);
	hisi_sas_write32(hisi_hba, I_T_NEXUS_LOSS_TIME, 0x64);
	hisi_sas_write32(hisi_hba, BUS_INACTIVE_LIMIT_TIME, 0x2710);
	hisi_sas_write32(hisi_hba, REJECT_TO_OPEN_LIMIT_TIME, 0x1);
	hisi_sas_write32(hisi_hba, CFG_AGING_TIME, 0x7a12);
	hisi_sas_write32(hisi_hba, HGC_DFX_CFG2, 0x9c40);
	hisi_sas_write32(hisi_hba, INT_COAL_EN, 0xC);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_TIME, 0x186A0);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_CNT, 1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_TIME, 0x1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_CNT, 0x1);
	hisi_sas_write32(hisi_hba, OQ_INT_SRC, 0xffffffff);
	hisi_sas_write32(hisi_hba, OQ_INT_SRC_MSK, 0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC1, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1, 0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC2, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK2, 0);
	hisi_sas_write32(hisi_hba, SAS_ECC_INTR_MSK, 0);
	hisi_sas_write32(hisi_hba, AXI_AHB_CLK_CFG, 0x2);

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/*phy registers init set 12G - see g_astPortRegConfig */
		hisi_sas_phy_write32(hisi_hba, i, PROG_PHY_LINK_RATE, 0x00000801);
		hisi_sas_phy_write32(hisi_hba, i, SL_TOUT_CFG, 0x7d7d7d7d);
		hisi_sas_phy_write32(hisi_hba, i, DONE_RECEIVED_TIME, 0x10);
		hisi_sas_phy_write32(hisi_hba, i, RXOP_CHECK_CFG_H, 0x1000);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK, 0xffffffc0);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK, 0xfff87fff);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0xffffffd6);
		hisi_sas_phy_write32(hisi_hba, i, SL_CFG, 0x23f801fc);
		hisi_sas_phy_write32(hisi_hba, i, INT_COAL_EN, 0x0);
	}

	for (i = 0; i < hisi_hba->queue_count; i++) {
		/* Delivery queue */
		hisi_sas_write32(hisi_hba, DLVRY_Q_0_BASE_ADDR_HI + (i * 0x14),
				 DMA_ADDR_HI(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, DLVRY_Q_0_BASE_ADDR_LO + (i * 0x14),
				 DMA_ADDR_LO(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, DLVRY_Q_0_DEPTH + (i * 0x14),
				 HISI_SAS_QUEUE_SLOTS);

		/* Completion queue */
		hisi_sas_write32(hisi_hba, COMPL_Q_0_BASE_ADDR_HI + (i * 0x14),
				 DMA_ADDR_HI(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, COMPL_Q_0_BASE_ADDR_LO + (i * 0x14),
				 DMA_ADDR_LO(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, COMPL_Q_0_DEPTH + (i * 0x14),
				 HISI_SAS_QUEUE_SLOTS);
	}

	/* itct */
	hisi_sas_write32(hisi_hba, ITCT_BASE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->itct_dma));

	hisi_sas_write32(hisi_hba, ITCT_BASE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->itct_dma));

	/* iost */
	hisi_sas_write32(hisi_hba, IOST_BASE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->iost_dma));

	hisi_sas_write32(hisi_hba, IOST_BASE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->iost_dma));

	/* breakpoint */
	hisi_sas_write32(hisi_hba, IO_BROKEN_MSG_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->breakpoint_dma));

	hisi_sas_write32(hisi_hba, IO_BROKEN_MSG_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->breakpoint_dma));

	/* SATA broken msg */
	hisi_sas_write32(hisi_hba, IO_SATA_BROKEN_MSG_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->sata_breakpoint_dma));

	hisi_sas_write32(hisi_hba, IO_SATA_BROKEN_MSG_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->sata_breakpoint_dma));

	/* SATA initial fis */
	hisi_sas_write32(hisi_hba, SATA_INITI_D2H_STORE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->initial_fis_dma));

	hisi_sas_write32(hisi_hba, SATA_INITI_D2H_STORE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->initial_fis_dma));
}

static int hw_init(struct hisi_hba *hisi_hba)
{
	init_reg(hisi_hba);

	/* maybe init serdes param j00310691 */
	init_id_frame(hisi_hba);

	return 0;
}

static void enable_phy(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG);
	cfg |= PHY_CFG_ENA_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG, cfg);
}

/* see Higgs_StartPhy */
static void start_phy(struct hisi_hba *hisi_hba, int phy)
{
	config_id_frame(hisi_hba, phy);
	config_phy_link_param(hisi_hba, phy, SAS_LINK_RATE_12_0_GBPS);
	config_phy_opt_mode(hisi_hba, phy);
	enable_phy(hisi_hba, phy);
}

static void start_phys(unsigned long data)
{
	struct hisi_hba *hisi_hba = (struct hisi_hba *)data;
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x0000032a);
		start_phy(hisi_hba, i);
	}

}

static void phys_up(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x36a);
		hisi_sas_phy_read32(hisi_hba, i, CHL_INT2_MSK);
	}
}

static int start_phy_layer(struct hisi_hba *hisi_hba)
{
	struct timer_list *timer = NULL;

	timer = vmalloc(sizeof(*timer)); /* j00310691 memory leak? Is this timer even needed? */
	if (!timer)
		return -ENOMEM;

	init_timer(timer);
	timer->data = (unsigned long)hisi_hba;
	timer->expires = jiffies + msecs_to_jiffies(1000);
	timer->function = start_phys;

	add_timer(timer);

	return 0;
}

static int phys_init(struct hisi_hba *hisi_hba)
{
	phys_up(hisi_hba);
	start_phy_layer(hisi_hba);

	return 0;
}

static int hisi_sas_prep_prd_sge(struct hisi_hba *hisi_hba,
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		dev_err(hisi_hba->dev, "%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT",
			__func__, n_elem);
		return -EINVAL;
	}

	slot->sge_page = dma_pool_alloc(hisi_hba->sge_page_pool, GFP_ATOMIC,
					&slot->sge_page_dma);
	if (!slot->sge_page)
		return -ENOMEM;

	dw1->pir_pres = 0;

	for_each_sg(scatter, sg, n_elem, i) {
		struct hisi_sas_sge *entry = &slot->sge_page->sge[i];

		entry->addr_lo = DMA_ADDR_LO(sg_dma_address(sg));
		entry->addr_hi = DMA_ADDR_HI(sg_dma_address(sg));
		entry->page_ctrl_0 = entry->page_ctrl_1 = 0;
		entry->data_len = sg_dma_len(sg);
		entry->data_off = 0;
	}

	hdr->prd_table_addr_lo = DMA_ADDR_LO(slot->sge_page_dma);
	hdr->prd_table_addr_hi = DMA_ADDR_HI(slot->sge_page_dma);

	hdr->data_sg_len = n_elem;

	return 0;
}

static int prep_smp(struct hisi_hba *hisi_hba,
			struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct scatterlist *sg_req, *sg_resp;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	dma_addr_t req_dma_addr;
	unsigned int req_len, resp_len;
	int elem, rc;
	struct hisi_sas_slot *slot = tei->slot;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/*
	* DMA-map SMP request, response buffers
	*/
	/* req */
	sg_req = &task->smp_task.smp_req; /* this is the request frame - see alloc_smp_req() */
	elem = dma_map_sg(hisi_hba->dev, sg_req, 1, DMA_TO_DEVICE); /* map to dma address */
	if (!elem)
		return -ENOMEM;
	req_len = sg_dma_len(sg_req);
	req_dma_addr = sg_dma_address(sg_req);
	pr_info("%s sg_req=%p elem=%d req_len=%d\n", __func__, sg_req, elem, req_len);

	/* resp */
	sg_resp = &task->smp_task.smp_resp; /* this is the response frame - see alloc_smp_resp() */
	elem = dma_map_sg(hisi_hba->dev, sg_resp, 1, DMA_FROM_DEVICE);
	if (!elem) {
		rc = -ENOMEM;
		goto err_out;
	}
	resp_len = sg_dma_len(sg_resp);
	if ((req_len & 0x3) || (resp_len & 0x3)) {
		rc = -EINVAL;
		goto err_out;
	}

	/* create header */
	/* dw0 */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	dw0->priority = 1; /* high priority */
	dw0->cmd = 2; /* smp */

	/* dw1 */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */
	/* dw1->reset only set in stp */
	dw1->dir = DIR_NO_DATA; /* no data */
	dw1->frame_type = 1; /* request frame */
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (req_len - 4) / 4; /* do not include the crc */
	/* hdr->leave_affil_open, ->ncq_tag only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SMP_RESP_SZ/4;
	/* hdr->sg_mode, ->first_burst not applicable to smp */

	/* dw3 */
	/* hdr->iptt, ->tptt not applicable to smp */

	/* hdr->data_transfer_len not applicable to smp */

	/* hdr->first_burst_num not applicable to smp */

	/* hdr->dif_prd_table_len, ->prd_table_len not applicable to smp */

	/* hdr->double_mode, ->abort_iptt not applicable to smp */

	/* j00310691 do not use slot->command_table */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(req_dma_addr);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(req_dma_addr);

	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* hdr->prd_table_addr_lo not applicable to smp */

	/* hdr->prd_table_addr_hi not applicable to smp */

	/* hdr->dif_prd_table_addr_lo not applicable to smp */

	/* hdr->dif_prd_table_addr_hi not applicable to smp */

	return 0;

err_out:
	/* fix error conditions j00310691 */
	return rc;
}

static int prep_ssp(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei, int is_tmf,
		struct hisi_sas_tmf_task *tmf)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct sas_ssp_task *ssp_task = &task->ssp_task;
	struct scsi_cmnd *scsi_cmnd = ssp_task->cmd;
	int has_data = 0, rc;
	struct hisi_sas_slot *slot = tei->slot;
	u8 *buf_cmd, fburst = 0;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/* create header */
	/* dw0 */
	/* hdr->abort_flag set in Higgs_PrepareBaseSSP */
	/* hdr->t10_flds_pres set in Higgs_PreparePrdSge */
	dw0->resp_report = 1;
	dw0->tlr_ctrl = 0x2; /* Do not enable */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	dw0->priority = 0; /* ordinary priority */
	dw0->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	dw1->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	dw1->enable_tlr = 0;
	dw1->verify_dtl = 1;
	if (is_tmf) {
		dw1->frame_type = 2;
		dw1->dir = DIR_NO_DATA;
	} else {
		dw1->frame_type = 1;
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_DEVICE;
			break;
		case DMA_FROM_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_INI;
			break;
		default:
			dw1->dir = DIR_RESERVED;
		}
	}
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct ssp_command_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	/* hdr->leave_affil_open only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	dw2->sg_mode = 2; /* see Higgs_DQGlobalConfig */
	dw2->first_burst = 0;

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = hisi_sas_prep_prd_sge(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	hdr->data_transfer_len = scsi_bufflen(scsi_cmnd);

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode is set only for DIF todo */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table + sizeof(struct ssp_frame_hdr);
	/* fill in IU for TASK and Command Frame */
	if (task->ssp_task.enable_first_burst) {
		fburst = (1 << 7);
		pr_warn("%s fburst enabled: edit hdr?\n", __func__);
	}

	memcpy(buf_cmd, &task->ssp_task.LUN, 8);
	if (!is_tmf) {
		buf_cmd[9] = fburst | task->ssp_task.task_attr |
				(task->ssp_task.task_prio << 3);
		memcpy(buf_cmd + 12, task->ssp_task.cmd->cmnd,
				task->ssp_task.cmd->cmd_len);
	} else {
		buf_cmd[10] = tmf->tmf;
		switch (tmf->tmf) {
		case TMF_ABORT_TASK:
		case TMF_QUERY_TASK:
			buf_cmd[12] =
				(tmf->tag_of_task_to_be_managed >> 8) & 0xff;
			buf_cmd[13] =
				tmf->tag_of_task_to_be_managed & 0xff;
			break;
		default:
			break;
		}
	}

	return 0;

}

static u8 get_ata_protocol(u8 cmd, int direction)
{
	switch (cmd) {
	case ATA_CMD_FPDMA_WRITE:
	case ATA_CMD_FPDMA_READ:
	return SATA_PROTOCOL_FPDMA;

	case ATA_CMD_ID_ATA:
	case ATA_CMD_PMP_READ:
	case ATA_CMD_READ_LOG_EXT:
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_READ_EXT:
	case ATA_CMD_PMP_WRITE:
	case ATA_CMD_WRITE_LOG_EXT:
	case ATA_CMD_PIO_WRITE:
	case ATA_CMD_PIO_WRITE_EXT:
	return SATA_PROTOCOL_PIO;

	case ATA_CMD_READ:
	case ATA_CMD_READ_EXT:
	case /* write dma queued */ 0xc7: /* j00310691 fixme */
	case /* write dma queued ext */ 0x26: /* j00310691 fixme */
	case ATA_CMD_READ_LOG_DMA_EXT:
	case ATA_CMD_WRITE:
	case ATA_CMD_WRITE_EXT:
	case /* write dma queued ext */ 0xcc: /* j00310691 fixme */
	case ATA_CMD_WRITE_QUEUED:
	case ATA_CMD_WRITE_LOG_DMA_EXT:
	return SATA_PROTOCOL_DMA;

	case 0x92: /* j00310691 fixme */
	case ATA_CMD_DEV_RESET:
	case ATA_CMD_CHK_POWER:
	case ATA_CMD_FLUSH:
	case ATA_CMD_FLUSH_EXT:
	case ATA_CMD_VERIFY:
	case ATA_CMD_VERIFY_EXT:
	case ATA_CMD_SET_FEATURES:
	case ATA_CMD_STANDBY:
	case ATA_CMD_STANDBYNOW1:
	return SATA_PROTOCOL_NONDATA;
	default:
		if (direction == DMA_NONE)
			return SATA_PROTOCOL_NONDATA;
		return SATA_PROTOCOL_PIO;
	}
}

static int prep_ata(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct asd_sas_port *sas_port = dev->port;
	struct hisi_sas_slot *slot = tei->slot;
	u8 *buf_cmd;
	int has_data = 0;
	int rc = 0;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/* create header */
	/* dw0 */
	/* dw0->resp_report, ->tlr_ctrl not applicable to STP */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->priority not applicable to STP */
	if (dev->dev_type == SAS_SATA_DEV)
		dw0->cmd = 4; /* sata */
	else
		dw0->cmd = 3; /* stp */

	/* dw1 */
	/* hdr->ssp_pass_through not applicable to stp */
	switch (task->data_dir) {
	case DMA_TO_DEVICE:
		dw1->dir = DIR_TO_DEVICE;
		has_data = 1;
	case DMA_FROM_DEVICE:
		dw1->dir = DIR_TO_INI;
		has_data = 1;
	default:
		pr_warn("%s unhandled direction, task->data_dir=%d\n", __func__, task->data_dir);
		dw1->dir = DIR_RESERVED;
	}

	/* j00310691 for IT code SOFT RESET MACRO is 0, but I am unsure if this is a valid command */
	if (0 == task->ata_task.fis.command) {
		dw1->reset = 1;
	}
	/* hdr->enable_tlr, ->pir_pres not applicable to stp */
	/* dw1->verify_dtl not set in IT code for STP */
	dw1->frame_type = get_ata_protocol(task->ata_task.fis.command,
				task->data_dir);
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct hisi_sas_command_table_stp) + 3) / 4;
	dw2->leave_affil_open = 0; /* j00310691 unset in IT code */
	/* hdr->ncq_tag todo j00310691 */
	dw2->max_resp_frame_len = HISI_SAS_MAX_STP_RESP_SZ/4;
	dw2->sg_mode = 2;
	/* hdr->first_burst not applicable to stp */

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = hisi_sas_prep_prd_sge(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	hdr->data_transfer_len = task->total_xfer_len;

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode not set in Higgs code */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table;

	if (likely(!task->ata_task.device_control_reg_update))
		task->ata_task.fis.flags |= 0x80; /* C=1: update ATA cmd reg */
	/* fill in command FIS and ATAPI CDB */
	memcpy(buf_cmd, &task->ata_task.fis, sizeof(struct host_to_dev_fis));
	if (dev->sata_dev.class == ATA_DEV_ATAPI)
		memcpy(buf_cmd + 0x20,
			task->ata_task.atapi_packet, ATAPI_CDB_LEN);

	return 0;
}

static int phy_up(int phy_no, struct hisi_hba *hisi_hba)
{
	int i, res = 0;
	u32 context, port_id, link_rate;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	u32 *frame_rcvd = (u32 *)sas_phy->frame_rcvd;
	struct hisi_sas_port *port;
	struct sas_identify_frame *id = (struct sas_identify_frame *)frame_rcvd;

	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_PHY_ENA_MSK, 1);

	context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);
	if (context & 1 << phy_no)
		goto end;

	port_id = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA);
	port_id = (port_id >> (4 * phy_no)) & 0xf;
	if (port_id == 0xf) {
		pr_err("%s phy = %d, invalid portid\n", __func__, phy_no);
		res = -1;
		goto end;
	}

	port = &hisi_hba->port[port_id];
	for (i = 0; i < 6; i++) {
		u32 idaf = hisi_sas_phy_read32(hisi_hba, phy_no,
			RX_IDAF_DWORD0 + (i * 4));
		frame_rcvd[i] = __swab32(idaf);
	}

	phy->frame_rcvd_size = sizeof(struct sas_identify_frame);
	phy->phy_attached = 1;

	if (id->dev_type == SAS_END_DEVICE) {
		u32 sl_control;

		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control |= SL_CONTROL_NOTIFY_ENA_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
		mdelay(1);
		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control &= ~SL_CONTROL_NOTIFY_ENA_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
	}

	/* Get the linkrate */
	link_rate = hisi_sas_read32(hisi_hba, PHY_CONN_RATE);
	link_rate = (link_rate >> (phy_no * 4)) & 0xf;
	sas_phy->linkrate = link_rate;
	pr_info("%s phy_no=%d hisi_hba->id=%d link_rate=%d\n", __func__, phy_no, hisi_hba->id, link_rate);
	phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
				if (context & 1 << phy_no)
		phy->phy_type |= PORT_TYPE_SATA;
	else
		phy->phy_type |= PORT_TYPE_SAS;

	hisi_sas_update_phyinfo(hisi_hba, phy_no, 1, (context & phy_no << 1) ? 1 : 0);
	hisi_sas_bytes_dmaed(hisi_hba, phy_no);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, CHL_INT0_SL_ENA_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_PHY_ENA_MSK, 0);

	return res;
}

static int phy_down(int phy_no, struct hisi_hba *hisi_hba)
{
	int res = 0;
	u32 context, phy_cfg, phy_state;

	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_NOT_RDY_MSK, 1);

	phy_cfg = hisi_sas_phy_read32(hisi_hba, phy_no, PHY_CFG);

	if (!(phy_cfg & PHY_CFG_ENA_MSK)) {
		res = -1;
		goto end;
	}

	phy_state = hisi_sas_read32(hisi_hba, PHY_STATE);
	context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);

	hisi_sas_phy_down(hisi_hba,
		phy_no,
		(phy_state & 1 << phy_no) ? 1 : 0,
		(context & 1 << phy_no) ? 1 : 0);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, CHL_INT0_NOT_RDY_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_PHY_ENA_MSK, 0);

	return res;
}

static irqreturn_t int_phy_updown(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 val;
	int phy_id = 0, res = 0;

	pr_info("%s id=%d phy_no=%d", __func__, hisi_hba->id, phy_no);
	val = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_MSK);
	val |= CHL_INT2_MSK_RX_INVLD_MSK;
	hisi_sas_phy_write32(hisi_hba, phy_no, ENT_INT_SRC_MSK2, val);

	phy_id = (hisi_sas_read32(hisi_hba, HGC_INVLD_DQE_INFO) >> 9) & 0x1FF; //fixme for proper names
	while (val) {
		if (val & 1 << phy_id) {
			u32 irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0);

			if (irq_value & CHL_INT0_SL_ENA_MSK)
				/* phy up */
				phy_up(phy_no, hisi_hba);

			if (irq_value & CHL_INT0_NOT_RDY_MSK)
				/* phy down */
				phy_down(phy_no, hisi_hba);
		}

		phy_id++;
	}

	return res;
}


static irqreturn_t int_hotplug(int phy_no, void *p)
{
	return 0;
}

static irqreturn_t int_chnl_int(int phy_no, void *p)
{
	return 0;
}

/* Interrupts */
irqreturn_t cq_interrupt(int queue, void *p)
{
	struct hisi_hba *hisi_hba = p;
	struct hisi_sas_slot *slot;
	struct hisi_sas_complete_hdr *complete_queue = hisi_hba->complete_hdr[queue];
	u32 irq_value;
	u32 rd_point, wr_point;

	irq_value = hisi_sas_read32(hisi_hba, OQ_INT_SRC);

	hisi_sas_write32(hisi_hba, OQ_INT_SRC, 1 << queue);

	rd_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_RD_PTR + (0x14 * queue));
	wr_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_WR_PTR + (0x14 * queue));

	while (rd_point != wr_point) {
		struct hisi_sas_complete_hdr *complete_hdr;
		int iptt;

		complete_hdr = &complete_queue[rd_point];
		iptt = complete_hdr->iptt;

		slot = &hisi_hba->slot_info[iptt];

		hisi_sas_slot_complete(hisi_hba, slot, 0);

		if (++rd_point >= HISI_SAS_QUEUE_SLOTS)
			rd_point = 0;
	}

	/* update rd_point */
	hisi_sas_write32(hisi_hba, COMPL_Q_0_RD_PTR + (0x14 * queue), rd_point);
	return IRQ_HANDLED;
}

irqreturn_t fatal_ecc_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;

	dev_info(hisi_hba->dev, "%s core = %d, irq = %d\n",
		 __func__, hisi_hba->id, irq);

	return IRQ_HANDLED;
}

irqreturn_t fatal_axi_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;

	dev_info(hisi_hba->dev, "%s core = %d, irq = %d\n",
		 __func__, hisi_hba->id, irq);

	return IRQ_HANDLED;
}

irqreturn_t sata_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;

	dev_info(hisi_hba->dev, "%s core = %d, irq = %d\n",
		 __func__, hisi_hba->id, irq);

	return IRQ_HANDLED;
}

#define DECLARE_PHY_INT_HANDLER_GROUP(phy)\
	DECLARE_INT_HANDLER(int_hotplug, phy)\
	DECLARE_INT_HANDLER(int_phy_updown, phy)\
	DECLARE_INT_HANDLER(int_chnl_int, phy)

#define DECLARE_PHY_INT_GROUP_PTR(phy)\
	INT_HANDLER_NAME(int_hotplug, phy),\
	INT_HANDLER_NAME(int_phy_updown, phy),\
	INT_HANDLER_NAME(int_chnl_int, phy),

DECLARE_PHY_INT_HANDLER_GROUP(0)
DECLARE_PHY_INT_HANDLER_GROUP(1)
DECLARE_PHY_INT_HANDLER_GROUP(2)
DECLARE_PHY_INT_HANDLER_GROUP(3)
DECLARE_PHY_INT_HANDLER_GROUP(4)
DECLARE_PHY_INT_HANDLER_GROUP(5)
DECLARE_PHY_INT_HANDLER_GROUP(6)
DECLARE_PHY_INT_HANDLER_GROUP(7)
DECLARE_PHY_INT_HANDLER_GROUP(8)

static const char phy_int_names[HISI_SAS_PHY_INT_NR][32] = {
	"HotPlug",
	"Phy up/down",
	"Chnl Int"
};
static const char cq_int_name[32] = "cq";
static const char fatal_int_name[HISI_SAS_FATAL_INT_NR][32] = {
	"fatal ecc",
	"fatal axi"
};
static const char sata_int_name[32] = {
	"sata int"
};

static irq_handler_t phy_interrupts[HISI_SAS_MAX_PHYS][HISI_SAS_PHY_INT_NR] = {
	{DECLARE_PHY_INT_GROUP_PTR(0)},
	{DECLARE_PHY_INT_GROUP_PTR(1)},
	{DECLARE_PHY_INT_GROUP_PTR(2)},
	{DECLARE_PHY_INT_GROUP_PTR(3)},
	{DECLARE_PHY_INT_GROUP_PTR(4)},
	{DECLARE_PHY_INT_GROUP_PTR(5)},
	{DECLARE_PHY_INT_GROUP_PTR(6)},
	{DECLARE_PHY_INT_GROUP_PTR(7)},
	{DECLARE_PHY_INT_GROUP_PTR(8)},
};

DECLARE_INT_HANDLER(cq_interrupt, 0)
DECLARE_INT_HANDLER(cq_interrupt, 1)
DECLARE_INT_HANDLER(cq_interrupt, 2)
DECLARE_INT_HANDLER(cq_interrupt, 3)
DECLARE_INT_HANDLER(cq_interrupt, 4)
DECLARE_INT_HANDLER(cq_interrupt, 5)
DECLARE_INT_HANDLER(cq_interrupt, 6)
DECLARE_INT_HANDLER(cq_interrupt, 7)
DECLARE_INT_HANDLER(cq_interrupt, 8)
DECLARE_INT_HANDLER(cq_interrupt, 9)
DECLARE_INT_HANDLER(cq_interrupt, 10)
DECLARE_INT_HANDLER(cq_interrupt, 11)
DECLARE_INT_HANDLER(cq_interrupt, 12)
DECLARE_INT_HANDLER(cq_interrupt, 13)
DECLARE_INT_HANDLER(cq_interrupt, 14)
DECLARE_INT_HANDLER(cq_interrupt, 15)

static irq_handler_t cq_interrupts[HISI_SAS_MAX_QUEUES] = {
	INT_HANDLER_NAME(cq_interrupt, 0),
	INT_HANDLER_NAME(cq_interrupt, 1),
	INT_HANDLER_NAME(cq_interrupt, 2),
	INT_HANDLER_NAME(cq_interrupt, 3),
	INT_HANDLER_NAME(cq_interrupt, 4),
	INT_HANDLER_NAME(cq_interrupt, 5),
	INT_HANDLER_NAME(cq_interrupt, 6),
	INT_HANDLER_NAME(cq_interrupt, 7),
	INT_HANDLER_NAME(cq_interrupt, 8),
	INT_HANDLER_NAME(cq_interrupt, 9),
	INT_HANDLER_NAME(cq_interrupt, 10),
	INT_HANDLER_NAME(cq_interrupt, 11),
	INT_HANDLER_NAME(cq_interrupt, 12),
	INT_HANDLER_NAME(cq_interrupt, 13),
	INT_HANDLER_NAME(cq_interrupt, 14),
	INT_HANDLER_NAME(cq_interrupt, 15),
};

static irq_handler_t fatal_interrupts[HISI_SAS_MAX_QUEUES] = {
	fatal_ecc_int,
	fatal_axi_int
};

DECLARE_INT_HANDLER(sata_int, 0)
DECLARE_INT_HANDLER(sata_int, 1)
DECLARE_INT_HANDLER(sata_int, 2)
DECLARE_INT_HANDLER(sata_int, 3)
DECLARE_INT_HANDLER(sata_int, 4)
DECLARE_INT_HANDLER(sata_int, 5)
DECLARE_INT_HANDLER(sata_int, 6)
DECLARE_INT_HANDLER(sata_int, 7)
DECLARE_INT_HANDLER(sata_int, 8)

static irq_handler_t sata_interrupts[HISI_SAS_MAX_PHYS] = {
	INT_HANDLER_NAME(sata_int, 0),
	INT_HANDLER_NAME(sata_int, 1),
	INT_HANDLER_NAME(sata_int, 2),
	INT_HANDLER_NAME(sata_int, 3),
	INT_HANDLER_NAME(sata_int, 4),
	INT_HANDLER_NAME(sata_int, 5),
	INT_HANDLER_NAME(sata_int, 6),
	INT_HANDLER_NAME(sata_int, 7),
	INT_HANDLER_NAME(sata_int, 8)
};

static int interrupt_init(struct hisi_hba *hisi_hba)
{
	int i, j, irq, rc, id = hisi_hba->id;
	struct device *dev = hisi_hba->dev;
	char *int_names = hisi_hba->int_names;

	if (!hisi_hba->np)
		return -ENOENT;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		for (j = 0; j < HISI_SAS_PHY_INT_NR; j++) {
			int idx = (i * HISI_SAS_PHY_INT_NR) + j;

			irq = irq_of_parse_and_map(hisi_hba->np, idx);
			if (!irq) {
				dev_err(dev, "%s [%d] fail map phy interrupt %d\n",
					__func__, hisi_hba->id, idx);
				return -ENOENT;
			}

			(void)snprintf(&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
					HISI_SAS_INT_NAME_LENGTH,
					DRV_NAME" %s [%d %d]", phy_int_names[j],
					id, i);
			rc = devm_request_irq(dev, irq, phy_interrupts[i][j], 0,
					&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
					hisi_hba);
			if (rc) {
				dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
					__func__, hisi_hba->id, irq, rc);
				return -ENOENT;
			}
		}
	}

	for (i = 0; i < hisi_hba->queue_count; i++) {
		int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) + i;

		irq = irq_of_parse_and_map(hisi_hba->np, idx);
		if (!irq) {
			dev_err(dev, "%s [%d] could not map cq interrupt %d\n",
				__func__, hisi_hba->id, idx);
			return -ENOENT;
		}
		(void)snprintf(&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				HISI_SAS_INT_NAME_LENGTH,
				DRV_NAME" %s [%d %d]", cq_int_name, id, i);
		rc = devm_request_irq(dev, irq, cq_interrupts[i], 0,
				&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
		idx++;
	}

	for (i = 0; i < HISI_SAS_FATAL_INT_NR; i++) {
		int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) +
				hisi_hba->queue_count + i;

		irq = irq_of_parse_and_map(hisi_hba->np, idx);
		if (!irq) {
			dev_err(dev, "%s [%d] could not map fatal interrupt %d\n",
				__func__, hisi_hba->id, idx);
			return -ENOENT;
		}
		(void)snprintf(&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				HISI_SAS_INT_NAME_LENGTH,
				DRV_NAME" %s [%d]", fatal_int_name[i], id);
		rc = devm_request_irq(dev, irq, fatal_interrupts[i], 0,
				&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
		idx++;
	}

	for (i = 0; i < hisi_hba->n_phy; i++) {
		int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) +
				hisi_hba->queue_count + HISI_SAS_FATAL_INT_NR
				+ i;

		irq = irq_of_parse_and_map(hisi_hba->np, idx);
		if (!irq) {
			dev_err(dev, "%s [%d] could not map sata interrupt %d\n",
				__func__, hisi_hba->id, idx);
			return -ENOENT;
		}
		(void)snprintf(&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				HISI_SAS_INT_NAME_LENGTH,
				DRV_NAME" %s [%d]", sata_int_name, id);
		rc = devm_request_irq(dev, irq, sata_interrupts[i], 0,
				&int_names[idx * HISI_SAS_INT_NAME_LENGTH],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
		idx++;
	}

	return 0;
}

const struct hisi_sas_dispatch hisi_sas_hi1610_dispatch = {
	.hw_init = hw_init,
	.phys_init = phys_init,
	.interrupt_init = interrupt_init,
	.prep_ssp = prep_ssp,
	.prep_smp = prep_smp,
	.prep_stp = prep_ata,
};


