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

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include "hns_roce_device.h"
#include "hns_roce_cmd.h"
#include "hns_roce_icm.h"
#include "hns_roce_common.h"

#define HW_SYNC_TIMEOUT_MSECS		500

#define HNS_ROCE_ICM_ALLOC_SIZE		(1 << 17)
#define HNS_ROCE_TABLE_CHUNK_SIZE	(1 << 17)

#define DMA_ADDR_T_SHIFT		12
#define BT_CMD_SYNC_SHIFT		31
#define BT_BA_SHIFT			32

static int hns_roce_alloc_icm_pages(struct scatterlist *mem, int order,
				    gfp_t gfp_mask)
{
	struct page *page;

	page = alloc_pages(gfp_mask, order);
	if (!page)
		return -ENOMEM;

	sg_set_page(mem, page, PAGE_SIZE << order, 0);

	return 0;
}

static int hns_roce_alloc_icm_coherent(struct device *dev,
				       struct scatterlist *mem, int order,
				       gfp_t gfp_mask)
{
	void *buf = dma_alloc_coherent(dev, PAGE_SIZE << order,
				       &sg_dma_address(mem), gfp_mask);
	if (!buf)
		return -ENOMEM;

	sg_set_buf(mem, buf, PAGE_SIZE << order);
	WARN_ON(mem->offset);
	sg_dma_len(mem) = PAGE_SIZE << order;
	return 0;
}

struct hns_roce_icm *hns_roce_alloc_icm(struct hns_roce_dev *hr_dev, int npages,
					gfp_t gfp_mask, int coherent)
{
	struct hns_roce_icm_chunk *chunk = NULL;
	struct hns_roce_icm *icm;
	int cur_order;
	int ret;

	WARN_ON(coherent && (gfp_mask & __GFP_HIGHMEM));

	icm = kmalloc(sizeof(*icm),
		      gfp_mask & ~(__GFP_HIGHMEM | __GFP_NOWARN));
	if (!icm)
		return NULL;

	icm->refcount = 0;
	INIT_LIST_HEAD(&icm->chunk_list);

	cur_order = get_order(HNS_ROCE_ICM_ALLOC_SIZE);

	while (npages > 0) {
		if (!chunk) {
			chunk = kmalloc(sizeof(*chunk),
				gfp_mask & ~(__GFP_HIGHMEM | __GFP_NOWARN));
			if (!chunk)
				goto fail;

			sg_init_table(chunk->mem, HNS_ROCE_ICM_CHUNK_LEN);
			chunk->npages = 0;
			chunk->nsg = 0;
			list_add_tail(&chunk->list, &icm->chunk_list);
		}

		while (1 << cur_order > npages)
			--cur_order;

		if (coherent)
			ret = hns_roce_alloc_icm_coherent(&hr_dev->pdev->dev,
						&chunk->mem[chunk->npages],
						cur_order, gfp_mask);
		else
			ret = hns_roce_alloc_icm_pages(
						&chunk->mem[chunk->npages],
						cur_order, gfp_mask);
		if (!ret) {
			++chunk->npages;
			if (coherent) {
				++chunk->nsg;
			} else if (chunk->npages == HNS_ROCE_ICM_CHUNK_LEN) {
				chunk->nsg = dma_map_sg(&hr_dev->pdev->dev,
						      chunk->mem, chunk->npages,
						      DMA_BIDIRECTIONAL);
				if (chunk->nsg <= 0)
					goto fail;

				chunk = NULL;
			}
			npages -= 1 << cur_order;
		} else {
			/*
			* If failed on alloc 128k memory one time,
			* no alloc small block memory,
			* directly return fail
			*/
			goto fail;
		}
	}

	if (!coherent && chunk) {
		chunk->nsg = dma_map_sg(&hr_dev->pdev->dev, chunk->mem,
					chunk->npages, DMA_BIDIRECTIONAL);
		if (chunk->nsg <= 0)
			goto fail;
	}

	return icm;

fail:
	hns_roce_free_icm(hr_dev, icm, coherent);
	return NULL;
}

static void hns_roce_free_icm_pages(struct hns_roce_dev *hr_dev,
				    struct hns_roce_icm_chunk *chunk)
{
	int i;

	if (chunk->nsg > 0)
		dma_unmap_sg(&hr_dev->pdev->dev, chunk->mem, chunk->npages,
			     DMA_BIDIRECTIONAL);

	for (i = 0; i < chunk->npages; ++i)
		__free_pages(sg_page(&chunk->mem[i]),
			     get_order(chunk->mem[i].length));
}

static void hns_roce_free_icm_coherent(struct hns_roce_dev *hr_dev,
				       struct hns_roce_icm_chunk *chunk)
{
	int i;

	for (i = 0; i < chunk->npages; ++i)
		dma_free_coherent(&hr_dev->pdev->dev, chunk->mem[i].length,
				  lowmem_page_address(sg_page(&chunk->mem[i])),
				  sg_dma_address(&chunk->mem[i]));
}

void hns_roce_free_icm(struct hns_roce_dev *hr_dev, struct hns_roce_icm *icm,
		       int coherent)
{
	struct hns_roce_icm_chunk *chunk, *tmp;

	if (!icm)
		return;

	list_for_each_entry_safe(chunk, tmp, &icm->chunk_list, list) {
		if (coherent)
			hns_roce_free_icm_coherent(hr_dev, chunk);
		else
			hns_roce_free_icm_pages(hr_dev, chunk);

		kfree(chunk);
	}

	kfree(icm);
}

static int hns_roce_map_icm(struct hns_roce_dev *hr_dev,
			    struct hns_roce_icm_table *table, unsigned long obj)
{
	struct device *dev = &hr_dev->pdev->dev;
	spinlock_t *lock = &hr_dev->bt_cmd_lock;
	unsigned long end = 0;
	unsigned long flags;
	struct hns_roce_icm_iter iter;
	void __iomem *bt_cmd;
	u32 bt_cmd_h_val = 0;
	u32 bt_cmd_val[2];
	u32 bt_cmd_l = 0;
	u64 bt_ba = 0;
	int ret = 0;

	/* Find the icm entry */
	unsigned long i = (obj & (table->num_obj - 1)) /
			  (HNS_ROCE_TABLE_CHUNK_SIZE / table->obj_size);

	switch (table->type) {
	case ICM_TYPE_QPC:
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S, ICM_TYPE_QPC);
		break;
	case ICM_TYPE_MTPT:
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S,
			       ICM_TYPE_MTPT);
		break;
	case ICM_TYPE_CQC:
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S, ICM_TYPE_CQC);
		break;
	case ICM_TYPE_SRQC:
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S,
			       ICM_TYPE_SRQC);
		break;
	default:
		return ret;
	}
	roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_IN_MDF_M,
		       ROCEE_BT_CMD_H_ROCEE_BT_CMD_IN_MDF_S, obj);
	roce_set_bit(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_S, 0);
	roce_set_bit(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_HW_SYNS_S, 1);

	/* Currently iter only a chunk */
	for (hns_roce_icm_first(table->icm[i], &iter);
	     !hns_roce_icm_last(&iter); hns_roce_icm_next(&iter)) {
		bt_ba = hns_roce_icm_addr(&iter) >> DMA_ADDR_T_SHIFT;

		spin_lock_irqsave(lock, flags);

		bt_cmd = hr_dev->reg_base + ROCEE_BT_CMD_H_REG;

		end = msecs_to_jiffies(HW_SYNC_TIMEOUT_MSECS) + jiffies;
		while (1) {
			if (readl(bt_cmd) >> BT_CMD_SYNC_SHIFT) {
				if (!(time_before(jiffies, end))) {
					dev_err(dev, "Write bt_cmd err,hw_sync is not zero.\n");
					spin_unlock_irqrestore(lock, flags);
					ret = -EBUSY;
					return ret;
				}
			} else {
				break;
			}
			msleep(20);
		}

		bt_cmd_l = (u32)bt_ba;
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_BA_H_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_BA_H_S,
			       bt_ba >> BT_BA_SHIFT);

		bt_cmd_val[0] = bt_cmd_l;
		bt_cmd_val[1] = bt_cmd_h_val;
		hns_roce_write64_k(bt_cmd_val,
				   hr_dev->reg_base + ROCEE_BT_CMD_L_REG);
		spin_unlock_irqrestore(lock, flags);
	}

	return ret;
}

static int hns_roce_unmap_icm(struct hns_roce_dev *hr_dev,
			      struct hns_roce_icm_table *table,
			      unsigned long obj)
{
	struct device *dev = &hr_dev->pdev->dev;
	unsigned long end = 0;
	unsigned long flags;
	void __iomem *bt_cmd;
	uint32_t bt_cmd_val[2];
	u32 bt_cmd_h_val = 0;
	int ret = 0;

	switch (table->type) {
	case ICM_TYPE_QPC:
		dev_dbg(dev, "UNMAP QPC BT  :\n");
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S, ICM_TYPE_QPC);
		break;
	case ICM_TYPE_MTPT:
		dev_dbg(dev, "UNMAP MTPT BT :\n");
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S,
			       ICM_TYPE_MTPT);
		break;
	case ICM_TYPE_CQC:
		dev_dbg(dev, "UNMAP CQC BT  :\n");
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S, ICM_TYPE_CQC);
		break;
	case ICM_TYPE_SRQC:
		roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_M,
			       ROCEE_BT_CMD_H_ROCEE_BT_CMD_MDF_S,
			       ICM_TYPE_SRQC);
		break;
	default:
		return ret;
	}
	roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_IN_MDF_M,
		       ROCEE_BT_CMD_H_ROCEE_BT_CMD_IN_MDF_S, obj);
	roce_set_bit(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_S, 1);
	roce_set_bit(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_HW_SYNS_S, 1);
	roce_set_field(bt_cmd_h_val, ROCEE_BT_CMD_H_ROCEE_BT_CMD_BA_H_M,
		       ROCEE_BT_CMD_H_ROCEE_BT_CMD_BA_H_S, 0);

	spin_lock_irqsave(&hr_dev->bt_cmd_lock, flags);

	bt_cmd = hr_dev->reg_base + ROCEE_BT_CMD_H_REG;

	end = msecs_to_jiffies(HW_SYNC_TIMEOUT_MSECS) + jiffies;
	while (1) {
		if (readl(bt_cmd) >> BT_CMD_SYNC_SHIFT) {
			if (!(time_before(jiffies, end))) {
				dev_err(dev, "Write bt_cmd err,hw_sync is not zero.\n");
				spin_unlock_irqrestore(&hr_dev->bt_cmd_lock,
						       flags);
				return -EBUSY;
			}
		} else {
			break;
		}
		msleep(20);
	}

	bt_cmd_val[0] = 0;
	bt_cmd_val[1] = bt_cmd_h_val;
	hns_roce_write64_k(bt_cmd_val, hr_dev->reg_base + ROCEE_BT_CMD_L_REG);
	spin_unlock_irqrestore(&hr_dev->bt_cmd_lock, flags);

	return ret;
}

int hns_roce_table_get(struct hns_roce_dev *hr_dev,
		       struct hns_roce_icm_table *table, unsigned long obj)
{
	struct device *dev = &hr_dev->pdev->dev;
	int ret = 0;
	unsigned long i;

	i = (obj & (table->num_obj - 1)) / (HNS_ROCE_TABLE_CHUNK_SIZE /
	     table->obj_size);

	mutex_lock(&table->mutex);

	if (table->icm[i]) {
		++table->icm[i]->refcount;
		goto out;
	}

	table->icm[i] = hns_roce_alloc_icm(hr_dev,
				  HNS_ROCE_TABLE_CHUNK_SIZE >> PAGE_SHIFT,
				  (table->lowmem ? GFP_KERNEL :
						   GFP_HIGHUSER) | __GFP_NOWARN,
				  table->coherent);
	if (!table->icm[i]) {
		ret = -ENOMEM;
		goto out;
	}

	/* Inform icm entry mem pa(128K/page, pa starting address)for hw */
	if (hns_roce_map_icm(hr_dev, table, obj)) {
		ret = -ENODEV;
		dev_err(dev, "map icm table failed.\n");
		goto out;
	}

	++table->icm[i]->refcount;
out:
	mutex_unlock(&table->mutex);
	return ret;
}

void hns_roce_table_put(struct hns_roce_dev *hr_dev,
			struct hns_roce_icm_table *table, unsigned long obj)
{
	struct device *dev = &hr_dev->pdev->dev;
	unsigned long i;

	i = (obj & (table->num_obj - 1)) /
	    (HNS_ROCE_TABLE_CHUNK_SIZE / table->obj_size);

	mutex_lock(&table->mutex);

	if (--table->icm[i]->refcount == 0) {
		/* Clear base address table */
		if (hns_roce_unmap_icm(hr_dev, table, obj))
			dev_warn(dev, "unmap icm table failed.\n");

		hns_roce_free_icm(hr_dev, table->icm[i], table->coherent);
		table->icm[i] = NULL;
	}

	mutex_unlock(&table->mutex);
}

void *hns_roce_table_find(struct hns_roce_icm_table *table, unsigned long obj,
			  dma_addr_t *dma_handle)
{
	struct hns_roce_icm_chunk *chunk;
	unsigned long idx;
	int i;
	int offset, dma_offset;
	struct hns_roce_icm *icm;
	struct page *page = NULL;

	if (!table->lowmem)
		return NULL;

	mutex_lock(&table->mutex);
	idx = (obj & (table->num_obj - 1)) * table->obj_size;
	icm = table->icm[idx / HNS_ROCE_TABLE_CHUNK_SIZE];
	dma_offset = offset = idx % HNS_ROCE_TABLE_CHUNK_SIZE;

	if (!icm)
		goto out;

	list_for_each_entry(chunk, &icm->chunk_list, list) {
		for (i = 0; i < chunk->npages; ++i) {
			if (dma_handle && dma_offset >= 0) {
				if (sg_dma_len(&chunk->mem[i]) >
				    (u32)dma_offset)
					*dma_handle = sg_dma_address(
						&chunk->mem[i]) + dma_offset;
				dma_offset -= sg_dma_len(&chunk->mem[i]);
			}

			if (chunk->mem[i].length > (u32)offset) {
				page = sg_page(&chunk->mem[i]);
				goto out;
			}
			offset -= chunk->mem[i].length;
		}
	}

out:
	mutex_unlock(&table->mutex);
	return page ? lowmem_page_address(page) + offset : NULL;
}

int hns_roce_table_get_range(struct hns_roce_dev *hr_dev,
			     struct hns_roce_icm_table *table,
			     unsigned long start, unsigned long end)
{
	unsigned long inc = HNS_ROCE_TABLE_CHUNK_SIZE / table->obj_size;
	unsigned long i = 0;
	int ret = 0;

	/* Allocate MTT entry memory according to chunk(128K) */
	for (i = start; i <= end; i += inc) {
		ret = hns_roce_table_get(hr_dev, table, i);
		if (ret)
			goto fail;
	}

	return 0;

fail:
	while (i > start) {
		i -= inc;
		hns_roce_table_put(hr_dev, table, i);
	}
	return ret;
}

void hns_roce_table_put_range(struct hns_roce_dev *hr_dev,
			      struct hns_roce_icm_table *table,
			      unsigned long start, unsigned long end)
{
	unsigned long i;

	for (i = start; i <= end;
		i += HNS_ROCE_TABLE_CHUNK_SIZE / table->obj_size)
		hns_roce_table_put(hr_dev, table, i);
}

int hns_roce_init_icm_table(struct hns_roce_dev *hr_dev,
			    struct hns_roce_icm_table *table, u32 type,
			    unsigned long obj_size, unsigned long nobj,
			    int reserved, int use_lowmem, int use_coherent)
{
	struct device *dev = &hr_dev->pdev->dev;
	u32 chunk_size;
	unsigned long obj_per_chunk;
	unsigned long num_icm;
	unsigned long i;

	obj_per_chunk = HNS_ROCE_TABLE_CHUNK_SIZE / obj_size;
	num_icm = (nobj + obj_per_chunk - 1) / obj_per_chunk;

	table->icm = kcalloc(num_icm, sizeof(*table->icm), GFP_KERNEL);
	if (!table->icm)
		return -ENOMEM;

	table->type = type;
	table->num_icm = num_icm;
	table->num_obj = nobj;
	table->obj_size = obj_size;
	table->lowmem = use_lowmem;
	table->coherent = use_coherent;
	mutex_init(&table->mutex);

	for (i = 0; i * HNS_ROCE_TABLE_CHUNK_SIZE < reserved * obj_size; ++i) {
		chunk_size = HNS_ROCE_TABLE_CHUNK_SIZE;
		if ((i + 1) * HNS_ROCE_TABLE_CHUNK_SIZE > nobj * obj_size)
			chunk_size = PAGE_ALIGN(nobj * obj_size -
						i * HNS_ROCE_TABLE_CHUNK_SIZE);

		table->icm[i] = hns_roce_alloc_icm(hr_dev,
				chunk_size >> PAGE_SHIFT, (use_lowmem ?
				GFP_KERNEL : GFP_HIGHUSER) | __GFP_NOWARN,
				use_coherent);
		if (!table->icm[i])
			goto error_failed_alloc_icm;

		if (hns_roce_map_icm(hr_dev, table,
			i * HNS_ROCE_TABLE_CHUNK_SIZE / obj_size)) {
			dev_err(dev, "map icm table failed.\n");
			goto error_failed_alloc_icm;
		}

		/*
		 * Add a reference to this ICM chunk so that it never
		 * Gets freed (since it contains reserved firmware objects).
		 */
		++table->icm[i]->refcount;
	}

	return 0;

error_failed_alloc_icm:
	for (i = 0; i < num_icm; ++i)
		if (table->icm[i]) {
			if (hns_roce_unmap_icm(hr_dev, table,
				i * HNS_ROCE_TABLE_CHUNK_SIZE / obj_size))
				dev_err(dev, "unmap icm table failed.\n");

			hns_roce_free_icm(hr_dev, table->icm[i], use_coherent);
		}

	kfree(table->icm);

	return -ENOMEM;
}

void hns_roce_cleanup_icm_table(struct hns_roce_dev *hr_dev,
				struct hns_roce_icm_table *table)
{
	struct device *dev = &hr_dev->pdev->dev;
	unsigned long i;

	for (i = 0; i < table->num_icm; ++i)
		if (table->icm[i]) {
			if (hns_roce_unmap_icm(hr_dev, table,
			    i * HNS_ROCE_TABLE_CHUNK_SIZE / table->obj_size))
				dev_err(dev, "unmap icm table failed.\n");

			hns_roce_free_icm(hr_dev, table->icm[i],
					  table->coherent);
		}

	kfree(table->icm);
}

void hns_roce_cleanup_icm(struct hns_roce_dev *hr_dev)
{
	hns_roce_cleanup_icm_table(hr_dev, &hr_dev->cq_table.table);
	hns_roce_cleanup_icm_table(hr_dev, &hr_dev->qp_table.irrl_table);
	hns_roce_cleanup_icm_table(hr_dev, &hr_dev->qp_table.qp_table);
	hns_roce_cleanup_icm_table(hr_dev, &hr_dev->mr_table.mtpt_table);
	hns_roce_cleanup_icm_table(hr_dev, &hr_dev->mr_table.mtt_table);
}
