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

#include <asm/page.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <rdma/ib_smi.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_user_verbs.h>
#include "hns_roce_common.h"
#include "hns_roce_device.h"

int hns_roce_init_pd_table(struct hns_roce_dev *hr_dev)
{
	return hns_roce_bitmap_init(&hr_dev->pd_bitmap, hr_dev->caps.num_pds,
				    hr_dev->caps.num_pds - 1,
				    hr_dev->caps.reserved_pds, 0);
}

void hns_roce_cleanup_pd_table(struct hns_roce_dev *hr_dev)
{
	hns_roce_bitmap_cleanup(&hr_dev->pd_bitmap);
}

int hns_roce_uar_alloc(struct hns_roce_dev *hr_dev, struct hns_roce_uar *uar)
{
	struct resource *res;
	int ret = 0;
	/* Using bitmap to manager UAR index */
	ret = hns_roce_bitmap_alloc(&hr_dev->uar_table.bitmap, &uar->index);
	if (ret == -1)
		return -ENOMEM;

	uar->index = (uar->index - 1) % hr_dev->caps.phy_num_uars + 1;

	res = platform_get_resource(hr_dev->pdev, IORESOURCE_MEM, 0);
	uar->pfn = ((res->start) >> PAGE_SHIFT) + uar->index;

	return 0;
}

void hns_roce_uar_free(struct hns_roce_dev *hr_dev, struct hns_roce_uar *uar)
{
	hns_roce_bitmap_free(&hr_dev->uar_table.bitmap, uar->index);
}

int hns_roce_init_uar_table(struct hns_roce_dev *hr_dev)
{
	return hns_roce_bitmap_init(&hr_dev->uar_table.bitmap,
				    hr_dev->caps.num_uars,
				    hr_dev->caps.num_uars - 1,
				    hr_dev->caps.reserved_uars, 0);
}

void hns_roce_cleanup_uar_table(struct hns_roce_dev *hr_dev)
{
	hns_roce_bitmap_cleanup(&hr_dev->uar_table.bitmap);
}
