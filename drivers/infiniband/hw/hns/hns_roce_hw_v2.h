/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _HNS_ROCE_HW_V2_H
#define _HNS_ROCE_HW_V2_H

#define HNS_ROCE_VF_QPC_BT_NUM			256
#define HNS_ROCE_VF_SRQC_BT_NUM			64
#define HNS_ROCE_VF_CQC_BT_NUM			64
#define HNS_ROCE_VF_MPT_BT_NUM			64
#define HNS_ROCE_VF_EQC_NUM			64
#define HNS_ROCE_VF_SMAC_NUM			32
#define HNS_ROCE_VF_SGID_NUM			32
#define HNS_ROCE_VF_SL_NUM			8

#define HNS_ROCE_V2_MAX_QP_NUM			0x2000
#define HNS_ROCE_V2_MAX_WQE_NUM			0x8000
#define HNS_ROCE_V2_MAX_CQ_NUM			0x8000
#define HNS_ROCE_V2_MAX_CQE_NUM			0x400000
#define HNS_ROCE_V2_MAX_RQ_SGE_NUM		0x100
#define HNS_ROCE_V2_MAX_SQ_SGE_NUM		0xff
#define HNS_ROCE_V2_MAX_SQ_INLINE		0x20
#define HNS_ROCE_V2_UAR_NUM			256
#define HNS_ROCE_V2_PHY_UAR_NUM			1
#define HNS_ROCE_V2_MAX_MTPT_NUM		0x8000
#define HNS_ROCE_V2_MAX_MTT_SEGS		0x100000
#define HNS_ROCE_V2_MAX_CQE_SEGS		0x10000
#define HNS_ROCE_V2_MAX_PD_NUM			0x400000
#define HNS_ROCE_V2_MAX_QP_INIT_RDMA		128
#define HNS_ROCE_V2_MAX_QP_DEST_RDMA		128
#define HNS_ROCE_V2_MAX_SQ_DESC_SZ		64
#define HNS_ROCE_V2_MAX_RQ_DESC_SZ		16
#define HNS_ROCE_V2_MAX_SRQ_DESC_SZ		64
#define HNS_ROCE_V2_QPC_ENTRY_SZ		256
#define HNS_ROCE_V2_IRRL_ENTRY_SZ		64
#define HNS_ROCE_V2_CQC_ENTRY_SZ		64
#define HNS_ROCE_V2_MTPT_ENTRY_SZ		64
#define HNS_ROCE_V2_MTT_ENTRY_SZ		64
#define HNS_ROCE_V2_CQE_ENTRY_SIZE		32
#define HNS_ROCE_V2_PAGE_SIZE_SUPPORTED		0xFFFFF000
#define HNS_ROCE_CMDQ_TX_TIMEOUT		200

#define HNS_ROCE_CONTEXT_HOP_NUM		1
#define HNS_ROCE_MTT_HOP_NUM			1

#define HNS_ROCE_CMD_FLAG_IN_VALID_SHIFT	0
#define HNS_ROCE_CMD_FLAG_OUT_VALID_SHIFT	1
#define HNS_ROCE_CMD_FLAG_NEXT_SHIFT		2
#define HNS_ROCE_CMD_FLAG_WR_OR_RD_SHIFT	3
#define HNS_ROCE_CMD_FLAG_NO_INTR_SHIFT		4
#define HNS_ROCE_CMD_FLAG_ERR_INTR_SHIFT	5

#define HNS_ROCE_CMD_FLAG_IN		BIT(HNS_ROCE_CMD_FLAG_IN_VALID_SHIFT)
#define HNS_ROCE_CMD_FLAG_OUT		BIT(HNS_ROCE_CMD_FLAG_OUT_VALID_SHIFT)
#define HNS_ROCE_CMD_FLAG_NEXT		BIT(HNS_ROCE_CMD_FLAG_NEXT_SHIFT)
#define HNS_ROCE_CMD_FLAG_WR		BIT(HNS_ROCE_CMD_FLAG_WR_OR_RD_SHIFT)
#define HNS_ROCE_CMD_FLAG_NO_INTR	BIT(HNS_ROCE_CMD_FLAG_NO_INTR_SHIFT)
#define HNS_ROCE_CMD_FLAG_ERR_INTR	BIT(HNS_ROCE_CMD_FLAG_ERR_INTR_SHIFT)

#define HNS_ROCE_CMQ_DESC_NUM_S		3
#define HNS_ROCE_CMQ_EN_B		16
#define HNS_ROCE_CMQ_ENABLE		BIT(HNS_ROCE_CMQ_EN_B)

#define check_whether_last_step(hop_num, step_idx) \
	((step_idx == 0 && hop_num == HNS_ROCE_HOP_NUM_0) || \
	(step_idx == 1 && hop_num == 1) || \
	(step_idx == 2 && hop_num == 2))

/* CMQ command */
enum hns_roce_opcode_type {
	HNS_ROCE_OPC_QUERY_HW_VER			= 0x8000,
	HNS_ROCE_OPC_CFG_GLOBAL_PARAM			= 0x8001,
	HNS_ROCE_OPC_ALLOC_PF_RES			= 0x8004,

	/* Need to enclosure to two cmd */
	HNS_ROCE_OPC_ALLOC_VF_RES			= 0x8401,
	HNS_ROCE_OPC_CFG_BT_ATTR			= 0x8506,
};

enum {
	TYPE_CRQ,
	TYPE_CSQ,
};

enum hns_roce_cmd_return_status {
	CMD_EXEC_SUCCESS	= 0,
	CMD_NO_AUTH		= 1,
	CMD_NOT_EXEC		= 2,
	CMD_QUEUE_FULL		= 3,
};

enum hns_roce_cmd_status {
	STATUS_SUCCESS	= 0,
	ERR_CSQ_FULL	= -1,
	ERR_CSQ_TIMEOUT	= -2,
	ERR_CSQ_ERROR	= -3
};

struct hns_roce_cmdq_desc {
	u16 opcode;
	u16 flag;
	u16 retval;
	u16 rsv;
	u32 data[6];
};

struct hns_roce_query_version {
	__le16 rocee_vendor_id;
	__le16 rocee_hw_version;
	__le32 rsv[5];
};

struct hns_roce_cfg_global_param {
	__le32 data_0;
	__le32 rsv[5];
};

#define CFG_GLOBAL_PARAM_DATA_0_ROCEE_TIME_1US_CFG_S 0
#define CFG_GLOBAL_PARAM_DATA_0_ROCEE_TIME_1US_CFG_M \
	(((1UL << 10) - 1) << CFG_GLOBAL_PARAM_DATA_0_ROCEE_TIME_1US_CFG_S)

#define CFG_GLOBAL_PARAM_DATA_0_ROCEE_UDP_PORT_S 16
#define CFG_GLOBAL_PARAM_DATA_0_ROCEE_UDP_PORT_M \
	(((1UL << 16) - 1) << CFG_GLOBAL_PARAM_DATA_0_ROCEE_UDP_PORT_S)

struct hns_roce_pf_res_a {
	__le32 data_0;
	__le32 data_1;
	__le32 data_2;
	__le32 data_3;
	__le32 data_4;
	__le32 data_5;
};
#define PF_RES_A_DATA_0_PF_ID_S 0
#define PF_RES_A_DATA_0_PF_ID_M \
	(((1UL << 8) - 1) << PF_RES_A_DATA_0_PF_ID_S)

#define PF_RES_A_DATA_1_PF_QPC_BT_IDX_S 0
#define PF_RES_A_DATA_1_PF_QPC_BT_IDX_M \
	(((1UL << 11) - 1) << PF_RES_A_DATA_1_PF_QPC_BT_IDX_S)

#define PF_RES_A_DATA_1_PF_QPC_BT_NUM_S 16
#define PF_RES_A_DATA_1_PF_QPC_BT_NUM_M \
	(((1UL << 12) - 1) << PF_RES_A_DATA_1_PF_QPC_BT_NUM_S)

#define PF_RES_A_DATA_2_PF_SRQC_BT_IDX_S 0
#define PF_RES_A_DATA_2_PF_SRQC_BT_IDX_M \
	(((1UL << 9) - 1) << PF_RES_A_DATA_2_PF_SRQC_BT_IDX_S)

#define PF_RES_A_DATA_2_PF_SRQC_BT_NUM_S 16
#define PF_RES_A_DATA_2_PF_SRQC_BT_NUM_M \
	(((1UL << 10) - 1) << PF_RES_A_DATA_2_PF_SRQC_BT_NUM_S)

#define PF_RES_A_DATA_3_PF_CQC_BT_IDX_S 0
#define PF_RES_A_DATA_3_PF_CQC_BT_IDX_M \
	(((1UL << 9) - 1) << PF_RES_A_DATA_3_PF_CQC_BT_IDX_S)

#define PF_RES_A_DATA_3_PF_CQC_BT_NUM_S 16
#define PF_RES_A_DATA_3_PF_CQC_BT_NUM_M \
	(((1UL << 10) - 1) << PF_RES_A_DATA_3_PF_CQC_BT_NUM_S)

#define PF_RES_A_DATA_4_PF_MPT_BT_IDX_S 0
#define PF_RES_A_DATA_4_PF_MPT_BT_IDX_M \
	(((1UL << 9) - 1) << PF_RES_A_DATA_4_PF_MPT_BT_IDX_S)

#define PF_RES_A_DATA_4_PF_MPT_BT_NUM_S 16
#define PF_RES_A_DATA_4_PF_MPT_BT_NUM_M \
	(((1UL << 10) - 1) << PF_RES_A_DATA_4_PF_MPT_BT_NUM_S)

#define PF_RES_A_DATA_5_PF_EQC_BT_IDX_S 0
#define PF_RES_A_DATA_5_PF_EQC_BT_IDX_M \
	(((1UL << 9) - 1) << PF_RES_A_DATA_5_PF_EQC_BT_IDX_S)

#define PF_RES_A_DATA_5_PF_EQC_BT_NUM_S 16
#define PF_RES_A_DATA_5_PF_EQC_BT_NUM_M \
	(((1UL << 10) - 1) << PF_RES_A_DATA_5_PF_EQC_BT_NUM_S)

struct hns_roce_pf_res_b {
	__le32 data_0;
	__le32 data_1;
	__le32 data_2;
	__le32 data_3;
	__le32 rsv[2];
};
#define PF_RES_B_DATA_1_PF_SMAC_IDX_S 0
#define PF_RES_B_DATA_1_PF_SMAC_IDX_M \
	(((1UL << 8) - 1) << PF_RES_B_DATA_1_PF_SMAC_IDX_S)

#define PF_RES_B_DATA_1_PF_SMAC_NUM_S 8
#define PF_RES_B_DATA_1_PF_SMAC_NUM_M \
	(((1UL << 9) - 1) << PF_RES_B_DATA_1_PF_SMAC_NUM_S)

#define PF_RES_B_DATA_2_PF_SGID_IDX_S 0
#define PF_RES_B_DATA_2_PF_SGID_IDX_M \
	(((1UL << 8) - 1) << PF_RES_B_DATA_2_PF_SGID_IDX_S)

#define PF_RES_B_DATA_2_PF_SGID_NUM_S 8
#define PF_RES_B_DATA_2_PF_SGID_NUM_M \
	(((1UL << 9) - 1) << PF_RES_B_DATA_2_PF_SGID_NUM_S)

#define PF_RES_B_DATA_3_PF_QID_IDX_S 0
#define PF_RES_B_DATA_3_PF_QID_IDX_M \
	(((1UL << 10) - 1) << PF_RES_B_DATA_3_PF_QID_IDX_S)

#define PF_RES_B_DATA_3_PF_SL_NUM_S 16
#define PF_RES_B_DATA_3_PF_SL_NUM_M \
	(((1UL << 4) - 1) << PF_RES_B_DATA_3_PF_SL_NUM_S)

struct hns_roce_vf_res_a {
	u32 data_0;
	u32 data_1;
	u32 data_2;
	u32 data_3;
	u32 data_4;
	u32 data_5;
};
#define VF_RES_A_DATA_0_VF_ID_S 0
#define VF_RES_A_DATA_0_VF_ID_M \
	(((1UL << 8) - 1) << VF_RES_A_DATA_0_VF_ID_S)

#define VF_RES_A_DATA_1_VF_QPC_BT_IDX_S 0
#define VF_RES_A_DATA_1_VF_QPC_BT_IDX_M \
	(((1UL << 11) - 1) << VF_RES_A_DATA_1_VF_QPC_BT_IDX_S)

#define VF_RES_A_DATA_1_VF_QPC_BT_NUM_S 16
#define VF_RES_A_DATA_1_VF_QPC_BT_NUM_M \
	(((1UL << 12) - 1) << VF_RES_A_DATA_1_VF_QPC_BT_NUM_S)

#define VF_RES_A_DATA_2_VF_SRQC_BT_IDX_S 0
#define VF_RES_A_DATA_2_VF_SRQC_BT_IDX_M \
	(((1UL << 9) - 1) << VF_RES_A_DATA_2_VF_SRQC_BT_IDX_S)

#define VF_RES_A_DATA_2_VF_SRQC_BT_NUM_S 16
#define VF_RES_A_DATA_2_VF_SRQC_BT_NUM_M \
	(((1UL << 10) - 1) << VF_RES_A_DATA_2_VF_SRQC_BT_NUM_S)

#define VF_RES_A_DATA_3_VF_CQC_BT_IDX_S 0
#define VF_RES_A_DATA_3_VF_CQC_BT_IDX_M \
	(((1UL << 9) - 1) << VF_RES_A_DATA_3_VF_CQC_BT_IDX_S)

#define VF_RES_A_DATA_3_VF_CQC_BT_NUM_S 16
#define VF_RES_A_DATA_3_VF_CQC_BT_NUM_M \
	(((1UL << 10) - 1) << VF_RES_A_DATA_3_VF_CQC_BT_NUM_S)

#define VF_RES_A_DATA_4_VF_MPT_BT_IDX_S 0
#define VF_RES_A_DATA_4_VF_MPT_BT_IDX_M \
	(((1UL << 9) - 1) << VF_RES_A_DATA_4_VF_MPT_BT_IDX_S)

#define VF_RES_A_DATA_4_VF_MPT_BT_NUM_S 16
#define VF_RES_A_DATA_4_VF_MPT_BT_NUM_M \
	(((1UL << 10) - 1) << VF_RES_A_DATA_4_VF_MPT_BT_NUM_S)

#define VF_RES_A_DATA_5_VF_EQC_IDX_S 0
#define VF_RES_A_DATA_5_VF_EQC_IDX_M \
	(((1UL << 9) - 1) << VF_RES_A_DATA_5_VF_EQC_IDX_S)

#define VF_RES_A_DATA_5_VF_EQC_NUM_S 16
#define VF_RES_A_DATA_5_VF_EQC_NUM_M \
	(((1UL << 10) - 1) << VF_RES_A_DATA_5_VF_EQC_NUM_S)

struct hns_roce_vf_res_b {
	u32 data_0;
	u32 data_1;
	u32 data_2;
	u32 data_3;
	u32 rsv[2];
};

#define VF_RES_B_DATA_0_VF_ID_S 0
#define VF_RES_B_DATA_0_VF_ID_M \
	(((1UL << 8) - 1) << VF_RES_B_DATA_0_VF_ID_S)

#define VF_RES_B_DATA_1_VF_SMAC_IDX_S 0
#define VF_RES_B_DATA_1_VF_SMAC_IDX_M \
	(((1UL << 8) - 1) << VF_RES_B_DATA_1_VF_SMAC_IDX_S)

#define VF_RES_B_DATA_1_VF_SMAC_NUM_S 8
#define VF_RES_B_DATA_1_VF_SMAC_NUM_M \
	(((1UL << 9) - 1) << VF_RES_B_DATA_1_VF_SMAC_NUM_S)

#define VF_RES_B_DATA_2_VF_SGID_IDX_S 0
#define VF_RES_B_DATA_2_VF_SGID_IDX_M \
	(((1UL << 8) - 1) << VF_RES_B_DATA_2_VF_SGID_IDX_S)

#define VF_RES_B_DATA_2_VF_SGID_NUM_S 8
#define VF_RES_B_DATA_2_VF_SGID_NUM_M \
	(((1UL << 9) - 1) << VF_RES_B_DATA_2_VF_SGID_NUM_S)

#define VF_RES_B_DATA_3_VF_QID_IDX_S 0
#define VF_RES_B_DATA_3_VF_QID_IDX_M \
	(((1UL << 10) - 1) << VF_RES_B_DATA_3_VF_QID_IDX_S)

#define VF_RES_B_DATA_3_VF_SL_NUM_S 16
#define VF_RES_B_DATA_3_VF_SL_NUM_M \
	(((1UL << 4) - 1) << VF_RES_B_DATA_3_VF_SL_NUM_S)

struct hns_roce_cfg_smac_tb {
	u32 vf_smac_h;
	u32 data_1;
	u32 rsv[4];
};
#define CFG_SMAC_TB_DATA_1_VF_SMAC_L_S 0
#define CFG_SMAC_TB_DATA_1_VF_SMAC_L_M \
	(((1UL << 16) - 1) << CFG_SMAC_TB_DATA_1_VF_SMAC_L_S)

#define CFG_SMAC_TB_DATA_1_VF_SMAC_IDX_S 16
#define CFG_SMAC_TB_DATA_1_VF_SMAC_IDX_M \
	(((1UL << 8) - 1) << CFG_SMAC_TB_DATA_1_VF_SMAC_IDX_S)

#define CFG_SMAC_TB_DATA_1_VF_SAMC_IDX_VLD_S 24

/* Reg field definition */
#define ROCEE_VF_SMAC_CFG1_VF_SMAC_H_S 0
#define ROCEE_VF_SMAC_CFG1_VF_SMAC_H_M \
	(((1UL << 16) - 1) << ROCEE_VF_SMAC_CFG1_VF_SMAC_H_S)

struct hns_roce_cfg_sgid_tb {
	u32 vf_sgid_l;
	u32 vf_sgid_ml;
	u32 vf_sgid_mh;
	u32 vf_sgid_h;
	u32 data_4;
	u32 rsv;
};
#define CFG_SGID_TB_DATA_4_VF_SGID_TYPE_S 0
#define CFG_SGID_TB_DATA_4_VF_SGID_TYPE_M \
	(((1UL << 2) - 1) << CFG_SGID_TB_DATA_4_VF_SGID_TYPE_S)

#define CFG_SGID_TB_DATA_4_VF_SGID_IDX_S 2
#define CFG_SGID_TB_DATA_4_VF_SGID_IDX_M \
	(((1UL << 8) - 1) << CFG_SGID_TB_DATA_4_VF_SGID_IDX_S)

#define CFG_SGID_TB_DATA_4_VF_GID_IDX_VLD_S 10

#define ROCEE_VF_SGID_CFG4_SGID_TYPE_S 0
#define ROCEE_VF_SGID_CFG4_SGID_TYPE_M \
	(((1UL << 2) - 1) << ROCEE_VF_SGID_CFG4_SGID_TYPE_S)

struct hns_roce_cfg_bt_attr {
	u32 data_0;
	u32 data_1;
	u32 data_2;
	u32 data_3;
	u32 rsv[2];
};

#define CFG_BT_ATTR_DATA_0_VF_QPC_BA_PGSZ_S 0
#define CFG_BT_ATTR_DATA_0_VF_QPC_BA_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_0_VF_QPC_BA_PGSZ_S)

#define CFG_BT_ATTR_DATA_0_VF_QPC_BUF_PGSZ_S 4
#define CFG_BT_ATTR_DATA_0_VF_QPC_BUF_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_0_VF_QPC_BUF_PGSZ_S)

#define CFG_BT_ATTR_DATA_0_VF_QPC_HOPNUM_S 8
#define CFG_BT_ATTR_DATA_0_VF_QPC_HOPNUM_M \
	(((1UL << 2) - 1) << CFG_BT_ATTR_DATA_0_VF_QPC_HOPNUM_S)

#define CFG_BT_ATTR_DATA_1_VF_SRQC_BA_PGSZ_S 0
#define CFG_BT_ATTR_DATA_1_VF_SRQC_BA_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_1_VF_SRQC_BA_PGSZ_S)

#define CFG_BT_ATTR_DATA_1_VF_SRQC_BUF_PGSZ_S 4
#define CFG_BT_ATTR_DATA_1_VF_SRQC_BUF_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_1_VF_SRQC_BUF_PGSZ_S)

#define CFG_BT_ATTR_DATA_1_VF_SRQC_HOPNUM_S 8
#define CFG_BT_ATTR_DATA_1_VF_SRQC_HOPNUM_M \
	(((1UL << 2) - 1) << CFG_BT_ATTR_DATA_1_VF_SRQC_HOPNUM_S)

#define CFG_BT_ATTR_DATA_2_VF_CQC_BA_PGSZ_S 0
#define CFG_BT_ATTR_DATA_2_VF_CQC_BA_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_2_VF_CQC_BA_PGSZ_S)

#define CFG_BT_ATTR_DATA_2_VF_CQC_BUF_PGSZ_S 4
#define CFG_BT_ATTR_DATA_2_VF_CQC_BUF_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_2_VF_CQC_BUF_PGSZ_S)

#define CFG_BT_ATTR_DATA_2_VF_CQC_HOPNUM_S 8
#define CFG_BT_ATTR_DATA_2_VF_CQC_HOPNUM_M \
	(((1UL << 2) - 1) << CFG_BT_ATTR_DATA_2_VF_CQC_HOPNUM_S)

#define CFG_BT_ATTR_DATA_3_VF_MPT_BA_PGSZ_S 0
#define CFG_BT_ATTR_DATA_3_VF_MPT_BA_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_3_VF_MPT_BA_PGSZ_S)

#define CFG_BT_ATTR_DATA_3_VF_MPT_BUF_PGSZ_S 4
#define CFG_BT_ATTR_DATA_3_VF_MPT_BUF_PGSZ_M \
	(((1UL << 4) - 1) << CFG_BT_ATTR_DATA_3_VF_MPT_BUF_PGSZ_S)

#define CFG_BT_ATTR_DATA_3_VF_MPT_HOPNUM_S 8
#define CFG_BT_ATTR_DATA_3_VF_MPT_HOPNUM_M \
	(((1UL << 2) - 1) << CFG_BT_ATTR_DATA_3_VF_MPT_HOPNUM_S)

#define ROCEE_VF_MB_CFG0_REG		0x40
#define ROCEE_VF_MB_STATUS_REG		0x58

#define HNS_ROCE_V2_GO_BIT_TIMEOUT_MSECS	10000

#define HNS_ROCE_HW_RUN_BIT_SHIFT	31
#define HNS_ROCE_HW_MB_STATUS_MASK	0xFF

#define HNS_ROCE_VF_MB4_TAG_MASK	0xFFFFFF00
#define HNS_ROCE_VF_MB4_TAG_SHIFT	8

#define HNS_ROCE_VF_MB4_CMD_MASK	0xFF
#define HNS_ROCE_VF_MB4_CMD_SHIFT	0

#define HNS_ROCE_VF_MB5_EVENT_MASK	0x10000
#define HNS_ROCE_VF_MB5_EVENT_SHIFT	16

#define HNS_ROCE_VF_MB5_TOKEN_MASK	0xFFFF
#define HNS_ROCE_VF_MB5_TOKEN_SHIFT	0

struct hns_roce_cmdq_desc_cb {
	dma_addr_t dma;
	void *va;
	u32 length;
};

struct hns_roce_v2_cmdq {
	dma_addr_t desc_dma_addr;
	struct hns_roce_cmdq_desc *desc;
	struct hns_roce_cmdq_desc_cb *desc_cb;
	u32 head;
	u32 tail;

	u16 buf_size;
	u16 desc_num;
	int next_to_use;
	int next_to_clean;
	u8 flag;
	struct mutex mutex; /* command queue lock */
};

struct hns_roce_v2_cmd {
	struct hns_roce_v2_cmdq csq;
	struct hns_roce_v2_cmdq crq;
	u16 tx_timeout;
	enum hns_roce_cmd_status last_status;
};

struct hns_roce_v2_priv {
	int num_vec;
	struct hns_roce_v2_cmd cmd;
};

#endif
