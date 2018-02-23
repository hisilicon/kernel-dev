/* SPDX-License-Identifier: (GPL-2.0 WITH Linux-syscall-note) OR BSD-2-Clause */
/*
 * Copyright (c) 2017 Hisilicon Limited.
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

#include <linux/platform_device.h>
#include <rdma/ib_umem.h>
#include "hns_roce_device.h"

int hns_roce_db_map_user(struct hns_roce_ucontext *context, unsigned long virt,
			 struct hns_roce_db *db)
{
	struct hns_roce_user_db_page *db_page;
	int ret = 0;

	mutex_lock(&context->db_page_mutex);

	list_for_each_entry(db_page, &context->db_page_list, list)
		if (db_page->user_virt == (virt & PAGE_MASK))
			goto found;

	db_page = kmalloc(sizeof(*db_page), GFP_KERNEL);
	if (!db_page) {
		ret = -ENOMEM;
		goto out;
	}

	db_page->user_virt = (virt & PAGE_MASK);
	db_page->refcount_t = 0;
	db_page->umem      = ib_umem_get(&context->ibucontext, virt & PAGE_MASK,
					 PAGE_SIZE, 0, 0);
	if (IS_ERR(db_page->umem)) {
		ret = PTR_ERR(db_page->umem);
		kfree(db_page);
		goto out;
	}

	list_add(&db_page->list, &context->db_page_list);

found:
	db->dma = sg_dma_address(db_page->umem->sg_head.sgl) +
		  (virt & ~PAGE_MASK);
	db->u.user_page = db_page;
	++db_page->refcount_t;

out:
	mutex_unlock(&context->db_page_mutex);

	return ret;
}
EXPORT_SYMBOL(hns_roce_db_map_user);

void hns_roce_db_unmap_user(struct hns_roce_ucontext *context,
			    struct hns_roce_db *db)
{
	mutex_lock(&context->db_page_mutex);

	if (!--db->u.user_page->refcount_t) {
		list_del(&db->u.user_page->list);
		ib_umem_release(db->u.user_page->umem);
		kfree(db->u.user_page);
	}

	mutex_unlock(&context->db_page_mutex);
}
EXPORT_SYMBOL(hns_roce_db_unmap_user);

static struct hns_roce_db_pgdir *hns_roce_alloc_db_pgdir(
					struct device *dma_device)
{
	struct hns_roce_db_pgdir *pgdir;

	pgdir = kzalloc(sizeof(*pgdir), GFP_KERNEL);
	if (!pgdir)
		return NULL;

	bitmap_fill(pgdir->order1, HNS_ROCE_DB_PER_PAGE / 2);
	pgdir->bits[0] = pgdir->order0;
	pgdir->bits[1] = pgdir->order1;
	pgdir->db_page = dma_alloc_coherent(dma_device, PAGE_SIZE,
					    &pgdir->db_dma, GFP_KERNEL);
	if (!pgdir->db_page) {
		kfree(pgdir);
		return NULL;
	}

	return pgdir;
}

static int hns_roce_alloc_db_from_pgdir(struct hns_roce_db_pgdir *pgdir,
					struct hns_roce_db *db, int order)
{
	int o;
	int i;

	for (o = order; o <= 1; ++o) {
		i = find_first_bit(pgdir->bits[o], HNS_ROCE_DB_PER_PAGE >> o);
		if (i < HNS_ROCE_DB_PER_PAGE >> o)
			goto found;
	}

	return -ENOMEM;

found:
	clear_bit(i, pgdir->bits[o]);

	i <<= o;

	if (o > order)
		set_bit(i ^ 1, pgdir->bits[order]);

	db->u.pgdir	= pgdir;
	db->index	= i;
	db->db_record	= pgdir->db_page + db->index;
	db->dma		= pgdir->db_dma  + db->index * 4;
	db->order	= order;

	return 0;
}

int hns_roce_alloc_db(struct hns_roce_dev *hr_dev, struct hns_roce_db *db,
		      int order)
{
	struct hns_roce_db_pgdir *pgdir;
	int ret = 0;

	mutex_lock(&hr_dev->pgdir_mutex);

	list_for_each_entry(pgdir, &hr_dev->pgdir_list, list)
		if (!hns_roce_alloc_db_from_pgdir(pgdir, db, order))
			goto out;

	pgdir = hns_roce_alloc_db_pgdir(hr_dev->dev);
	if (!pgdir) {
		ret = -ENOMEM;
		goto out;
	}

	list_add(&pgdir->list, &hr_dev->pgdir_list);

	/* This should never fail -- we just allocated an empty page: */
	WARN_ON(hns_roce_alloc_db_from_pgdir(pgdir, db, order));

out:
	mutex_unlock(&hr_dev->pgdir_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(hns_roce_alloc_db);

void hns_roce_free_db(struct hns_roce_dev *hr_dev, struct hns_roce_db *db)
{
	int o;
	int i;

	mutex_lock(&hr_dev->pgdir_mutex);

	o = db->order;
	i = db->index;

	if (db->order == 0 && test_bit(i ^ 1, db->u.pgdir->order0)) {
		clear_bit(i ^ 1, db->u.pgdir->order0);
		++o;
	}

	i >>= o;
	set_bit(i, db->u.pgdir->bits[o]);

	if (bitmap_full(db->u.pgdir->order1, HNS_ROCE_DB_PER_PAGE / 2)) {
		dma_free_coherent(hr_dev->dev, PAGE_SIZE, db->u.pgdir->db_page,
				  db->u.pgdir->db_dma);
		list_del(&db->u.pgdir->list);
		kfree(db->u.pgdir);
	}

	mutex_unlock(&hr_dev->pgdir_mutex);
}
EXPORT_SYMBOL_GPL(hns_roce_free_db);
