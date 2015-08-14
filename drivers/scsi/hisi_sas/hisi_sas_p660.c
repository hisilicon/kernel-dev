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
#define BROKEN_MSG_ADDR_LO		0x18
#define BROKEN_MSG_ADDR_HI		0x1c
#define PHY_CONTEXT			0x20
#define PHY_STATE			0x24
#define PHY_PORT_NUM_MA			0x28
#define PORT_STATE			0x2c
#define PHY_CONN_RATE			0x30
#define HGC_TRANS_TASK_CNT_LIMIT	0x38
#define AXI_AHB_CLK_CFG			0x3c
#define HGC_SAS_TXFAIL_RETRY_CTRL	0x84
#define HGC_GET_ITV_TIME		0x90
#define DEVICE_MSG_WORK_MODE		0x94
#define MAX_BURST_BYTES			0x98
#define SMP_TIMEOUT_TIMER		0x9c
#define I_T_NEXUS_LOSS_TIME		0xa0
#define MAX_CON_TIME_LIMIT_TIME	0xa4
#define BUS_INACTIVE_LIMIT_TIME	0xa8
#define REJECT_TO_OPEN_LIMIT_TIME	0xac
#define CFG_AGING_TIME			0xbc
#define HGC_DFX_CFG2			0xc0
#define FIS_LIST_BADDR_L		0xc4
#define CFG_1US_TIMER_TRSH		0xcc
#define CFG_SAS_CONFIG			0xd4
#define HGC_IOST_ECC_ADDR		0x140
#define HGC_IOST_ECC_ADDR_BAD_OFF	16
#define HGC_IOST_ECC_ADDR_BAD_MSK	0x3ff0000
#define HGC_DQ_ECC_ADDR			0x144
#define HGC_DQ_ECC_ADDR_BAD_OFF		16
#define HGC_DQ_ECC_ADDR_BAD_MSK		0xfff0000
#define HGC_INVLD_DQE_INFO		0x148
#define HGC_INVLD_DQE_INFO_DQ_OFF	0
#define HGC_INVLD_DQE_INFO_DQ_MSK	0xffff
#define HGC_INVLD_DQE_INFO_TYPE_OFF	16
#define HGC_INVLD_DQE_INFO_TYPE_MSK	0x10000
#define HGC_INVLD_DQE_INFO_FORCE_OFF	17
#define HGC_INVLD_DQE_INFO_FORCE_MSK	0x20000
#define HGC_INVLD_DQE_INFO_PHY_OFF	18
#define HGC_INVLD_DQE_INFO_PHY_MSK	0x40000
#define HGC_INVLD_DQE_INFO_ABORT_OFF	19
#define HGC_INVLD_DQE_INFO_ABORT_MSK	0x80000
#define HGC_INVLD_DQE_INFO_IPTT_OF_OFF	20
#define HGC_INVLD_DQE_INFO_IPTT_OF_MSK	0x100000
#define HGC_INVLD_DQE_INFO_SSP_ERR_OFF	21
#define HGC_INVLD_DQE_INFO_SSP_ERR_MSK	0x200000
#define HGC_INVLD_DQE_INFO_OFL_OFF	22
#define HGC_INVLD_DQE_INFO_OFL_MSK	0x400000
#define HGC_ITCT_ECC_ADDR		0x150
#define HGC_ITCT_ECC_ADDR_1B_OFF	0
#define HGC_ITCT_ECC_ADDR_1B_MSK	0x3ff
#define HGC_ITCT_ECC_ADDR_BAD_OFF	16
#define HGC_ITCT_ECC_ADDR_BAD_MSK	0x3ff0000
#define HGC_AXI_FIFO_ERR_INFO		0x154
#define INT_COAL_EN			0x1bc
#define OQ_INT_COAL_TIME		0x1c0
#define OQ_INT_COAL_CNT			0x1c4
#define ENT_INT_COAL_TIME		0x1c8
#define ENT_INT_COAL_CNT		0x1cc
#define OQ_INT_SRC			0x1d0
#define OQ_INT_SRC_MSK			0x1d4
#define ENT_INT_SRC1			0x1d8
#define ENT_INT_SRC2			0x1dc
#define ENT_INT_SRC2_AXI_WRONG_INT_OFF	28
#define ENT_INT_SRC2_AXI_WRONG_INT_MSK	0x10000000
#define ENT_INT_SRC2_AXI_OVERLF_INT_OFF	29
#define ENT_INT_SRC2_AXI_OVERLF_INT_MSK	0x20000000
#define ENT_INT_SRC_MSK1		0x1e0
#define ENT_INT_SRC_MSK2		0x1e4
#define SAS_ECC_INTR			0x1e8
#define SAS_ECC_INTR_DQ_ECC1B_OFF	0
#define SAS_ECC_INTR_DQ_ECC1B_MSK	0x1
#define SAS_ECC_INTR_DQ_ECCBAD_OFF	1
#define SAS_ECC_INTR_DQ_ECCBAD_MSK	0x2
#define SAS_ECC_INTR_IOST_ECC1B_OFF	2
#define SAS_ECC_INTR_IOST_ECC1B_MSK	0x4
#define SAS_ECC_INTR_IOST_ECCBAD_OFF	3
#define SAS_ECC_INTR_IOST_ECCBAD_MSK	0x8
#define SAS_ECC_INTR_ITCT_ECC1B_OFF	4
#define SAS_ECC_INTR_ITCT_ECC1B_MSK	0x10
#define SAS_ECC_INTR_ITCT_ECCBAD_OFF	5
#define SAS_ECC_INTR_ITCT_ECCBAD_MSK	0x20
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
#define HGC_ECC_ERR			0x7d0

/* phy registers need init */
#define PORT_BASE			(0x800)

#define PHY_CFG				(PORT_BASE + 0x0)
#define PHY_CFG_ENA_OFF			0
#define PHY_CFG_ENA_MSK			0x1
#define PHY_CFG_SATA_OFF		1
#define PHY_CFG_SATA_MSK		0x2
#define PHY_CFG_DC_OPT_OFF		2
#define PHY_CFG_DC_OPT_MSK		0x4
#define HARD_PHY_LINK_RATE		(PORT_BASE + 0x4)
#define HARD_PHY_LINK_RATE_MAX_OFF	0
#define HARD_PHY_LINK_RATE_MAX_MSK	0xf
#define HARD_PHY_LINK_RATE_MIN_OFF	4
#define HARD_PHY_LINK_RATE_MIN_MSK	0xf0
#define HARD_PHY_LINK_RATE_NEG_OFF	8
#define HARD_PHY_LINK_RATE_NEG_MSK	0xf00
#define PROG_PHY_LINK_RATE		(PORT_BASE + 0xc)
#define PROG_PHY_LINK_RATE_MAX_OFF	0
#define PROG_PHY_LINK_RATE_MAX_MSK	0xf
#define PROG_PHY_LINK_RATE_MIN_OFF	4
#define PROG_PHY_LINK_RATE_MIN_MSK	0xf0
#define PROG_PHY_LINK_RATE_OOB_OFF	8
#define PROG_PHY_LINK_RATE_OOB_MSK	0xf00
#define PHY_CTRL			(PORT_BASE + 0x14)
#define PHY_CTRL_RESET_OFF		0
#define PHY_CTRL_RESET_MSK		0x1
#define PHY_RATE_NEGO			(PORT_BASE + 0x30)
#define PHY_PCN				(PORT_BASE + 0x44)
#define SL_TOUT_CFG			(PORT_BASE + 0x8c)
#define SL_CONTROL			(PORT_BASE + 0x94)
#define SL_CONTROL_NOTIFY_EN_OFF	0
#define SL_CONTROL_NOTIFY_EN_MSK	0x1
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
#define DONE_RECEIVED_TIME		(PORT_BASE + 0x12c)
#define CON_CFG_DRIVER			(PORT_BASE + 0x130)
#define PHY_CONFIG2			(PORT_BASE + 0x1a8)
#define PHY_CONFIG2_RXCLTEPRES_OFF	0
#define PHY_CONFIG2_RXCLTEPRES_MSK	0xFF
#define CFG_TX_TRAIN_COMP_OFF		24
#define CFG_TX_TRAIN_COMP_MSK		0x1
#define CHL_INT_COAL_EN			(PORT_BASE + 0x1d0)
#define DMA_TX_STATUS			(PORT_BASE + 0x2d0)
#define DMA_TX_STATUS_BUSY_OFF		0
#define DMA_TX_STATUS_BUSY_MSK		0x1
#define DMA_RX_STATUS			(PORT_BASE + 0x2e8)
#define DMA_RX_STATUS_BUSY_OFF		0
#define DMA_RX_STATUS_BUSY_MSK		0x1

/* phy intr registers */
#define CHL_INT0			(PORT_BASE + 0x1b0)
#define CHL_INT0_PHYCTRL_NOTRDY_OFF 	0
#define CHL_INT0_PHYCTRL_NOTRDY_MSK 	0x1
#define CHL_INT0_SN_FAIL_NGR_OFF	2
#define CHL_INT0_SN_FAIL_NGR_MSK	0x4
#define CHL_INT0_DWS_LOST_OFF		4
#define CHL_INT0_DWS_LOST_MSK		0x10
#define CHL_INT0_SL_IDAF_FAIL_OFF	10
#define CHL_INT0_SL_IDAF_FAIL_MSK	0x400
#define CHL_INT0_ID_TIMEOUT_OFF	11
#define CHL_INT0_ID_TIMEOUT_MSK	0x800
#define CHL_INT0_SL_OPAF_FAIL_OFF	12
#define CHL_INT0_SL_OPAF_FAIL_MSK	0x1000
#define CHL_INT0_SL_PS_FAIL_OFF		21
#define CHL_INT0_SL_PS_FAIL_MSK		0x200000
#define CHL_INT1			(PORT_BASE + 0x1b4)
#define CHL_INT2			(PORT_BASE + 0x1b8)
#define CHL_INT2_CTRL_PHY_RDY_OFF	0
#define CHL_INT2_CTRL_PHY_RDY_MSK	0x1
#define CHL_INT2_PHY_HP_TOUT_OFF	1
#define CHL_INT2_PHY_HP_TOUT_MSK	0x2
#define CHL_INT2_SL_RX_BC_ACK_OFF	2
#define CHL_INT2_SL_RX_BC_ACK_MSK	0x4
#define CHL_INT2_SL_RX_HARDRST_OFF	4
#define CHL_INT2_SL_RX_HARDRST_MSK	0x10
#define CHL_INT2_PHY_STATUS_CHG_OFF	5
#define CHL_INT2_PHY_STATUS_CHG_MSK	0x20
#define CHL_INT2_SL_PHY_ENA_OFF	6
#define CHL_INT2_SL_PHY_ENA_MSK	0x40
#define CHL_INT0_MSK			(PORT_BASE + 0x1bc)
#define CHL_INT1_MSK			(PORT_BASE + 0x1c0)
#define CHL_INT2_MSK			(PORT_BASE + 0x1c4)

/* nego query windown */
#define NEGO_QUERY_WINDOW_12G		40

#define AXI_CFG				(0x5100)

enum {
	HISI_SAS_PHY_CTRL_RDY = 0,
	HISI_SAS_PHY_BCAST_ACK,
	HISI_SAS_PHY_STATUS_CHG,
	HISI_SAS_PHY_SL_PHY_ENABLED,
	HISI_SAS_PHY_INT_ABNORMAL,
	HISI_SAS_PHY_INT_NR
};

enum {
	DMA_TX_ERR_BASE = 0x0,
	DMA_RX_ERR_BASE = 0x100,
	TRANS_TX_FAIL_BASE = 0x200,
	TRANS_RX_FAIL_BASE = 0x300,

	/* dma tx */
	DMA_TX_DIF_CRC_ERR = DMA_TX_ERR_BASE,
	DMA_TX_DIF_APP_ERR,
	DMA_TX_DIF_RPP_ERR,
	DMA_TX_AXI_BUS_ERR,
	DMA_TX_DATA_SGL_OVERFLOW_ERR,
	DMA_TX_DIF_SGL_OVERFLOW_ERR,
	DMA_TX_UNEXP_XFER_RDY_ERR,
	DMA_TX_XFER_RDY_OFFSET_ERR,
	DMA_TX_DATA_UNDERFLOW_ERR,
	DMA_TX_XFER_RDY_LENGTH_OVERFLOW_ERR,

	/* dma rx */
	DMA_RX_BUFFER_ECC_ERR = DMA_RX_ERR_BASE,
	DMA_RX_DIF_CRC_ERR,
	DMA_RX_DIF_APP_ERR,
	DMA_RX_DIF_RPP_ERR,
	DMA_RX_RESP_BUFFER_OVERFLOW_ERR,
	DMA_RX_AXI_BUS_ERR,
	DMA_RX_DATA_SGL_OVERFLOW_ERR,
	DMA_RX_DIF_SGL_OVERFLOW_ERR,
	DMA_RX_DATA_OFFSET_ERR,
	DMA_RX_UNEXP_RX_DATA_ERR,
	DMA_RX_DATA_OVERFLOW_ERR,
	DMA_RX_DATA_UNDERFLOW_ERR,
	DMA_RX_UNEXP_RETRANS_RESP_ERR,

	/* trans tx */
	TRANS_TX_RSVD0_ERR = TRANS_TX_FAIL_BASE,
	TRANS_TX_PHY_NOT_ENABLE_ERR,
	TRANS_TX_OPEN_REJCT_WRONG_DEST_ERR,
	TRANS_TX_OPEN_REJCT_ZONE_VIOLATION_ERR,
	TRANS_TX_OPEN_REJCT_BY_OTHER_ERR,
	TRANS_TX_RSVD1_ERR,
	TRANS_TX_OPEN_REJCT_AIP_TIMEOUT_ERR,
	TRANS_TX_OPEN_REJCT_STP_BUSY_ERR,
	TRANS_TX_OPEN_REJCT_PROTOCOL_NOT_SUPPORT_ERR,
	TRANS_TX_OPEN_REJCT_RATE_NOT_SUPPORT_ERR,
	TRANS_TX_OPEN_REJCT_BAD_DEST_ERR,
	TRANS_TX_OPEN_BREAK_RECEIVE_ERR,
	TRANS_TX_LOW_PHY_POWER_ERR,
	TRANS_TX_OPEN_REJCT_PATHWAY_BLOCKED_ERR,
	TRANS_TX_OPEN_TIMEOUT_ERR,
	TRANS_TX_OPEN_REJCT_NO_DEST_ERR,
	TRANS_TX_OPEN_RETRY_ERR,
	TRANS_TX_RSVD2_ERR,
	TRANS_TX_BREAK_TIMEOUT_ERR,
	TRANS_TX_BREAK_REQUEST_ERR,
	TRANS_TX_BREAK_RECEIVE_ERR,
	TRANS_TX_CLOSE_TIMEOUT_ERR,
	TRANS_TX_CLOSE_NORMAL_ERR,
	TRANS_TX_CLOSE_PHYRESET_ERR,
	TRANS_TX_WITH_CLOSE_DWS_TIMEOUT_ERR,
	TRANS_TX_WITH_CLOSE_COMINIT_ERR,
	TRANS_TX_NAK_RECEIVE_ERR,
	TRANS_TX_ACK_NAK_TIMEOUT_ERR,
	TRANS_TX_CREDIT_TIMEOUT_ERR,
	TRANS_TX_IPTT_CONFLICT_ERR,
	TRANS_TX_TXFRM_TYPE_ERR,
	TRANS_TX_TXSMP_LENGTH_ERR,

	/* trans rx */
	TRANS_RX_FRAME_CRC_ERR = TRANS_RX_FAIL_BASE,
	TRANS_RX_FRAME_DONE_ERR,
	TRANS_RX_FRAME_ERRPRM_ERR,
	TRANS_RX_FRAME_NO_CREDIT_ERR,
	TRANS_RX_RSVD0_ERR,
	TRANS_RX_FRAME_OVERRUN_ERR,
	TRANS_RX_FRAME_NO_EOF_ERR,
	TRANS_RX_LINK_BUF_OVERRUN_ERR,
	TRANS_RX_BREAK_TIMEOUT_ERR,
	TRANS_RX_BREAK_REQUEST_ERR,
	TRANS_RX_BREAK_RECEIVE_ERR,
	TRANS_RX_CLOSE_TIMEOUT_ERR,
	TRANS_RX_CLOSE_NORMAL_ERR,
	TRANS_RX_CLOSE_PHYRESET_ERR,
	TRANS_RX_WITH_CLOSE_DWS_TIMEOUT_ERR,
	TRANS_RX_WITH_CLOSE_COMINIT_ERR,
	TRANS_RX_DATA_LENGTH0_ERR,
	TRANS_RX_BAD_HASH_ERR,
	TRANS_RX_XRDY_ZERO_ERR,
	TRANS_RX_SSP_FRAME_LEN_ERR,
	TRANS_RX_TRANS_RX_RSVD1_ERR,
	TRANS_RX_NO_BALANCE_ERR,
	TRANS_RX_TRANS_RX_RSVD2_ERR,
	TRANS_RX_TRANS_RX_RSVD3_ERR,
	TRANS_RX_BAD_FRAME_TYPE_ERR,
	TRANS_RX_SMP_FRAME_LEN_ERR,
	TRANS_RX_SMP_RESP_TIMEOUT_ERR
};

#define HISI_SAS_PHY_MAX_INT_NR (HISI_SAS_PHY_INT_NR * HISI_SAS_MAX_PHYS)
#define HISI_SAS_CQ_MAX_INT_NR (HISI_SAS_MAX_QUEUES)
#define HISI_SAS_FATAL_INT_NR (2)

#define HISI_SAS_MAX_INT_NR (HISI_SAS_PHY_MAX_INT_NR + HISI_SAS_CQ_MAX_INT_NR + HISI_SAS_FATAL_INT_NR)

struct hisi_sas_cmd_hdr_dw0 {
	u32 abort_flag:2;
	u32 rsvd0:2;
	u32 t10_flds_pres:1;
	u32 resp_report:1;
	u32 tlr_ctrl:2;
	u32 phy_id:8;
	u32 force_phy:1;
	u32 port:3;
	u32 sata_reg_set:7;
	u32 priority:1;
	u32 mode:1;
	u32 cmd:3;
};

struct hisi_sas_cmd_hdr_dw1 {
	u32 port_multiplier:4;
	u32 bist_activate:1;
	u32 atapi:1;
	u32 first_part_dma:1;
	u32 reset:1;
	u32 pir_pres:1;
	u32 enable_tlr:1;
	u32 verify_dtl:1;
	u32 rsvd1:1;
	u32 ssp_pass_through:1;
	u32 ssp_frame_type:3;
	u32 device_id:16;
};

struct hisi_sas_cmd_hdr_dw2 {
	u32 cmd_frame_len:9;
	u32 leave_affil_open:1;
	u32 rsvd2:5;
	u32 max_resp_frame_len:9;
	u32 sg_mode:1;
	u32 first_burst:1;
	u32 rsvd3:6;
};

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
	case SAS_LINK_RATE_12_0_GBPS:
		rate |= SAS_LINK_RATE_12_0_GBPS << PROG_PHY_LINK_RATE_MAX_OFF;
		pcn = 0x80aa0001;
		break;

	default:
		dev_warn(hisi_hba->dev, "%s unsupported linkrate, %d",
			 __func__, linkrate);
		return;
	}

	rate &= ~PROG_PHY_LINK_RATE_OOB_MSK;
	rate |= SAS_LINK_RATE_1_5_GBPS << PROG_PHY_LINK_RATE_OOB_OFF;
	rate &= ~PROG_PHY_LINK_RATE_MIN_MSK;
	rate |= SAS_LINK_RATE_1_5_GBPS << PROG_PHY_LINK_RATE_MIN_OFF;
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

static void config_tx_tfe_autoneg(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CONFIG2);
	cfg &= ~PHY_CONFIG2_RXCLTEPRES_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CONFIG2, cfg);
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

static int reset_hw(struct hisi_hba *hisi_hba)
{
	int i;
	unsigned long end_time;
	int reg_value;
	void __iomem *sub_ctrl_base;
	u32 sub_ctrl_range;
	u64 reset_reg_addr;
	u64 dereset_reg_addr;
	u32 reset_value;
	u32 dereset_value;
	u64 reset_status_reg_addr;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 phy_ctrl = hisi_sas_phy_read32(hisi_hba, i, PHY_CTRL);

		phy_ctrl |= PHY_CTRL_RESET_MSK;
		hisi_sas_phy_write32(hisi_hba, i, PHY_CTRL, phy_ctrl);
	}
	udelay(50);

	/* Ensure DMA tx & rx idle */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 dma_tx_status, dma_rx_status;

		end_time = jiffies + msecs_to_jiffies(1000);

		while (1) {
			dma_tx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_TX_STATUS);
			dma_rx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_RX_STATUS);

			if (!(dma_tx_status & DMA_TX_STATUS_BUSY_MSK) &&
				!(dma_rx_status & DMA_RX_STATUS_BUSY_MSK))
				break;

			msleep(20);
			if (time_after(jiffies, end_time))
				return -EIO;
		}
	}

	/* Ensure axi bus idle */
	end_time = jiffies + msecs_to_jiffies(1000);
	while (1) {
		u32 axi_status =
			hisi_sas_read32(hisi_hba, AXI_CFG);

		if (axi_status == 0)
			break;

		msleep(20);
		if (time_after(jiffies, end_time))
			return -EIO;
	}

/* do you mean i put them here ?*/
#define DSAF_SUBCTL_BASE			(0xc0000000ull)
#define DSAF_SUBCTL_RANGE			(0xffff)
#define DSAF_SUB_CTRL_RESET_OFFSET		(0xa60)
#define DSAF_SUB_CTRL_DERESET_OFFSET		(0xa64)
#define DSAF_SUB_CTRL_RESET_STATUS_OFFSET	(0x5a30)
#define DSAF_SUB_CTRL_RESET_VALUE		(0x7ffff)
#define DSAF_SUB_CTRL_DERESET_VALUE		(0x7ffff)

#define PCIE_SUBCTL_BASE			(0xb0000000ull)
#define PCIE_SUBCTL_RANGE			(0xffff)
#define PCIE_SUB_CTRL_RESET_OFFSET		(0xa18)
#define PCIE_SUB_CTRL_DERESET_OFFSET		(0xa1c)
#define PCIE_SUB_CTRL_RESET_STATUS_OFFSET	(0x5a0c)
#define PCIE_SUB_CTRL_RESET_VALUE		(0x7ffff)
#define PCIE_SUB_CTRL_DERESET_VALUE		(0x7ffff)

/* reg & mask used for bus */
#define RESET_STATUS_MSK		0x7ffff
#define RESET_STATUS_RESET		0x7ffff
#define RESET_STATUS_DERESET		0x0

	if (0 == hisi_hba->id) {
		sub_ctrl_base = (void __iomem *)DSAF_SUBCTL_BASE;
		sub_ctrl_range = DSAF_SUBCTL_RANGE;
		reset_reg_addr = DSAF_SUB_CTRL_RESET_OFFSET;
		dereset_reg_addr = DSAF_SUB_CTRL_DERESET_OFFSET;
		reset_status_reg_addr = DSAF_SUB_CTRL_RESET_STATUS_OFFSET;
		reset_value = DSAF_SUB_CTRL_RESET_VALUE;
		dereset_value = DSAF_SUB_CTRL_DERESET_VALUE;
	} else {
		sub_ctrl_base = (void __iomem *)PCIE_SUBCTL_BASE;
		sub_ctrl_range = PCIE_SUBCTL_RANGE;
		reset_reg_addr = PCIE_SUB_CTRL_RESET_OFFSET;
		dereset_reg_addr = PCIE_SUB_CTRL_DERESET_OFFSET;
		reset_status_reg_addr = PCIE_SUB_CTRL_RESET_STATUS_OFFSET;
		reset_value = PCIE_SUB_CTRL_RESET_VALUE;
		dereset_value = PCIE_SUB_CTRL_DERESET_VALUE;
	}

	/* reset */
	sub_ctrl_base = (void __iomem *)ioremap(
			(unsigned long)sub_ctrl_base,
			sub_ctrl_range);
	writel(reset_value, sub_ctrl_base + reset_reg_addr);
	mdelay(1);
	reg_value = readl(sub_ctrl_base + reset_status_reg_addr);
	if (RESET_STATUS_RESET != (reg_value & RESET_STATUS_MSK)) {
		pr_err("%s card:%d sas reset failed", __func__, hisi_hba->id);
		return -1;
	}

	/* dereset */
	writel(dereset_value, sub_ctrl_base + dereset_reg_addr);
	mdelay(1);
	reg_value = readl(sub_ctrl_base + reset_status_reg_addr);
	if (RESET_STATUS_DERESET != (reg_value & RESET_STATUS_MSK)) {
		pr_err("%s card:%d sas dereset failed",
			__func__,
			hisi_hba->id);
		return -1; /* sorry i don't konw about the right errcode.. */
	}

	/* io unmap */
	iounmap(sub_ctrl_base);
	return 0;
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
	hisi_sas_write32(hisi_hba, FIS_LIST_BADDR_L, 0x2);
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
	hisi_sas_write32(hisi_hba, CFG_SAS_CONFIG, 0x22000000);

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/* phy registers init set 12G - see g_astPortRegConfig */
		hisi_sas_phy_write32(hisi_hba, i, PROG_PHY_LINK_RATE, 0x0000088a);
		hisi_sas_phy_write32(hisi_hba, i, PHY_CONFIG2, 0x80c7c084);
		hisi_sas_phy_write32(hisi_hba, i, PHY_RATE_NEGO, 0x415ee00);
		hisi_sas_phy_write32(hisi_hba, i, PHY_PCN, 0x80aa0001);

		hisi_sas_phy_write32(hisi_hba, i, SL_TOUT_CFG, 0x7d7d7d7d);
		hisi_sas_phy_write32(hisi_hba, i, DONE_RECEIVED_TIME, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, RXOP_CHECK_CFG_H, 0x1000);
		hisi_sas_phy_write32(hisi_hba, i, DONE_RECEIVED_TIME, 0);
		hisi_sas_phy_write32(hisi_hba, i, CON_CFG_DRIVER, 0x13f0a);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT_COAL_EN, 3);
		hisi_sas_phy_write32(hisi_hba, i, DONE_RECEIVED_TIME, 8);
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
	hisi_sas_write32(hisi_hba, BROKEN_MSG_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->breakpoint_dma));

	hisi_sas_write32(hisi_hba, BROKEN_MSG_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->breakpoint_dma));
}

static int hw_init(struct hisi_hba *hisi_hba)
{
	int rc;

	rc = reset_hw(hisi_hba);
	if (rc) {
		dev_err(hisi_hba->dev, "hisi_sas_reset_hw failed, rc=%d", rc);
		return rc;
	}

	msleep(100);
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

static void start_phy(struct hisi_hba *hisi_hba, int phy)
{
	config_id_frame(hisi_hba, phy);
	config_phy_link_param(hisi_hba, phy, SAS_LINK_RATE_12_0_GBPS);
	config_phy_opt_mode(hisi_hba, phy);
	config_tx_tfe_autoneg(hisi_hba, phy);
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

static int get_free_slot(struct hisi_hba *hisi_hba, int *q, int *s)
{
	u32 r, w;
	int queue = smp_processor_id() % hisi_hba->queue_count;

	while (1) {
		w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14));
		r = hisi_sas_read32(hisi_hba, DLVRY_Q_0_RD_PTR + (queue * 0x14));

		if (r == w+1 % HISI_SAS_QUEUE_SLOTS) {
			dev_warn(hisi_hba->dev, "%s full queue=%d r=%d w=%d\n",
				 __func__, queue, r, w);
			queue = (queue + 1) % hisi_hba->queue_count;
			continue;
		}
		break;
	}

	*q = queue;
	*s = w;

	return 0;
}

void start_delivery(struct hisi_hba *hisi_hba)
{
	int queue = hisi_hba->slot_prep->queue;
	u32 w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14));

	hisi_sas_write32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14), ++w % HISI_SAS_QUEUE_SLOTS);
}

static int is_phy_ready(struct hisi_hba *hisi_hba, int phy_no)
{
	u32 phy_state, port_state, phy_port_dis_state;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct hisi_sas_port *port = phy->port;

	/* j00310691 fimxe (check on phy rdy register) */
	port_state = hisi_sas_read32(hisi_hba, PORT_STATE);
	phy_port_dis_state = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA);

	phy_state = hisi_sas_read32(hisi_hba, PHY_STATE);
	if (phy_state & (1 << phy_no)) {
		if (!port)
			phy->phy_attached = 1;
		return 1;
	}

	/* phy is not ready, so update port */
	if (port) {
		u32 wide_port_phymap = (hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA) >> (phy_no * 4)) & 0xf;

		if (phy->phy_type & PORT_TYPE_SAS) {
			if (wide_port_phymap == 0xf)
				port->port_attached = 0;
		} else if (phy->phy_type & PORT_TYPE_SATA) {
			port->port_attached = 0; /* j00310691 we cannot have a SATA wideport */
		}
		phy->port = NULL;
		phy->phy_attached = 0;
		phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
	}

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
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
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

	dw0->t10_flds_pres = 0;
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
	dw0->abort_flag = 0; /* not sure */
	/* hdr->t10_flds_pres not set in Higgs_PrepareSMP */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	dw0->priority = 1; /* high priority */
	dw0->mode = 1; /* ini mode */
	dw0->cmd = 2; /* smp */

	/* dw1 */
	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */

	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (req_len - 4) / 4; /* do not include the crc */
	/* hdr->leave_affil_open only applicable to stp */
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
	dw0->mode = 1; /* ini mode */
	dw0->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	dw1->enable_tlr = 0;
	dw1->verify_dtl = 1;
	dw1->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	if (is_tmf) {
		dw1->ssp_frame_type = 3;
	} else {
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			dw1->ssp_frame_type = 2;
			has_data = 1;
			break;
		case DMA_FROM_DEVICE:
			dw1->ssp_frame_type = 1;
			has_data = 1;
			break;
		default:
			dw1->ssp_frame_type = 0;
		}
	}

	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct ssp_command_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	/* hdr->leave_affil_open only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	dw2->sg_mode = 0; /* see Higgs_DQGlobalConfig */
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

void config_serdes_12G_timer_handler(unsigned long arg)
{
	struct hisi_sas_phy *phy = (struct hisi_sas_phy *)arg;
	struct hisi_hba *hisi_hba = phy->hisi_hba;
	int phy_id = phy->phy_id;

	u32 val = hisi_sas_phy_read32(hisi_hba, phy_id, PHY_CONFIG2);
	val |= (CFG_TX_TRAIN_COMP_MSK << CFG_TX_TRAIN_COMP_OFF);
	hisi_sas_phy_write32(hisi_hba, phy_id, PHY_CONFIG2, val);
}

static int config_serdes_12G(struct hisi_hba *hisi_hba, int phy_id)
{
	u32 link_rate = 0, val;
	unsigned long end_time;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_id];
	int loop_count = 0;

	phy->phy_id = phy_id;
	end_time = jiffies + msecs_to_jiffies(NEGO_QUERY_WINDOW_12G);

	while(!time_after(jiffies, end_time)) {
		/*check for phy down interrupt*/
		val = hisi_sas_phy_read32(hisi_hba, phy_id, CHL_INT0);
		if (val & CHL_INT0_PHYCTRL_NOTRDY_MSK) {
			dev_dbg(hisi_hba->dev, "%s phy come while ctrl rdy for phy %d\n",
				__func__, phy_id);
			hisi_sas_phy_write32(hisi_hba, phy_id, CHL_INT0,
					CHL_INT0_PHYCTRL_NOTRDY_MSK);
			return 0;
		}

		val = hisi_sas_phy_read32(hisi_hba, phy_id, HARD_PHY_LINK_RATE);
		link_rate = (val & HARD_PHY_LINK_RATE_NEG_MSK) >>
				HARD_PHY_LINK_RATE_NEG_OFF;

		if (link_rate == SAS_LINK_RATE_12_0_GBPS) {
			dev_dbg(hisi_hba->dev, "%s a link_rate = %d loop count = %d\n",
				__func__, link_rate, loop_count);
			mdelay(20);
			break;
		}
		loop_count++;
		udelay(100);
	}

	if (link_rate == SAS_LINK_RATE_12_0_GBPS) {
		struct timer_list *timer = &phy->serdes_timer;
		if (!timer_pending(timer)) {
			init_timer(timer);
			timer->data = (unsigned long)phy;
			timer->expires = jiffies + msecs_to_jiffies(300);
			timer->function = config_serdes_12G_timer_handler;
			add_timer(timer);
		} else {
			mod_timer(timer, jiffies + msecs_to_jiffies(300));
		}
	}

	return 0;
}

static int hisi_sas_slot_err(struct hisi_hba *hisi_hba,
		struct sas_task *task, struct hisi_sas_slot *slot)
{
	int stat = SAM_STAT_CHECK_CONDITION;
	struct hisi_sas_err_record *err_record = slot->status_buffer;

	switch (task->task_proto) {
	case SAS_PROTOCOL_SSP:
	{
		int error = -1;

		/* dma tx err */
		if (err_record->dma_tx_err_type)
			error = ffs(err_record->dma_tx_err_type) + DMA_TX_ERR_BASE;
		/* dma rx err */
		if (err_record->dma_rx_err_type)
			error = ffs(err_record->dma_rx_err_type) + DMA_RX_ERR_BASE;
		/* trans tx err */
		if (err_record->trans_tx_fail_type)
			error = ffs(err_record->trans_tx_fail_type) + TRANS_TX_FAIL_BASE;
		/* trans rx err */
		if (err_record->trans_rx_fail_type)
			error = ffs(err_record->trans_rx_fail_type) + TRANS_RX_FAIL_BASE;

		switch (error) {
		case DMA_TX_DATA_UNDERFLOW_ERR:
		case DMA_RX_DATA_UNDERFLOW_ERR:
		{
			stat = SAS_DATA_UNDERRUN;
			break;
		}
		case DMA_TX_DATA_SGL_OVERFLOW_ERR:
		case DMA_TX_DIF_SGL_OVERFLOW_ERR:
		case DMA_TX_XFER_RDY_LENGTH_OVERFLOW_ERR:
		case DMA_RX_DATA_OVERFLOW_ERR:
		case TRANS_RX_FRAME_OVERRUN_ERR:
		case TRANS_RX_LINK_BUF_OVERRUN_ERR:
		{
			stat = SAS_DATA_OVERRUN;
			break;
		}
		case TRANS_TX_PHY_NOT_ENABLE_ERR:
		{
			stat = SAS_PHY_DOWN;
			break;
		}
		case TRANS_TX_OPEN_REJCT_WRONG_DEST_ERR:
		case TRANS_TX_OPEN_REJCT_ZONE_VIOLATION_ERR:
		case TRANS_TX_OPEN_REJCT_BY_OTHER_ERR:
		case TRANS_TX_OPEN_REJCT_AIP_TIMEOUT_ERR:
		case TRANS_TX_OPEN_REJCT_STP_BUSY_ERR:
		case TRANS_TX_OPEN_REJCT_PROTOCOL_NOT_SUPPORT_ERR:
		case TRANS_TX_OPEN_REJCT_RATE_NOT_SUPPORT_ERR:
		case TRANS_TX_OPEN_REJCT_BAD_DEST_ERR:
		case TRANS_TX_OPEN_BREAK_RECEIVE_ERR:
		case TRANS_TX_OPEN_REJCT_PATHWAY_BLOCKED_ERR:
		case TRANS_TX_OPEN_REJCT_NO_DEST_ERR:
		case TRANS_TX_OPEN_RETRY_ERR:
		{
			stat = SAS_OPEN_REJECT;
			break;
		}
		case TRANS_TX_OPEN_TIMEOUT_ERR:
		{
			stat = SAS_OPEN_TO;
			break;
		}
		case TRANS_TX_NAK_RECEIVE_ERR:
		case TRANS_TX_ACK_NAK_TIMEOUT_ERR:
		{
			stat = SAS_NAK_R_ERR;
			break;
		}
		default:
		{
			stat = SAS_DATA_UNDERRUN;
			break;
		}
		}
		return stat;

	}
	case SAS_PROTOCOL_SMP:
		stat = SAM_STAT_CHECK_CONDITION;
		break;

	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP:
	{
		task->ata_task.use_ncq = 0;
		stat = SAS_PROTO_RESPONSE;
		/* j00310691 fixme mvs_sata_done(mvi, task, slot_idx, err_dw0); */
	}
		break;
	default:
		break;
	}

	return stat;
}

static int slot_complete(struct hisi_hba *hisi_hba, struct hisi_sas_slot *slot, u32 abort)
{
	struct sas_task *task = slot->task;
	struct hisi_sas_device *hisi_sas_dev;
	struct task_status_struct *tstat;
	struct domain_device *dev;
	void *to;
	enum exec_status sts;
	struct hisi_sas_complete_hdr *complete_queue = hisi_hba->complete_hdr[slot->queue];
	struct hisi_sas_complete_hdr *complete_hdr;
	complete_hdr = &complete_queue[slot->queue_slot];

	if (unlikely(!task || !task->lldd_task || !task->dev))
		return -1;

	tstat = &task->task_status;
	dev = task->dev;
	hisi_sas_dev = dev->lldd_dev;

	task->task_state_flags &=
		~(SAS_TASK_STATE_PENDING | SAS_TASK_AT_INITIATOR);
	task->task_state_flags |= SAS_TASK_STATE_DONE;

	memset(tstat, 0, sizeof(*tstat));
	tstat->resp = SAS_TASK_COMPLETE;

	/* when no device attaching, go ahead and complete by error handling */
	if (unlikely(!hisi_sas_dev || abort)) {
		if (!hisi_sas_dev)
			dev_dbg(hisi_hba->dev, "%s port has not device.\n",
				__func__);
		tstat->stat = SAS_PHY_DOWN;
		goto out;
	}

	if (complete_hdr->io_cfg_err) {
		u32 info_reg = hisi_sas_read32(hisi_hba, HGC_INVLD_DQE_INFO);

		if (info_reg & HGC_INVLD_DQE_INFO_DQ_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq IPTT error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_TYPE_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq type error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_FORCE_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq force phy error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_PHY_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq phy id error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_ABORT_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq abort flag error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_IPTT_OF_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq IPTT or ICT error",
				__func__, slot->queue_slot);

		if (info_reg & HGC_INVLD_DQE_INFO_SSP_ERR_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq SSP frame type error",
				__func__, slot->queue_slot);


		if (info_reg & HGC_INVLD_DQE_INFO_OFL_MSK)
			dev_err(hisi_hba->dev, "%s slot %d has dq order frame length error",
				__func__, slot->queue_slot);

		tstat->resp = SAS_TASK_UNDELIVERED;
		tstat->stat = SAS_OPEN_REJECT;
		goto out;
	}

	if (complete_hdr->err_rcrd_xfrd) {
		tstat->stat = SAS_DATA_UNDERRUN;
		tstat->resp = SAS_TASK_COMPLETE;
		goto out;
	}

	switch (task->task_proto) {
	case SAS_PROTOCOL_SSP:
	{
		/* j00310691 for SMP, IU contains just the SSP IU */
		struct ssp_response_iu *iu = slot->status_buffer +
			sizeof(struct hisi_sas_err_record);
		sas_ssp_task_response(hisi_hba->dev, task, iu);
		break;
	}
	case SAS_PROTOCOL_SMP:
	{
		struct scatterlist *sg_resp = &task->smp_task.smp_resp;
			tstat->stat = SAM_STAT_GOOD;
		to = kmap_atomic(sg_page(sg_resp));
		/*for expander*/
		dma_unmap_sg(hisi_hba->dev, &task->smp_task.smp_resp, 1,
			DMA_FROM_DEVICE);/*fixme*/
		dma_unmap_sg(hisi_hba->dev, &task->smp_task.smp_req, 1,
			DMA_TO_DEVICE);/*fixme*/
			/* j00310691 for SMP, buffer contains the full SMP frame */
		memcpy(to + sg_resp->offset,
			slot->status_buffer + sizeof(struct hisi_sas_err_record),
			sg_dma_len(sg_resp));
		kunmap_atomic(to);
		break;
	}
	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP:
		dev_err(hisi_hba->dev, "%s STP not supported", __func__);
		break;

	default:
		tstat->stat = SAM_STAT_CHECK_CONDITION;
		break;
	}

	if (!slot->port->port_attached) {
		dev_err(hisi_hba->dev, "%s port %d has removed\n",
			__func__, slot->port->sas_port.id);
		tstat->stat = SAS_PHY_DOWN;
	}

out:
	if (hisi_sas_dev && hisi_sas_dev->running_req)
		hisi_sas_dev->running_req--;

	hisi_sas_slot_task_free(hisi_hba, task, slot);
	sts = tstat->stat;

	if (task->task_done)
		task->task_done(task);

	return sts;
}

static irqreturn_t int_ctrlrdy(int phy, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;
	u32 context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);
	pr_info("%s phy=%d context=0x%x\n", __func__, phy, context);
	irq_value = hisi_sas_phy_read32(hisi_hba, phy, CHL_INT2);

	if (!(irq_value & CHL_INT2_CTRL_PHY_RDY_MSK)) {
		dev_dbg(hisi_hba->dev, "%s irq_value = %x not set enable bit",
			__func__, irq_value);
		hisi_sas_phy_write32(hisi_hba, phy, CHL_INT2, CHL_INT2_CTRL_PHY_RDY_MSK);
		return IRQ_NONE;
	}

	config_serdes_12G(hisi_hba, phy);
	hisi_sas_phy_write32(hisi_hba, phy, CHL_INT2,
			CHL_INT2_CTRL_PHY_RDY_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t int_phyup(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value, context, port_id, link_rate;
	int i;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	u32 *frame_rcvd = (u32 *)sas_phy->frame_rcvd;
	struct sas_identify_frame *id = (struct sas_identify_frame *)frame_rcvd;
	irqreturn_t res = IRQ_HANDLED;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2);

	if (!(irq_value & CHL_INT2_SL_PHY_ENA_MSK)) {
		dev_dbg(hisi_hba->dev, "%s irq_value = %x not set enable bit\n",
			__func__, irq_value);
		res = IRQ_NONE;
		goto end;
	}

	context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);
	if (context & 1 << phy_no) {
		dev_info(hisi_hba->dev, "%s SATA attached equipment\n", __func__);
		goto end;
	}

	port_id = (hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA) >> (4 * phy_no)) & 0xf;
	if (port_id == 0xf) {
		dev_err(hisi_hba->dev, "%s phy = %d, invalid portid\n", __func__, phy_no);
		res = IRQ_NONE;
		goto end;
	}

	dev_err(hisi_hba->dev, "%s phy = %d portid=0x%x\n", __func__, phy_no, port_id);

	/* j00310691 todo stop serdes fw timer */
	for (i = 0; i < 6; i++) {
		u32 idaf = hisi_sas_phy_read32(hisi_hba, phy_no,
					RX_IDAF_DWORD0 + (i * 4));
		frame_rcvd[i] = __swab32(idaf);
	}

	if (id->dev_type == SAS_END_DEVICE) {
		u32 sl_control;

		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control |= SL_CONTROL_NOTIFY_EN_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
		mdelay(1);
		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control &= ~SL_CONTROL_NOTIFY_EN_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
	}

	/* Get the linkrate */
	link_rate = hisi_sas_read32(hisi_hba, PHY_CONN_RATE);
	link_rate = (link_rate >> (phy_no * 4)) & 0xf;
	sas_phy->linkrate = link_rate;
	sas_phy->oob_mode = SAS_OOB_MODE;
	memcpy(sas_phy->attached_sas_addr,
		&id->sas_addr, SAS_ADDR_SIZE);
	dev_info(hisi_hba->dev, "%s phy_no=%d hisi_hba->id=%d link_rate=%d\n", __func__, phy_no, hisi_hba->id, link_rate);
	phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
	phy->phy_type |= PORT_TYPE_SAS;
	phy->phy_attached = 1;
	phy->identify.device_type = id->dev_type;
	phy->frame_rcvd_size =	sizeof(struct sas_identify_frame);
	if (phy->identify.device_type == SAS_END_DEVICE)
		phy->identify.target_port_protocols =
			SAS_PROTOCOL_SSP;
	else if (phy->identify.device_type != SAS_PHY_UNUSED)
		phy->identify.target_port_protocols =
			SAS_PROTOCOL_SMP;

	hisi_sas_bytes_dmaed(hisi_hba, phy_no);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2,
			CHL_INT2_SL_PHY_ENA_MSK);

	if (irq_value & CHL_INT2_SL_PHY_ENA_MSK) {
		/* Higgs_BypassChipBugUnmaskAbnormalIntr */
		u32 chl_int0 = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0);
		chl_int0 &= ~CHL_INT0_PHYCTRL_NOTRDY_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, chl_int0);
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK, 0x003ce3ee);
	}

	return res;
}

static irqreturn_t int_bcast(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;
	irqreturn_t res = IRQ_HANDLED;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha = hisi_hba->sas;

	dev_err(hisi_hba->dev, "%s\n", __func__);
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2);

	if (!(irq_value & CHL_INT2_SL_RX_BC_ACK_MSK)) {
		dev_err(hisi_hba->dev, "%s irq_value = %x not set enable bit",
			__func__, irq_value);
		res = IRQ_NONE;
		goto end;
	}

	sas_ha->notify_port_event(sas_phy, PORTE_BROADCAST_RCVD);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2,
			CHL_INT2_SL_RX_BC_ACK_MSK);

	return res;
}

static irqreturn_t int_statuscg(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	dev_err(hisi_hba->dev, "%s\n", __func__);
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2);

	if (!(irq_value & CHL_INT2_PHY_STATUS_CHG_MSK))
		dev_err(hisi_hba->dev, "%s irq_value = %x not set enable bit",
			__func__, irq_value);

	dev_info(hisi_hba->dev, "%s phy = %d, irq_value = %x\n",
		 __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2,
			CHL_INT2_PHY_STATUS_CHG_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t int_abnormal(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;
	u32 irq_mask_old;

	dev_err(hisi_hba->dev, "%s\n", __func__);
	/* mask_int0 */
	irq_mask_old = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK, 0x003FFFFF);

	/* read int0 */
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0);

	if (irq_value & CHL_INT0_PHYCTRL_NOTRDY_MSK) {
		u32 val = hisi_sas_phy_read32(hisi_hba, phy_no, PHY_CFG);

		if (val & PHY_CFG_ENA_MSK) {
			u32 phy_state = hisi_sas_read32(hisi_hba, PHY_STATE);

			/* Enabled */
			/* Stop serdes fw timer */
			/* serdes lane reset */
			/* todo */

			hisi_sas_phy_down(hisi_hba,
				phy_no,
				(phy_state & 1 << phy_no) ? 1 : 0);
		} else {
			/* Disabled */
			/* Ignore phydown event if disabled */
			pr_warn("%s phy = %d phydown event and already disabled\n", __func__, phy_no);
		}

	} else if (irq_value & CHL_INT0_ID_TIMEOUT_MSK) {
		pr_info("%s phy = %d identify timeout todo\n", __func__, phy_no);
	} else {
		if (irq_value & CHL_INT0_DWS_LOST_MSK) {
			pr_info("%s phy = %d dws lost\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_SN_FAIL_NGR_MSK) {
			pr_info("%s phy = %d sn fail ngr\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_SL_IDAF_FAIL_MSK ||
			irq_value & CHL_INT0_SL_OPAF_FAIL_MSK) {
			pr_info("%s phy = %d check address frame err\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_SL_PS_FAIL_OFF) {
			pr_debug("%s phy = %d ps req fail\n", __func__, phy_no);
		}
	}

	/* write to zero */
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, irq_value);

	if (irq_value & CHL_INT0_PHYCTRL_NOTRDY_MSK)
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK,
				0x003FFFFF & ~1);
	else
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK,
				irq_mask_old);

	return IRQ_HANDLED;
}

/* Interrupts */
static irqreturn_t cq_interrupt(int queue, void *p)
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

		slot_complete(hisi_hba, slot, 0);

		if (++rd_point >= HISI_SAS_QUEUE_SLOTS)
			rd_point = 0;
	}

	/* update rd_point */
	hisi_sas_write32(hisi_hba, COMPL_Q_0_RD_PTR + (0x14 * queue), rd_point);
	return IRQ_HANDLED;
}

static irqreturn_t fatal_ecc_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 ecc_int = hisi_sas_read32(hisi_hba, SAS_ECC_INTR);

	if (ecc_int & SAS_ECC_INTR_DQ_ECC1B_MSK) {
		u32 ecc_err = hisi_sas_read32(hisi_hba, HGC_ECC_ERR);

		hisi_hba->fatal_stat.dq_1b_ecc_err_cnt = ecc_err;
		dev_err(hisi_hba->dev, "fatal DQ 1b ECC interrupt on core %d (0x%x)\n",
			hisi_hba->id, ecc_err);
	}

	if (ecc_int & SAS_ECC_INTR_DQ_ECCBAD_MSK) {
		u32 addr = (hisi_sas_read32(hisi_hba, HGC_DQ_ECC_ADDR) &
				HGC_DQ_ECC_ADDR_BAD_MSK) >>
				HGC_DQ_ECC_ADDR_BAD_OFF;

		hisi_hba->fatal_stat.dq_multib_ecc_err_cnt++;
		dev_err(hisi_hba->dev, "fatal DQ RAM ECC interrupt on core %d @ 0x%08x\n",
			hisi_hba->id, addr);
	}

	if (ecc_int & SAS_ECC_INTR_IOST_ECC1B_MSK) {
		u32 ecc_err = hisi_sas_read32(hisi_hba, HGC_ECC_ERR);

		hisi_hba->fatal_stat.iost_1b_ecc_err_cnt = ecc_err;
		dev_err(hisi_hba->dev, "fatal IOST 1b ECC interrupt on core %d (0x%x)\n",
			hisi_hba->id, ecc_err);
	}

	if (ecc_int & SAS_ECC_INTR_IOST_ECCBAD_MSK) {
		u32 addr = (hisi_sas_read32(hisi_hba, HGC_IOST_ECC_ADDR) &
				HGC_IOST_ECC_ADDR_BAD_MSK) >>
				HGC_IOST_ECC_ADDR_BAD_OFF;

		hisi_hba->fatal_stat.iost_multib_ecc_err_cnt++;
		dev_err(hisi_hba->dev, "fatal IOST RAM ECC interrupt on core %d @ 0x%08x\n",
			hisi_hba->id, addr);
	}

	if (ecc_int & SAS_ECC_INTR_ITCT_ECCBAD_MSK) {
		u32 addr = (hisi_sas_read32(hisi_hba, HGC_ITCT_ECC_ADDR) &
				HGC_ITCT_ECC_ADDR_BAD_MSK) >>
				HGC_ITCT_ECC_ADDR_BAD_OFF;

		hisi_hba->fatal_stat.itct_multib_ecc_err_cnt++;
		dev_err(hisi_hba->dev, "fatal TCT RAM ECC interrupt on core %d @ 0x%08x\n",
			hisi_hba->id, addr);
	}

	if (ecc_int & SAS_ECC_INTR_ITCT_ECC1B_MSK) {
		u32 ecc_err = hisi_sas_read32(hisi_hba, HGC_ECC_ERR);

		hisi_hba->fatal_stat.itct_1b_ecc_err_cnt++;
		dev_err(hisi_hba->dev, "fatal ITCT 1b ECC interrupt on core %d (0x%x)\n",
			hisi_hba->id, ecc_err);
	}

	hisi_sas_write32(hisi_hba, SAS_ECC_INTR, ecc_int | 0x3f);

	return IRQ_HANDLED;
}

static irqreturn_t fatal_axi_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 axi_int = hisi_sas_read32(hisi_hba, ENT_INT_SRC2);
	u32 axi_info = hisi_sas_read32(hisi_hba, HGC_AXI_FIFO_ERR_INFO);

	if (axi_int & ENT_INT_SRC2_AXI_WRONG_INT_MSK)
		dev_err(hisi_hba->dev, "fatal AXI incorrect interrupt on core %d (0x%x)\n",
			hisi_hba->id, axi_info);

	if (axi_int & ENT_INT_SRC2_AXI_OVERLF_INT_MSK) {
		hisi_hba->fatal_stat.overfl_axi_err_cnt++;
		dev_err(hisi_hba->dev, "fatal AXI incorrect interrupt on core %d (0x%x)\n",
			hisi_hba->id, axi_info);

	}

	hisi_sas_write32(hisi_hba, ENT_INT_SRC2, axi_int | 0x30000000);

	return IRQ_HANDLED;
}

#define DECLARE_PHY_INT_HANDLER_GROUP(phy)\
	DECLARE_INT_HANDLER(int_ctrlrdy, phy)\
	DECLARE_INT_HANDLER(int_bcast, phy)\
	DECLARE_INT_HANDLER(int_statuscg, phy)\
	DECLARE_INT_HANDLER(int_phyup, phy)\
	DECLARE_INT_HANDLER(int_abnormal, phy)\


#define DECLARE_PHY_INT_GROUP_PTR(phy)\
	INT_HANDLER_NAME(int_ctrlrdy, phy),\
	INT_HANDLER_NAME(int_bcast, phy),\
	INT_HANDLER_NAME(int_statuscg, phy),\
	INT_HANDLER_NAME(int_phyup, phy),\
	INT_HANDLER_NAME(int_abnormal, phy),\

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
	{"CTRL Rdy"},
	{"Bcast"},
	{"StatusCG"},
	{"Phy Up"},
	{"Abnormal"},
};

static const char cq_int_name[32] = "cq";
static const char fatal_int_name[HISI_SAS_FATAL_INT_NR][32] = {
	"fatal ecc",
	"fatal axi"
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
DECLARE_INT_HANDLER(cq_interrupt, 16)
DECLARE_INT_HANDLER(cq_interrupt, 17)
DECLARE_INT_HANDLER(cq_interrupt, 18)
DECLARE_INT_HANDLER(cq_interrupt, 19)
DECLARE_INT_HANDLER(cq_interrupt, 20)
DECLARE_INT_HANDLER(cq_interrupt, 21)
DECLARE_INT_HANDLER(cq_interrupt, 22)
DECLARE_INT_HANDLER(cq_interrupt, 23)
DECLARE_INT_HANDLER(cq_interrupt, 24)
DECLARE_INT_HANDLER(cq_interrupt, 25)
DECLARE_INT_HANDLER(cq_interrupt, 26)
DECLARE_INT_HANDLER(cq_interrupt, 27)
DECLARE_INT_HANDLER(cq_interrupt, 28)
DECLARE_INT_HANDLER(cq_interrupt, 29)
DECLARE_INT_HANDLER(cq_interrupt, 30)
DECLARE_INT_HANDLER(cq_interrupt, 31)

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
	INT_HANDLER_NAME(cq_interrupt, 16),
	INT_HANDLER_NAME(cq_interrupt, 17),
	INT_HANDLER_NAME(cq_interrupt, 18),
	INT_HANDLER_NAME(cq_interrupt, 19),
	INT_HANDLER_NAME(cq_interrupt, 20),
	INT_HANDLER_NAME(cq_interrupt, 21),
	INT_HANDLER_NAME(cq_interrupt, 22),
	INT_HANDLER_NAME(cq_interrupt, 23),
	INT_HANDLER_NAME(cq_interrupt, 24),
	INT_HANDLER_NAME(cq_interrupt, 25),
	INT_HANDLER_NAME(cq_interrupt, 26),
	INT_HANDLER_NAME(cq_interrupt, 27),
	INT_HANDLER_NAME(cq_interrupt, 28),
	INT_HANDLER_NAME(cq_interrupt, 29),
	INT_HANDLER_NAME(cq_interrupt, 30),
	INT_HANDLER_NAME(cq_interrupt, 31)
};

static irq_handler_t fatal_interrupts[HISI_SAS_MAX_QUEUES] = {
	fatal_ecc_int,
	fatal_axi_int
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

	return 0;
}

static int interrupt_openall(struct hisi_hba *hisi_hba)
{
	int i;
	u32 val;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/* Clear interrupt status */
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT0);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0, val);
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT1);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1, val);
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT2);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2, val);

		/* Unmask interrupt */
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK, 0x003ce3ee);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK, 0x00017fff);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x0000032a);

		/* bypass chip bug mask abnormal intr */
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK, 0x003fffff & ~1);
	}

	return 0;
}


const struct hisi_sas_dispatch hisi_sas_p660_dispatch = {
	.hw_init = hw_init,
	.phys_init = phys_init,
	.interrupt_init = interrupt_init,
	.interrupt_openall = interrupt_openall,
	.get_free_slot = get_free_slot,
	.start_delivery = start_delivery,
	.prep_ssp = prep_ssp,
	.prep_smp = prep_smp,
	.is_phy_ready = is_phy_ready,
	.slot_complete = slot_complete,
	/* p660 does not support STP */
};
