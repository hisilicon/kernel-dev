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

#define HNS_ROCE_CMDQ_TX_TIMEOUT		200

#define HNS_ROCE_CMQ_DESC_NUM_S		3
#define HNS_ROCE_CMQ_EN_B		16
#define HNS_ROCE_CMQ_ENABLE		BIT(HNS_ROCE_CMQ_EN_B)

enum {
	TYPE_CRQ,
	TYPE_CSQ,
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
