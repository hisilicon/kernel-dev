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

#include <linux/platform_device.h>
#include <rdma/ib_umem.h>
#include "hns_roce_common.h"
#include "hns_roce_device.h"
#include "hns_roce_hw_v1.h"

static void hns_roce_set_db_event_mode(struct hns_roce_dev *hr_dev,
				       int sdb_mode, int odb_mode)
{
	u32 val;

	val = roce_read(hr_dev, ROCEE_GLB_CFG_REG);
	roce_set_bit(val, ROCEE_GLB_CFG_ROCEE_DB_SQ_MODE_S, sdb_mode);
	roce_set_bit(val, ROCEE_GLB_CFG_ROCEE_DB_OTH_MODE_S, odb_mode);
	roce_write(hr_dev, ROCEE_GLB_CFG_REG, val);
}

static void hns_roce_set_db_ext_mode(struct hns_roce_dev *hr_dev, u32 sdb_mode,
				     u32 odb_mode)
{
	u32 val;

	/* Configure SDB/ODB extend mode */
	val = roce_read(hr_dev, ROCEE_GLB_CFG_REG);
	roce_set_bit(val, ROCEE_GLB_CFG_SQ_EXT_DB_MODE_S, sdb_mode);
	roce_set_bit(val, ROCEE_GLB_CFG_OTH_EXT_DB_MODE_S, odb_mode);
	roce_write(hr_dev, ROCEE_GLB_CFG_REG, val);
}

static void hns_roce_set_sdb(struct hns_roce_dev *hr_dev, u32 sdb_alept,
			     u32 sdb_alful)
{
	u32 val;

	/* Configure SDB */
	val = roce_read(hr_dev, ROCEE_DB_SQ_WL_REG);
	roce_set_field(val, ROCEE_DB_SQ_WL_ROCEE_DB_SQ_WL_M,
		       ROCEE_DB_SQ_WL_ROCEE_DB_SQ_WL_S, sdb_alful);
	roce_set_field(val, ROCEE_DB_SQ_WL_ROCEE_DB_SQ_WL_EMPTY_M,
		       ROCEE_DB_SQ_WL_ROCEE_DB_SQ_WL_EMPTY_S, sdb_alept);
	roce_write(hr_dev, ROCEE_DB_SQ_WL_REG, val);
}

static void hns_roce_set_odb(struct hns_roce_dev *hr_dev, u32 odb_alept,
			     u32 odb_alful)
{
	u32 val;

	/* Configure ODB */
	val = roce_read(hr_dev, ROCEE_DB_OTHERS_WL_REG);
	roce_set_field(val, ROCEE_DB_OTHERS_WL_ROCEE_DB_OTH_WL_M,
		       ROCEE_DB_OTHERS_WL_ROCEE_DB_OTH_WL_S, odb_alful);
	roce_set_field(val, ROCEE_DB_OTHERS_WL_ROCEE_DB_OTH_WL_EMPTY_M,
		       ROCEE_DB_OTHERS_WL_ROCEE_DB_OTH_WL_EMPTY_S, odb_alept);
	roce_write(hr_dev, ROCEE_DB_OTHERS_WL_REG, val);
}

static void hns_roce_set_sdb_ext(struct hns_roce_dev *hr_dev, u32 ext_sdb_alept,
				 u32 ext_sdb_alful)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_db_table *db;
	dma_addr_t sdb_dma_addr;
	u32 val;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	db = &priv->db_table;

	/* Configure extend SDB threshold */
	roce_write(hr_dev, ROCEE_EXT_DB_SQ_WL_EMPTY_REG, ext_sdb_alept);
	roce_write(hr_dev, ROCEE_EXT_DB_SQ_WL_REG, ext_sdb_alful);

	/* Configure extend SDB base addr */
	sdb_dma_addr = db->ext_db->sdb_buf_list->map;
	roce_write(hr_dev, ROCEE_EXT_DB_SQ_REG, (u32)(sdb_dma_addr >> 12));

	/* Configure extend SDB depth */
	val = roce_read(hr_dev, ROCEE_EXT_DB_SQ_H_REG);
	roce_set_field(val, ROCEE_EXT_DB_SQ_H_EXT_DB_SQ_SHIFT_M,
		       ROCEE_EXT_DB_SQ_H_EXT_DB_SQ_SHIFT_S,
		       db->ext_db->esdb_dep);
	/*
	 * 44 = 32 + 12, When evaluating addr to hardware, shift 12 because of
	 * using 4K page, and shift more 32 because of
	 * caculating the high 32 bit value evaluated to hardware.
	 */
	roce_set_field(val, ROCEE_EXT_DB_SQ_H_EXT_DB_SQ_BA_H_M,
		       ROCEE_EXT_DB_SQ_H_EXT_DB_SQ_BA_H_S, sdb_dma_addr >> 44);
	roce_write(hr_dev, ROCEE_EXT_DB_SQ_H_REG, val);

	dev_dbg(dev, "ext SDB depth: 0x%x\n", db->ext_db->esdb_dep);
	dev_dbg(dev, "ext SDB threshold: epmty: 0x%x, ful: 0x%x\n",
		ext_sdb_alept, ext_sdb_alful);
}

static void hns_roce_set_odb_ext(struct hns_roce_dev *hr_dev, u32 ext_odb_alept,
				 u32 ext_odb_alful)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_db_table *db;
	dma_addr_t odb_dma_addr;
	u32 val;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	db = &priv->db_table;

	/* Configure extend ODB threshold */
	roce_write(hr_dev, ROCEE_EXT_DB_OTHERS_WL_EMPTY_REG, ext_odb_alept);
	roce_write(hr_dev, ROCEE_EXT_DB_OTHERS_WL_REG, ext_odb_alful);

	/* Configure extend ODB base addr */
	odb_dma_addr = db->ext_db->odb_buf_list->map;
	roce_write(hr_dev, ROCEE_EXT_DB_OTH_REG, (u32)(odb_dma_addr >> 12));

	/* Configure extend ODB depth */
	val = roce_read(hr_dev, ROCEE_EXT_DB_OTH_H_REG);
	roce_set_field(val, ROCEE_EXT_DB_OTH_H_EXT_DB_OTH_SHIFT_M,
		       ROCEE_EXT_DB_OTH_H_EXT_DB_OTH_SHIFT_S,
		       db->ext_db->eodb_dep);
	roce_set_field(val, ROCEE_EXT_DB_SQ_H_EXT_DB_OTH_BA_H_M,
		       ROCEE_EXT_DB_SQ_H_EXT_DB_OTH_BA_H_S,
		       db->ext_db->eodb_dep);
	roce_write(hr_dev, ROCEE_EXT_DB_OTH_H_REG, val);

	dev_dbg(dev, "ext ODB depth: 0x%x\n", db->ext_db->eodb_dep);
	dev_dbg(dev, "ext ODB threshold: empty: 0x%x, ful: 0x%x\n",
		ext_odb_alept, ext_odb_alful);
}

static int hns_roce_db_ext_init(struct hns_roce_dev *hr_dev, u32 sdb_ext_mod,
				u32 odb_ext_mod)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_db_table *db;
	dma_addr_t sdb_dma_addr;
	dma_addr_t odb_dma_addr;
	int ret = 0;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	db = &priv->db_table;

	db->ext_db = kmalloc(sizeof(*db->ext_db), GFP_KERNEL);
	if (!db->ext_db)
		return -ENOMEM;

	if (sdb_ext_mod) {
		db->ext_db->sdb_buf_list = kmalloc(
				sizeof(*db->ext_db->sdb_buf_list), GFP_KERNEL);
		if (!db->ext_db->sdb_buf_list) {
			ret = -ENOMEM;
			goto ext_sdb_buf_fail_out;
		}

		db->ext_db->sdb_buf_list->buf = dma_alloc_coherent(dev,
						     HNS_ROCE_V1_EXT_SDB_SIZE,
						     &sdb_dma_addr, GFP_KERNEL);
		if (!db->ext_db->sdb_buf_list->buf) {
			ret = -ENOMEM;
			goto alloc_sq_db_buf_fail;
		}
		db->ext_db->sdb_buf_list->map = sdb_dma_addr;

		db->ext_db->esdb_dep = ilog2(HNS_ROCE_V1_EXT_SDB_DEPTH);
		hns_roce_set_sdb_ext(hr_dev, HNS_ROCE_V1_EXT_SDB_ALEPT,
				     HNS_ROCE_V1_EXT_SDB_ALFUL);
	} else
		hns_roce_set_sdb(hr_dev, HNS_ROCE_V1_SDB_ALEPT,
				 HNS_ROCE_V1_SDB_ALFUL);

	if (odb_ext_mod) {
		db->ext_db->odb_buf_list = kmalloc(
				sizeof(*db->ext_db->odb_buf_list), GFP_KERNEL);
		if (!db->ext_db->odb_buf_list) {
			ret = -ENOMEM;
			goto ext_odb_buf_fail_out;
		}

		db->ext_db->odb_buf_list->buf = dma_alloc_coherent(dev,
						     HNS_ROCE_V1_EXT_ODB_SIZE,
						     &odb_dma_addr, GFP_KERNEL);
		if (!db->ext_db->odb_buf_list->buf) {
			ret = -ENOMEM;
			goto alloc_otr_db_buf_fail;
		}
		db->ext_db->odb_buf_list->map = odb_dma_addr;

		db->ext_db->eodb_dep = ilog2(HNS_ROCE_V1_EXT_ODB_DEPTH);
		hns_roce_set_odb_ext(hr_dev, HNS_ROCE_V1_EXT_ODB_ALEPT,
				     HNS_ROCE_V1_EXT_ODB_ALFUL);
	} else
		hns_roce_set_odb(hr_dev, HNS_ROCE_V1_ODB_ALEPT,
				 HNS_ROCE_V1_ODB_ALFUL);

	hns_roce_set_db_ext_mode(hr_dev, sdb_ext_mod, odb_ext_mod);

	return 0;

alloc_otr_db_buf_fail:
	kfree(db->ext_db->odb_buf_list);

ext_odb_buf_fail_out:
	if (sdb_ext_mod) {
		dma_free_coherent(dev, HNS_ROCE_V1_EXT_SDB_SIZE,
				  db->ext_db->sdb_buf_list->buf,
				  db->ext_db->sdb_buf_list->map);
	}

alloc_sq_db_buf_fail:
	if (sdb_ext_mod)
		kfree(db->ext_db->sdb_buf_list);

ext_sdb_buf_fail_out:
	kfree(db->ext_db);
	return ret;
}

static int hns_roce_db_init(struct hns_roce_dev *hr_dev)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_db_table *db;
	u32 sdb_ext_mod;
	u32 odb_ext_mod;
	u32 sdb_evt_mod;
	u32 odb_evt_mod;
	int ret = 0;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	db = &priv->db_table;

	memset(db, 0, sizeof(*db));

	/* Default DB mode */
	sdb_ext_mod = HNS_ROCE_SDB_EXTEND_MODE;
	odb_ext_mod = HNS_ROCE_ODB_EXTEND_MODE;
	sdb_evt_mod = HNS_ROCE_SDB_NORMAL_MODE;
	odb_evt_mod = HNS_ROCE_ODB_POLL_MODE;

	db->sdb_ext_mod = sdb_ext_mod;
	db->odb_ext_mod = odb_ext_mod;

	/* Init extend DB */
	ret = hns_roce_db_ext_init(hr_dev, sdb_ext_mod, odb_ext_mod);
	if (ret) {
		dev_err(dev, "Failed in extend DB configuration.\n");
		return ret;
	}

	hns_roce_set_db_event_mode(hr_dev, sdb_evt_mod, odb_evt_mod);

	return 0;
}

static void hns_roce_db_free(struct hns_roce_dev *hr_dev)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_db_table *db;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	db = &priv->db_table;

	if (db->sdb_ext_mod) {
		dma_free_coherent(dev, HNS_ROCE_V1_EXT_SDB_SIZE,
				  db->ext_db->sdb_buf_list->buf,
				  db->ext_db->sdb_buf_list->map);
		kfree(db->ext_db->sdb_buf_list);
	}

	if (db->odb_ext_mod) {
		dma_free_coherent(dev, HNS_ROCE_V1_EXT_ODB_SIZE,
				  db->ext_db->odb_buf_list->buf,
				  db->ext_db->odb_buf_list->map);
		kfree(db->ext_db->odb_buf_list);
	}

	kfree(db->ext_db);
}

static int hns_roce_raq_init(struct hns_roce_dev *hr_dev)
{
	int ret;
	int raq_shift = 0;
	dma_addr_t addr;
	u32 val;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_raq_table *raq;
	struct device *dev = &hr_dev->pdev->dev;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	raq = &priv->raq_table;

	raq->e_raq_buf = kzalloc(sizeof(*(raq->e_raq_buf)), GFP_KERNEL);
	if (!raq->e_raq_buf)
		return -ENOMEM;

	raq->e_raq_buf->buf = dma_alloc_coherent(dev, HNS_ROCE_V1_RAQ_SIZE,
						 &addr, GFP_KERNEL);
	if (!raq->e_raq_buf->buf) {
		ret = -ENOMEM;
		goto err_dma_alloc_raq;
	}
	raq->e_raq_buf->map = addr;

	/* Configure raq extended address. 48bit 4K align*/
	roce_write(hr_dev, ROCEE_EXT_RAQ_REG, raq->e_raq_buf->map >> 12);

	/* Configure raq_shift */
	raq_shift = ilog2(HNS_ROCE_V1_RAQ_SIZE / HNS_ROCE_V1_RAQ_ENTRY);
	val = roce_read(hr_dev, ROCEE_EXT_RAQ_H_REG);
	roce_set_field(val, ROCEE_EXT_RAQ_H_EXT_RAQ_SHIFT_M,
		       ROCEE_EXT_RAQ_H_EXT_RAQ_SHIFT_S, raq_shift);
	/*
	 * 44 = 32 + 12, When evaluating addr to hardware, shift 12 because of
	 * using 4K page, and shift more 32 because of
	 * caculating the high 32 bit value evaluated to hardware.
	 */
	roce_set_field(val, ROCEE_EXT_RAQ_H_EXT_RAQ_BA_H_M,
		       ROCEE_EXT_RAQ_H_EXT_RAQ_BA_H_S,
		       raq->e_raq_buf->map >> 44);
	roce_write(hr_dev, ROCEE_EXT_RAQ_H_REG, val);
	dev_dbg(dev, "Configure raq_shift 0x%x.\n", val);

	/* Configure raq threshold */
	val = roce_read(hr_dev, ROCEE_RAQ_WL_REG);
	roce_set_field(val, ROCEE_RAQ_WL_ROCEE_RAQ_WL_M,
		       ROCEE_RAQ_WL_ROCEE_RAQ_WL_S,
		       HNS_ROCE_V1_EXT_RAQ_WF);
	roce_write(hr_dev, ROCEE_RAQ_WL_REG, val);
	dev_dbg(dev, "Configure raq_wl 0x%x.\n", val);

	/* Enable extend raq */
	val = roce_read(hr_dev, ROCEE_WRMS_POL_TIME_INTERVAL_REG);
	roce_set_field(val,
		       ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_POL_TIME_INTERVAL_M,
		       ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_POL_TIME_INTERVAL_S,
		       POL_TIME_INTERVAL_VAL);
	roce_set_bit(val, ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_EXT_RAQ_MODE, 1);
	roce_set_field(val,
		       ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_RAQ_TIMEOUT_CHK_CFG_M,
		       ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_RAQ_TIMEOUT_CHK_CFG_S,
		       2);
	roce_set_bit(val,
		     ROCEE_WRMS_POL_TIME_INTERVAL_WRMS_RAQ_TIMEOUT_CHK_EN_S, 1);
	roce_write(hr_dev, ROCEE_WRMS_POL_TIME_INTERVAL_REG, val);
	dev_dbg(dev, "Configure WrmsPolTimeInterval 0x%x.\n", val);

	/* Enable raq drop */
	val = roce_read(hr_dev, ROCEE_GLB_CFG_REG);
	roce_set_bit(val, ROCEE_GLB_CFG_TRP_RAQ_DROP_EN_S, 1);
	roce_write(hr_dev, ROCEE_GLB_CFG_REG, val);
	dev_dbg(dev, "Configure GlbCfg = 0x%x.\n", val);

	return 0;

err_dma_alloc_raq:
	kfree(raq->e_raq_buf);
	return ret;
}

static void hns_roce_raq_free(struct hns_roce_dev *hr_dev)
{
	struct device *dev = &hr_dev->pdev->dev;
	struct hns_roce_v1_priv *priv;
	struct hns_roce_raq_table *raq;

	priv = (struct hns_roce_v1_priv *)hr_dev->hw->priv;
	raq = &priv->raq_table;

	dma_free_coherent(dev, HNS_ROCE_V1_RAQ_SIZE, raq->e_raq_buf->buf,
			  raq->e_raq_buf->map);
	kfree(raq->e_raq_buf);
}

static void hns_roce_port_enable(struct hns_roce_dev *hr_dev, int enable_flag)
{
	u32 val;

	if (enable_flag) {
		val = roce_read(hr_dev, ROCEE_GLB_CFG_REG);
		 /* Open all ports */
		roce_set_field(val, ROCEE_GLB_CFG_ROCEE_PORT_ST_M,
			       ROCEE_GLB_CFG_ROCEE_PORT_ST_S,
			       ALL_PORT_VAL_OPEN);
		roce_write(hr_dev, ROCEE_GLB_CFG_REG, val);
	} else {
		val = roce_read(hr_dev, ROCEE_GLB_CFG_REG);
		/* Close all ports */
		roce_set_field(val, ROCEE_GLB_CFG_ROCEE_PORT_ST_M,
			       ROCEE_GLB_CFG_ROCEE_PORT_ST_S, 0x0);
		roce_write(hr_dev, ROCEE_GLB_CFG_REG, val);
	}
}

/**
 * hns_roce_v1_reset - reset RoCE
 * @hr_dev: RoCE device struct pointer
 * @enable: true -- drop reset, false -- reset
 * return 0 - success , negative --fail
 */
int hns_roce_v1_reset(struct hns_roce_dev *hr_dev, bool enable)
{
	struct device_node *dsaf_node;
	struct device *dev = &hr_dev->pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;

	dsaf_node = of_parse_phandle(np, "dsaf-handle", 0);
	if (!dsaf_node) {
		dev_err(dev, "Unable to get dsaf node by dsaf-handle!\n");
		return -EINVAL;
	}

	ret = hns_dsaf_roce_reset(&dsaf_node->fwnode, false);
	if (ret)
		return ret;

	if (enable) {
		msleep(SLEEP_TIME_INTERVAL);
		return hns_dsaf_roce_reset(&dsaf_node->fwnode, true);
	}

	return 0;
}

void hns_roce_v1_profile(struct hns_roce_dev *hr_dev)
{
	int i = 0;
	struct hns_roce_caps *caps = &hr_dev->caps;

	hr_dev->vendor_id = le32_to_cpu(roce_read(hr_dev, ROCEE_VENDOR_ID_REG));
	hr_dev->vendor_part_id = le32_to_cpu(roce_read(hr_dev,
					     ROCEE_VENDOR_PART_ID_REG));
	hr_dev->hw_rev = le32_to_cpu(roce_read(hr_dev, ROCEE_HW_VERSION_REG));

	hr_dev->sys_image_guid = le32_to_cpu(roce_read(hr_dev,
					     ROCEE_SYS_IMAGE_GUID_L_REG)) |
				((u64)le32_to_cpu(roce_read(hr_dev,
					    ROCEE_SYS_IMAGE_GUID_H_REG)) << 32);

	caps->num_qps		= HNS_ROCE_V1_MAX_QP_NUM;
	caps->max_wqes		= HNS_ROCE_V1_MAX_WQE_NUM;
	caps->num_cqs		= HNS_ROCE_V1_MAX_CQ_NUM;
	caps->max_cqes		= HNS_ROCE_V1_MAX_CQE_NUM;
	caps->max_sq_sg		= HNS_ROCE_V1_SG_NUM;
	caps->max_rq_sg		= HNS_ROCE_V1_SG_NUM;
	caps->max_sq_inline	= HNS_ROCE_V1_INLINE_SIZE;
	caps->num_uars		= HNS_ROCE_V1_UAR_NUM;
	caps->phy_num_uars	= HNS_ROCE_V1_PHY_UAR_NUM;
	caps->num_aeq_vectors	= HNS_ROCE_AEQE_VEC_NUM;
	caps->num_comp_vectors	= HNS_ROCE_COMP_VEC_NUM;
	caps->num_other_vectors	= HNS_ROCE_AEQE_OF_VEC_NUM;
	caps->num_mtpts		= HNS_ROCE_V1_MAX_MTPT_NUM;
	caps->num_mtt_segs	= HNS_ROCE_V1_MAX_MTT_SEGS;
	caps->num_pds		= HNS_ROCE_V1_MAX_PD_NUM;
	caps->max_qp_init_rdma	= HNS_ROCE_V1_MAX_QP_INIT_RDMA;
	caps->max_qp_dest_rdma	= HNS_ROCE_V1_MAX_QP_DEST_RDMA;
	caps->max_sq_desc_sz	= HNS_ROCE_V1_MAX_SQ_DESC_SZ;
	caps->max_rq_desc_sz	= HNS_ROCE_V1_MAX_RQ_DESC_SZ;
	caps->qpc_entry_sz	= HNS_ROCE_V1_QPC_ENTRY_SIZE;
	caps->irrl_entry_sz	= HNS_ROCE_V1_IRRL_ENTRY_SIZE;
	caps->cqc_entry_sz	= HNS_ROCE_V1_CQC_ENTRY_SIZE;
	caps->mtpt_entry_sz	= HNS_ROCE_V1_MTPT_ENTRY_SIZE;
	caps->mtt_entry_sz	= HNS_ROCE_V1_MTT_ENTRY_SIZE;
	caps->cq_entry_sz	= HNS_ROCE_V1_CQE_ENTRY_SIZE;
	caps->page_size_cap	= HNS_ROCE_V1_PAGE_SIZE_SUPPORT;
	caps->sqp_start		= 0;
	caps->reserved_lkey	= 0;
	caps->reserved_pds	= 0;
	caps->reserved_mrws	= 1;
	caps->reserved_uars	= 0;
	caps->reserved_cqs	= 0;

	for (i = 0; i < caps->num_ports; i++)
		caps->pkey_table_len[i] = 1;

	for (i = 0; i < caps->num_ports; i++) {
		/* Six ports shared 16 GID in v1 engine */
		if (i >= (HNS_ROCE_V1_GID_NUM % caps->num_ports))
			caps->gid_table_len[i] = HNS_ROCE_V1_GID_NUM /
						 caps->num_ports;
		else
			caps->gid_table_len[i] = HNS_ROCE_V1_GID_NUM /
						 caps->num_ports + 1;
	}

	for (i = 0; i < caps->num_comp_vectors; i++)
		caps->ceqe_depth[i] = HNS_ROCE_V1_NUM_COMP_EQE;

	caps->aeqe_depth = HNS_ROCE_V1_NUM_ASYNC_EQE;
	caps->local_ca_ack_delay = le32_to_cpu(roce_read(hr_dev,
							 ROCEE_ACK_DELAY_REG));
	caps->max_mtu = IB_MTU_2048;
}

int hns_roce_v1_init(struct hns_roce_dev *hr_dev)
{
	int ret;
	u32 val;
	struct device *dev = &hr_dev->pdev->dev;

	/* DMAE user config */
	val = roce_read(hr_dev, ROCEE_DMAE_USER_CFG1_REG);
	roce_set_field(val, ROCEE_DMAE_USER_CFG1_ROCEE_CACHE_TB_CFG_M,
		       ROCEE_DMAE_USER_CFG1_ROCEE_CACHE_TB_CFG_S, 0xf);
	roce_set_field(val, ROCEE_DMAE_USER_CFG1_ROCEE_STREAM_ID_TB_CFG_M,
		       ROCEE_DMAE_USER_CFG1_ROCEE_STREAM_ID_TB_CFG_S,
		       1 << PAGES_SHIFT_16);
	roce_write(hr_dev, ROCEE_DMAE_USER_CFG1_REG, val);

	val = roce_read(hr_dev, ROCEE_DMAE_USER_CFG2_REG);
	roce_set_field(val, ROCEE_DMAE_USER_CFG2_ROCEE_CACHE_PKT_CFG_M,
		       ROCEE_DMAE_USER_CFG2_ROCEE_CACHE_PKT_CFG_S, 0xf);
	roce_set_field(val, ROCEE_DMAE_USER_CFG2_ROCEE_STREAM_ID_PKT_CFG_M,
		       ROCEE_DMAE_USER_CFG2_ROCEE_STREAM_ID_PKT_CFG_S,
		       1 << PAGES_SHIFT_16);

	ret = hns_roce_db_init(hr_dev);
	if (ret) {
		dev_err(dev, "doorbell init failed!\n");
		return ret;
	}

	ret = hns_roce_raq_init(hr_dev);
	if (ret) {
		dev_err(dev, "raq init failed!\n");
		goto error_failed_raq_init;
	}

	hns_roce_port_enable(hr_dev, HNS_ROCE_PORT_UP);

	return 0;

error_failed_raq_init:
	hns_roce_db_free(hr_dev);
	return ret;
}

void hns_roce_v1_exit(struct hns_roce_dev *hr_dev)
{
	hns_roce_port_enable(hr_dev, HNS_ROCE_PORT_DOWN);
	hns_roce_raq_free(hr_dev);
	hns_roce_db_free(hr_dev);
}

void hns_roce_v1_set_gid(struct hns_roce_dev *hr_dev, u8 port, int gid_index,
			 union ib_gid *gid)
{
	u32 *p = NULL;
	u8 gid_idx = 0;

	gid_idx = hns_get_gid_index(hr_dev, port, gid_index);

	p = (u32 *)&gid->raw[0];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_PORT_GID_L_0_REG +
		       (HNS_ROCE_V1_GID_NUM * gid_idx));

	p = (u32 *)&gid->raw[4];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_PORT_GID_ML_0_REG +
		       (HNS_ROCE_V1_GID_NUM * gid_idx));

	p = (u32 *)&gid->raw[8];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_PORT_GID_MH_0_REG +
		       (HNS_ROCE_V1_GID_NUM * gid_idx));

	p = (u32 *)&gid->raw[0xc];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_PORT_GID_H_0_REG +
		       (HNS_ROCE_V1_GID_NUM * gid_idx));
}

void hns_roce_v1_set_mac(struct hns_roce_dev *hr_dev, u8 phy_port, u8 *addr)
{
	u32 reg_smac_l;
	u16 reg_smac_h;
	u16 *p_h;
	u32 *p;
	u32 val;

	p = (u32 *)(&addr[0]);
	reg_smac_l = *p;
	roce_raw_write(reg_smac_l, hr_dev->reg_base + ROCEE_SMAC_L_0_REG +
		       PHY_PORT_OFFSET * phy_port);

	val = roce_read(hr_dev,
			ROCEE_SMAC_H_0_REG + phy_port * PHY_PORT_OFFSET);
	p_h = (u16 *)(&addr[4]);
	reg_smac_h  = *p_h;
	roce_set_field(val, ROCEE_SMAC_H_ROCEE_SMAC_H_M,
		       ROCEE_SMAC_H_ROCEE_SMAC_H_S, reg_smac_h);
	roce_write(hr_dev, ROCEE_SMAC_H_0_REG + phy_port * PHY_PORT_OFFSET,
		   val);
}

void hns_roce_v1_set_mtu(struct hns_roce_dev *hr_dev, u8 phy_port,
			 enum ib_mtu mtu)
{
	u32 val;

	val = roce_read(hr_dev,
			ROCEE_SMAC_H_0_REG + phy_port * PHY_PORT_OFFSET);
	roce_set_field(val, ROCEE_SMAC_H_ROCEE_PORT_MTU_M,
		       ROCEE_SMAC_H_ROCEE_PORT_MTU_S, mtu);
	roce_write(hr_dev, ROCEE_SMAC_H_0_REG + phy_port * PHY_PORT_OFFSET,
		   val);
}

struct hns_roce_hw hns_roce_hw_v1 = {
	.reset = hns_roce_v1_reset,
	.hw_profile = hns_roce_v1_profile,
	.hw_init = hns_roce_v1_init,
	.hw_exit = hns_roce_v1_exit,
	.set_gid = hns_roce_v1_set_gid,
	.set_mac = hns_roce_v1_set_mac,
	.set_mtu = hns_roce_v1_set_mtu,
};
