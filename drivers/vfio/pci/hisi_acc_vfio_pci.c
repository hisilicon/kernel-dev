// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, HiSilicon Ltd.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/sysfs.h>
#include <linux/vfio.h>
#include <linux/vfio_pci_core.h>

#include "hisi_acc_vfio_pci.h"

static struct dentry *mig_debugfs_root;
static int mig_root_ref;

/* return 0 VM acc device ready, -ETIMEDOUT hardware timeout */
static int qm_wait_dev_ready(struct hisi_qm *qm)
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

/* Check the  PF's RAS state and Function INT state */
static int qm_check_int_state(struct acc_vf_migration *acc_vf_dev)
{
	struct hisi_qm *vfqm = acc_vf_dev->vf_qm;
	struct hisi_qm *qm = acc_vf_dev->pf_qm;
	struct pci_dev *vf_pdev = acc_vf_dev->vf_dev;
	struct device *dev = &qm->pdev->dev;
	u32 state;

	/* Check RAS state */
	state = acc_check_reg_state(qm, QM_ABNORMAL_INT_STATUS);
	if (state) {
		dev_err(dev, "failed to check QM RAS state!\n");
		return -EBUSY;
	}

	/* Check Function Communication  state between PF and VF */
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
	case HISI_SEC_VF_DEV_ID:
		state = acc_check_reg_state(qm, SEC_CORE_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM SEC Core INT state!\n");
			return -EBUSY;
		}
		break;
	case HISI_HPRE_VF_DEV_ID:
		state = acc_check_reg_state(qm, HPRE_HAC_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM HPRE HAC INT state!\n");
			return -EBUSY;
		}
		break;
	case HISI_ZIP_VF_DEV_ID:
		state = acc_check_reg_state(qm, HZIP_CORE_INT_STATUS);
		if (state) {
			dev_err(dev, "failed to check QM ZIP Core INT state!\n");
			return -EBUSY;
		}
		break;
	default:
		dev_err(dev, "failed to detect acc module type!\n");
		return -EINVAL;
	}

	return 0;
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

static int qm_write_reg(struct hisi_qm *qm, u32 reg_addr,
			u32 *data, u8 nums)
{
	int i;

	if (nums < 1 || nums > QM_REGS_MAX_LEN)
		return -EINVAL;

	for (i = 0; i < nums; i++) {
		writel(data[i], qm->io_base + reg_addr);
		reg_addr += QM_REG_ADDR_OFFSET;
	}

	return 0;
}

static int qm_get_vft(struct hisi_qm *qm, u32 *base, u32 *number)
{
	u64 sqc_vft;
	int ret;

	ret = qm_mb(qm, QM_MB_CMD_SQC_VFT_V2, 0, 0, 1);
	if (ret)
		return ret;

	sqc_vft = readl(qm->io_base + QM_MB_CMD_DATA_ADDR_L) |
		  ((u64)readl(qm->io_base + QM_MB_CMD_DATA_ADDR_H) <<
		  QM_XQC_ADDR_OFFSET);
	*base = QM_SQC_VFT_BASE_MASK_V2 & (sqc_vft >> QM_SQC_VFT_BASE_SHIFT_V2);
	*number = (QM_SQC_VFT_NUM_MASK_V2 &
		  (sqc_vft >> QM_SQC_VFT_NUM_SHIFT_V2)) + 1;

	return 0;
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

	ret = qm_read_reg(qm, QM_VF_STATE, &vf_data->vf_state, 1);
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

	ret = qm_write_reg(qm, QM_VF_STATE, &vf_data->vf_state, 1);
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

/*
 * the vf QM have unbind from host, insmod in the VM
 * so, qm just have the addr from pci dev
 * others is null.
 * so we need read from the SEC hardware REGs.
 */
static int vf_migration_data_store(struct hisi_qm *qm,
				   struct acc_vf_migration *acc_vf_dev)
{
	struct acc_vf_data *vf_data = acc_vf_dev->vf_data;
	struct device *dev = &qm->pdev->dev;
	int ret;

	ret = qm_rw_regs_read(qm, vf_data);
	if (ret)
		return -EINVAL;

	/*
	 * every Reg is 32 bit, the dma address is 64 bit
	 * so, the dma address is store in the Reg2 and Reg1
	 */
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
	/* clear VF communication status registers. */
	writel(0x1, qm->io_base + QM_IFC_INT_SOURCE_V);

	/* enable pf and vf communication. */
	writel(0x0, qm->io_base + QM_IFC_INT_MASK);
}

static void vf_qm_fun_restart(struct hisi_qm *qm,
			      struct acc_vf_migration *acc_vf_dev)
{
	struct acc_vf_data *vf_data = acc_vf_dev->vf_data;
	struct device *dev = &qm->pdev->dev;
	int i;

	/* Check if we need to restart VF */
	if (vf_data->vf_state == VF_PREPARE) {
		dev_info(dev, "No need to restart VF\n");
		return;
	}

	for (i = 0; i < qm->qp_num; i++)
		qm_db(qm, i, QM_DOORBELL_CMD_SQ, 0, 1);
}

static int vf_migration_data_recover(struct hisi_qm *qm,
				     struct acc_vf_migration *acc_vf_dev)
{
	struct device *dev = &qm->pdev->dev;
	struct acc_vf_data *vf_data = acc_vf_dev->vf_data;
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	int ret;

	if (!mig_ctl->data_size)
		return 0;

	qm->eqe_dma = vf_data->eqe_dma;
	qm->aeqe_dma = vf_data->aeqe_dma;
	qm->sqc_dma = vf_data->sqc_dma;
	qm->cqc_dma = vf_data->cqc_dma;

	qm->qp_base = vf_data->qp_base;
	qm->qp_num = vf_data->qp_num;

	ret = qm_rw_regs_write(qm, vf_data);
	if (ret) {
		dev_err(dev, "Set VF regs failed!\n");
		return ret;
	}

	ret = qm_mb(qm, QM_MB_CMD_SQC_BT, qm->sqc_dma, 0, 0);
	if (ret) {
		dev_err(dev, "Set sqc failed!\n");
		return ret;
	}

	ret = qm_mb(qm, QM_MB_CMD_CQC_BT, qm->cqc_dma, 0, 0);
	if (ret) {
		dev_err(dev, "Set cqc failed!\n");
		return ret;
	}

	/* which ACC module need to reinit? */
	qm_dev_cmd_init(qm);

	return 0;
}

static int vf_qm_cache_wb(struct hisi_qm *qm)
{
	unsigned int val;

	writel(0x1, qm->io_base + QM_CACHE_WB_START);
	if (readl_relaxed_poll_timeout(qm->io_base + QM_CACHE_WB_DONE,
				       val, val & BIT(0), MB_POLL_PERIOD_US,
				       MB_POLL_TIMEOUT_US)) {
		dev_err(&qm->pdev->dev, "vf QM writeback sqc cache fail!\n");
		return -EINVAL;
	}

	return 0;
}

static int vf_qm_func_stop(struct hisi_qm *qm)
{
	return qm_mb(qm, QM_MB_CMD_PAUSE_QM, 0, 0, 0);
}

static int pf_qm_get_qp_num(struct hisi_qm *qm, int vf_id,
			    u32 *rbase, u32 *rnumber)
{
	unsigned int val;
	u64 sqc_vft;
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
	*rnumber = (QM_SQC_VFT_NUM_MASK_V2 &
		  (sqc_vft >> QM_SQC_VFT_NUM_SHIFT_V2)) + 1;

	return 0;
}

static int pf_qm_state_pre_save(struct hisi_qm *qm,
				struct acc_vf_migration *acc_vf_dev)
{
	struct acc_vf_data *vf_data = acc_vf_dev->vf_data;
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	struct device *dev = &qm->pdev->dev;
	int vf_id = acc_vf_dev->vf_id;
	int ret;

	/* save device id */
	vf_data->dev_id = acc_vf_dev->vf_dev->device;

	/* vf qp num save from PF */
	ret = pf_qm_get_qp_num(qm, vf_id, &qm->qp_base, &qm->qp_num);
	if (ret || qm->qp_num <= 1) {
		dev_err(dev, "failed to get vft qp nums!\n");
		return -EINVAL;
	}
	vf_data->qp_base = qm->qp_base;
	vf_data->qp_num = qm->qp_num;

	/* vf isolation state save from PF */
	ret = qm_read_reg(qm, QM_QUE_ISO_CFG, &vf_data->que_iso_cfg, 1);
	if (ret) {
		dev_err(dev, "failed to read QM_QUE_ISO_CFG!\n");
		return ret;
	}

	mig_ctl->data_size = QM_MATCH_SIZE;
	mig_ctl->pending_bytes = mig_ctl->data_size;

	return 0;
}

static int vf_qm_state_save(struct hisi_qm *qm,
			    struct acc_vf_migration *acc_vf_dev)
{
	struct device *dev = &acc_vf_dev->vf_dev->dev;
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	int ret;

	mig_ctl->data_size = 0;
	mig_ctl->pending_bytes = 0;

	/*
	 * check VM task driver state
	 * if vf_ready == 0x1, skip migrate.
	 */
	if (unlikely(qm_wait_dev_ready(qm))) {
		dev_info(&qm->pdev->dev, "QM device not ready, no data to transfer\n");
		return 0;
	}

	/* First stop the ACC vf function */
	ret = vf_qm_func_stop(qm);
	if (ret) {
		dev_err(dev, "failed to stop QM VF function!\n");
		return ret;
	}

	/* Check the VF's RAS and Interrution state */
	ret = qm_check_int_state(acc_vf_dev);
	if (ret) {
		dev_err(dev, "failed to check QM INT state!\n");
		goto state_error;
	}

	/* hisi_qm_cache_wb store cache data to DDR */
	ret = vf_qm_cache_wb(qm);
	if (ret) {
		dev_err(dev, "failed to writeback QM Cache!\n");
		goto state_error;
	}

	ret = vf_migration_data_store(qm, acc_vf_dev);
	if (ret) {
		dev_err(dev, "failed to get and store migration data!\n");
		goto state_error;
	}

	mig_ctl->data_size = sizeof(struct acc_vf_data);
	mig_ctl->pending_bytes = mig_ctl->data_size;

	return 0;

state_error:
	vf_qm_fun_restart(qm, acc_vf_dev);
	return ret;
}

static int vf_qm_state_resume(struct hisi_qm *qm,
			      struct acc_vf_migration *acc_vf_dev)
{
	struct device *dev = &acc_vf_dev->vf_dev->dev;
	int ret;

	/* recover data to VF */
	ret = vf_migration_data_recover(qm, acc_vf_dev);
	if (ret) {
		dev_err(dev, "failed to recover the VF!\n");
		return ret;
	}

	/* restart all destination VF's QP */
	vf_qm_fun_restart(qm, acc_vf_dev);

	return 0;
}

/*
 * HiSilicon ACC VF dev MMIO space contains both the functional register
 * space and the migration control register space. We hide the migration
 * control space from the Guest. But to successfully complete the live
 * migration, we still need access to the functional MMIO space assigned
 * to the Guest. We only need to access these while the Guest vCPUS are
 * not running. To avoid any potential security issue, we need to be
 * careful not to access these regions while the Guest vCPUs are running.
 *
 * Hence check the device state before we map the region.
 */
static int hisi_acc_vf_ioremap(struct acc_vf_migration *acc_vf_dev, u32 state)
{
	struct hisi_qm *vfqm = acc_vf_dev->vf_qm;
	struct pci_dev *pdev = acc_vf_dev->vf_dev;

	if (state == (VFIO_DEVICE_STATE_SAVING | VFIO_DEVICE_STATE_RUNNING))
		return -EINVAL;

	if (vfqm->io_base)
		return 0;

	vfqm->io_base = ioremap(vfqm->phys_base,
				pci_resource_len(pdev, VFIO_PCI_BAR2_REGION_INDEX));
	if (!vfqm->io_base)
		return -EIO;
	return 0;
}

static void hisi_acc_vf_iounmap(struct acc_vf_migration *acc_vf_dev)
{
	struct hisi_qm *vfqm = acc_vf_dev->vf_qm;

	if (!vfqm->io_base)
		return;

	iounmap(vfqm->io_base);
	vfqm->io_base = NULL;
}

static int hisi_acc_vf_set_device_state(struct acc_vf_migration *acc_vf_dev,
					u32 state)
{
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	struct hisi_qm *pfqm = acc_vf_dev->pf_qm;
	struct hisi_qm *vfqm = acc_vf_dev->vf_qm;
	struct device *dev = &acc_vf_dev->vf_dev->dev;
	int ret = 0;

	if (state == mig_ctl->device_state)
		return 0;

	switch (state) {
	case VFIO_DEVICE_STATE_RUNNING:
		if (mig_ctl->device_state == VFIO_DEVICE_STATE_RESUMING) {
			ret = hisi_acc_vf_ioremap(acc_vf_dev, state);
			if (ret)
				return ret;

			ret = vf_qm_state_resume(vfqm, acc_vf_dev);
			if (ret)
				goto out;
		}
		break;
	case VFIO_DEVICE_STATE_SAVING | VFIO_DEVICE_STATE_RUNNING:
		/* ACC should in the pre cycle to read match information data */
		ret = pf_qm_state_pre_save(pfqm, acc_vf_dev);
		if (ret)
			return ret;

		break;
	case VFIO_DEVICE_STATE_SAVING:
		/* stop the vf function */
		ret = hisi_acc_vf_ioremap(acc_vf_dev, state);
		if (ret)
			return ret;

		ret = vf_qm_state_save(vfqm, acc_vf_dev);
		if (ret)
			goto out;

		break;
	case VFIO_DEVICE_STATE_STOP:
		ret = hisi_acc_vf_ioremap(acc_vf_dev, state);
		if (ret)
			return ret;
		/* restart all  VF's QP */
		vf_qm_fun_restart(vfqm, acc_vf_dev);
		break;
	case VFIO_DEVICE_STATE_RESUMING:
		break;
	default:
		return -EFAULT;
	}

	dev_info(dev, "migration state: %s ----------> %s!\n",
		 vf_dev_state[mig_ctl->device_state],
		 vf_dev_state[state]);

	mig_ctl->device_state = state;

out:
	hisi_acc_vf_iounmap(acc_vf_dev);
	return ret;
}

static int hisi_acc_vf_match_check(struct acc_vf_migration *acc_vf_dev)
{
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	struct acc_vf_data *vf_data = acc_vf_dev->vf_data;
	struct hisi_qm *qm = acc_vf_dev->vf_qm;
	struct device *dev = &qm->pdev->dev;
	u32 que_iso_state;
	int ret;

	/*
	 * Check we are in the correct dev state and have enough data to
	 * perform the check.
	 */
	if (mig_ctl->device_state != VFIO_DEVICE_STATE_RESUMING ||
	    mig_ctl->pending_bytes != QM_MATCH_SIZE ||
	    mig_ctl->data_size != QM_MATCH_SIZE)
		return 0;

	/* vf acc dev type check */
	if (vf_data->dev_id != acc_vf_dev->vf_dev->device) {
		dev_err(dev, "failed to match VF devices\n");
		return -EINVAL;
	}

	ret = hisi_acc_vf_ioremap(acc_vf_dev, mig_ctl->device_state);
	if (ret)
		return ret;

	/* vf qp num check */
	ret = qm_get_vft(qm, &qm->qp_base, &qm->qp_num);
	if (ret || qm->qp_num <= 1) {
		dev_err(dev, "failed to get vft qp nums\n");
		goto out;
	}
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
	mig_ctl->pending_bytes = 0;
	mig_ctl->data_size = 0;

out:
	hisi_acc_vf_iounmap(acc_vf_dev);
	return ret;
}

static int hisi_acc_vf_data_transfer(struct acc_vf_migration *acc_vf_dev,
				     char __user *buf, size_t count, u64 pos,
				     bool iswrite)
{
	struct vfio_device_migration_info *mig_ctl = acc_vf_dev->mig_ctl;
	void *data_addr = acc_vf_dev->vf_data;
	int ret = 0;

	if (!count || pos < mig_ctl->data_offset || pos > MIGRATION_REGION_SZ)
		return -EINVAL;

	data_addr += pos - mig_ctl->data_offset;
	if (iswrite)  {
		ret = copy_from_user(data_addr, buf, count) ?
				     -EFAULT : count;
		if (ret == count)
			mig_ctl->pending_bytes += count;
	} else {
		ret = copy_to_user(buf, data_addr, count) ?
				   -EFAULT : count;
		if (ret == count)
			mig_ctl->pending_bytes -= count;
	}

	return ret;
}

static size_t hisi_acc_vf_migrn_rw(struct vfio_pci_core_device *vdev,
				   char __user *buf, size_t count, loff_t *ppos,
				   bool iswrite)
{
	unsigned int index = VFIO_PCI_OFFSET_TO_INDEX(*ppos) -
				VFIO_PCI_NUM_REGIONS;
	struct vfio_pci_region	*region = &vdev->region[index];
	struct vfio_device_migration_info *mig_ctl;
	struct acc_vf_migration *acc_vf_dev;
	u64 pos = *ppos & VFIO_PCI_OFFSET_MASK;
	struct device *dev;
	u32 device_state;
	int ret = 0;

	if (!region)
		return -EINVAL;

	acc_vf_dev = region->data;
	dev = &acc_vf_dev->vf_dev->dev;
	mig_ctl = acc_vf_dev->mig_ctl;

	if (pos >= region->size)
		return -EINVAL;

	count = min(count, (size_t)(region->size - pos));

	switch (pos) {
	case VDM_OFFSET(device_state):
		if (count != sizeof(mig_ctl->device_state)) {
			ret = -EINVAL;
			break;
		}

		if (iswrite) {
			if (copy_from_user(&device_state, buf, count)) {
				ret = -EFAULT;
				break;
			}

			ret = hisi_acc_vf_set_device_state(acc_vf_dev,
							   device_state) ? ret : count;
		} else {
			ret = copy_to_user(buf, &mig_ctl->device_state,
					   count) ? -EFAULT : count;
		}
		break;
	case VDM_OFFSET(reserved):
		return 0;
	case VDM_OFFSET(pending_bytes):
		if (count != sizeof(mig_ctl->pending_bytes))
			return -EINVAL;

		if (iswrite)
			return -EFAULT;
		else
			ret = copy_to_user(buf, &mig_ctl->pending_bytes,
					   count) ? -EFAULT : count;
		break;
	case VDM_OFFSET(data_offset):
		if (count != sizeof(mig_ctl->data_offset))
			return -EINVAL;

		if (iswrite)
			ret = copy_from_user(&mig_ctl->data_offset, buf,
					     count) ? -EFAULT : count;
		else
			ret = copy_to_user(buf, &mig_ctl->data_offset,
					   count) ? -EFAULT : count;
		break;
	case VDM_OFFSET(data_size):
		if (count != sizeof(mig_ctl->data_size))
			return -EINVAL;

		if (iswrite) {
			ret = copy_from_user(&mig_ctl->data_size, buf,
					     count) ? -EFAULT : count;
			if (ret > 0) {
				/* Check whether the src and dst VF's match */
				ret = hisi_acc_vf_match_check(acc_vf_dev);
				if (!ret)
					ret = count;
			}
		} else {
			ret = copy_to_user(buf, &mig_ctl->data_size,
					   count) ? -EFAULT : count;
		}
		break;
	default:
		/* Transfer data */
		ret = hisi_acc_vf_data_transfer(acc_vf_dev, buf,
						count, pos, iswrite);
	}

	return ret;
}

static ssize_t acc_vf_debug_read(struct file *filp, char __user *buffer,
			   size_t count, loff_t *pos)
{
	char buf[VFIO_DEV_DBG_LEN];
	int len;

	len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
			"echo 0: test vf data store\n"
			"echo 1: test vf data writeback\n"
			"echo 2: test vf send mailbox\n"
			"echo 3: dump vf dev data\n"
			"echo 4: dump migration state\n");

	return simple_read_from_buffer(buffer, count, pos, buf, len);
}

static ssize_t acc_vf_debug_write(struct file *filp, const char __user *buffer,
			    size_t count, loff_t *pos)
{
	struct acc_vf_migration *acc_vf_dev = filp->private_data;
	struct device *dev = &acc_vf_dev->vf_dev->dev;
	struct hisi_qm *qm = acc_vf_dev->vf_qm;
	char tbuf[VFIO_DEV_DBG_LEN];
	unsigned long val;
	u64 data;
	int len, ret;

	if (*pos)
		return 0;

	if (count >= VFIO_DEV_DBG_LEN)
		return -ENOSPC;

	len = simple_write_to_buffer(tbuf, VFIO_DEV_DBG_LEN - 1,
					pos, buffer, count);
	if (len < 0)
		return len;
	tbuf[len] = '\0';
	if (kstrtoul(tbuf, 0, &val))
		return -EFAULT;

	ret = hisi_acc_vf_ioremap(acc_vf_dev, VFIO_DEVICE_STATE_STOP);
	if (ret)
		return -EINVAL;

	switch (val) {
	case STATE_SAVE:
		ret = vf_qm_state_save(qm, acc_vf_dev);
		if (ret)
			goto out;
		break;
	case STATE_RESUME:
		ret = vf_qm_state_resume(qm, acc_vf_dev);
		if (ret)
			goto out;
		break;
	case MB_TEST:
		data = readl(qm->io_base + QM_MB_CMD_SEND_BASE);
		dev_info(dev, "debug mailbox addr: 0x%lx, mailbox val: 0x%llx\n",
			 (uintptr_t)qm->io_base, data);
		break;
	case MIG_DATA_DUMP:
		dev_info(dev, "dumped vf migration data:\n");
		print_hex_dump(KERN_INFO, "Mig Data:", DUMP_PREFIX_OFFSET,
				VFIO_DBG_LOG_LEN, 1,
				(unsigned char *)acc_vf_dev->vf_data,
				sizeof(struct acc_vf_data), false);
		break;
	case MIG_DEV_SHOW:
		if (!acc_vf_dev->mig_ctl)
			dev_info(dev, "migration region have release!\n");
		else
			dev_info(dev,
				 "device  state: %u\n"
				 "pending bytes: %llu\n"
				 "data   offset: %llu\n"
				 "data     size: %llu\n"
				 "data     addr: 0x%lx\n",
				 acc_vf_dev->mig_ctl->device_state,
				 acc_vf_dev->mig_ctl->pending_bytes,
				 acc_vf_dev->mig_ctl->data_offset,
				 acc_vf_dev->mig_ctl->data_size,
				 (uintptr_t)acc_vf_dev->vf_data);
		break;
	default:
		ret = -EINVAL;
	}

	if (!ret)
		ret = count;
out:
	hisi_acc_vf_iounmap(acc_vf_dev);
	return ret;
}

static const struct file_operations acc_vf_debug_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = acc_vf_debug_read,
	.write = acc_vf_debug_write,
};

static ssize_t acc_vf_state_read(struct file *filp, char __user *buffer,
			   size_t count, loff_t *pos)
{
	struct acc_vf_migration *acc_vf_dev = filp->private_data;
	char buf[VFIO_DEV_DBG_LEN];
	u32 state;
	int len;

	if (!acc_vf_dev->mig_ctl) {
		len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n", "Invalid\n");
	} else {
		state = acc_vf_dev->mig_ctl->device_state;
		switch (state) {
		case VFIO_DEVICE_STATE_RUNNING:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"RUNNING\n");
			break;
		case VFIO_DEVICE_STATE_SAVING | VFIO_DEVICE_STATE_RUNNING:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"SAVING and RUNNING\n");
			break;
		case VFIO_DEVICE_STATE_SAVING:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"SAVING\n");
			break;
		case VFIO_DEVICE_STATE_STOP:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"STOP\n");
			break;
		case VFIO_DEVICE_STATE_RESUMING:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"RESUMING\n");
			break;
		default:
			len = scnprintf(buf, VFIO_DEV_DBG_LEN, "%s\n",
				"Error\n");
		}
	}

	return simple_read_from_buffer(buffer, count, pos, buf, len);
}

static const struct file_operations acc_vf_state_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = acc_vf_state_read,
};

static void vf_debugfs_init(struct acc_vf_migration *acc_vf_dev)
{
	char name[VFIO_DEV_DBG_LEN];
	int node_id;

	if (!mig_root_ref)
		mig_debugfs_root = debugfs_create_dir("vfio_acc", NULL);
	mutex_lock(&acc_vf_dev->reflock);
	mig_root_ref++;
	mutex_unlock(&acc_vf_dev->reflock);

	node_id = dev_to_node(&acc_vf_dev->vf_dev->dev);
	if (node_id < 0)
		node_id = 0;

	if (acc_vf_dev->vf_dev->device == HISI_SEC_VF_DEV_ID)
		scnprintf(name, VFIO_DEV_DBG_LEN, "sec_vf%d-%d",
			  node_id, acc_vf_dev->vf_id);
	else if (acc_vf_dev->vf_dev->device == HISI_HPRE_VF_DEV_ID)
		scnprintf(name, VFIO_DEV_DBG_LEN, "hpre_vf%d-%d",
			  node_id, acc_vf_dev->vf_id);
	else
		scnprintf(name, VFIO_DEV_DBG_LEN, "zip_vf%d-%d",
			  node_id, acc_vf_dev->vf_id);

	acc_vf_dev->debug_root = debugfs_create_dir(name, mig_debugfs_root);

	debugfs_create_file("debug", 0644, acc_vf_dev->debug_root,
			      acc_vf_dev, &acc_vf_debug_fops);
	debugfs_create_file("state", 0444, acc_vf_dev->debug_root,
			      acc_vf_dev, &acc_vf_state_fops);
}

static void vf_debugfs_exit(struct acc_vf_migration *acc_vf_dev)
{
	debugfs_remove_recursive(acc_vf_dev->debug_root);

	mutex_lock(&acc_vf_dev->reflock);
	mig_root_ref--;
	mutex_unlock(&acc_vf_dev->reflock);

	if (!mig_root_ref)
		debugfs_remove_recursive(mig_debugfs_root);
}

static void hisi_acc_vfio_pci_uninit(struct acc_vf_migration *acc_vf_dev)
{
	kfree(acc_vf_dev->mig_ctl);
	kfree(acc_vf_dev->vf_qm);
	kfree(acc_vf_dev);
	vf_debugfs_exit(acc_vf_dev);
}

static void hisi_acc_vf_migrn_release(struct vfio_pci_core_device *vdev,
				      struct vfio_pci_region *region)
{
	struct acc_vf_migration *acc_vf_dev = region->data;

	hisi_acc_vfio_pci_uninit(acc_vf_dev);
}

static const struct vfio_pci_regops hisi_acc_vfio_pci_regops = {
	.rw = hisi_acc_vf_migrn_rw,
	.release = hisi_acc_vf_migrn_release,
};

static int hisi_acc_vf_dev_init(struct pci_dev *pdev, struct hisi_qm *pf_qm,
				struct acc_vf_migration *acc_vf_dev)
{
	struct vfio_device_migration_info *mig_ctl;
	struct hisi_qm *vf_qm;
	__u64 mig_offset;
	void *vf_data;

	vf_qm = kzalloc(sizeof(*vf_qm), GFP_KERNEL);
	if (!vf_qm)
		return -ENOMEM;

	vf_qm->dev_name = pf_qm->dev_name;
	vf_qm->fun_type = QM_HW_VF;
	vf_qm->phys_base = pci_resource_start(pdev, VFIO_PCI_BAR2_REGION_INDEX);
	vf_qm->pdev = pdev;
	mutex_init(&vf_qm->mailbox_lock);

	acc_vf_dev->vf_qm = vf_qm;
	acc_vf_dev->pf_qm = pf_qm;

	/* the data region must follow migration info */
	mig_offset = sizeof(*mig_ctl);
	mig_ctl = kzalloc(MIGRATION_REGION_SZ, GFP_KERNEL);
	if (!mig_ctl)
		goto init_qm_error;

	acc_vf_dev->mig_ctl = mig_ctl;

	vf_data = (void *)mig_ctl + mig_offset;
	acc_vf_dev->vf_data = vf_data;

	mig_ctl->device_state = VFIO_DEVICE_STATE_RUNNING;
	mig_ctl->data_offset = mig_offset;
	mig_ctl->data_size = sizeof(struct acc_vf_data);

	return 0;

init_qm_error:
	kfree(vf_qm);
	return -ENOMEM;
}

static int hisi_acc_vfio_pci_init(struct vfio_pci_core_device *vdev)
{
	struct acc_vf_migration *acc_vf_dev;
	struct pci_dev *pdev = vdev->pdev;
	struct pci_dev *pf_dev, *vf_dev;
	struct hisi_qm *pf_qm;
	int vf_id, ret;

	pf_dev = pdev->physfn;
	vf_dev = pdev;

	pf_qm = pci_get_drvdata(pf_dev);
	if (!pf_qm) {
		pr_err("HiSi ACC qm driver not loaded\n");
		return -EINVAL;
	}

	if (pf_qm->ver < QM_HW_V3) {
		dev_err(&pdev->dev,
			"Migration not supported, hw version: 0x%x\n",
			 pf_qm->ver);
		return -ENODEV;
	}

	vf_id = PCI_FUNC(vf_dev->devfn);
	acc_vf_dev = kzalloc(sizeof(*acc_vf_dev), GFP_KERNEL);
	if (!acc_vf_dev)
		return -ENOMEM;

	acc_vf_dev->vf_id = vf_id;
	acc_vf_dev->pf_dev = pf_dev;
	acc_vf_dev->vf_dev = vf_dev;
	mutex_init(&acc_vf_dev->reflock);

	ret = hisi_acc_vf_dev_init(pdev, pf_qm, acc_vf_dev);
	if (ret) {
		kfree(acc_vf_dev);
		return -ENOMEM;
	}

	ret = vfio_pci_register_dev_region(vdev, VFIO_REGION_TYPE_MIGRATION,
					   VFIO_REGION_SUBTYPE_MIGRATION,
					   &hisi_acc_vfio_pci_regops,
					   MIGRATION_REGION_SZ,
					   VFIO_REGION_INFO_FLAG_READ |
					   VFIO_REGION_INFO_FLAG_WRITE,
					   acc_vf_dev);
	if (ret)
		hisi_acc_vfio_pci_uninit(acc_vf_dev);
	else
		vf_debugfs_init(acc_vf_dev);

	return ret;
}

static long hisi_acc_vfio_pci_ioctl(struct vfio_device *core_vdev, unsigned int cmd,
				    unsigned long arg)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);
	unsigned long minsz;

	if (cmd == VFIO_DEVICE_GET_REGION_INFO) {
		struct pci_dev *pdev = vdev->pdev;
		struct vfio_region_info info;

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

static int hisi_acc_vfio_pci_open(struct vfio_device *core_vdev)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);
	int ret;

	lockdep_assert_held(&core_vdev->reflck->lock);

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	ret = hisi_acc_vfio_pci_init(vdev);
	if (ret) {
		vfio_pci_core_disable(vdev);
		return ret;
	}

	vfio_pci_core_finish_enable(vdev);

	return 0;
}

static const struct vfio_device_ops hisi_acc_vfio_pci_ops = {
	.name		= "hisi-acc-vfio-pci",
	.open		= hisi_acc_vfio_pci_open,
	.release	= vfio_pci_core_release,
	.ioctl		= hisi_acc_vfio_pci_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
	.reflck_attach	= vfio_pci_core_reflck_attach,
};

static int hisi_acc_vfio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct vfio_pci_core_device *vdev;
	int ret;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev)
		return -ENOMEM;

	ret = vfio_pci_core_register_device(vdev, pdev, &hisi_acc_vfio_pci_ops);
	if (ret)
		goto out_free;

	dev_set_drvdata(&pdev->dev, vdev);
	return 0;

out_free:
	kfree(vdev);
	return ret;
}

static void hisi_acc_vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_pci_core_device *vdev = dev_get_drvdata(&pdev->dev);

	vfio_pci_core_unregister_device(vdev);
	kfree(vdev);
}

static const struct pci_device_id hisi_acc_vfio_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, HISI_SEC_VF_DEV_ID) },
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, HISI_HPRE_VF_DEV_ID) },
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, HISI_ZIP_VF_DEV_ID) },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, hisi_acc_vfio_pci_table);

static struct pci_driver hisi_acc_vfio_pci_driver = {
	.name			= "hisi-acc-vfio-pci",
	.id_table		= hisi_acc_vfio_pci_table,
	.probe			= hisi_acc_vfio_pci_probe,
	.remove			= hisi_acc_vfio_pci_remove,
#ifdef CONFIG_PCI_IOV
	.sriov_configure	= vfio_pci_core_sriov_configure,
#endif
	.err_handler		= &vfio_pci_core_err_handlers,
};

static void __exit hisi_acc_pci_cleanup(void)
{
	pci_unregister_driver(&hisi_acc_vfio_pci_driver);
}

static int __init hisi_acc_pci_init(void)
{
	return pci_register_driver(&hisi_acc_vfio_pci_driver);
}

module_init(hisi_acc_pci_init);
module_exit(hisi_acc_pci_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liu Longfang <liulongfang@huawei.com>");
MODULE_AUTHOR("Shameer Kolothum <shameerali.kolothum.thodi@huawei.com>");
MODULE_DESCRIPTION("HiSilicon VFIO PCI - User Level meta-driver for HiSilicon ACC device family");
