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

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "hns_roce_device.h"
#include "hns_roce_icm.h"

static int hns_roce_buddy_alloc(struct hns_roce_buddy *buddy, int order,
				unsigned long *seg)
{
	int o;
	u32 m;

	spin_lock(&buddy->lock);

	for (o = order; o <= buddy->max_order; ++o) {
		if (buddy->num_free[o]) {
			m = 1 << (buddy->max_order - o);
			*seg = find_first_bit(buddy->bits[o], m);
			if (*seg < m)
				goto found;
		}
	}
	spin_unlock(&buddy->lock);
	return -1;

 found:
	clear_bit(*seg, buddy->bits[o]);
	--buddy->num_free[o];

	while (o > order) {
		--o;
		*seg <<= 1;
		set_bit(*seg ^ 1, buddy->bits[o]);
		++buddy->num_free[o];
	}

	spin_unlock(&buddy->lock);

	*seg <<= order;
	return 0;
}

static void hns_roce_buddy_free(struct hns_roce_buddy *buddy, unsigned long seg,
				int order)
{
	seg >>= order;

	spin_lock(&buddy->lock);

	while (test_bit(seg ^ 1, buddy->bits[order])) {
		clear_bit(seg ^ 1, buddy->bits[order]);
		--buddy->num_free[order];
		seg >>= 1;
		++order;
	}

	set_bit(seg, buddy->bits[order]);
	++buddy->num_free[order];

	spin_unlock(&buddy->lock);
}

static int hns_roce_buddy_init(struct hns_roce_buddy *buddy, int max_order)
{
	int i, s;

	buddy->max_order = max_order;
	spin_lock_init(&buddy->lock);

	buddy->bits = kzalloc((buddy->max_order + 1) * sizeof(long *),
			       GFP_KERNEL);
	buddy->num_free = kzalloc((buddy->max_order + 1) * sizeof(int *),
				   GFP_KERNEL);
	if (!buddy->bits || !buddy->num_free)
		goto err_out;

	for (i = 0; i <= buddy->max_order; ++i) {
		s = BITS_TO_LONGS(1 << (buddy->max_order - i));
		buddy->bits[i] = kmalloc_array(s, sizeof(long), GFP_KERNEL);
		if (!buddy->bits[i])
			goto err_out_free;

		bitmap_zero(buddy->bits[i], 1 << (buddy->max_order - i));
	}

	set_bit(0, buddy->bits[buddy->max_order]);
	buddy->num_free[buddy->max_order] = 1;

	return 0;

err_out_free:
	for (i = 0; i <= buddy->max_order; ++i)
		kfree(buddy->bits[i]);

err_out:
	kfree(buddy->bits);
	kfree(buddy->num_free);
	return -ENOMEM;
}

static void hns_roce_buddy_cleanup(struct hns_roce_buddy *buddy)
{
	int i;

	for (i = 0; i <= buddy->max_order; ++i)
		kfree(buddy->bits[i]);

	kfree(buddy->bits);
	kfree(buddy->num_free);
}

static int hns_roce_alloc_mtt_range(struct hns_roce_dev *hr_dev, int order,
				    unsigned long *seg)
{
	struct hns_roce_mr_table *mr_table = &hr_dev->mr_table;
	int ret = 0;

	ret = hns_roce_buddy_alloc(&mr_table->mtt_buddy, order, seg);
	if (ret == -1)
		return -1;

	if (hns_roce_table_get_range(hr_dev, &mr_table->mtt_table, *seg,
				     *seg + (1 << order) - 1)) {
		hns_roce_buddy_free(&mr_table->mtt_buddy, *seg, order);
		return -1;
	}

	return 0;
}

int hns_roce_mtt_init(struct hns_roce_dev *hr_dev, int npages, int page_shift,
		      struct hns_roce_mtt *mtt)
{
	int ret = 0;
	int i;

	/* Page num is zero, correspond to DMA memory register */
	if (!npages) {
		mtt->order = -1;
		mtt->page_shift = HNS_ROCE_ICM_PAGE_SHIFT;
		return 0;
	}

	/* Note: if page_shift is zero, FAST memory regsiter */
	mtt->page_shift = page_shift;

	/* Compute MTT entry necessary */
	for (mtt->order = 0, i = HNS_ROCE_MTT_ENTRY_PER_SEG; i < npages;
	     i <<= 1)
		++mtt->order;

	/* Allocate MTT entry */
	ret = hns_roce_alloc_mtt_range(hr_dev, mtt->order, &mtt->first_seg);
	if (ret == -1)
		return -ENOMEM;

	return 0;
}

void hns_roce_mtt_cleanup(struct hns_roce_dev *hr_dev, struct hns_roce_mtt *mtt)
{
	struct hns_roce_mr_table *mr_table = &hr_dev->mr_table;

	if (mtt->order < 0)
		return;

	hns_roce_buddy_free(&mr_table->mtt_buddy, mtt->first_seg, mtt->order);
	hns_roce_table_put_range(hr_dev, &mr_table->mtt_table, mtt->first_seg,
				 mtt->first_seg + (1 << mtt->order) - 1);
}

static int hns_roce_write_mtt_chunk(struct hns_roce_dev *hr_dev,
				    struct hns_roce_mtt *mtt, u32 start_index,
				    u32 npages, u64 *page_list)
{
	u32 i = 0;
	__le64 *mtts = NULL;
	dma_addr_t dma_handle;
	u32 s = start_index * sizeof(u64);

	/* All MTTs must fit in the same page */
	if (start_index / (PAGE_SIZE / sizeof(u64)) !=
		(start_index + npages - 1) / (PAGE_SIZE / sizeof(u64)))
		return -EINVAL;

	if (start_index & (HNS_ROCE_MTT_ENTRY_PER_SEG - 1))
		return -EINVAL;

	mtts = hns_roce_table_find(&hr_dev->mr_table.mtt_table,
				mtt->first_seg + s / hr_dev->caps.mtt_entry_sz,
				&dma_handle);
	if (!mtts)
		return -ENOMEM;

	/* Save page addr, low 12 bits : 0 */
	for (i = 0; i < npages; ++i)
		mtts[i] = (cpu_to_le64(page_list[i])) >> PAGE_ADDR_SHIFT;

	return 0;
}

int hns_roce_write_mtt(struct hns_roce_dev *hr_dev, struct hns_roce_mtt *mtt,
		       u32 start_index, u32 npages, u64 *page_list)
{
	int chunk;
	int ret;

	if (mtt->order < 0)
		return -EINVAL;

	while (npages > 0) {
		chunk = min_t(int, PAGE_SIZE / sizeof(u64), npages);

		ret = hns_roce_write_mtt_chunk(hr_dev, mtt, start_index, chunk,
					       page_list);
		if (ret)
			return ret;

		npages -= chunk;
		start_index += chunk;
		page_list += chunk;
	}

	return 0;
}

int hns_roce_buf_write_mtt(struct hns_roce_dev *hr_dev,
			   struct hns_roce_mtt *mtt, struct hns_roce_buf *buf)
{
	u32 i = 0;
	int ret = 0;
	u64 *page_list = NULL;

	page_list = kmalloc_array(buf->npages, sizeof(*page_list), GFP_KERNEL);
	if (!page_list)
		return -ENOMEM;

	for (i = 0; i < buf->npages; ++i) {
		if (buf->nbufs == 1)
			page_list[i] = buf->direct.map + (i << buf->page_shift);
		else
			page_list[i] = buf->page_list[i].map;

	}
	ret = hns_roce_write_mtt(hr_dev, mtt, 0, buf->npages, page_list);

	kfree(page_list);

	return ret;
}

int hns_roce_init_mr_table(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_mr_table *mr_table = &hr_dev->mr_table;
	struct device *dev = &hr_dev->pdev->dev;
	unsigned long first_seg;
	int ret = 0;

	ret = hns_roce_bitmap_init(&mr_table->mtpt_bitmap,
				   hr_dev->caps.num_mtpts,
				   hr_dev->caps.num_mtpts - 1,
				   hr_dev->caps.reserved_mrws, 0);
	if (ret)
		return ret;

	ret = hns_roce_buddy_init(&mr_table->mtt_buddy,
				  ilog2(hr_dev->caps.num_mtt_segs));
	if (ret)
		goto err_buddy;

	if (hr_dev->caps.reserved_mtts) {
		if (hns_roce_alloc_mtt_range(hr_dev,
			fls(hr_dev->caps.reserved_mtts - 1),
			&first_seg) == -1) {
			dev_err(dev, "MTT table of order %d is too small.\n",
				mr_table->mtt_buddy.max_order);
			ret = -ENOMEM;
			goto err_reserve_mtts;
		}
	}

	return 0;

err_reserve_mtts:
	hns_roce_buddy_cleanup(&mr_table->mtt_buddy);

err_buddy:
	hns_roce_bitmap_cleanup(&mr_table->mtpt_bitmap);
	return ret;
}

void hns_roce_cleanup_mr_table(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_mr_table *mr_table = &hr_dev->mr_table;

	hns_roce_buddy_cleanup(&mr_table->mtt_buddy);
	hns_roce_bitmap_cleanup(&mr_table->mtpt_bitmap);
}

int hns_roce_ib_umem_write_mtt(struct hns_roce_dev *hr_dev,
			       struct hns_roce_mtt *mtt, struct ib_umem *umem)
{
	struct scatterlist *sg;
	int i, k, entry;
	int ret = 0;
	u64 *pages;
	u32 n;
	int len;

	pages = (u64 *) __get_free_page(GFP_KERNEL);
	if (!pages)
		return -ENOMEM;

	i = n = 0;

	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, entry) {
		len = sg_dma_len(sg) >> mtt->page_shift;
		for (k = 0; k < len; ++k) {
			pages[i++] = sg_dma_address(sg) + umem->page_size * k;
			if (i == PAGE_SIZE / sizeof(u64)) {
				ret = hns_roce_write_mtt(hr_dev, mtt, n, i,
							 pages);
				if (ret)
					goto out;
				n += i;
				i = 0;
			}
		}
	}

	if (i)
		ret = hns_roce_write_mtt(hr_dev, mtt, n, i, pages);

out:
	free_page((unsigned long) pages);
	return ret;
}
