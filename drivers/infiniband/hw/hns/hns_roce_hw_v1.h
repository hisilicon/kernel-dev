/*
 * Copyright (c) 2016 Hisilicon Limited.
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

#ifndef _HNS_ROCE_HW_V1_H
#define _HNS_ROCE_HW_V1_H

#define HNS_ROCE_V1_MAX_PD_NUM				0x8000
#define HNS_ROCE_V1_MAX_CQ_NUM				0x10000
#define HNS_ROCE_V1_MAX_CQE_NUM				0x8000

#define HNS_ROCE_V1_MAX_QP_NUM				0x40000
#define HNS_ROCE_V1_MAX_WQE_NUM				0x4000

#define HNS_ROCE_V1_MAX_MTPT_NUM			0x80000

#define HNS_ROCE_V1_MAX_MTT_SEGS			0x100000

#define HNS_ROCE_V1_MAX_QP_INIT_RDMA			128
#define HNS_ROCE_V1_MAX_QP_DEST_RDMA			128

#define HNS_ROCE_V1_MAX_SQ_DESC_SZ			64
#define HNS_ROCE_V1_MAX_RQ_DESC_SZ			64
#define HNS_ROCE_V1_SG_NUM				2
#define HNS_ROCE_V1_INLINE_SIZE				32

#define HNS_ROCE_V1_UAR_NUM				256
#define HNS_ROCE_V1_PHY_UAR_NUM				8

#define HNS_ROCE_V1_GID_NUM				16

#define HNS_ROCE_V1_NUM_COMP_EQE			0x8000
#define HNS_ROCE_V1_NUM_ASYNC_EQE			0x400

#define HNS_ROCE_V1_QPC_ENTRY_SIZE			256
#define HNS_ROCE_V1_IRRL_ENTRY_SIZE			8
#define HNS_ROCE_V1_CQC_ENTRY_SIZE			64
#define HNS_ROCE_V1_MTPT_ENTRY_SIZE			64
#define HNS_ROCE_V1_MTT_ENTRY_SIZE			64

#define HNS_ROCE_V1_CQE_ENTRY_SIZE			32
#define HNS_ROCE_V1_PAGE_SIZE_SUPPORT			0xFFFFF000

#define HNS_ROCE_V1_EXT_RAQ_WF				8
#define HNS_ROCE_V1_RAQ_ENTRY				64
#define HNS_ROCE_V1_RAQ_DEPTH				32768
#define HNS_ROCE_V1_RAQ_SIZE	(HNS_ROCE_V1_RAQ_ENTRY * HNS_ROCE_V1_RAQ_DEPTH)

#define HNS_ROCE_V1_SDB_DEPTH				0x400
#define HNS_ROCE_V1_ODB_DEPTH				0x400

#define HNS_ROCE_V1_DB_RSVD				0x80

#define HNS_ROCE_V1_SDB_ALEPT				HNS_ROCE_V1_DB_RSVD
#define HNS_ROCE_V1_SDB_ALFUL	(HNS_ROCE_V1_SDB_DEPTH - HNS_ROCE_V1_DB_RSVD)
#define HNS_ROCE_V1_ODB_ALEPT				HNS_ROCE_V1_DB_RSVD
#define HNS_ROCE_V1_ODB_ALFUL	(HNS_ROCE_V1_ODB_DEPTH - HNS_ROCE_V1_DB_RSVD)

#define HNS_ROCE_V1_EXT_SDB_DEPTH			0x4000
#define HNS_ROCE_V1_EXT_ODB_DEPTH			0x4000
#define HNS_ROCE_V1_EXT_SDB_ENTRY			16
#define HNS_ROCE_V1_EXT_ODB_ENTRY			16
#define HNS_ROCE_V1_EXT_SDB_SIZE  \
	(HNS_ROCE_V1_EXT_SDB_DEPTH * HNS_ROCE_V1_EXT_SDB_ENTRY)
#define HNS_ROCE_V1_EXT_ODB_SIZE  \
	(HNS_ROCE_V1_EXT_ODB_DEPTH * HNS_ROCE_V1_EXT_ODB_ENTRY)

#define HNS_ROCE_V1_EXT_SDB_ALEPT			HNS_ROCE_V1_DB_RSVD
#define HNS_ROCE_V1_EXT_SDB_ALFUL  \
	(HNS_ROCE_V1_EXT_SDB_DEPTH - HNS_ROCE_V1_DB_RSVD)
#define HNS_ROCE_V1_EXT_ODB_ALEPT			HNS_ROCE_V1_DB_RSVD
#define HNS_ROCE_V1_EXT_ODB_ALFUL	\
	(HNS_ROCE_V1_EXT_ODB_DEPTH - HNS_ROCE_V1_DB_RSVD)

#define HNS_ROCE_ODB_POLL_MODE				0

#define HNS_ROCE_SDB_NORMAL_MODE			0
#define HNS_ROCE_SDB_EXTEND_MODE			1

#define HNS_ROCE_ODB_EXTEND_MODE			1

#define PHY_PORT_OFFSET					0x8
#define ALL_PORT_VAL_OPEN				0x3f
#define POL_TIME_INTERVAL_VAL				0x80
#define SLEEP_TIME_INTERVAL				20

struct hns_roce_ext_db {
	int esdb_dep;
	int eodb_dep;
	struct hns_roce_buf_list *sdb_buf_list;
	struct hns_roce_buf_list *odb_buf_list;
};

struct hns_roce_db_table {
	int  sdb_ext_mod;
	int  odb_ext_mod;
	struct hns_roce_ext_db *ext_db;
};

struct hns_roce_v1_priv {
	struct hns_roce_db_table  db_table;
	struct hns_roce_raq_table raq_table;
};

int hns_dsaf_roce_reset(struct fwnode_handle *dsaf_fwnode, bool enable);

#endif
