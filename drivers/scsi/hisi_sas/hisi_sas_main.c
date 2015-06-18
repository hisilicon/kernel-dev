#include "hisi_sas.h"
#include <linux/swab.h>

#define DEV_IS_EXPANDER(type) \
	((type == SAS_EDGE_EXPANDER_DEVICE) || (type == SAS_FANOUT_EXPANDER_DEVICE))

#define DEV_IS_GONE(dev) \
	((!dev) || (dev->dev_type == SAS_PHY_UNUSED))


#define DMA_ADDR_LO(addr) ((u32)(addr&0xffffffff))
#define DMA_ADDR_HI(addr) ((u32)(addr>>32))


/* registers */
#define GLOBAL_BASE_REG		(0x0)
#define IOST_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x8)
#define IOST_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0xc)
#define ITCT_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x10)
#define ITCT_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0x14)
#define BROKEN_MSG_ADDR_LO	(GLOBAL_BASE_REG + 0x18)
#define BROKEN_MSG_ADDR_HI	(GLOBAL_BASE_REG + 0x1c)
#define DLVRY_Q_0_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x260)
#define DLVRY_Q_0_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0x264)
#define DLVRY_Q_0_DEPTH		(GLOBAL_BASE_REG + 0x268)
#define DLVRY_Q_0_WR_PTR	(GLOBAL_BASE_REG + 0x26C)
#define DLVRY_Q_0_RD_PTR	(GLOBAL_BASE_REG + 0x270)
#define COMPL_Q_0_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x4e0)
#define COMPL_Q_0_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0x4e4)
#define COMPL_Q_0_DEPTH		(GLOBAL_BASE_REG + 0x4e8)
#define COMPL_Q_0_WR_PTR	(GLOBAL_BASE_REG + 0x4ec)
#define COMPL_Q_0_RD_PTR	(GLOBAL_BASE_REG + 0x4f0)

#define PORT_BASE_REG		(0x800)
#define PHY_CFG_REG		(PORT_BASE_REG + 0x0)
#define PHY_CFG_REG_RESET_OFF	0
#define PHY_CFG_REG_RESET_MASK	1
#define PHY_CTRL_REG		(PORT_BASE_REG + 0x14)
#define TX_ID_DWORD0_REG	(PORT_BASE_REG + 0x9c)
#define TX_ID_DWORD1_REG	(PORT_BASE_REG + 0xa0)
#define TX_ID_DWORD2_REG	(PORT_BASE_REG + 0xa4)
#define TX_ID_DWORD3_REG	(PORT_BASE_REG + 0xa8)
#define TX_ID_DWORD4_REG	(PORT_BASE_REG + 0xaC)
#define TX_ID_DWORD5_REG	(PORT_BASE_REG + 0xb0)
#define TX_ID_DWORD6_REG	(PORT_BASE_REG + 0xb4)
#define DMA_TX_STATUS_REG	(PORT_BASE_REG + 0x2d0)
#define DMA_TX_STATUS_BUSY_OFF	0
#define DMA_TX_STATUS_BUSY_MASK	1
#define DMA_RX_STATUS_REG	(PORT_BASE_REG + 0x2e8)
#define DMA_RX_STATUS_BUSY_OFF	0
#define DMA_RX_STATUS_BUSY_MASK	1
#define AXI_CFG_REG		(0x5100)

/*global registers need init*/
#define DLVRY_QUEUE_ENABLE_REG		(GLOBAL_BASE_REG + 0x0)
#define HGC_TRANS_TASK_CNT_LIMIT_REG	(GLOBAL_BASE_REG + 0x38)
#define DEVICE_MSG_WORK_MODE_REG	(GLOBAL_BASE_REG + 0x94)
#define MAX_BURST_BYTES_REG		(GLOBAL_BASE_REG + 0x98)
#define SMP_TIMEOUT_TIMER_REG		(GLOBAL_BASE_REG + 0x9c)
#define MAX_CON_TIME_LIMIT_TIME_REG	(GLOBAL_BASE_REG + 0xa4)
#define HGC_SAS_TXFAIL_RETRY_CTRL_REG	(GLOBAL_BASE_REG + 0x84)
#define HGC_ERR_STAT_EN_REG		(GLOBAL_BASE_REG + 0x238)
#define CFG_1US_TIMER_TRSH_REG		(GLOBAL_BASE_REG + 0xcc)
#define HGC_GET_ITV_TIME_REG		(GLOBAL_BASE_REG + 0x90)
#define I_T_NEXUS_LOSS_TIME_REG		(GLOBAL_BASE_REG + 0xa0)
#define BUS_INACTIVE_LIMIT_TIME_REG	(GLOBAL_BASE_REG + 0xa8)
#define REJECT_TO_OPEN_LIMIT_TIME_REG	(GLOBAL_BASE_REG + 0xac)
#define CFG_AGING_TIME_REG		(GLOBAL_BASE_REG + 0xbc)
#define HGC_DFX_CFG_REG2_REG		(GLOBAL_BASE_REG + 0xc0)
#define FIS_LIST_BADDR_L_REG		(GLOBAL_BASE_REG + 0xc4)
#define INT_COAL_EN_REG			(GLOBAL_BASE_REG + 0x1bc)
#define OQ_INT_COAL_TIME_REG		(GLOBAL_BASE_REG + 0x1c0)
#define OQ_INT_COAL_CNT_REG		(GLOBAL_BASE_REG + 0x1c4)
#define ENT_INT_COAL_TIME_REG		(GLOBAL_BASE_REG + 0x1c8)
#define ENT_INT_COAL_CNT_REG		(GLOBAL_BASE_REG + 0x1cc)
#define OQ_INT_SRC_REG			(GLOBAL_BASE_REG + 0x1d0)
#define OQ_INT_SRC_MSK_REG		(GLOBAL_BASE_REG + 0x1d4)
#define ENT_INT_SRC_MSK1_REG		(GLOBAL_BASE_REG + 0x1e0)
#define ENT_INT_SRC2_REG		(GLOBAL_BASE_REG + 0x1dc)
#define ENT_INT_SRC_MSK2_REG		(GLOBAL_BASE_REG + 0x1e4)
#define SAS_ECC_INTR_MSK_REG		(GLOBAL_BASE_REG + 0x1ec)
#define AXI_AHB_CLK_CFG_REG		(GLOBAL_BASE_REG + 0x3c)
#define CFG_SAS_CONFIG_REG		(GLOBAL_BASE_REG + 0xd4)

/*phy registers need init*/
#define PROG_PHY_LINK_RATE_REG          (PORT_BASE_REG + 0xC)
#define PHY_CONFIG2_REG                 (PORT_BASE_REG + 0x1A8)
#define PHY_RATE_NEGO_REG               (PORT_BASE_REG + 0x30)
#define PHY_PCN_REG                     (PORT_BASE_REG + 0x44)
#define SL_TOUT_CFG_REG                 (PORT_BASE_REG + 0x8C)
#define DONE_RECEVIED_TIME_REG          (PORT_BASE_REG + 0x12C)
#define RXOP_CHECK_CFG_H_REG            (PORT_BASE_REG + 0xFC)
#define CON_CFG_DRIVER_REG              (PORT_BASE_REG + 0x130)
#define CHL_INT_COAL_EN_REG             (PORT_BASE_REG + 0x1D0)
/*phy intr registers*/
#define CHL_INT0_REG			(PORT_BASE_REG + 0x1B0)
#define CHL_INT1_REG			(PORT_BASE_REG + 0x1B4)
#define CHL_INT2_REG			(PORT_BASE_REG + 0x1B8)
/*phy intr_mask registers need unmask*/
#define CHL_INT0_MSK_REG		(PORT_BASE_REG + 0x1bC)
#define CHL_INT1_MSK_REG		(PORT_BASE_REG + 0x1c0)
#define CHL_INT2_MSK_REG		(PORT_BASE_REG + 0x1c4)

static inline u32 hisi_sas_read32(struct hisi_hba *hisi_hba, u32 off)
{
	void __iomem *regs = hisi_hba->regs + off;

	return readl(regs);
}

static inline void hisi_sas_write32(struct hisi_hba *hisi_hba, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + off;

	writel(val, regs);
}

static inline void hisi_sas_phy_write32(struct hisi_hba *hisi_hba, int phy, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	writel(val, regs);
}

static inline u32 hisi_sas_phy_read32(struct hisi_hba *hisi_hba, int phy, u32 off)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	return readl(regs);
}

void hisi_sas_iptt_clear(struct hisi_hba *hisi_hba, int iptt)
{
	void *bitmap = hisi_hba->iptt_tags;

	clear_bit(iptt, bitmap);
}

void hisi_sas_iptt_free(struct hisi_hba *hisi_hba, int iptt)
{
	hisi_sas_iptt_clear(hisi_hba, iptt);
}

void hisi_sas_iptt_set(struct hisi_hba *hisi_hba, int iptt)
{
	void *bitmap = hisi_hba->iptt_tags;

	set_bit(iptt, bitmap);
}

int hisi_sas_iptt_alloc(struct hisi_hba *hisi_hba, int *iptt)
{
	unsigned int index;
	void *bitmap = hisi_hba->iptt_tags;

	index = find_first_zero_bit(bitmap, hisi_hba->iptt_count);
	if (index >= hisi_hba->iptt_count)
		return -SAS_QUEUE_FULL;
	hisi_sas_iptt_set(hisi_hba, index);
	*iptt = index;
	return 0;
}

void hisi_sas_iptt_init(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->iptt_count; ++i)
		hisi_sas_iptt_clear(hisi_hba, i);

}

static void hisi_sas_slot_task_free(struct hisi_hba *hisi_hba, struct sas_task *task,
			  struct hisi_sas_slot *slot)
{
	if (!slot->task)
		return;
	if (!sas_protocol_ata(task->task_proto))
		if (slot->n_elem)
			dma_unmap_sg(hisi_hba->dev, task->scatter,
				     slot->n_elem, task->data_dir);

	switch (task->task_proto) {
	case SAS_PROTOCOL_SMP:
		dma_unmap_sg(hisi_hba->dev, &task->smp_task.smp_resp, 1,
			     DMA_FROM_DEVICE);
		dma_unmap_sg(hisi_hba->dev, &task->smp_task.smp_req, 1,
			     DMA_TO_DEVICE);
		break;

	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SSP:
	default:
		/* do nothing */
		break;
	}

	if (hisi_hba->command_table_pool) {
		dma_pool_free(hisi_hba->command_table_pool, slot->cmd_hdr, slot->cmd_hdr_dma);
		slot->cmd_hdr = NULL;
	}
	if (hisi_hba->status_buffer_pool) {
		dma_pool_free(hisi_hba->status_buffer_pool, slot->status_buffer, slot->status_buffer_dma);
		slot->cmd_hdr = NULL;
	}
	list_del_init(&slot->entry);
	task->lldd_task = NULL;
	slot->task = NULL;
	slot->port = NULL;
	hisi_sas_iptt_free(hisi_hba, slot->iptt);
	memset(&hisi_hba->iptt[slot->iptt], 0, sizeof(struct hisi_sas_iptt));
	memset(slot, 0, sizeof(*slot));
}

int hisi_sas_slot_complete(struct hisi_hba *hisi_hba, struct hisi_sas_slot *slot, u32 flags)
{
	struct sas_task *task = slot->task;
	struct hisi_sas_device *hisi_sas_dev = NULL;
	struct task_status_struct *tstat;
	struct domain_device *dev;

	void *to;
	enum exec_status sts;

	if (unlikely(!task || !task->lldd_task || !task->dev))
		return -1;

	tstat = &task->task_status;
	dev = task->dev;
	hisi_sas_dev = dev->lldd_dev;

	task->task_state_flags &=
		~(SAS_TASK_STATE_PENDING | SAS_TASK_AT_INITIATOR);
	task->task_state_flags |= SAS_TASK_STATE_DONE;

	memset(tstat, 0, sizeof(*tstat));
	tstat->resp = SAS_TASK_COMPLETE;

	/* when no device attaching, go ahead and complete by error handling */
	if (unlikely(!hisi_sas_dev || flags)) {
		if (!hisi_sas_dev)
			pr_err("%s port has not device.\n", __func__);
		tstat->stat = SAS_PHY_DOWN;
		goto out;
	}

	switch (task->task_proto) {
	case SAS_PROTOCOL_SSP: {
			/* j00310691 for SMP, IU contains just the SSP IU */
			struct ssp_response_iu *iu = slot->status_buffer + sizeof(struct hisi_sas_err_record);

			sas_ssp_task_response(hisi_hba->dev, task, iu);
			break;
		}

	case SAS_PROTOCOL_SMP: {
			struct scatterlist *sg_resp = &task->smp_task.smp_resp;

			tstat->stat = SAM_STAT_GOOD;
			to = kmap_atomic(sg_page(sg_resp));
			/* j00310691 for SMP, buffer contains the full SMP frame */
			memcpy(to + sg_resp->offset,
				slot->status_buffer + sizeof(struct hisi_sas_err_record),
				sg_dma_len(sg_resp));
			kunmap_atomic(to);
			break;
		}

	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP: {
			pr_warn("%s STP not supported", __func__);
			break;
		}

	default:
		tstat->stat = SAM_STAT_CHECK_CONDITION;
		break;
	}
	if (!slot->port->port_attached) {
		pr_err("%s port %d has removed.\n", __func__, slot->port->sas_port.id);
		tstat->stat = SAS_PHY_DOWN;
	}

out:
	if (hisi_sas_dev && hisi_sas_dev->running_req)
		hisi_sas_dev->running_req--;

	hisi_sas_slot_task_free(hisi_hba, task, slot);
	sts = tstat->stat;

	if (task->task_done)
		task->task_done(task);

	return sts;
}

static int hisi_sas_get_free_slot(struct hisi_hba *hisi_hba, int *q, int *s)
{
	u32 r, w;
	int queue = smp_processor_id() % hisi_hba->queue_count;

	pr_info("%s queue=%d\n", __func__, queue);

	while (1) {
		w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14));
		r = hisi_sas_read32(hisi_hba, DLVRY_Q_0_RD_PTR + (queue * 0x14));

		if (r == w+1 % HISI_SAS_QUEUE_SLOTS) {
			dev_warn(hisi_hba->dev, "%s queue full queue=%d r=%d w=%d\n", __func__, queue, r, w);
			queue = (queue + 1) % hisi_hba->queue_count;
			continue;
		}
		break;
	}

	*q = queue;
	*s = w;

	return 0;
}

static int hisi_sas_task_prep_smp(struct hisi_hba *hisi_hba,
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
	pr_info("%s sg_resp=%p elem=%d req_len=%d\n", __func__, sg_resp, elem, resp_len);

	/* create header */
	hdr->abort_flag = 0; /* not sure */
	/* hdr->t10_flds_pres not set in Higgs_PrepareSMP */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	hdr->phy_id = 1; /* this is what Higgs_PrepareSMP does */
	hdr->force_phy = 0; /* due not force ordering in phy */
	hdr->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	hdr->priority = 1; /* high priority */
	hdr->mode = 1; /* ini mode */
	hdr->cmd = 2; /* smp */

	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */

	hdr->device_id = hisi_sas_dev->device_id; /* map itct entry */

	hdr->cmd_frame_len = req_len/4;
	/* hdr->leave_affil_open only applicable to stp */
	hdr->max_resp_frame_len = HISI_SAS_MAX_SMP_RESP_SZ/4;
	/* hdr->sg_mode, ->first_burst not applicable to smp */

	/* hdr->iptt, ->tptt not applicable to smp */

	/* hdr->data_transfer_len not applicable to smp */

	/* hdr->first_burst_num not applicable to smp */

	/* hdr->dif_prd_table_len, ->prd_table_len not applicable to smp */

	/* hdr->double_mode, ->abort_iptt not applicable to smp */

	/* j00310691 do not use slot->command_table */
	hdr->cmd_table_addr_lo  = DMA_ADDR_LO(req_dma_addr);
	hdr->cmd_table_addr_hi  = DMA_ADDR_HI(req_dma_addr);

	hdr->sts_buffer_addr_lo  = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi  = DMA_ADDR_HI(slot->status_buffer_dma);

	/* hdr->prd_table_addr_lo not applicable to smp */

	/* hdr->prd_table_addr_hi not applicable to smp */

	/* hdr->dif_prd_table_addr_lo not applicable to smp */

	/* hdr->dif_prd_table_addr_hi not applicable to smp */

	return 0;

err_out:
	/* fix error conditions j00310691 */
	return rc;
}

static int hisi_sas_prep_prd_sge(struct hisi_hba *hisi_hba,
		struct hisi_sas_cmd_hdr *hdr,
		struct scatterlist *scatter,
		int n_elem)
{
	struct hisi_sas_sge_page *sge_page = NULL;
	dma_addr_t dma_addr;
	struct scatterlist *sg;
	int i;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		pr_err("%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT", __func__, n_elem);
		return -EINVAL;
	}

	/* j00310691 fixme need to deal with dealloc of sge_page */
	sge_page = dma_pool_alloc(hisi_hba->sge_page_pool, GFP_KERNEL, &dma_addr);
	if (!sge_page)
		return -ENOMEM;

	hdr->pir_pres = 0;
	hdr->t10_flds_pres = 0;

	for_each_sg(scatter, sg, n_elem, i) {
		struct hisi_sas_sge *entry = &sge_page->sge[i];

		entry->addr_lo = DMA_ADDR_LO(sg_dma_address(sg));
		entry->addr_hi = DMA_ADDR_HI(sg_dma_address(sg));
		entry->page_ctrl_0 = entry->page_ctrl_1 = 0;
		entry->data_len = sg_dma_len(sg);
		entry->data_off = 0;
	}

	hdr->prd_table_addr_lo = DMA_ADDR_LO(dma_addr);
	hdr->prd_table_addr_hi = DMA_ADDR_HI(dma_addr);

	hdr->data_sg_len = n_elem;

	return 0;
}

/* Refer to Higgs_PrepareBaseSSP */
static int hisi_sas_task_prep_ssp(struct hisi_hba *hisi_hba,
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
	struct ssp_frame_hdr *ssp_hdr;
	u8 *buf_cmd, fburst;

	/* create header */
	/* dw0 */
	/* hdr->abort_flag set in Higgs_PrepareBaseSSP */
	/* hdr->t10_flds_pres set in Higgs_PreparePrdSge */
	hdr->resp_report = 1;
	hdr->tlr_ctrl = 0x2; /* Do not enable */
	hdr->phy_id = 1 << sphy->number; /* double-check */
	hdr->force_phy = 0; /* due not force ordering in phy */
	hdr->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	hdr->priority = 0; /* ordinary priority */
	hdr->mode = 1; /* ini mode */
	hdr->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	hdr->enable_tlr = 0;
	hdr->verify_dtl = 1;
	hdr->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	if (is_tmf) {
		hdr->ssp_frame_type = 3;
	} else {
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			hdr->ssp_frame_type = 2;
			has_data = 1;
			break;
		case DMA_FROM_DEVICE:
			hdr->ssp_frame_type = 1;
			has_data = 1;
			break;
		default:
			hdr->ssp_frame_type = 0;
		}
	}

	hdr->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	hdr->cmd_frame_len = (sizeof(struct ssp_command_iu) +
				sizeof(struct ssp_frame_hdr) +
				3) / 4;
	/* hdr->leave_affil_open only applicable to stp */
	hdr->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	hdr->sg_mode = 0; /* see Higgs_DQGlobalConfig */

	hdr->first_burst = 0;

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = hisi_sas_prep_prd_sge(hisi_hba, hdr, task->scatter, tei->n_elem);
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

	/* fill-in ssp header */
	ssp_hdr = (struct ssp_frame_hdr *)slot->command_table;

	if (is_tmf)
		ssp_hdr->frame_type = SSP_TASK;
	else
		ssp_hdr->frame_type = SSP_COMMAND;

	memcpy(ssp_hdr->hashed_dest_addr, dev->hashed_sas_addr,
	       HASHED_SAS_ADDR_SIZE);
	memcpy(ssp_hdr->hashed_src_addr,
	       dev->hashed_sas_addr, HASHED_SAS_ADDR_SIZE);
	ssp_hdr->tag = cpu_to_be16(tei->iptt);

	/* fill in IU for TASK and Command Frame */
	if (task->ssp_task.enable_first_burst) {
		fburst = (1 << 7);
		pr_warn("%s fburst enabled: edit hdr?\n", __func__);
	}
	buf_cmd = (u8 *)ssp_hdr + sizeof(*ssp_hdr);
	memcpy(buf_cmd, &task->ssp_task.LUN, 8);

	if (ssp_hdr->frame_type != SSP_TASK) {
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

static int hisi_sas_task_prep_ata(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	return 0;
}

static int hisi_sas_task_prep(struct sas_task *task,
		struct hisi_hba *hisi_hba,
		int is_tmf,
		struct hisi_sas_tmf_task *tmf,
		int *pass)
{
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_tei tei;
	struct hisi_sas_slot *slot;
	struct hisi_sas_cmd_hdr	*cmd_hdr_base;
	int queue_slot = -1, queue = -1, n_elem = 0, rc = 0, iptt = -1;

	if (!dev->port) {
		struct task_status_struct *tsm = &task->task_status;

		tsm->resp = SAS_TASK_UNDELIVERED;
		tsm->stat = SAS_PHY_DOWN;
		/*
		 * libsas will use dev->port, should
		 * not call task_done for sata
		 */
		if (dev->dev_type != SAS_SATA_DEV)
			task->task_done(task);
		return rc;
	}

	if (DEV_IS_GONE(hisi_sas_dev)) {
		if (hisi_sas_dev)
			pr_info("device %llu not ready.\n",
				hisi_sas_dev->device_id);
		else
			pr_info("device %016llx not ready.\n",
				SAS_ADDR(dev->sas_addr));

			rc = SAS_PHY_DOWN;
			return rc;
	}
	tei.port = dev->port->lldd_port;
	if (tei.port && !tei.port->port_attached && !tmf) {
		if (sas_protocol_ata(task->task_proto)) {
			struct task_status_struct *ts = &task->task_status;

			pr_info("SATA/STP port %d does not attach device.\n",
				dev->port->id);
			ts->resp = SAS_TASK_COMPLETE;
			ts->stat = SAS_PHY_DOWN;

			task->task_done(task);

		} else {
			struct task_status_struct *ts = &task->task_status;

			pr_info("SAS port %d does not attach device.\n",
				dev->port->id);
			ts->resp = SAS_TASK_UNDELIVERED;
			ts->stat = SAS_PHY_DOWN;
			task->task_done(task);
		}
		return rc;
	}

	if (!sas_protocol_ata(task->task_proto)) {
		if (task->num_scatter) {
			n_elem = dma_map_sg(hisi_hba->dev,
					    task->scatter,
					    task->num_scatter,
					    task->data_dir);
			if (!n_elem) {
				rc = -ENOMEM;
				goto err_out;
			}
		}
	} else {
		n_elem = task->num_scatter;
	}

	rc = hisi_sas_iptt_alloc(hisi_hba, &iptt);
	if (rc)
		goto err_out;

	rc = hisi_sas_get_free_slot(hisi_hba, &queue, &queue_slot);
	if (rc)
		goto err_out;

	slot = &hisi_hba->slot_info[queue*hisi_hba->queue_count+queue_slot];
	memset(slot, 0, sizeof(*slot));

	hisi_hba->iptt[iptt].queue = queue;
	hisi_hba->iptt[iptt].queue_slot = queue_slot;
	hisi_hba->iptt[iptt].active = 1;

	task->lldd_task = NULL;
	slot->iptt = iptt;
	slot->n_elem = n_elem;
	slot->queue = queue;
	slot->queue_slot = queue_slot;
	cmd_hdr_base = hisi_hba->cmd_hdr[queue];
	slot->cmd_hdr = &cmd_hdr_base[queue_slot];

	slot->status_buffer = dma_pool_alloc(hisi_hba->status_buffer_pool, GFP_ATOMIC,
				&slot->status_buffer_dma);
	if (!slot->status_buffer)
		goto err_out;
	memset(slot->status_buffer, 0, HISI_SAS_STATUS_BUF_SZ);

	slot->command_table = dma_pool_alloc(hisi_hba->command_table_pool, GFP_ATOMIC,
				&slot->command_table_dma);
	if (!slot->command_table)
		goto err_out;
	memset(slot->command_table, 0, sizeof(*slot->command_table));

	tei.hdr = slot->cmd_hdr;
	tei.task = task;
	tei.n_elem = n_elem;
	tei.iptt = iptt;
	tei.slot = slot;
	switch (task->task_proto) {
	case SAS_PROTOCOL_SMP:
		rc = hisi_sas_task_prep_smp(hisi_hba, &tei);
		break;
	case SAS_PROTOCOL_SSP:
		rc = hisi_sas_task_prep_ssp(hisi_hba, &tei, is_tmf, tmf);
		break;
	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP:
		rc = hisi_sas_task_prep_ata(hisi_hba, &tei);
		break;
	default:
		dev_err(hisi_hba->dev,
			"unknown sas_task proto: 0x%x\n",
			task->task_proto);
		rc = -EINVAL;
		break;
	}

	if (rc) {
		pr_info("rc is %x\n", rc);
		goto err_out;
	}
	slot->task = task;
	slot->port = tei.port;
	task->lldd_task = slot;
	list_add_tail(&slot->entry, &tei.port->list);
	spin_lock(&task->task_state_lock);
	task->task_state_flags |= SAS_TASK_AT_INITIATOR;
	spin_unlock(&task->task_state_lock);

	hisi_hba->slot_prep = slot;

	hisi_sas_dev->running_req++;
	++(*pass);

	return rc;

err_out:
	/* Add proper labels j00310691 */
	return rc;
}

void hisi_sas_start_delivery(struct hisi_hba *hisi_hba)
{
	int queue = hisi_hba->slot_prep->queue;
	u32 w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14));

	hisi_sas_write32(hisi_hba, DLVRY_Q_0_WR_PTR + (queue * 0x14), ++w % HISI_SAS_QUEUE_SLOTS);
}

static int hisi_sas_task_exec(struct sas_task *task,
	gfp_t gfp_flags,
	struct completion *completion,
	int is_tmf,
	struct hisi_sas_tmf_task *tmf)
{
	struct hisi_hba *hisi_hba = NULL;
	u32 rc = 0;
	u32 pass = 0;
	unsigned long flags = 0;

	hisi_hba = ((struct hisi_sas_device *)task->dev->lldd_dev)->hisi_hba;

	spin_lock_irqsave(&hisi_hba->lock, flags);
	rc = hisi_sas_task_prep(task, hisi_hba, is_tmf, tmf, &pass);
	if (rc)
		dev_err(hisi_hba->dev, "hisi_sas exec failed[%d]!\n", rc);

	if (likely(pass))
		hisi_sas_start_delivery(hisi_hba);
	spin_unlock_irqrestore(&hisi_hba->lock, flags);

	return rc;
}

static void hisi_sas_bytes_dmaed(struct hisi_hba *hisi_hba, int i)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[i];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha;

	if (!phy->phy_attached)
		return;

	if (!(phy->att_dev_info & PORT_DEV_TRGT_MASK)
		&& phy->phy_type & PORT_TYPE_SAS) {
		return;
	}

	sas_ha = hisi_hba->sas;
	sas_ha->notify_phy_event(sas_phy, PHYE_OOB_DONE);

	if (sas_phy->phy) {
		struct sas_phy *sphy = sas_phy->phy;

		sphy->negotiated_linkrate = sas_phy->linkrate;
		sphy->minimum_linkrate = phy->minimum_linkrate;
		sphy->minimum_linkrate_hw = SAS_LINK_RATE_1_5_GBPS;
		sphy->maximum_linkrate = phy->maximum_linkrate;
		/* sphy->maximum_linkrate_hw = MVS_CHIP_DISP->phy_max_link_rate(); */
	}

	if (phy->phy_type & PORT_TYPE_SAS) {
		struct sas_identify_frame *id;

		id = (struct sas_identify_frame *)phy->frame_rcvd;
		id->dev_type = phy->identify.device_type;
		id->initiator_bits = SAS_PROTOCOL_ALL;
		id->target_bits = phy->identify.target_port_protocols;

		/* direct attached SAS device */
		if (phy->att_dev_info & PORT_SSP_TRGT_MASK) {
			/* MVS_CHIP_DISP->write_port_cfg_addr(mvi, i, PHYR_PHY_STAT); */
			/* MVS_CHIP_DISP->write_port_cfg_data(mvi, i, 0x00); */
		}
	} else if (phy->phy_type & PORT_TYPE_SATA) {
		/*Nothing*/
	}

	pr_info("core %d phy %d byte dmaded.\n", hisi_hba->id, i);

	sas_phy->frame_rcvd_size = phy->frame_rcvd_size;

	hisi_hba->sas->notify_port_event(sas_phy,
				   PORTE_BYTES_DMAED);
}

struct hisi_hba *hisi_sas_find_dev(struct domain_device *dev)
{
	unsigned long i = 0, j = 0, hi = 0;
	struct sas_ha_struct *sha = dev->port->ha;
	struct hisi_hba *hisi_hba = NULL;
	struct asd_sas_phy *phy;

	pr_info("%s\n", __func__);

	while (sha->sas_port[i]) {
		if (sha->sas_port[i] == dev->port) {
			phy = container_of(sha->sas_port[i]->phy_list.next,
				struct asd_sas_phy, port_phy_el);
			j = 0;
			while (sha->sas_phy[i]) {
				if (sha->sas_phy[j] == phy)
					break;
				j++;
			}
		}
		i++;
	}

	hi = j/((struct hisi_hba_priv_info *)sha->lldd_ha)->n_phy;
	hisi_hba = ((struct hisi_hba_priv_info *)sha->lldd_ha)->hisi_hba[hi];

	return hisi_hba;
}

struct hisi_sas_device *hisi_sas_alloc_dev(struct hisi_hba *hisi_hba)
{
	int dev_id;

	for (dev_id = 0; dev_id < HISI_SAS_MAX_DEVICES; dev_id++) {
		if (hisi_hba->devices[dev_id].dev_type == SAS_PHY_UNUSED) {
			hisi_hba->devices[dev_id].device_id = dev_id;
			return &hisi_hba->devices[dev_id];
		}
	}

	pr_err("%s: max support %d devices - could not alloc\n", __func__, HISI_SAS_MAX_DEVICES);

	return NULL;
}

void hisi_sas_setup_itct(struct hisi_hba *hisi_hba, struct hisi_sas_device *device)
{
	struct domain_device *dev = device->sas_device;
	u32 device_id = device->device_id;

	device->itct = &hisi_hba->itct[device_id];
	memset(device->itct, 0, sizeof(*device->itct));

	/* qw0 */
	if (dev->dev_type == SAS_END_DEVICE)
		device->itct->dev_type = HISI_SAS_DEV_TYPE_SSP;
	else
		pr_warn("%s unsupported dev type\n", __func__);
	device->itct->valid = 1;
	device->itct->break_reply_ena = 0;
	device->itct->awt_control = 1;
	device->itct->max_conn_rate = dev->max_linkrate; /* j00310691 todo doublecheck, see enum sas_linkrate */
	device->itct->valid_link_number = 1;
	device->itct->port_id = dev->port->id;
	device->itct->smp_timeout = 0;
	device->itct->max_burst_byte = 0;

	/* qw1 */
	memcpy(&device->itct->sas_addr, dev->sas_addr, SAS_ADDR_SIZE);

	/* qw2 */
	device->itct->IT_nexus_loss_time = 500;
	device->itct->bus_inactive_time_limit = 0xff00;
	device->itct->max_conn_time_limit = 0xff00;
	device->itct->reject_open_time_limit = 0xff00;
}

int hisi_sas_dev_found_notify(struct domain_device *dev, int lock)
{
	unsigned long flags = 0;
	int res = 0;
	struct hisi_hba *hisi_hba = NULL;
	/* struct domain_device  *parent_dev = dev->parent; */
	struct hisi_sas_device *hisi_sas_device;

	pr_info("%s\n", __func__);

	hisi_hba = hisi_sas_find_dev(dev);

	if (lock)
		spin_lock_irqsave(&hisi_hba->lock, flags);

	hisi_sas_device = hisi_sas_alloc_dev(hisi_hba);
	if (!hisi_sas_device) {
		res = -1;
		goto found_out;
	}

	dev->lldd_dev = hisi_sas_device;
	hisi_sas_device->dev_status = HISI_SAS_DEV_NORMAL;
	hisi_sas_device->dev_type = dev->dev_type;
	hisi_sas_device->hisi_hba = hisi_hba;
	hisi_sas_device->sas_device = dev;

	hisi_sas_setup_itct(hisi_hba, hisi_sas_device);

/*	if (parent_dev && DEV_IS_EXPANDER(parent_dev->dev_type)) {

	} */

found_out:
	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
	return res;
}

static int hisi_sas_reset_hw(struct hisi_hba *hisi_hba)
{
	int i;
	unsigned long end_time;

	/* inline Higgs_PrepareResetHw j00310691 */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 phy_ctrl = hisi_sas_phy_read32(hisi_hba, i, PHY_CTRL_REG);

		phy_ctrl |= PHY_CFG_REG_RESET_MASK;
		hisi_sas_phy_write32(hisi_hba, i, PHY_CTRL_REG, phy_ctrl);
	}
	udelay(50);

	/* Ensure DMA tx&rx idle */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 dma_tx_status, dma_rx_status;

		end_time = jiffies + msecs_to_jiffies(1000);

		while (1) {
			dma_tx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_TX_STATUS_REG);
			dma_rx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_RX_STATUS_REG);

			if (!(dma_tx_status & DMA_TX_STATUS_BUSY_MASK) &&
				!(dma_rx_status & DMA_RX_STATUS_BUSY_MASK))
				break;

			msleep(20);
			if (time_after(jiffies, end_time))
				return -EIO;
		}
	}

	/* Ensure axi bus idle */
	end_time = jiffies + msecs_to_jiffies(1000);
	while (1) {
		u32 axi_status =
			hisi_sas_read32(hisi_hba, DMA_TX_STATUS_REG);

		if (axi_status == 0)
			return 0;

		msleep(20);
		if (time_after(jiffies, end_time))
			return -EIO;
	}

	return 0;
}

static int hisi_sas_init_reg(struct hisi_hba *hisi_hba)
{
	int i;

	/* Global registers init*/
	hisi_sas_write32(hisi_hba, DLVRY_QUEUE_ENABLE_REG, (1<<hisi_hba->n_phy)-1);
	hisi_sas_write32(hisi_hba, HGC_TRANS_TASK_CNT_LIMIT_REG, 0x11);
	hisi_sas_write32(hisi_hba, DEVICE_MSG_WORK_MODE_REG, 0x1);
	hisi_sas_write32(hisi_hba, MAX_BURST_BYTES_REG, 0);
	hisi_sas_write32(hisi_hba, SMP_TIMEOUT_TIMER_REG, 0);
	hisi_sas_write32(hisi_hba, MAX_CON_TIME_LIMIT_TIME_REG, 0);
	hisi_sas_write32(hisi_hba, HGC_SAS_TXFAIL_RETRY_CTRL_REG, 0x211ff);
	hisi_sas_write32(hisi_hba, HGC_ERR_STAT_EN_REG, 0x401);
	hisi_sas_write32(hisi_hba, CFG_1US_TIMER_TRSH_REG, 0x64);
	hisi_sas_write32(hisi_hba, HGC_GET_ITV_TIME_REG, 0x1);
	hisi_sas_write32(hisi_hba, I_T_NEXUS_LOSS_TIME_REG, 0x64);
	hisi_sas_write32(hisi_hba, BUS_INACTIVE_LIMIT_TIME_REG, 0x2710);
	hisi_sas_write32(hisi_hba, REJECT_TO_OPEN_LIMIT_TIME_REG, 0x1);
	hisi_sas_write32(hisi_hba, CFG_AGING_TIME_REG, 0x7a12);
	hisi_sas_write32(hisi_hba, HGC_DFX_CFG_REG2_REG, 0x9c40);
	hisi_sas_write32(hisi_hba, FIS_LIST_BADDR_L_REG, 0x2);
	hisi_sas_write32(hisi_hba, INT_COAL_EN_REG, 0xC);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_TIME_REG, 0x186A0);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_CNT_REG, 1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_TIME_REG, 0x1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_CNT_REG, 0x1);
	hisi_sas_write32(hisi_hba, OQ_INT_SRC_REG, 0xffffffff);
	hisi_sas_write32(hisi_hba, OQ_INT_SRC_MSK_REG, 0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1_REG, 0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC2_REG, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK2_REG, 0);
	hisi_sas_write32(hisi_hba, SAS_ECC_INTR_MSK_REG, 0);
	hisi_sas_write32(hisi_hba, AXI_AHB_CLK_CFG_REG, 0x2);
	hisi_sas_write32(hisi_hba, CFG_SAS_CONFIG_REG, 0x22000000);

	for (i = 0; i < hisi_hba->queue_count; i++) {
		/* Delivery queue */
		hisi_sas_write32(hisi_hba,
			DLVRY_Q_0_BASE_ADDR_HI + (i * 0x14),
			DMA_ADDR_HI(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba,
			DLVRY_Q_0_BASE_ADDR_LO + (i * 0x14),
			DMA_ADDR_LO(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba,
			DLVRY_Q_0_DEPTH + (i * 0x14),
			HISI_SAS_QUEUE_SLOTS);

		/* Completion queue */
		hisi_sas_write32(hisi_hba,
			COMPL_Q_0_BASE_ADDR_HI + (i * 0x14),
			DMA_ADDR_HI(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba,
			COMPL_Q_0_BASE_ADDR_LO + (i * 0x14),
			DMA_ADDR_LO(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba,
			COMPL_Q_0_DEPTH + (i * 0x14),
			HISI_SAS_QUEUE_SLOTS);
	}

	/* itct */
	hisi_sas_write32(hisi_hba,
		ITCT_BASE_ADDR_LO,
		DMA_ADDR_LO(hisi_hba->itct_dma));

	hisi_sas_write32(hisi_hba,
		ITCT_BASE_ADDR_HI,
		DMA_ADDR_HI(hisi_hba->itct_dma));

	/* iost */
	hisi_sas_write32(hisi_hba,
		IOST_BASE_ADDR_LO,
		DMA_ADDR_LO(hisi_hba->iost_dma));

	hisi_sas_write32(hisi_hba,
		IOST_BASE_ADDR_HI,
		DMA_ADDR_HI(hisi_hba->iost_dma));

	/* breakpoint */
	hisi_sas_write32(hisi_hba,
		BROKEN_MSG_ADDR_LO,
		DMA_ADDR_LO(hisi_hba->breakpoint_dma));

	hisi_sas_write32(hisi_hba,
		BROKEN_MSG_ADDR_HI,
		DMA_ADDR_HI(hisi_hba->breakpoint_dma));

	return 0;
}

static int hisi_sas_init_id_frame(struct hisi_hba *hisi_hba)
{
	int i;
	struct sas_identify_frame identify_frame;
	u32 *identify_buffer;

	/*ifdef _LITTLE_ENDIAN_BITFIELD,
	*sas_identify_frame the same as the structure in IT code*/
	for (i = 0; i < hisi_hba->n_phy; i++) {
		memset(&identify_frame, 0, sizeof(identify_frame));
		/*dev_type is [6-4]bit, frame_type is [3-0]bit
		 *according to IT code, the byte is set to 0x10 */
		identify_frame.dev_type = 1;
		identify_frame.frame_type = 0;
		/*_un1 is the second byte,the byte is set to 0x1 in IT code*/
		identify_frame._un1 = 1;

		identify_frame.initiator_bits = SAS_PROTOCOL_ALL;
		identify_frame.target_bits = SAS_PROTOCOL_NONE;
		memcpy(identify_frame._un4_11,
			hisi_hba->sas_addr,
			SAS_ADDR_SIZE);
		memcpy(identify_frame.sas_addr,
			hisi_hba->sas_addr,
			SAS_ADDR_SIZE);
		identify_frame.phy_id = i;

		identify_buffer = (u32 *)((void *)&identify_buffer);
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD0_REG,
			__swab32(identify_buffer[0]));
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD1_REG,
			identify_buffer[2]);
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD2_REG,
			identify_buffer[1]);
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD3_REG,
			identify_buffer[4]);
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD4_REG,
			identify_buffer[3]);
		hisi_sas_phy_write32(hisi_hba,
			i,
			TX_ID_DWORD5_REG,
			__swab32(identify_buffer[5]));
	}
	return 0;
}


int hisi_sas_hw_init(struct hisi_hba *hisi_hba)
{
	int rc;

	rc = hisi_sas_reset_hw(hisi_hba);
	if (rc) {
		pr_err("hisi_sas_reset_hw failed, rc=%d", rc);
		return rc;
	}

	rc = hisi_sas_init_reg(hisi_hba);
	if (rc) {
		pr_err("hisi_sas_init_reg failed, rc=%d", rc);
		return rc;
	}

	/* maybe init serdes param j00310691 */
	rc = hisi_sas_init_id_frame(hisi_hba);
	if (rc) {
		pr_err("hisi_sas_init_id_frame failed, rc=%d", rc);
		return rc;
	}

	return 0;
}


int hisi_sas_scan_finished(struct Scsi_Host *shost, unsigned long time)
{
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	struct hisi_hba_priv_info *hisi_hba_priv = sha->lldd_ha;

	if (hisi_hba_priv->scan_finished == 0)
		return 0;

	sas_drain_work(sha);
	return 1;
}

void hisi_sas_scan_start(struct Scsi_Host *shost)
{
	int i, j;
	unsigned short core_nr;
	struct hisi_hba *hisi_hba;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	struct hisi_hba_priv_info *hisi_hba_priv = sha->lldd_ha;

	core_nr = hisi_hba_priv->n_core;

	for (j = 0; j < core_nr; j++) {
		hisi_hba = ((struct hisi_hba_priv_info *)sha->lldd_ha)->hisi_hba[j];
		for (i = 0; i < hisi_hba->n_phy; ++i)
			hisi_sas_bytes_dmaed(hisi_hba, i);
	}

	hisi_hba_priv->scan_finished = 1;
}

void hisi_sas_phy_init(struct hisi_hba *hisi_hba, int i)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[i];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;

	phy->hisi_hba = hisi_hba;
	phy->port = NULL;
	init_timer(&phy->timer);
	sas_phy->enabled = (i < hisi_hba->n_phy) ? 1 : 0;
	sas_phy->class = SAS;
	sas_phy->iproto = SAS_PROTOCOL_ALL;
	sas_phy->tproto = 0;
	sas_phy->type = PHY_TYPE_PHYSICAL;
	sas_phy->role = PHY_ROLE_INITIATOR;
	sas_phy->oob_mode = OOB_NOT_CONNECTED;
	sas_phy->linkrate = SAS_LINK_RATE_UNKNOWN;

	sas_phy->id = i;
	sas_phy->sas_addr = &hisi_hba->sas_addr[0];
	sas_phy->frame_rcvd = &phy->frame_rcvd[0];
	sas_phy->ha = (struct sas_ha_struct *)hisi_hba->shost->hostdata;
	sas_phy->lldd_phy = phy;

	/*phy registers init set 12G*/
	hisi_sas_phy_write32(hisi_hba, i, PROG_PHY_LINK_RATE_REG, 0x0000088a);
	hisi_sas_phy_write32(hisi_hba, i, PHY_CONFIG2_REG, 0x80c7c084);
	hisi_sas_phy_write32(hisi_hba, i, PHY_RATE_NEGO_REG, 0x415ee00);
	hisi_sas_phy_write32(hisi_hba, i, PHY_PCN_REG, 0x80aa0001);

	hisi_sas_phy_write32(hisi_hba, i, SL_TOUT_CFG_REG, 0x7d7d7d7d);
	hisi_sas_phy_write32(hisi_hba, i, DONE_RECEVIED_TIME_REG, 0x0);
	hisi_sas_phy_write32(hisi_hba, i, RXOP_CHECK_CFG_H_REG, 0x1000);
	hisi_sas_phy_write32(hisi_hba, i, DONE_RECEVIED_TIME_REG, 0);
	hisi_sas_phy_write32(hisi_hba, i, CON_CFG_DRIVER_REG, 0x13f0a);
	hisi_sas_phy_write32(hisi_hba, i, CHL_INT_COAL_EN_REG, 3);
	hisi_sas_phy_write32(hisi_hba, i, DONE_RECEVIED_TIME_REG, 8);
	/*unmask phy intr*/
	hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK_REG, 0x003ce3ee);
	hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK_REG, 0x00017fff);
	hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK_REG, 0x0000032a);
}


int hisi_sas_dev_found(struct domain_device *dev)
{
	pr_info("%s\n", __func__);

	return hisi_sas_dev_found_notify(dev, 1);
}

void hisi_sas_dev_gone(struct domain_device *dev)
{
	pr_info("%s\n", __func__);
}

int hisi_sas_queue_command(struct sas_task *task, gfp_t gfp_flags)
{
	struct hisi_sas_device *dev = task->dev->lldd_dev;
	struct sas_ha_struct *sas = dev->hisi_hba->sas;

	pr_info("%s dev=%p sas=%p\n", __func__, dev, sas);

	return hisi_sas_task_exec(task, gfp_flags, NULL, 0, NULL);
}

int hisi_sas_control_phy(struct asd_sas_phy *sas_phy,
			enum phy_func func,
			void *funcdata)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_abort_task(struct sas_task *task)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_abort_task_set(struct domain_device *dev, u8 *lun)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_clear_aca(struct domain_device *dev, u8 *lun)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_clear_task_set(struct domain_device *dev, u8 *lun)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_I_T_nexus_reset(struct domain_device *dev)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_lu_reset(struct domain_device *dev, u8 *lun)
{
	pr_info("%s\n", __func__);
	return 0;
}

int hisi_sas_query_task(struct sas_task *task)
{
	pr_info("%s\n", __func__);
	return 0;
}

void hisi_sas_port_formed(struct asd_sas_phy *sas_phy)
{
	pr_info("%s\n", __func__);
}

void hisi_sas_port_deformed(struct asd_sas_phy *sas_phy)
{
	pr_info("%s\n", __func__);
}

static irqreturn_t hisi_sas_int_phyup(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & PHY_ENABLED))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phy_up\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, PHY_ENABLED);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_ctrlrdy(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & PHY_CTRLRDY))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phy_ctrlrdy\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, PHY_CTRLRDY);

	return IRQ_HANDLED;
}


static irqreturn_t hisi_sas_int_dmaerr(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & DMA_RESP_ERR))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in dma_resp_err\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, DMA_RESP_ERR);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_hotplug(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & PHYCTRL_HOTPLUG_TOUT))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in hotplug_tout\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, PHYCTRL_HOTPLUG_TOUT);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_bcast(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & SL_RX_BCAST_ACK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in sl_rx_bcast_ack\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, SL_RX_BCAST_ACK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_oobrst(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & PHYCTRL_OOB_RESTART_CI))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phyctrl_oob_restart_ci\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, PHYCTRL_OOB_RESTART_CI);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_hardrst(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & SL_RX_HARDRST))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in sl_rx_hardrst\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, SL_RX_HARDRST);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_statuscg(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & PHYCTRL_STATUS_CHG))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phyctrl_status_chg\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, PHYCTRL_STATUS_CHG);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_int0(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;
	u32 irq_mask_save;

	/* mask_int0 */
	irq_mask_save = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_MSK_REG);
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, 0x003FFFFF);

	/* read int0 */
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_REG);

	pr_info("%s phy = %d, irq0_value = %x\n", __func__, phy_no, irq_value);

	/* write to zero */
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_REG, irq_value);
	/* recovery int0_mask */
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, irq_mask_save);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_int1(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT1_REG);

	pr_info("%s phy = %d, irq1_value = %x\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT1_REG, irq_value);

	return IRQ_HANDLED;
}

/* Interrupts */
/* j00310691 dummy interrupts */
irqreturn_t hisi_sas_cq_interrupt(struct hisi_hba *hisi_hba, int queue)
{
	struct hisi_sas_slot *slot;
	struct hisi_sas_complete_hdr *complete_queue = hisi_hba->complete_hdr[queue];

	for (;;) {
		/* j00310691 read through read pointer in cq */
		struct hisi_sas_complete_hdr *complete_hdr;
		struct hisi_sas_iptt *iptt;
		int iptt_tag;
		int queue_slot = 0; /* j00310691 fixme - should be read from hw */

		complete_hdr = &complete_queue[queue_slot];
		iptt_tag = complete_hdr->iptt;
		iptt = &hisi_hba->iptt[iptt_tag];

		slot = &hisi_hba->slot_info[iptt->queue*hisi_hba->queue_count+iptt->queue_slot];

		hisi_sas_slot_complete(hisi_hba, slot, 0);
		/* j00310691 fixme - add escape */
	}

	return IRQ_HANDLED;
}

#define DECLARE_INT_HANDLER(handler, index)\
irqreturn_t handler##index(int irq, void *p)\
{\
	return	handler(index, p);\
}

#define INT_HANDLER_NAME(handler, index)\
	handler##index

#define DECLARE_PHY_INT_HANDLER_GROUP(phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_ctrlrdy, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_dmaerr, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_hotplug, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_bcast, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_oobrst, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_hardrst, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_statuscg, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_phyup, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_int0, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_int1, phy)\


#define DECLARE_PHY_INT_GROUP_PTR(phy)\
	INT_HANDLER_NAME(hisi_sas_int_ctrlrdy, phy),\
	INT_HANDLER_NAME(hisi_sas_int_dmaerr, phy),\
	INT_HANDLER_NAME(hisi_sas_int_hotplug, phy),\
	INT_HANDLER_NAME(hisi_sas_int_bcast, phy),\
	INT_HANDLER_NAME(hisi_sas_int_oobrst, phy),\
	INT_HANDLER_NAME(hisi_sas_int_hardrst, phy),\
	INT_HANDLER_NAME(hisi_sas_int_statuscg, phy),\
	INT_HANDLER_NAME(hisi_sas_int_phyup, phy),\
	INT_HANDLER_NAME(hisi_sas_int_int0, phy),\
	INT_HANDLER_NAME(hisi_sas_int_int1, phy),

DECLARE_PHY_INT_HANDLER_GROUP(0)
DECLARE_PHY_INT_HANDLER_GROUP(1)
DECLARE_PHY_INT_HANDLER_GROUP(2)
DECLARE_PHY_INT_HANDLER_GROUP(3)
DECLARE_PHY_INT_HANDLER_GROUP(4)
DECLARE_PHY_INT_HANDLER_GROUP(5)
DECLARE_PHY_INT_HANDLER_GROUP(6)
DECLARE_PHY_INT_HANDLER_GROUP(7)

static const char phy_int_names[MSI_PHY_INT_COUNT][32] = {
	{"CTRL Rdy"},
	{"DMA Err"},
	{"HotPlug"},
	{"Bcast"},
	{"OOBRst"},
	{"HardRst"},
	{"StatusCG"},
	{"Phy Up"},
	{"Int0"},
	{"Int1"}
};

static char int_names[HISI_SAS_MAX_INTERRUPTS][32];

irq_handler_t phy_interrupt_handlers[HISI_SAS_MAX_PHYS][MSI_PHY_INT_COUNT] = {
	{DECLARE_PHY_INT_GROUP_PTR(0)},
	{DECLARE_PHY_INT_GROUP_PTR(1)},
	{DECLARE_PHY_INT_GROUP_PTR(2)},
	{DECLARE_PHY_INT_GROUP_PTR(3)},
	{DECLARE_PHY_INT_GROUP_PTR(4)},
	{DECLARE_PHY_INT_GROUP_PTR(5)},
	{DECLARE_PHY_INT_GROUP_PTR(6)},
	{DECLARE_PHY_INT_GROUP_PTR(7)},
};

int hisi_sas_interrupt_init(struct hisi_hba *hisi_hba)
{
	int i, j, irq, rc;

	if (!hisi_hba->np)
		return -ENOENT;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		for (j = 0; j < MSI_PHY_INT_COUNT; j++) {
			irq = irq_of_parse_and_map(hisi_hba->np, j + (i * MSI_PHY_INT_COUNT));
			if (!irq) {
				pr_err("%s core %d (np=%p) could not map interrupt %d\n", __func__, hisi_hba->id, hisi_hba->np, j + (i * MSI_PHY_INT_COUNT));
				return -ENOENT;
			}
			(void)snprintf(int_names[j + (i * MSI_PHY_INT_COUNT)], 32, "hisi sas %s [%d %d]", &phy_int_names[j][0],  hisi_hba->id, i);
			rc = request_irq(irq, phy_interrupt_handlers[i][j], 0, int_names[j + (i * MSI_PHY_INT_COUNT)], hisi_hba);
			if (rc) {
				pr_err("%s core %d could not request interrupt %d, rc=%d\n", __func__, hisi_hba->id, irq, rc);
				return -ENOENT;
			}
		}


	}

	return 0;
}


