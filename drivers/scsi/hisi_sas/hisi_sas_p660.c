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
	u32 rsvd0:2;
	u32 t10_flds_pres:1;
	u32 resp_report:1;
	u32 tlr_ctrl:2;
	u32 phy_id:8;
	u32 force_phy:1;
	u32 port:3;
	u32 sata_reg_set:7;
	u32 priority:1;
	u32 mode:1;
	u32 cmd:3;
};

struct hisi_sas_cmd_hdr_dw1 {
	u32 port_multiplier:4;
	u32 bist_activate:1;
	u32 atapi:1;
	u32 first_part_dma:1;
	u32 reset:1;
	u32 pir_pres:1;
	u32 enable_tlr:1;
	u32 verify_dtl:1;
	u32 rsvd1:1;
	u32 ssp_pass_through:1;
	u32 ssp_frame_type:3;
	u32 device_id:16;
};

struct hisi_sas_cmd_hdr_dw2 {
	u32 cmd_frame_len:9;
	u32 leave_affil_open:1;
	u32 rsvd2:5;
	u32 max_resp_frame_len:9;
	u32 sg_mode:1;
	u32 first_burst:1;
	u32 rsvd3:6;
};

static int hisi_sas_prep_prd_sge(struct hisi_hba *hisi_hba,
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;
	struct hisi_sas_cmd_hdr_dw0 *dw0 =
		(struct hisi_sas_cmd_hdr_dw0 *)&hdr->dw0;
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

	dw0->t10_flds_pres = 0;
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
	dw0->abort_flag = 0; /* not sure */
	/* hdr->t10_flds_pres not set in Higgs_PrepareSMP */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	dw0->phy_id = 1; /* this is what Higgs_PrepareSMP does */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	dw0->priority = 1; /* high priority */
	dw0->mode = 1; /* ini mode */
	dw0->cmd = 2; /* smp */

	/* dw1 */
	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */

	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (req_len - 4) / 4; /* do not include the crc */
	/* hdr->leave_affil_open only applicable to stp */
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
	dw0->mode = 1; /* ini mode */
	dw0->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	dw1->enable_tlr = 0;
	dw1->verify_dtl = 1;
	dw1->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	if (is_tmf) {
		dw1->ssp_frame_type = 3;
	} else {
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			dw1->ssp_frame_type = 2;
			has_data = 1;
			break;
		case DMA_FROM_DEVICE:
			dw1->ssp_frame_type = 1;
			has_data = 1;
			break;
		default:
			dw1->ssp_frame_type = 0;
		}
	}

	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct ssp_command_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	/* hdr->leave_affil_open only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	dw2->sg_mode = 0; /* see Higgs_DQGlobalConfig */
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


const struct hisi_sas_dispatch hisi_sas_p660_dispatch = {
	prep_ssp,
	prep_smp,
	NULL, /* p660 does not support STP */
};
