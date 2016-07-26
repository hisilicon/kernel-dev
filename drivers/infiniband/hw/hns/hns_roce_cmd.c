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

#include <linux/dmapool.h>
#include <linux/platform_device.h>
#include "hns_roce_common.h"
#include "hns_roce_device.h"
#include "hns_roce_cmd.h"

#define CMD_MAX_NUM		32
#define CMD_TOKEN_MASK		0x1f

void hns_roce_cmd_event(struct hns_roce_dev *hr_dev, u16 token, u8 status,
			u64 out_param)
{
	struct hns_roce_cmd_context
		*context = &hr_dev->cmd.context[token & hr_dev->cmd.token_mask];

	if (token != context->token)
		return;

	context->result = (status == HNS_ROCE_CMD_SUCCESS) ? 0 : (-EIO);
	context->out_param = out_param;
	complete(&context->done);
}

int hns_roce_cmd_init(struct hns_roce_dev *hr_dev)
{
	struct device *dev = &hr_dev->pdev->dev;

	mutex_init(&hr_dev->cmd.hcr_mutex);
	sema_init(&hr_dev->cmd.poll_sem, 1);
	hr_dev->cmd.use_events = 0;
	hr_dev->cmd.toggle = 1;
	hr_dev->cmd.max_cmds = CMD_MAX_NUM;
	hr_dev->cmd.hcr = hr_dev->reg_base + ROCEE_MB1_REG;
	hr_dev->cmd.pool = dma_pool_create("hns_roce_cmd", dev,
					   HNS_ROCE_MAILBOX_SIZE,
					   HNS_ROCE_MAILBOX_SIZE, 0);
	if (!hr_dev->cmd.pool)
		return -ENOMEM;

	return 0;
}

void hns_roce_cmd_cleanup(struct hns_roce_dev *hr_dev)
{
	dma_pool_destroy(hr_dev->cmd.pool);
}

int hns_roce_cmd_use_events(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq *hr_cmd = &hr_dev->cmd;
	int i;

	hr_cmd->context = kmalloc(hr_cmd->max_cmds *
				  sizeof(struct hns_roce_cmd_context),
				  GFP_KERNEL);
	if (!hr_cmd->context)
		return -ENOMEM;

	for (i = 0; i < hr_cmd->max_cmds; ++i) {
		hr_cmd->context[i].token = i;
		hr_cmd->context[i].next = i + 1;
	}

	hr_cmd->context[hr_cmd->max_cmds - 1].next = -1;
	hr_cmd->free_head = 0;

	sema_init(&hr_cmd->event_sem, hr_cmd->max_cmds);
	spin_lock_init(&hr_cmd->context_lock);

	hr_cmd->token_mask = CMD_TOKEN_MASK;
	hr_cmd->use_events = 1;

	down(&hr_cmd->poll_sem);

	return 0;
}

void hns_roce_cmd_use_polling(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq *hr_cmd = &hr_dev->cmd;
	int i;

	hr_cmd->use_events = 0;

	for (i = 0; i < hr_cmd->max_cmds; ++i)
		down(&hr_cmd->event_sem);

	kfree(hr_cmd->context);
	up(&hr_cmd->poll_sem);
}
