/*
 * Copyright (c) 2015 Linaro Ltd.
 * Copyright (c) 2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "hisi_sas.h"
#include <linux/swab.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>

struct hisi_sas_cmd_hdr_dw0 {
	u32 abort_flag:2;
	u32 abort_device_type:1;
	u32 rsvd:2;
	u32 resp_report:1;
	u32 tlr_ctrl:2;
	u32 phy_id:9;
	u32 force_phy:1;
	u32 port:4;
	u32 rsvd0:5;
	u32 priority:1;
	u32 rsvd1:1;
	u32 cmd:3;
};

struct hisi_sas_cmd_hdr_dw1 {
	u32 rsvd:4;
	u32 ssp_pass_through:1;
	u32 dir:2;
	u32 reset:1;
	u32 pir_pres:1;
	u32 enable_tlr:1;
	u32 verify_dtl:1;
	u32 frame_type:5;
	u32 device_id:16;
};

struct hisi_sas_cmd_hdr_dw2 {
	u32 cmd_frame_len:9;
	u32 leave_affil_open:1;
	u32 ncq_tag:5;
	u32 max_resp_frame_len:9;
	u32 sg_mode:2;
	u32 first_burst:1;
	u32 rsvd3:5;
};

#define DIR_NO_DATA 0
#define DIR_TO_INI 1
#define DIR_TO_DEVICE 2
#define DIR_RESERVED 3

#define SATA_PROTOCOL_NONDATA		0x1
#define SATA_PROTOCOL_PIO		0x2
#define SATA_PROTOCOL_DMA		0x4
#define SATA_PROTOCOL_FPDMA		0x8
#define SATA_PROTOCOL_ATAPI		0x10

static u8 get_ata_protocol(u8 cmd, int direction)
{
	switch (cmd) {
	case ATA_CMD_FPDMA_WRITE:
	case ATA_CMD_FPDMA_READ:
	return SATA_PROTOCOL_FPDMA;

	case ATA_CMD_ID_ATA:
	case ATA_CMD_PMP_READ:
	case ATA_CMD_READ_LOG_EXT:
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_READ_EXT:
	case ATA_CMD_PMP_WRITE:
	case ATA_CMD_WRITE_LOG_EXT:
	case ATA_CMD_PIO_WRITE:
	case ATA_CMD_PIO_WRITE_EXT:
	return SATA_PROTOCOL_PIO;

	case ATA_CMD_READ:
	case ATA_CMD_READ_EXT:
	case /* write dma queued */ 0xc7: /* j00310691 fixme */
	case /* write dma queued ext */ 0x26: /* j00310691 fixme */
	case ATA_CMD_READ_LOG_DMA_EXT:
	case ATA_CMD_WRITE:
	case ATA_CMD_WRITE_EXT:
	case /* write dma queued ext */ 0xcc: /* j00310691 fixme */
	case ATA_CMD_WRITE_QUEUED:
	case ATA_CMD_WRITE_LOG_DMA_EXT:
	return SATA_PROTOCOL_DMA;

	case 0x92: /* j00310691 fixme */
	case ATA_CMD_DEV_RESET:
	case ATA_CMD_CHK_POWER:
	case ATA_CMD_FLUSH:
	case ATA_CMD_FLUSH_EXT:
	case ATA_CMD_VERIFY:
	case ATA_CMD_VERIFY_EXT:
	case ATA_CMD_SET_FEATURES:
	case ATA_CMD_STANDBY:
	case ATA_CMD_STANDBYNOW1:
	return SATA_PROTOCOL_NONDATA;

	default:
		if (direction == DMA_NONE)
			return SATA_PROTOCOL_NONDATA;
		return SATA_PROTOCOL_PIO;
	}
}

static int hisi_sas_prep_prd_sge(struct hisi_hba *hisi_hba,
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		dev_err(hisi_hba->dev, "%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT",
			__func__, n_elem);
		return -EINVAL;
	}

	slot->sge_page = dma_pool_alloc(hisi_hba->sge_page_pool, GFP_ATOMIC,
					&slot->sge_page_dma);
	if (!slot->sge_page)
		return -ENOMEM;

	dw1->pir_pres = 0;

	for_each_sg(scatter, sg, n_elem, i) {
		struct hisi_sas_sge *entry = &slot->sge_page->sge[i];

		entry->addr_lo = DMA_ADDR_LO(sg_dma_address(sg));
		entry->addr_hi = DMA_ADDR_HI(sg_dma_address(sg));
		entry->page_ctrl_0 = entry->page_ctrl_1 = 0;
		entry->data_len = sg_dma_len(sg);
		entry->data_off = 0;
	}

	hdr->prd_table_addr_lo = DMA_ADDR_LO(slot->sge_page_dma);
	hdr->prd_table_addr_hi = DMA_ADDR_HI(slot->sge_page_dma);

	hdr->data_sg_len = n_elem;

	return 0;
}


static int prep_smp(struct hisi_hba *hisi_hba,
			struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct scatterlist *sg_req, *sg_resp;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct sas_phy *sphy = dev->phy;
	dma_addr_t req_dma_addr;
	unsigned int req_len, resp_len;
	int elem, rc;
	struct hisi_sas_slot *slot = tei->slot;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/*
	* DMA-map SMP request, response buffers
	*/
	/* req */
	sg_req = &task->smp_task.smp_req; /* this is the request frame - see alloc_smp_req() */
	elem = dma_map_sg(hisi_hba->dev, sg_req, 1, DMA_TO_DEVICE); /* map to dma address */
	if (!elem)
		return -ENOMEM;
	req_len = sg_dma_len(sg_req);
	req_dma_addr = sg_dma_address(sg_req);
	pr_info("%s sg_req=%p elem=%d req_len=%d\n", __func__, sg_req, elem, req_len);

	/* resp */
	sg_resp = &task->smp_task.smp_resp; /* this is the response frame - see alloc_smp_resp() */
	elem = dma_map_sg(hisi_hba->dev, sg_resp, 1, DMA_FROM_DEVICE);
	if (!elem) {
		rc = -ENOMEM;
		goto err_out;
	}
	resp_len = sg_dma_len(sg_resp);
	if ((req_len & 0x3) || (resp_len & 0x3)) {
		rc = -EINVAL;
		goto err_out;
	}

	/* create header */
	/* dw0 */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	dw0->phy_id = 1 << sphy->number; /* this is what Higgs_PrepareSMP does */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	dw0->priority = 1; /* high priority */
	dw0->cmd = 2; /* smp */

	/* dw1 */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */
	/* dw1->reset only set in stp */
	dw1->dir = DIR_NO_DATA; /* no data */
	dw1->frame_type = 1; /* request frame */
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (req_len - 4) / 4; /* do not include the crc */
	/* hdr->leave_affil_open, ->ncq_tag only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SMP_RESP_SZ/4;
	/* hdr->sg_mode, ->first_burst not applicable to smp */

	/* dw3 */
	/* hdr->iptt, ->tptt not applicable to smp */

	/* hdr->data_transfer_len not applicable to smp */

	/* hdr->first_burst_num not applicable to smp */

	/* hdr->dif_prd_table_len, ->prd_table_len not applicable to smp */

	/* hdr->double_mode, ->abort_iptt not applicable to smp */

	/* j00310691 do not use slot->command_table */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(req_dma_addr);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(req_dma_addr);

	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* hdr->prd_table_addr_lo not applicable to smp */

	/* hdr->prd_table_addr_hi not applicable to smp */

	/* hdr->dif_prd_table_addr_lo not applicable to smp */

	/* hdr->dif_prd_table_addr_hi not applicable to smp */

	return 0;

err_out:
	/* fix error conditions j00310691 */
	return rc;
}

static int prep_ssp(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei, int is_tmf,
		struct hisi_sas_tmf_task *tmf)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct sas_phy *sphy = dev->phy;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct sas_ssp_task *ssp_task = &task->ssp_task;
	struct scsi_cmnd *scsi_cmnd = ssp_task->cmd;
	int has_data = 0, rc;
	struct hisi_sas_slot *slot = tei->slot;
	u8 *buf_cmd, fburst = 0;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/* create header */
	/* dw0 */
	/* hdr->abort_flag set in Higgs_PrepareBaseSSP */
	/* hdr->t10_flds_pres set in Higgs_PreparePrdSge */
	dw0->resp_report = 1;
	dw0->tlr_ctrl = 0x2; /* Do not enable */
	dw0->phy_id = 1 << sphy->number; /* double-check */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	dw0->priority = 0; /* ordinary priority */
	dw0->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	dw1->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	dw1->enable_tlr = 0;
	dw1->verify_dtl = 1;
	if (is_tmf) {
		dw1->frame_type = 2;
		dw1->dir = DIR_NO_DATA;
	} else {
		dw1->frame_type = 1;
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_DEVICE;
			break;
		case DMA_FROM_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_INI;
			break;
		default:
			dw1->dir = DIR_RESERVED;
		}
	}
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct ssp_command_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	/* hdr->leave_affil_open only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	dw2->sg_mode = 2; /* see Higgs_DQGlobalConfig */
	dw2->first_burst = 0;

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = hisi_sas_prep_prd_sge(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	hdr->data_transfer_len = scsi_bufflen(scsi_cmnd);

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode is set only for DIF todo */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table + sizeof(struct ssp_frame_hdr);
	/* fill in IU for TASK and Command Frame */
	if (task->ssp_task.enable_first_burst) {
		fburst = (1 << 7);
		pr_warn("%s fburst enabled: edit hdr?\n", __func__);
	}

	memcpy(buf_cmd, &task->ssp_task.LUN, 8);
	if (!is_tmf) {
		buf_cmd[9] = fburst | task->ssp_task.task_attr |
				(task->ssp_task.task_prio << 3);
		memcpy(buf_cmd + 12, task->ssp_task.cmd->cmnd,
				task->ssp_task.cmd->cmd_len);
	} else {
		buf_cmd[10] = tmf->tmf;
		switch (tmf->tmf) {
		case TMF_ABORT_TASK:
		case TMF_QUERY_TASK:
			buf_cmd[12] =
				(tmf->tag_of_task_to_be_managed >> 8) & 0xff;
			buf_cmd[13] =
				tmf->tag_of_task_to_be_managed & 0xff;
			break;
		default:
			break;
		}
	}

	return 0;

}

static int prep_ata(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct asd_sas_port *sas_port = dev->port;
	struct hisi_sas_slot *slot = tei->slot;
	u8 *buf_cmd;
	int has_data = 0;
	int rc = 0;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1 *dw1 =
		(struct hisi_sas_cmd_hdr_dw1 *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2 *dw2 =
		(struct hisi_sas_cmd_hdr_dw2 *)&hdr->dw2;

	/* create header */
	/* dw0 */
	/* dw0->resp_report, ->tlr_ctrl not applicable to STP */
	dw0->phy_id = 0; /* don't care - see Higgs_PrepareBaseSTP */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->priority not applicable to STP */
	if (dev->dev_type == SAS_SATA_DEV)
		dw0->cmd = 4; /* sata */
	else
		dw0->cmd = 3; /* stp */

	/* dw1 */
	/* hdr->ssp_pass_through not applicable to stp */
	switch (task->data_dir) {
	case DMA_TO_DEVICE:
		dw1->dir = DIR_TO_DEVICE;
		has_data = 1;
	case DMA_FROM_DEVICE:
		dw1->dir = DIR_TO_INI;
		has_data = 1;
	default:
		pr_warn("%s unhandled direction, task->data_dir=%d\n", __func__, task->data_dir);
		dw1->dir = DIR_RESERVED;
	}

	/* j00310691 for IT code SOFT RESET MACRO is 0, but I am unsure if this is a valid command */
	if (0 == task->ata_task.fis.command) {
		dw1->reset = 1;
	}
	/* hdr->enable_tlr, ->pir_pres not applicable to stp */
	/* dw1->verify_dtl not set in IT code for STP */
	dw1->frame_type = get_ata_protocol(task->ata_task.fis.command,
				task->data_dir);
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct hisi_sas_command_table_stp) + 3) / 4;
	dw2->leave_affil_open = 0; /* j00310691 unset in IT code */
	/* hdr->ncq_tag todo j00310691 */
	dw2->max_resp_frame_len = HISI_SAS_MAX_STP_RESP_SZ/4;
	dw2->sg_mode = 2;
	/* hdr->first_burst not applicable to stp */

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = hisi_sas_prep_prd_sge(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	hdr->data_transfer_len = task->total_xfer_len;

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode not set in Higgs code */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table;

	if (likely(!task->ata_task.device_control_reg_update))
		task->ata_task.fis.flags |= 0x80; /* C=1: update ATA cmd reg */
	/* fill in command FIS and ATAPI CDB */
	memcpy(buf_cmd, &task->ata_task.fis, sizeof(struct host_to_dev_fis));
	if (dev->sata_dev.class == ATA_DEV_ATAPI)
		memcpy(buf_cmd + 0x20,
			task->ata_task.atapi_packet, ATAPI_CDB_LEN);

	return 0;
}

const struct hisi_sas_dispatch hisi_sas_hi1610_dispatch = {
	prep_ssp,
	prep_smp,
	prep_ata,
};

