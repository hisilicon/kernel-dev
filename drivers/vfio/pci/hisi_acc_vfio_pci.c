// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, HiSilicon Ltd.
 */

#include <linux/device.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/hisi_acc_qm.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/vfio.h>
#include <linux/vfio_pci_core.h>

#include "hisi_acc_vfio_pci.h"

enum {
	HISI_ACC_SUPPORTED_DEVICE_STATES = VFIO_DEVICE_STATE_RUNNING |
					   VFIO_DEVICE_STATE_SAVING |
					   VFIO_DEVICE_STATE_RESUMING,
};

/* return 0 on VM acc device ready, -ETIMEDOUT hardware timeout */
static int qm_wait_dev_not_ready(struct hisi_qm *qm)
{
	u32 val;

	return readl_relaxed_poll_timeout(qm->io_base + QM_VF_STATE,
				val, !(val & 0x1), MB_POLL_PERIOD_US,
				MB_POLL_TIMEOUT_US);
}

/*
 * Each state Reg is checked 100 times,
 * with a delay of 100 microseconds after each check
 */
static u32 acc_check_reg_state(struct hisi_qm *qm, u32 regs)
{
	int check_times = 0;
	u32 state;

	state = readl(qm->io_base + regs);
	while (state && check_times < ERROR_CHECK_TIMEOUT) {
		udelay(CHECK_DELAY_TIME);
		state = readl(qm->io_base + regs);
		check_times++;
	}

	return state;
}

/* Check the PF's RAS state and Function INT state */
static int qm_check_int_state(struct hisi_acc_vf_mig_info *vmig)
{
	struct hisi_qm *vfqm = &vmig->vf_qm;
	struct hisi_qm *qm = vmig->pf_qm;
	struct pci_dev *vf_pdev = vmig->vf_dev;
	struct device *dev = &qm->pdev->dev;
	u32 state;

	/* Check RAS state */
	state = acc_check_reg_state(qm, QM_ABNORMAL_INT_STATUS);
	if (state) {
		dev_err(dev, "failed to check QM RAS state!\n");
		return -EBUSY;
	}

	/* Check Function Communication state between PF and VF */
	state = acc_check_reg_state(vfqm, QM_IFC_INT_STATUS);
	if (state) {
		dev_err(dev, "failed to check QM IFC INT state!\n");
		return -EBUSY;
	}
	state = acc_check_reg_state(vfqm, QM_IFC_INT_SET_V);
	if (state) {
		dev_err(dev, "failed to check QM IFC INT SET state!\n");
		return -EBUSY;
	}

	/* Check submodule task state */
	switch (vf_pdev->device) {
	case PCI_DEVICE_ID_HUAWEI_SEC_VF:
		state = acc_check_reg_state(qm, SEC_CORE_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM SEC Core INT state!\n");
			return -EBUSY;
		}
		return 0;
	case PCI_DEVICE_ID_HUAWEI_HPRE_VF:
		state = acc_check_reg_state(qm, HPRE_HAC_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM HPRE HAC INT state!\n");
			return -EBUSY;
		}
		return 0;
	case PCI_DEVICE_ID_HUAWEI_ZIP_VF:
		state = acc_check_reg_state(qm, HZIP_CORE_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM ZIP Core INT state!\n");
			return -EBUSY;
		}
		return 0;
	default:
		dev_err(dev, "failed to detect acc module type!\n");
		return -EINVAL;
	}
}

static int qm_read_reg(struct hisi_qm *qm, u32 reg_addr,
		       u32 *data, u8 nums)
{
	int i;

	if (nums < 1 || nums > QM_REGS_MAX_LEN)
		return -EINVAL;

	for (i = 0; i < nums; i++) {
		data[i] = readl(qm->io_base + reg_addr);
		reg_addr += QM_REG_ADDR_OFFSET;
	}

	return 0;
}

static int qm_write_reg(struct hisi_qm *qm, u32 reg,
			u32 *data, u8 nums)
{
	int i;

	if (nums < 1 || nums > QM_REGS_MAX_LEN)
		return -EINVAL;

	for (i = 0; i < nums; i++)
		writel(data[i], qm->io_base + reg + i * QM_REG_ADDR_OFFSET);

	return 0;
}

static int qm_get_vft(struct hisi_qm *qm, u32 *base)
{
	u64 sqc_vft;
	u32 qp_num;
	int ret;

	ret = qm_mb(qm, QM_MB_CMD_SQC_VFT_V2, 0, 0, 1);
	if (ret)
		return ret;

	sqc_vft = readl(qm->io_base + QM_MB_CMD_DATA_ADDR_L) |
		  ((u64)readl(qm->io_base + QM_MB_CMD_DATA_ADDR_H) <<
		  QM_XQC_ADDR_OFFSET);
	*base = QM_SQC_VFT_BASE_MASK_V2 & (sqc_vft >> QM_SQC_VFT_BASE_SHIFT_V2);
	qp_num = (QM_SQC_VFT_NUM_MASK_V2 &
		  (sqc_vft >> QM_SQC_VFT_NUM_SHIFT_V2)) + 1;

	return qp_num;
}

static int qm_get_sqc(struct hisi_qm *qm, u64 *addr)
{
	int ret;

	ret = qm_mb(qm, QM_MB_CMD_SQC_BT, 0, 0, 1);
	if (ret)
		return ret;

	*addr = readl(qm->io_base + QM_MB_CMD_DATA_ADDR_L) |
		  ((u64)readl(qm->io_base + QM_MB_CMD_DATA_ADDR_H) <<
		  QM_XQC_ADDR_OFFSET);

	return 0;
}

static int qm_get_cqc(struct hisi_qm *qm, u64 *addr)
{
	int ret;

	ret = qm_mb(qm, QM_MB_CMD_CQC_BT, 0, 0, 1);
	if (ret)
		return ret;

	*addr = readl(qm->io_base + QM_MB_CMD_DATA_ADDR_L) |
		  ((u64)readl(qm->io_base + QM_MB_CMD_DATA_ADDR_H) <<
		  QM_XQC_ADDR_OFFSET);

	return 0;
}

static int qm_rw_regs_read(struct hisi_qm *qm, struct acc_vf_data *vf_data)
{
	struct device *dev = &qm->pdev->dev;
	int ret;

	ret = qm_read_reg(qm, QM_VF_AEQ_INT_MASK, &vf_data->aeq_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_VF_AEQ_INT_MASK\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_VF_EQ_INT_MASK, &vf_data->eq_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_VF_EQ_INT_MASK\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_IFC_INT_SOURCE_V,
			  &vf_data->ifc_int_source, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_IFC_INT_SOURCE_V\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_IFC_INT_MASK, &vf_data->ifc_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_IFC_INT_MASK\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_IFC_INT_SET_V, &vf_data->ifc_int_set, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_IFC_INT_SET_V\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_QUE_ISO_CFG_V, &vf_data->que_iso_cfg, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_QUE_ISO_CFG_V\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_PAGE_SIZE, &vf_data->page_size, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_PAGE_SIZE\n");
		return ret;
	}

	ret = qm_read_reg(qm, QM_STATE_REG, &vf_data->vf_qm_state, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_VF_STATE\n");
		return ret;
	}

	/* QM_EQC_DW has 7 regs */
	ret = qm_read_reg(qm, QM_EQC_DW0, vf_data->qm_eqc_dw, 7);
	if (ret) {
		dev_err(dev, "failed to read QM_EQC_DW\n");
		return ret;
	}

	/* QM_AEQC_DW has 7 regs */
	ret = qm_read_reg(qm, QM_AEQC_DW0, vf_data->qm_aeqc_dw, 7);
	if (ret) {
		dev_err(dev, "failed to read QM_AEQC_DW\n");
		return ret;
	}

	return 0;
}

static int qm_rw_regs_write(struct hisi_qm *qm, struct acc_vf_data *vf_data)
{
	struct device *dev = &qm->pdev->dev;
	int ret;

	/* check VF state */
	if (unlikely(qm_wait_mb_ready(qm))) {
		dev_err(&qm->pdev->dev, "QM device is not ready to write\n");
		return -EBUSY;
	}

	ret = qm_write_reg(qm, QM_VF_AEQ_INT_MASK, &vf_data->aeq_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_VF_AEQ_INT_MASK\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_VF_EQ_INT_MASK, &vf_data->eq_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_VF_EQ_INT_MASK\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_IFC_INT_SOURCE_V,
			   &vf_data->ifc_int_source, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_IFC_INT_SOURCE_V\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_IFC_INT_MASK, &vf_data->ifc_int_mask, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_IFC_INT_MASK\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_IFC_INT_SET_V, &vf_data->ifc_int_set, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_IFC_INT_SET_V\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_QUE_ISO_CFG_V, &vf_data->que_iso_cfg, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_QUE_ISO_CFG_V\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_PAGE_SIZE, &vf_data->page_size, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_PAGE_SIZE\n");
		return ret;
	}

	ret = qm_write_reg(qm, QM_STATE_REG, &vf_data->vf_qm_state, 1);
	if (ret) {
		dev_err(dev, "failed to write QM_VF_STATE\n");
		return ret;
	}

	/* QM_EQC_DW has 7 regs */
	ret = qm_write_reg(qm, QM_EQC_DW0, vf_data->qm_eqc_dw, 7);
	if (ret) {
		dev_err(dev, "failed to write QM_EQC_DW\n");
		return ret;
	}

	/* QM_AEQC_DW has 7 regs */
	ret = qm_write_reg(qm, QM_AEQC_DW0, vf_data->qm_aeqc_dw, 7);
	if (ret) {
		dev_err(dev, "failed to write QM_AEQC_DW\n");
		return ret;
	}

	return 0;
}

static void qm_db(struct hisi_qm *qm, u16 qn, u8 cmd,
		  u16 index, u8 priority)
{
	u64 doorbell;
	u64 dbase;
	u16 randata = 0;

	if (cmd == QM_DOORBELL_CMD_SQ || cmd == QM_DOORBELL_CMD_CQ)
		dbase = QM_DOORBELL_SQ_CQ_BASE_V2;
	else
		dbase = QM_DOORBELL_EQ_AEQ_BASE_V2;

	doorbell = qn | ((u64)cmd << QM_DB_CMD_SHIFT_V2) |
		   ((u64)randata << QM_DB_RAND_SHIFT_V2) |
		   ((u64)index << QM_DB_INDEX_SHIFT_V2)	 |
		   ((u64)priority << QM_DB_PRIORITY_SHIFT_V2);

	writeq(doorbell, qm->io_base + dbase);
}

static int vf_migration_data_store(struct hisi_qm *qm,
				   struct hisi_acc_vf_mig_info *vmig)
{
	struct acc_vf_data *vf_data = &vmig->vf_data;
	struct device *dev = &qm->pdev->dev;
	int ret;

	ret = qm_rw_regs_read(qm, vf_data);
	if (ret)
		return -EINVAL;

	/* Every reg is 32 bit, the dma address is 64 bit. */
	vf_data->eqe_dma = vf_data->qm_eqc_dw[2];
	vf_data->eqe_dma <<= QM_XQC_ADDR_OFFSET;
	vf_data->eqe_dma |= vf_data->qm_eqc_dw[1];
	vf_data->aeqe_dma = vf_data->qm_aeqc_dw[2];
	vf_data->aeqe_dma <<= QM_XQC_ADDR_OFFSET;
	vf_data->aeqe_dma |= vf_data->qm_aeqc_dw[1];

	/* Through SQC_BT/CQC_BT to get sqc and cqc address */
	ret = qm_get_sqc(qm, &vf_data->sqc_dma);
	if (ret) {
		dev_err(dev, "failed to read SQC addr!\n");
		return -EINVAL;
	}

	ret = qm_get_cqc(qm, &vf_data->cqc_dma);
	if (ret) {
		dev_err(dev, "failed to read CQC addr!\n");
		return -EINVAL;
	}

	return 0;
}

static void qm_dev_cmd_init(struct hisi_qm *qm)
{
	/* Clear VF communication status registers. */
	writel(0x1, qm->io_base + QM_IFC_INT_SOURCE_V);

	/* Enable pf and vf communication. */
	writel(0x0, qm->io_base + QM_IFC_INT_MASK);
}

static int vf_qm_cache_wb(struct hisi_qm *qm)
{
	unsigned int val;

	writel(0x1, qm->io_base + QM_CACHE_WB_START);
	if (readl_relaxed_poll_timeout(qm->io_base + QM_CACHE_WB_DONE,
				       val, val & BIT(0), MB_POLL_PERIOD_US,
				       MB_POLL_TIMEOUT_US)) {
		dev_err(&qm->pdev->dev, "vf QM writeback sqc cache fail\n");
		return -EINVAL;
	}

	return 0;
}

static void vf_qm_fun_reset(struct hisi_qm *qm,
			    struct hisi_acc_vf_mig_info *vmig)
{
	struct acc_vf_data *vf_data = &vmig->vf_data;
	int i;

	if (vf_data->vf_qm_state != QM_READY)
		return;

	for (i = 0; i < qm->qp_num; i++)
		qm_db(qm, i, QM_DOORBELL_CMD_SQ, 0, 1);
}

static int vf_qm_func_stop(struct hisi_qm *qm)
{
	return qm_mb(qm, QM_MB_CMD_PAUSE_QM, 0, 0, 0);
}

static int pf_qm_get_qp_num(struct hisi_qm *qm, int vf_id, u32 *rbase)
{
	unsigned int val;
	u64 sqc_vft;
	u32 qp_num;
	int ret;

	ret = readl_relaxed_poll_timeout(qm->io_base + QM_VFT_CFG_RDY, val,
					 val & BIT(0), MB_POLL_PERIOD_US,
					 MB_POLL_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x1, qm->io_base + QM_VFT_CFG_OP_WR);
	/* 0 mean SQC VFT */
	writel(0x0, qm->io_base + QM_VFT_CFG_TYPE);
	writel(vf_id, qm->io_base + QM_VFT_CFG);

	writel(0x0, qm->io_base + QM_VFT_CFG_RDY);
	writel(0x1, qm->io_base + QM_VFT_CFG_OP_ENABLE);

	ret = readl_relaxed_poll_timeout(qm->io_base + QM_VFT_CFG_RDY, val,
					 val & BIT(0), MB_POLL_PERIOD_US,
					 MB_POLL_TIMEOUT_US);
	if (ret)
		return ret;

	sqc_vft = readl(qm->io_base + QM_VFT_CFG_DATA_L) |
		  ((u64)readl(qm->io_base + QM_VFT_CFG_DATA_H) <<
		  QM_XQC_ADDR_OFFSET);
	*rbase = QM_SQC_VFT_BASE_MASK_V2 &
		  (sqc_vft >> QM_SQC_VFT_BASE_SHIFT_V2);
	qp_num = (QM_SQC_VFT_NUM_MASK_V2 &
		  (sqc_vft >> QM_SQC_VFT_NUM_SHIFT_V2)) + 1;

	return qp_num;
}

static int vf_migration_data_recover(struct hisi_qm *qm,
				     struct hisi_acc_vf_mig_info *vmig)
{
	struct device *dev = &qm->pdev->dev;
	struct acc_vf_data *vf_data = &vmig->vf_data;
	int ret;

	qm->eqe_dma = vf_data->eqe_dma;
	qm->aeqe_dma = vf_data->aeqe_dma;
	qm->sqc_dma = vf_data->sqc_dma;
	qm->cqc_dma = vf_data->cqc_dma;

	qm->qp_base = vf_data->qp_base;
	qm->qp_num = vf_data->qp_num;

	ret = qm_rw_regs_write(qm, vf_data);
	if (ret) {
		dev_err(dev, "Set VF regs failed\n");
		return ret;
	}

	ret = qm_mb(qm, QM_MB_CMD_SQC_BT, qm->sqc_dma, 0, 0);
	if (ret) {
		dev_err(dev, "Set sqc failed\n");
		return ret;
	}

	ret = qm_mb(qm, QM_MB_CMD_CQC_BT, qm->cqc_dma, 0, 0);
	if (ret) {
		dev_err(dev, "Set cqc failed\n");
		return ret;
	}

	qm_dev_cmd_init(qm);

	return 0;
}

static int pf_qm_state_pre_save(struct hisi_acc_vf_mig_info *vmig)
{
	struct acc_vf_data *vf_data = &vmig->vf_data;
	struct hisi_qm *pf_qm = vmig->pf_qm;
	struct device *dev = &pf_qm->pdev->dev;
	int vf_id = vmig->vf_id;
	int ret;

	/* save device id */
	vf_data->dev_id = vmig->vf_dev->device;

	/* vf qp num save from PF */
	ret = pf_qm_get_qp_num(pf_qm, vf_id, &pf_qm->qp_base);
	if (ret <= 0) {
		dev_err(dev, "failed to get vft qp nums!\n");
		return -EINVAL;
	}
	pf_qm->qp_num = ret;
	vf_data->qp_base = pf_qm->qp_base;
	vf_data->qp_num = pf_qm->qp_num;

	/* vf isolation state save from PF */
	ret = qm_read_reg(pf_qm, QM_QUE_ISO_CFG, &vf_data->que_iso_cfg, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_QUE_ISO_CFG!\n");
		return ret;
	}

	vmig->data_size = QM_MATCH_SIZE;
	vmig->pending_bytes = vmig->data_size;

	return 0;
}

static int vf_qm_state_save(struct hisi_acc_vf_mig_info *vmig)
{
	struct device *dev = &vmig->vf_dev->dev;
	struct hisi_qm *vf_qm = &vmig->vf_qm;
	int ret;

	vmig->data_size = 0;
	vmig->pending_bytes = 0;

	ret = vf_qm_cache_wb(vf_qm);
	if (ret) {
		dev_err(dev, "failed to writeback QM Cache!\n");
		goto state_error;
	}

	ret = vf_migration_data_store(vf_qm, vmig);
	if (ret) {
		dev_err(dev, "failed to get and store migration data!\n");
		goto state_error;
	}

	vmig->data_size = sizeof(struct acc_vf_data);
	vmig->pending_bytes = vmig->data_size;

	return 0;

state_error:
	vf_qm_fun_reset(vf_qm, vmig);
	return ret;
}

static void hisi_acc_start_device(struct hisi_acc_vf_mig_info *vmig)
{
	struct hisi_qm *vf_qm = &vmig->vf_qm;

	vf_qm_fun_reset(vf_qm, vmig);
}

static int hisi_acc_post_copy(struct hisi_acc_vf_mig_info *vmig)
{
	struct hisi_qm *vf_qm = &vmig->vf_qm;
	struct device *dev = &vmig->vf_dev->dev;

	if (unlikely(qm_wait_dev_not_ready(vf_qm))) {
		dev_info(dev, "QM device not ready, no data to transfer\n");
		return 0;
	}

	return vf_qm_state_save(vmig);
}

static int hisi_acc_pre_copy(struct hisi_acc_vf_mig_info *vmig)
{
	return pf_qm_state_pre_save(vmig);
}

static int hisi_acc_load_state(struct hisi_acc_vf_mig_info *vmig)
{
	struct device *dev = &vmig->vf_dev->dev;
	struct hisi_qm *vf_qm = &vmig->vf_qm;
	int ret;

	if (!vmig->data_size)
		return 0;

	/* recover data to VF */
	ret = vf_migration_data_recover(vf_qm, vmig);
	if (ret) {
		dev_err(dev, "failed to recover the VF!\n");
		return ret;
	}

	return 0;
}

static int hisi_acc_stop_device(struct hisi_acc_vf_mig_info *vmig)
{
	struct device *dev = &vmig->vf_dev->dev;
	struct hisi_qm *vf_qm = &vmig->vf_qm;
	int ret;

	ret = vf_qm_func_stop(vf_qm);
	if (ret) {
		dev_err(dev, "failed to stop QM VF function!\n");
		return ret;
	}

	ret = qm_check_int_state(vmig);
	if (ret) {
		dev_err(dev, "failed to check QM INT state!\n");
		return ret;
	}

	return 0;
}

static int hisi_acc_vf_set_device_state(struct hisi_acc_vf_core_device *vdev,
					u32 state)
{
	struct hisi_acc_vf_mig_info *vmig = &vdev->vmig;
	u32 old_state = vmig->device_state;
	u32 flipped_bits = old_state ^ state;
	int ret;

	if (old_state == VFIO_DEVICE_STATE_ERROR ||
	    !VFIO_DEVICE_STATE_VALID(state) ||
	    (state & ~HISI_ACC_SUPPORTED_DEVICE_STATES))
		return -EINVAL;

	/* Saving switches on and running */
	if ((flipped_bits &
	    (VFIO_DEVICE_STATE_RUNNING | VFIO_DEVICE_STATE_SAVING)) &&
	    ((state & (VFIO_DEVICE_STATE_RUNNING |
	     VFIO_DEVICE_STATE_SAVING)) == (VFIO_DEVICE_STATE_RUNNING |
	     VFIO_DEVICE_STATE_SAVING))) {
		ret = hisi_acc_pre_copy(vmig);
		if (ret)
			return ret;
	}

	/* Running switches off */
	if ((flipped_bits & VFIO_DEVICE_STATE_RUNNING) &&
	    !(state & VFIO_DEVICE_STATE_RUNNING)) {
		ret = hisi_acc_stop_device(vmig);
		if (ret) {
			vmig->device_state = VFIO_DEVICE_STATE_ERROR;
			return ret;
		}
	}

	/* Saving switches on and not running */
	if ((flipped_bits &
	    (VFIO_DEVICE_STATE_RUNNING | VFIO_DEVICE_STATE_SAVING)) &&
	    ((state & (VFIO_DEVICE_STATE_RUNNING |
	     VFIO_DEVICE_STATE_SAVING)) == VFIO_DEVICE_STATE_SAVING)) {
		ret = hisi_acc_post_copy(vmig);
		if (ret)
			return ret;
	}

	/* Resuming switches off */
	if ((flipped_bits & VFIO_DEVICE_STATE_RESUMING) &&
	    !(state & VFIO_DEVICE_STATE_RESUMING)) {
		ret = hisi_acc_load_state(vmig);
		if (ret)
			return ret;
	}

	/* Running switches on */
	if ((flipped_bits & VFIO_DEVICE_STATE_RUNNING) &&
	    (state & VFIO_DEVICE_STATE_RUNNING))
		hisi_acc_start_device(vmig);

	vmig->device_state = state;
	return 0;
}

static int hisi_acc_vf_match_check(struct hisi_acc_vf_mig_info *vmig)
{
	struct acc_vf_data *vf_data = &vmig->vf_data;
	struct hisi_qm *qm = &vmig->vf_qm;
	struct device *dev = &qm->pdev->dev;
	u32 que_iso_state;
	int ret;

	/*
	 * Check we are in the correct dev state and have enough data to
	 * perform the check.
	 */
	if (vmig->device_state != VFIO_DEVICE_STATE_RESUMING ||
	    vmig->data_size != QM_MATCH_SIZE)
		return 0;

	/* vf acc dev type check */
	if (vf_data->dev_id != vmig->vf_dev->device) {
		dev_err(dev, "failed to match VF devices\n");
		return -EINVAL;
	}

	/* vf qp num check */
	ret = qm_get_vft(qm, &qm->qp_base);
	if (ret <= 0) {
		dev_err(dev, "failed to get vft qp nums\n");
		ret = -EINVAL;
		goto out;
	}
	qm->qp_num = ret;

	if (vf_data->qp_num != qm->qp_num) {
		dev_err(dev, "failed to match VF qp num\n");
		ret = -EINVAL;
		goto out;
	}

	/* vf isolation state check */
	ret = qm_read_reg(qm, QM_QUE_ISO_CFG_V, &que_iso_state, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_QUE_ISO_CFG_V\n");
		goto out;
	}
	if (vf_data->que_iso_cfg != que_iso_state) {
		dev_err(dev, "failed to match isolation state\n");
		ret = -EINVAL;
		goto out;
	}

	/* clear the VF match data size */
	vmig->pending_bytes = 0;
	vmig->data_size = 0;

out:
	return ret;
}

static int hisi_acc_vf_data_transfer(struct hisi_acc_vf_core_device *vdev,
				     char __user *buf, size_t count, u64 offset,
				     bool iswrite)
{
	struct hisi_acc_vf_mig_info *vmig = &vdev->vmig;
	void *data_addr = &vmig->vf_data;
	int ret;

	data_addr += offset;
	if (iswrite)  {
		ret = copy_from_user(data_addr, buf, count);
		if (ret)
			return -EFAULT;

		vmig->pending_bytes += count;
	} else {
		ret = copy_to_user(buf, data_addr, count);
		if (ret)
			return -EFAULT;

		vmig->pending_bytes -= count;
	}

	return count;
}

static void hisi_acc_state_mutex_unlock(struct hisi_acc_vf_core_device *vdev)
{
	struct hisi_acc_vf_mig_info *vmig = &vdev->vmig;
again:
	spin_lock(&vdev->reset_lock);
	if (vdev->deferred_reset) {
		vdev->deferred_reset = 0;
		spin_unlock(&vdev->reset_lock);
		vmig->vf_data.vf_qm_state = QM_NOT_READY;
		vmig->device_state = VFIO_DEVICE_STATE_RUNNING;
		goto again;
	}
	spin_unlock(&vdev->reset_lock);
	mutex_unlock(&vdev->state_mutex);
}

static void hisi_acc_pci_aer_reset_done(struct pci_dev *pdev)
{
	struct hisi_acc_vf_core_device *vdev = dev_get_drvdata(&pdev->dev);

	if (!vdev->migration_support)
		return;

	spin_lock(&vdev->reset_lock);
	vdev->deferred_reset = 1;
	if (!mutex_trylock(&vdev->state_mutex)) {
		spin_unlock(&vdev->reset_lock);
		return;
	}
	spin_unlock(&vdev->reset_lock);
	hisi_acc_state_mutex_unlock(vdev);
}

static ssize_t
hisi_acc_rw_data_size(struct hisi_acc_vf_core_device *vdev,
		      char __user *buf, bool iswrite)
{
	struct hisi_acc_vf_mig_info *vmig = &vdev->vmig;
	int ret;

	if (iswrite) {
		ret = copy_from_user(&vmig->data_size, buf,
				     sizeof(vmig->data_size));
		if (ret)
			return -EFAULT;

		/* Check whether the src and dst VF's match */
		ret = hisi_acc_vf_match_check(vmig);
		if (ret)
			return ret;
	} else {
		ret = copy_to_user(buf, &vmig->data_size,
				   sizeof(vmig->data_size));
		if (ret)
			return -EFAULT;
	}
	return sizeof(vmig->data_size);
}

static ssize_t
hisi_acc_rw_data_offset(struct hisi_acc_vf_core_device *vdev,
			char __user *buf, bool iswrite)
{
	u64 data_offset = HISI_ACC_MIG_REGION_DATA_OFFSET;
	int ret;

	if (iswrite)
		return -EFAULT;

	ret = copy_to_user(buf, &data_offset, sizeof(data_offset));
	if (ret)
		return -EFAULT;

	return sizeof(data_offset);
}

static ssize_t
hisi_acc_rw_pending_bytes(struct hisi_acc_vf_core_device *vdev,
			  char __user *buf, bool iswrite)
{
	struct hisi_acc_vf_mig_info *vmig = &vdev->vmig;
	int ret;

	if (iswrite)
		return  -EFAULT;

	ret = copy_to_user(buf, &vmig->pending_bytes,
			   sizeof(vmig->pending_bytes));
	if (ret)
		return -EFAULT;

	return sizeof(vmig->pending_bytes);
}

static ssize_t
hisi_acc_rw_device_state(struct hisi_acc_vf_core_device *vdev,
			 char __user *buf, bool iswrite)
{
	size_t count = sizeof(vdev->vmig.device_state);
	int ret;

	if (iswrite) {
		u32 device_state;

		ret = copy_from_user(&device_state, buf, count);
		if (ret)
			return -EFAULT;

		ret = hisi_acc_vf_set_device_state(vdev, device_state);
		if (ret)
			return ret;
	} else {
		ret = copy_to_user(buf, &vdev->vmig.device_state, count);
		if (ret)
			return -EFAULT;
	}

	return count;
}

static ssize_t hisi_acc_vf_migrn_rw(struct vfio_pci_core_device *core_dev,
				    char __user *buf, size_t count, loff_t *ppos,
				    bool iswrite)
{
	struct hisi_acc_vf_core_device *vdev =
		container_of(core_dev, struct hisi_acc_vf_core_device, core_device);
	u64 pos = *ppos & VFIO_PCI_OFFSET_MASK;
	int ret;

	mutex_lock(&vdev->state_mutex);

	if (pos >= HISI_ACC_MIG_REGION_DATA_OFFSET) {
		ret = hisi_acc_vf_data_transfer(vdev, buf, count,
					pos - HISI_ACC_MIG_REGION_DATA_OFFSET,
					iswrite);
		goto out;
	}

	switch (pos) {
	case VDM_OFFSET(device_state):
		if (count != sizeof(vdev->vmig.device_state)) {
			ret = -EINVAL;
			break;
		}

		ret = hisi_acc_rw_device_state(vdev, buf, iswrite);
		break;
	case VDM_OFFSET(pending_bytes):
		ret = hisi_acc_rw_pending_bytes(vdev, buf, iswrite);
		break;
	case VDM_OFFSET(data_offset):
		ret = hisi_acc_rw_data_offset(vdev, buf, iswrite);
		break;
	case VDM_OFFSET(data_size):
		if (count != sizeof(vdev->vmig.data_size)) {
			ret = -EINVAL;
			break;
		}

		ret = hisi_acc_rw_data_size(vdev, buf, iswrite);
		break;
	default:
		ret = -EFAULT;
	}
out:
	hisi_acc_state_mutex_unlock(vdev);
	return ret;
}

static const struct vfio_pci_regops hisi_acc_vfio_pci_regops = {
	.rw = hisi_acc_vf_migrn_rw,
};

static int hisi_acc_vfio_pci_init(struct hisi_acc_vf_core_device *hisi_acc_vdev)
{
	struct vfio_pci_core_device *vdev = &hisi_acc_vdev->core_device;
	struct hisi_acc_vf_mig_info *vmig = &hisi_acc_vdev->vmig;
	struct pci_dev *vf_dev = vdev->pdev;
	struct hisi_qm *vf_qm = &vmig->vf_qm;
	int ret;

	/*
	 * ACC VF dev BAR2 region consists of both functional register space
	 * and migration control register space. For migration to work, we
	 * need access to both. Hence, we map the entire BAR2 region here.
	 * But from a security point of view, we restrict access to the
	 * migration control space from Guest(Please see mmap/ioctl/read/write
	 * override functions).
	 *
	 * Also the HiSilicon ACC VF devices supported by this driver on
	 * HiSilicon hardware platforms are integrated end point devices
	 * and has no capability to perform PCIe P2P.
	 */
	vf_qm->io_base =
		ioremap(pci_resource_start(vf_dev, VFIO_PCI_BAR2_REGION_INDEX),
			pci_resource_len(vf_dev, VFIO_PCI_BAR2_REGION_INDEX));
	if (!vf_qm->io_base)
		return -EIO;

	vf_qm->fun_type = QM_HW_VF;
	vf_qm->pdev = vf_dev;
	mutex_init(&vf_qm->mailbox_lock);

	vmig->vf_id = PCI_FUNC(vf_dev->devfn);
	vmig->vf_dev = vf_dev;
	vf_qm->dev_name = vmig->pf_qm->dev_name;
	vmig->pf_qm = vmig->pf_qm;

	vmig->data_size = sizeof(struct acc_vf_data);

	ret = vfio_pci_register_dev_region(vdev, VFIO_REGION_TYPE_MIGRATION,
					   VFIO_REGION_SUBTYPE_MIGRATION,
					   &hisi_acc_vfio_pci_regops,
					   HISI_ACC_MIG_REGION_DATA_OFFSET +
					   HISI_ACC_MIG_REGION_DATA_SIZE,
					   VFIO_REGION_INFO_FLAG_READ |
					   VFIO_REGION_INFO_FLAG_WRITE,
					   NULL);
	if (ret)
		goto out;

	return 0;
out:
	iounmap(vf_qm->io_base);
	return ret;
}

static int hisi_acc_pci_rw_access_check(struct vfio_device *core_vdev,
					size_t count, loff_t *ppos,
					size_t *new_count)
{
	unsigned int index = VFIO_PCI_OFFSET_TO_INDEX(*ppos);
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);

	if (index == VFIO_PCI_BAR2_REGION_INDEX) {
		loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;
		resource_size_t end = pci_resource_len(vdev->pdev, index) / 2;

		/* Check if access is for migration control region */
		if (pos >= end)
			return -EINVAL;

		*new_count = min(count, (size_t)(end - pos));
	}

	return 0;
}

static int hisi_acc_vfio_pci_mmap(struct vfio_device *core_vdev,
				  struct vm_area_struct *vma)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev = container_of(core_vdev,
			struct hisi_acc_vf_core_device, core_device.vdev);
	struct vfio_pci_core_device *vdev = &hisi_acc_vdev->core_device;
	unsigned int index;

	index = vma->vm_pgoff >> (VFIO_PCI_OFFSET_SHIFT - PAGE_SHIFT);
	if (index == VFIO_PCI_BAR2_REGION_INDEX) {
		u64 req_len, pgoff, req_start;
		resource_size_t end = pci_resource_len(vdev->pdev, index) / 2;

		req_len = vma->vm_end - vma->vm_start;
		pgoff = vma->vm_pgoff &
			((1U << (VFIO_PCI_OFFSET_SHIFT - PAGE_SHIFT)) - 1);
		req_start = pgoff << PAGE_SHIFT;

		if (req_start + req_len > end)
			return -EINVAL;
	}

	return vfio_pci_core_mmap(core_vdev, vma);
}

static ssize_t hisi_acc_vfio_pci_write(struct vfio_device *core_vdev,
				       const char __user *buf, size_t count,
				       loff_t *ppos)
{
	size_t new_count = count;
	int ret;

	ret = hisi_acc_pci_rw_access_check(core_vdev, count, ppos, &new_count);
	if (ret)
		return ret;

	return vfio_pci_core_write(core_vdev, buf, new_count, ppos);
}

static ssize_t hisi_acc_vfio_pci_read(struct vfio_device *core_vdev,
				      char __user *buf, size_t count,
				      loff_t *ppos)
{
	size_t new_count = count;
	int ret;

	ret = hisi_acc_pci_rw_access_check(core_vdev, count, ppos, &new_count);
	if (ret)
		return ret;

	return vfio_pci_core_read(core_vdev, buf, new_count, ppos);
}

static long hisi_acc_vfio_pci_ioctl(struct vfio_device *core_vdev, unsigned int cmd,
				    unsigned long arg)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev = container_of(core_vdev,
			struct hisi_acc_vf_core_device, core_device.vdev);
	struct vfio_pci_core_device *vdev = &hisi_acc_vdev->core_device;

	if (cmd == VFIO_DEVICE_GET_REGION_INFO) {
		struct pci_dev *pdev = vdev->pdev;
		struct vfio_region_info info;
		unsigned long minsz;

		minsz = offsetofend(struct vfio_region_info, offset);

		if (copy_from_user(&info, (void __user *)arg, minsz))
			return -EFAULT;

		if (info.argsz < minsz)
			return -EINVAL;

		if (info.index == VFIO_PCI_BAR2_REGION_INDEX) {
			info.offset = VFIO_PCI_INDEX_TO_OFFSET(info.index);

			/*
			 * ACC VF dev BAR2 region(64K) consists of both functional
			 * register space and migration control register space.
			 * Report only the first 32K(functional region) to Guest.
			 */
			info.size = pci_resource_len(pdev, info.index) / 2;

			info.flags = VFIO_REGION_INFO_FLAG_READ |
					VFIO_REGION_INFO_FLAG_WRITE |
					VFIO_REGION_INFO_FLAG_MMAP;

			return copy_to_user((void __user *)arg, &info, minsz) ?
					    -EFAULT : 0;
		}
	}
	return vfio_pci_core_ioctl(core_vdev, cmd, arg);
}

static int hisi_acc_vfio_pci_open_device(struct vfio_device *core_vdev)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev = container_of(core_vdev,
			struct hisi_acc_vf_core_device, core_device.vdev);
	struct vfio_pci_core_device *vdev = &hisi_acc_vdev->core_device;
	int ret;

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	if (!hisi_acc_vdev->migration_support) {
		vfio_pci_core_finish_enable(vdev);
		return 0;
	}

	ret = hisi_acc_vfio_pci_init(hisi_acc_vdev);
	if (ret) {
		vfio_pci_core_disable(vdev);
		return ret;
	}

	vfio_pci_core_finish_enable(vdev);
	return 0;
}

static void hisi_acc_vfio_pci_close_device(struct vfio_device *core_vdev)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev = container_of(core_vdev,
			struct hisi_acc_vf_core_device, core_device.vdev);
	struct hisi_acc_vf_mig_info *vmig = &hisi_acc_vdev->vmig;
	struct hisi_qm *vf_qm = &vmig->vf_qm;

	iounmap(vf_qm->io_base);
	vfio_pci_core_close_device(core_vdev);
}

static const struct vfio_device_ops hisi_acc_vfio_pci_ops = {
	.name = "hisi-acc-vfio-pci",
	.open_device = hisi_acc_vfio_pci_open_device,
	.close_device = hisi_acc_vfio_pci_close_device,
	.ioctl = hisi_acc_vfio_pci_ioctl,
	.read = hisi_acc_vfio_pci_read,
	.write = hisi_acc_vfio_pci_write,
	.mmap = hisi_acc_vfio_pci_mmap,
	.request = vfio_pci_core_request,
	.match = vfio_pci_core_match,
};

static int hisi_acc_vfio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev;
	struct hisi_qm *pf_qm;
	int ret;

	hisi_acc_vdev = kzalloc(sizeof(*hisi_acc_vdev), GFP_KERNEL);
	if (!hisi_acc_vdev)
		return -ENOMEM;

	vfio_pci_core_init_device(&hisi_acc_vdev->core_device, pdev,
				  &hisi_acc_vfio_pci_ops);

	pf_qm = hisi_qm_get_pf_qm(pdev);
	if (pf_qm && pf_qm->ver >= QM_HW_V3) {
		struct hisi_acc_vf_mig_info *vmig = &hisi_acc_vdev->vmig;

		hisi_acc_vdev->migration_support = 1;
		vmig->pf_qm = pf_qm;
		mutex_init(&hisi_acc_vdev->state_mutex);
		spin_lock_init(&hisi_acc_vdev->reset_lock);
	}

	ret = vfio_pci_core_register_device(&hisi_acc_vdev->core_device);
	if (ret)
		goto out_free;

	dev_set_drvdata(&pdev->dev, hisi_acc_vdev);
	return 0;

out_free:
	vfio_pci_core_uninit_device(&hisi_acc_vdev->core_device);
	kfree(hisi_acc_vdev);
	return ret;
}

static void hisi_acc_vfio_pci_remove(struct pci_dev *pdev)
{
	struct hisi_acc_vf_core_device *hisi_acc_vdev = dev_get_drvdata(&pdev->dev);

	vfio_pci_core_unregister_device(&hisi_acc_vdev->core_device);
	vfio_pci_core_uninit_device(&hisi_acc_vdev->core_device);
	kfree(hisi_acc_vdev);
}

static const struct pci_device_id hisi_acc_vfio_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, PCI_DEVICE_ID_HUAWEI_SEC_VF) },
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, PCI_DEVICE_ID_HUAWEI_HPRE_VF) },
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, PCI_DEVICE_ID_HUAWEI_ZIP_VF) },
	{ }
};

MODULE_DEVICE_TABLE(pci, hisi_acc_vfio_pci_table);

const struct pci_error_handlers hisi_acc_err_handlers = {
	.reset_done = hisi_acc_pci_aer_reset_done,
	.error_detected = vfio_pci_aer_err_detected,
};

static struct pci_driver hisi_acc_vfio_pci_driver = {
	.name = KBUILD_MODNAME,
	.id_table = hisi_acc_vfio_pci_table,
	.probe = hisi_acc_vfio_pci_probe,
	.remove = hisi_acc_vfio_pci_remove,
	.err_handler = &hisi_acc_err_handlers,
};

module_pci_driver(hisi_acc_vfio_pci_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liu Longfang <liulongfang@huawei.com>");
MODULE_AUTHOR("Shameer Kolothum <shameerali.kolothum.thodi@huawei.com>");
MODULE_DESCRIPTION("HiSilicon VFIO PCI - VFIO PCI driver with live migration support for HiSilicon ACC device family");
