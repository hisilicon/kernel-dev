/*
 * Copyright (c) 2016 Hisilicon Limited.
 * Copyright (c) 2007, 2008 Mellanox Technologies. All rights reserved.
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

#include <linux/bitmap.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "hns_roce_device.h"

int hns_roce_bitmap_alloc(struct hns_roce_bitmap *bitmap, unsigned long *obj)
{
	int ret = 0;

	spin_lock(&bitmap->lock);
	*obj = find_next_zero_bit(bitmap->table, bitmap->max, bitmap->last);
	if (*obj >= bitmap->max) {
		bitmap->top = (bitmap->top + bitmap->max + bitmap->reserved_top)
			       & bitmap->mask;
		*obj = find_first_zero_bit(bitmap->table, bitmap->max);
	}

	if (*obj < bitmap->max) {
		set_bit(*obj, bitmap->table);
		bitmap->last = (*obj + 1);
		if (bitmap->last == bitmap->max)
			bitmap->last = 0;
		*obj |= bitmap->top;
	} else {
		ret = -1;
	}

	spin_unlock(&bitmap->lock);

	return ret;
}

void hns_roce_bitmap_free(struct hns_roce_bitmap *bitmap, unsigned long obj)
{
	hns_roce_bitmap_free_range(bitmap, obj, 1);
}

void hns_roce_bitmap_free_range(struct hns_roce_bitmap *bitmap,
				unsigned long obj, int cnt)
{
	int i;

	obj &= bitmap->max + bitmap->reserved_top - 1;

	spin_lock(&bitmap->lock);
	for (i = 0; i < cnt; i++)
		clear_bit(obj + i, bitmap->table);

	bitmap->last = min(bitmap->last, obj);
	bitmap->top = (bitmap->top + bitmap->max + bitmap->reserved_top)
		       & bitmap->mask;
	spin_unlock(&bitmap->lock);
}

int hns_roce_bitmap_init(struct hns_roce_bitmap *bitmap, u32 num, u32 mask,
			 u32 reserved_bot, u32 reserved_top)
{
	u32 i;

	if (num != roundup_pow_of_two(num))
		return -EINVAL;

	bitmap->last = 0;
	bitmap->top = 0;
	bitmap->max = num - reserved_top;
	bitmap->mask = mask;
	bitmap->reserved_top = reserved_top;
	spin_lock_init(&bitmap->lock);
	bitmap->table = kcalloc(BITS_TO_LONGS(bitmap->max), sizeof(long),
				GFP_KERNEL);
	if (!bitmap->table)
		return -ENOMEM;

	for (i = 0; i < reserved_bot; ++i)
		set_bit(i, bitmap->table);

	return 0;
}

void hns_roce_bitmap_cleanup(struct hns_roce_bitmap *bitmap)
{
	kfree(bitmap->table);
}

void hns_roce_cleanup_bitmap(struct hns_roce_dev *hr_dev)
{
	hns_roce_cleanup_qp_table(hr_dev);
	hns_roce_cleanup_cq_table(hr_dev);
	hns_roce_cleanup_mr_table(hr_dev);
	hns_roce_cleanup_pd_table(hr_dev);
	hns_roce_cleanup_uar_table(hr_dev);
}
