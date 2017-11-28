/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "hclge_mbx.h"
#include "hclgevf_main.h"
#include "hnae3.h"

static void hclgevf_reset_mbx_resp_status(struct hclgevf_dev *hdev)
{
	hdev->mbx_resp.received_resp  = false;
	hdev->mbx_resp.origin_mbx_msg = 0;
	hdev->mbx_resp.resp_status    = 0;
	memset(hdev->mbx_resp.addtional_info, 0, HCLGE_MBX_MAX_RESP_DATA_SIZE);
}

/**
 * hclgevf_get_mbx_resp: used to get a response from PF after VF sends a mailbox
 * message to PF.
 * @hdev: pointer to struct hclgevf_dev
 * @resp_msg: pointer to store the original message type and response status
 * @len: the resp_msg data array length.
 */
int hclgevf_get_mbx_resp(struct hclgevf_dev *hdev,
				u16 code0,
				u16 code1,
				u8 *resp_data,
				u16 resp_len)
{
#define HCLGEVF_MAX_TRY_TIMES	500
#define HCLGEVF_SLEEP_USCOEND	1000
	struct hclgevf_mbx_resp_status *mbx_resp;
	u16 r_code0, r_code1;
	int i = 0;

	if (resp_len > HCLGE_MBX_MAX_RESP_DATA_SIZE) {
		dev_err(&hdev->pdev->dev,
			"VF mbx response len(=%d) exceeds maximum(=%d)\n",
			resp_len,
			HCLGE_MBX_MAX_RESP_DATA_SIZE);
		return -EINVAL;
	}
	while ((!hdev->mbx_resp.received_resp) &&
	       (i < HCLGEVF_MAX_TRY_TIMES)) {
		udelay(HCLGEVF_SLEEP_USCOEND);
		i++;
	}

	if (i >= HCLGEVF_MAX_TRY_TIMES) {
		dev_err(&hdev->pdev->dev,
			"VF could not get mbx resp(=%d) from PF in %d tries\n",
			hdev->mbx_resp.received_resp, i);
		return -EIO;
	}

	mbx_resp = &hdev->mbx_resp;
	r_code0 = (u16)(mbx_resp->origin_mbx_msg >> 16);
	r_code1 = (u16)(mbx_resp->origin_mbx_msg & 0xff);
	if (resp_data) {
		for (i = 0; i < resp_len; i++)
			resp_data[i] = mbx_resp->addtional_info[i];
	}
	hclgevf_reset_mbx_resp_status(hdev);

	if (!(r_code0 == code0 && r_code1 == code1 && !mbx_resp->resp_status)) {
		dev_err(&hdev->pdev->dev,
			"VF could not match resp code(code0=%d,code1=%d), %d",
			code0, code1, mbx_resp->resp_status);
		return -EIO;
	}

	return 0;
}

int hclgevf_send_mbx_msg(struct hclgevf_dev *hdev, u16 code, u16 subcode,
			 const u8 *msg_data, u8 msg_len, bool need_resp,
			 u8 *resp_data, u16 resp_len)
{
	struct hclge_mbx_vf_to_pf_cmd *req;
	struct hclgevf_desc desc;
	int status, i;

	req = (struct hclge_mbx_vf_to_pf_cmd *)desc.data;

	if (msg_len > HCLGE_MBX_MAX_MSG_SIZE) {
		dev_err(&hdev->pdev->dev,
			"VF send mbx msg fail, msg len %d exceeds max len %d\n",
			msg_len, HCLGE_MBX_MAX_MSG_SIZE);
		return -EINVAL;
	}

	hclgevf_cmd_setup_basic_desc(&desc, HCLGEVF_OPC_MBX_VF_TO_PF, false);
	req->msg[0] = code;
	req->msg[1] = subcode;
	for (i = 0; i < msg_len; i++)
		req->msg[i + 2] = msg_data[i];

	/* synchronous send */
	if (need_resp) {
		mutex_lock(&hdev->mbx_resp.mbx_mutex);
		hclgevf_reset_mbx_resp_status(hdev);
		status = hclgevf_cmd_send(&hdev->hw, &desc, 1);
		if (status) {
			dev_err(&hdev->pdev->dev,
				"VF failed(=%d) to send mbx message to PF\n",
				status);
			mutex_unlock(&hdev->mbx_resp.mbx_mutex);
			return status;
		}

		status = hclgevf_get_mbx_resp(hdev, code, subcode, resp_data,
					      resp_len);
		mutex_unlock(&hdev->mbx_resp.mbx_mutex);
	} else {
		/* asynchronous send */
		status = hclgevf_cmd_send(&hdev->hw, &desc, 1);
		if (status) {
			dev_err(&hdev->pdev->dev,
				"VF failed(=%d) to send mbx message to PF\n",
				status);
			return status;
		}
	}

	return status;
}

void hclgevf_mbx_handler(struct hclgevf_dev *hdev)
{
	struct hclgevf_mbx_resp_status *resp;
	struct hclge_mbx_pf_to_vf_cmd *req;
	struct hclgevf_cmq_ring *crq;
	struct hclgevf_desc *desc;
	u16  link_status;
	u8 *temp;
	int i;

	resp = &hdev->mbx_resp;
	crq = &hdev->hw.cmq.crq;

	while (hnae_get_bit(crq->desc[crq->next_to_use].flag,
			    HCLGEVF_CMDQ_RX_OUTVLD_B)) {
		desc = &crq->desc[crq->next_to_use];
		req = (struct hclge_mbx_pf_to_vf_cmd *)desc->data;

		switch (req->msg[0]) {
		case HCLGE_MBX_PF_VF_RESP:
			if (resp->received_resp)
				dev_warn(&hdev->pdev->dev,
					 "VF mbx resp flag not clear(%d)\n",
					 req->msg[1]);

			resp->origin_mbx_msg = (req->msg[1] << 16);
			resp->origin_mbx_msg |= req->msg[2];
			resp->received_resp = true;

			resp->resp_status = req->msg[3];

			temp = (u8 *)&req->msg[4];
			for (i = 0; i < HCLGE_MBX_MAX_RESP_DATA_SIZE; i++) {
				resp->addtional_info[i] = *temp;
				temp++;
			}
			break;
		case HCLGE_MBX_LINK_STAT_CHANGE:
			link_status = le16_to_cpu(req->msg[1]);

			/* update upper layer with new link link status */
			hclgevf_update_link_status(hdev, link_status);

			break;
		default:
			dev_err(&hdev->pdev->dev,
				"VF received unsupported(%d) mbx msg from PF\n",
				req->msg[0]);
			break;
		}
		hclge_mbx_ring_ptr_move_crq(crq, next_to_use);
	}

	/* Write back CMDQ_RQ header pointer, M7 need this pointer */
	hclgevf_write_dev(&hdev->hw, HCLGEVF_NIC_CRQ_HEAD_REG,
			  crq->next_to_use);
}
