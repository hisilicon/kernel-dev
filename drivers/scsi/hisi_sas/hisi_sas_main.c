#include "hisi_sas.h"
#include <linux/swab.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>


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
#define PHY_STATE_REG		(GLOBAL_BASE_REG + 0x24)
#define PHY_PORT_NUM_MA_REG	(GLOBAL_BASE_REG + 0x28)
#define PORT_STATE_REG		(GLOBAL_BASE_REG + 0x2c)
#define PHY_CONN_RATE_REG	(GLOBAL_BASE_REG + 0x30)
#define DLVRY_Q_0_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x260)
#define DLVRY_Q_0_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0x264)
#define DLVRY_Q_0_DEPTH		(GLOBAL_BASE_REG + 0x268)
#define DLVRY_Q_0_WR_PTR	(GLOBAL_BASE_REG + 0x26c)
#define DLVRY_Q_0_RD_PTR	(GLOBAL_BASE_REG + 0x270)
#define COMPL_Q_0_BASE_ADDR_LO	(GLOBAL_BASE_REG + 0x4e0)
#define COMPL_Q_0_BASE_ADDR_HI	(GLOBAL_BASE_REG + 0x4e4)
#define COMPL_Q_0_DEPTH		(GLOBAL_BASE_REG + 0x4e8)
#define COMPL_Q_0_WR_PTR	(GLOBAL_BASE_REG + 0x4ec)
#define COMPL_Q_0_RD_PTR	(GLOBAL_BASE_REG + 0x4f0)

#define PORT_BASE_REG		(0x800)

#define PHY_CTRL_REG		(PORT_BASE_REG + 0x14)
#define PHY_CTRL_REG_RESET_OFF	0
#define PHY_CTRL_REG_RESET_MSK	0x1
#define TX_ID_DWORD0_REG	(PORT_BASE_REG + 0x9c)
#define TX_ID_DWORD1_REG	(PORT_BASE_REG + 0xa0)
#define TX_ID_DWORD2_REG	(PORT_BASE_REG + 0xa4)
#define TX_ID_DWORD3_REG	(PORT_BASE_REG + 0xa8)
#define TX_ID_DWORD4_REG	(PORT_BASE_REG + 0xaC)
#define TX_ID_DWORD5_REG	(PORT_BASE_REG + 0xb0)
#define TX_ID_DWORD6_REG	(PORT_BASE_REG + 0xb4)
#define DMA_TX_STATUS_REG	(PORT_BASE_REG + 0x2d0)
#define DMA_TX_STATUS_BUSY_OFF	0
#define DMA_TX_STATUS_BUSY_MSK	0x1
#define DMA_RX_STATUS_REG	(PORT_BASE_REG + 0x2e8)
#define DMA_RX_STATUS_BUSY_OFF	0
#define DMA_RX_STATUS_BUSY_MSK	0x1
#define AXI_CFG_REG		(0x5100)

/*global registers need init*/
#define DLVRY_QUEUE_ENABLE_REG  (GLOBAL_BASE_REG + 0x0)
#define HGC_TRANS_TASK_CNT_LIMIT_REG    (GLOBAL_BASE_REG + 0x38)
#define DEVICE_MSG_WORK_MODE_REG        (GLOBAL_BASE_REG + 0x94)
#define MAX_BURST_BYTES_REG             (GLOBAL_BASE_REG + 0x98)
#define SMP_TIMEOUT_TIMER_REG           (GLOBAL_BASE_REG + 0x9c)
#define MAX_CON_TIME_LIMIT_TIME_REG     (GLOBAL_BASE_REG + 0xa4)
#define HGC_SAS_TXFAIL_RETRY_CTRL_REG   (GLOBAL_BASE_REG + 0x84)
#define HGC_ERR_STAT_EN_REG             (GLOBAL_BASE_REG + 0x238)
#define CFG_1US_TIMER_TRSH_REG          (GLOBAL_BASE_REG + 0xcc)
#define HGC_GET_ITV_TIME_REG            (GLOBAL_BASE_REG + 0x90)
#define I_T_NEXUS_LOSS_TIME_REG         (GLOBAL_BASE_REG + 0xa0)
#define BUS_INACTIVE_LIMIT_TIME_REG     (GLOBAL_BASE_REG + 0xa8)
#define REJECT_TO_OPEN_LIMIT_TIME_REG   (GLOBAL_BASE_REG + 0xac)
#define CFG_AGING_TIME_REG              (GLOBAL_BASE_REG + 0xbc)
#define HGC_DFX_CFG_REG2_REG            (GLOBAL_BASE_REG + 0xc0)
#define FIS_LIST_BADDR_L_REG            (GLOBAL_BASE_REG + 0xc4)
#define INT_COAL_EN_REG                 (GLOBAL_BASE_REG + 0x1bc)
#define OQ_INT_COAL_TIME_REG            (GLOBAL_BASE_REG + 0x1c0)
#define OQ_INT_COAL_CNT_REG             (GLOBAL_BASE_REG + 0x1c4)
#define ENT_INT_COAL_TIME_REG           (GLOBAL_BASE_REG + 0x1c8)
#define ENT_INT_COAL_CNT_REG            (GLOBAL_BASE_REG + 0x1cc)
#define OQ_INT_SRC_REG                  (GLOBAL_BASE_REG + 0x1d0)
#define OQ_INT_SRC_MSK_REG              (GLOBAL_BASE_REG + 0x1d4)
#define ENT_INT_SRC1_REG		(GLOBAL_BASE_REG + 0x1d8)
#define ENT_INT_SRC_MSK1_REG            (GLOBAL_BASE_REG + 0x1e0)
#define ENT_INT_SRC2_REG                (GLOBAL_BASE_REG + 0x1dc)
#define ENT_INT_SRC_MSK2_REG            (GLOBAL_BASE_REG + 0x1e4)
#define SAS_ECC_INTR_MSK_REG            (GLOBAL_BASE_REG + 0x1ec)
#define AXI_AHB_CLK_CFG_REG             (GLOBAL_BASE_REG + 0x3c)
#define CFG_SAS_CONFIG_REG              (GLOBAL_BASE_REG + 0xd4)

/*phy registers need init*/
#define PHY_CFG_REG			(PORT_BASE_REG + 0x0)
#define PHY_CFG_REG_ENA_OFF		0
#define PHY_CFG_REG_ENA_MSK		0x1
#define PHY_CFG_REG_SATA_OFF		1
#define PHY_CFG_REG_SATA_MSK		0x2
#define PHY_CFG_REG_DC_OPT_OFF		2
#define PHY_CFG_REG_DC_OPT_MSK		0x4
#define HARD_PHY_LINK_RATE_REG          (PORT_BASE_REG + 0x4)
#define HARD_PHY_LINK_RATE_REG_MAX_OFF	0
#define HARD_PHY_LINK_RATE_REG_MAX_MSK	0xf
#define HARD_PHY_LINK_RATE_REG_MIN_OFF	4
#define HARD_PHY_LINK_RATE_REG_MIN_MSK	0xf0
#define HARD_PHY_LINK_RATE_REG_NEG_OFF	8
#define HARD_PHY_LINK_RATE_REG_NEG_MSK	0xf00
#define PROG_PHY_LINK_RATE_REG          (PORT_BASE_REG + 0xc)
#define PROG_PHY_LINK_RATE_REG_MAX_OFF	0
#define PROG_PHY_LINK_RATE_REG_MAX_MSK	0xf
#define PROG_PHY_LINK_RATE_REG_MIN_OFF	4
#define PROG_PHY_LINK_RATE_REG_MIN_MSK	0xf0
#define PROG_PHY_LINK_RATE_REG_OOB_OFF	8
#define PROG_PHY_LINK_RATE_REG_OOB_MSK	0xf00
#define PHY_CONFIG2_REG                 (PORT_BASE_REG + 0x1a8)
#define PHY_CONFIG2_REG_RXCLTEPRES_OFF	0
#define PHY_CONFIG2_REG_RXCLTEPRES_MSK	0xFF
#define CFG_TX_TRAIN_COMP_OFF           24
#define CFG_TX_TRAIN_COMP_MSK		0x1
#define PHY_RATE_NEGO_REG               (PORT_BASE_REG + 0x30)
#define PHY_PCN_REG                     (PORT_BASE_REG + 0x44)
#define SL_TOUT_CFG_REG                 (PORT_BASE_REG + 0x8c)
#define SL_CONTROL_REG			(PORT_BASE_REG + 0x94)
#define SL_CONTROL_REG_NOTIFY_EN_OFF	0
#define SL_CONTROL_REG_NOTIFY_EN_MSK	0x1
#define RX_IDAF_DWORD0_REG		(PORT_BASE_REG + 0xc4)
#define RX_IDAF_DWORD1_REG		(PORT_BASE_REG + 0xc8)
#define RX_IDAF_DWORD2_REG		(PORT_BASE_REG + 0xcc)
#define RX_IDAF_DWORD3_REG		(PORT_BASE_REG + 0xd0)
#define RX_IDAF_DWORD4_REG		(PORT_BASE_REG + 0xd4)
#define RX_IDAF_DWORD5_REG		(PORT_BASE_REG + 0xd8)
#define RX_IDAF_DWORD6_REG		(PORT_BASE_REG + 0xdc)
#define RXOP_CHECK_CFG_H_REG            (PORT_BASE_REG + 0xfc)
#define DONE_RECEVIED_TIME_REG          (PORT_BASE_REG + 0x12c)
#define CON_CFG_DRIVER_REG              (PORT_BASE_REG + 0x130)
#define CHL_INT_COAL_EN_REG             (PORT_BASE_REG + 0x1d0)
/*phy intr registers*/
#define CHL_INT0_REG                    (PORT_BASE_REG + 0x1b0)
#define CHL_INT0_REG_PHYCTRL_NOTRDY_OFF 0
#define CHL_INT0_REG_PHYCTRL_NOTRDY_MSK 0x1
#define CHL_INT0_REG_SN_FAIL_NGR_OFF	2
#define CHL_INT0_REG_SN_FAIL_NGR_MSK	0x4
#define CHL_INT0_REG_DWS_LOST_OFF	4
#define CHL_INT0_REG_DWS_LOST_MSK	0x10
#define CHL_INT0_REG_SL_IDAF_FAIL_OFF	10
#define CHL_INT0_REG_SL_IDAF_FAIL_MSK	0x400
#define CHL_INT0_REG_ID_TIMEOUT_OFF	11
#define CHL_INT0_REG_ID_TIMEOUT_MSK	0x800
#define CHL_INT0_REG_SL_OPAF_FAIL_OFF	12
#define CHL_INT0_REG_SL_OPAF_FAIL_MSK	0x1000
#define CHL_INT0_REG_SL_PS_FAIL_OFF	21
#define CHL_INT0_REG_SL_PS_FAIL_MSK	0x200000
#define CHL_INT1_REG                    (PORT_BASE_REG + 0x1b4)
#define CHL_INT2_REG                    (PORT_BASE_REG + 0x1b8)
#define CHL_INT2_REG_CTRL_PHY_RDY_OFF	0
#define CHL_INT2_REG_CTRL_PHY_RDY_MSK	0x1
#define CHL_INT2_REG_PHY_HP_TOUT_OFF	1
#define CHL_INT2_REG_PHY_HP_TOUT_MSK	0x2
#define CHL_INT2_REG_SL_RX_BC_ACK_OFF	2
#define CHL_INT2_REG_SL_RX_BC_ACK_MSK	0x4
#define CHL_INT2_REG_OOB_RESTART_OFF	3
#define CHL_INT2_REG_OOB_RESTART_MSK	0x8
#define CHL_INT2_SL_RX_HARDRST_OFF	4
#define CHL_INT2_SL_RX_HARDRST_MSK	0x10
#define CHL_INT2_PHY_STATUS_CHG_OFF	5
#define CHL_INT2_PHY_STATUS_CHG_MSK	0x20
#define CHL_INT2_REG_SL_PHY_ENA_OFF	6
#define CHL_INT2_REG_SL_PHY_ENA_MSK	0x40
#define CHL_INT2_REG_DMA_RESP_ERR_OFF	7
#define CHL_INT2_REG_DMA_RESP_ERR_MSK	0x80
/*phy intr_mask registers need unmask*/
#define CHL_INT0_MSK_REG                (PORT_BASE_REG + 0x1bc)
#define CHL_INT1_MSK_REG                (PORT_BASE_REG + 0x1c0)
#define CHL_INT2_MSK_REG                (PORT_BASE_REG + 0x1c4)

/* reg & mask used for bus */
#define RESET_STATUS_MSK		(0x7ffff)
#define RESET_STATUS_RESET		(0x7ffff)
#define RESET_STATUS_DERESET		(0x0)

/*nego query windown*/
#define NEGO_QUERY_WINDOW_12G       40

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

	if (slot->command_table)
		dma_pool_free(hisi_hba->command_table_pool, slot->command_table,
			      slot->command_table_dma);

	if (slot->status_buffer)
		dma_pool_free(hisi_hba->status_buffer_pool, slot->status_buffer,
			      slot->status_buffer_dma);

	if (slot->sge_page)
		dma_pool_free(hisi_hba->sge_page_pool, slot->sge_page,
			      slot->sge_page_dma);

	list_del_init(&slot->entry);
	task->lldd_task = NULL;
	slot->task = NULL;
	slot->port = NULL;
	hisi_sas_iptt_free(hisi_hba, slot->iptt);
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
	struct hisi_sas_complete_hdr *complete_queue = hisi_hba->complete_hdr[slot->queue];
	struct hisi_sas_complete_hdr *complete_hdr;
	complete_hdr = &complete_queue[slot->queue_slot];

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
			pr_debug("%s port has not device.\n", __func__);
		tstat->stat = SAS_PHY_DOWN;
		goto out;
	}

	if (complete_hdr->err_rcrd_xfrd) {
		pr_debug("%s slot %d has error info"
			"0x%x\n", __func__, slot->queue_slot,
			 complete_hdr->err_rcrd_xfrd);
		//tstat->stat = hisi_sas_slot_err(hisi_hba, task, slot);
		tstat->stat = SAS_DATA_UNDERRUN;
		tstat->resp = SAS_TASK_COMPLETE;
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
			pr_err("%s STP not supported", __func__);
			break;
		}

	default:
		tstat->stat = SAM_STAT_CHECK_CONDITION;
		break;
	}
	if (!slot->port->port_attached) {
		pr_debug("%s port %d has removed.\n", __func__, slot->port->sas_port.id);
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
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		pr_err("%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT", __func__, n_elem);
		return -EINVAL;
	}

	slot->sge_page = dma_pool_alloc(hisi_hba->sge_page_pool, GFP_ATOMIC,
					&slot->sge_page_dma);
	if (!slot->sge_page)
		return -ENOMEM;

	hdr->pir_pres = 0;
	hdr->t10_flds_pres = 0;

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
	u8 *buf_cmd, fburst = 0;

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

	slot = &hisi_hba->slot_info[iptt];
	memset(slot, 0, sizeof(struct hisi_sas_slot));

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
	memset(slot->command_table, 0, HISI_SAS_COMMAND_TABLE_SZ);

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

static void hisi_sas_bytes_dmaed(struct hisi_hba *hisi_hba, int phy_no)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha;

	if (!phy->phy_attached)
		return;


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
		pr_info("%s fixme att_dev_info\n", __func__);
		if (phy->att_dev_info & PORT_SSP_TRGT_MASK) {
			/* MVS_CHIP_DISP->write_port_cfg_addr(mvi, i, PHYR_PHY_STAT); */
			/* MVS_CHIP_DISP->write_port_cfg_data(mvi, i, 0x00); */
		}
	} else if (phy->phy_type & PORT_TYPE_SATA) {
		/*Nothing*/
	}

	sas_phy->frame_rcvd_size = phy->frame_rcvd_size;

	hisi_hba->sas->notify_port_event(sas_phy,
				   PORTE_BYTES_DMAED);
}

struct hisi_hba *hisi_sas_find_dev_hba(struct domain_device *dev)
{
	unsigned long i = 0, j = 0, hi = 0;
	struct sas_ha_struct *sha = dev->port->ha;
	struct hisi_hba *hisi_hba = NULL;
	struct asd_sas_phy *phy;

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
	struct hisi_sas_itct *itct = &hisi_hba->itct[device_id];

	memset(itct, 0, sizeof(*itct));

	/* qw0 */
	if (dev->dev_type == SAS_END_DEVICE)
		itct->dev_type = HISI_SAS_DEV_TYPE_SSP;
	else
		pr_warn("%s unsupported dev type\n", __func__);
	itct->valid = 1;
	itct->break_reply_ena = 0;
	itct->awt_control = 1;
	itct->max_conn_rate = dev->max_linkrate; /* j00310691 todo doublecheck, see enum sas_linkrate */
	itct->valid_link_number = 1;
	itct->port_id = dev->port->id;
	itct->smp_timeout = 0;
	itct->max_burst_byte = 0;

	/* qw1 */
	memcpy(&itct->sas_addr, dev->sas_addr, SAS_ADDR_SIZE);
	itct->sas_addr = __swab64(itct->sas_addr);

	/* qw2 */
	itct->IT_nexus_loss_time = 500;
	itct->bus_inactive_time_limit = 0xff00;
	itct->max_conn_time_limit = 0xff00;
	itct->reject_open_time_limit = 0xff00;
}

int hisi_sas_dev_found_notify(struct domain_device *dev, int lock)
{
	unsigned long flags = 0;
	int res = 0;
	struct hisi_hba *hisi_hba = NULL;
	struct domain_device  *parent_dev = dev->parent;
	struct hisi_sas_device *hisi_sas_device;

	hisi_hba = hisi_sas_find_dev_hba(dev);

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

	if (parent_dev && DEV_IS_EXPANDER(parent_dev->dev_type)) {
		int phy_id;
		u8 phy_num = parent_dev->ex_dev.num_phys;
		struct ex_phy *phy;

		for (phy_id = 0; phy_id < phy_num; phy_id++) {
			phy = &parent_dev->ex_dev.ex_phy[phy_id];
			if (SAS_ADDR(phy->attached_sas_addr) ==
				SAS_ADDR(dev->sas_addr)) {
				hisi_sas_device->attached_phy = phy_id;
				break;
			}
		}

		if (phy_id == phy_num) {
			pr_info("%s Error: no attached dev:%016llx"
				"at ex:%016llx.\n",
				__func__,
				SAS_ADDR(dev->sas_addr),
				SAS_ADDR(parent_dev->sas_addr));
			res = -1;
		}
	}

found_out:
	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
	return res;
}

static int hisi_sas_reset_hw(struct hisi_hba *hisi_hba)
{
	int i;
	unsigned long end_time;
	int reg_value = 0;
	void __iomem *sub_ctrl_base = 0;
	u32 sub_ctrl_range = 0;
	u64 reset_reg_addr = 0;
	u64 dereset_reg_addr = 0;
	u32 reset_value = 0;
	u32 dereset_value = 0;
	u64 reset_status_reg_addr = 0;

	/* inline Higgs_PrepareResetHw j00310691 */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 phy_ctrl = hisi_sas_phy_read32(hisi_hba, i, PHY_CTRL_REG);

		phy_ctrl |= PHY_CTRL_REG_RESET_MSK;
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

			if (!(dma_tx_status & DMA_TX_STATUS_BUSY_MSK) &&
				!(dma_rx_status & DMA_RX_STATUS_BUSY_MSK))
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
			break;

		msleep(20);
		if (time_after(jiffies, end_time))
			return -EIO;
	}


/* do you mean i put them here ?*/
#define DSAF_SUBCTL_BASE                    (0xc0000000ull)
#define DSAF_SUBCTL_RANGE                   (0xffff)
#define DSAF_SUB_CTRL_RESET_OFFSET          (0xa60)
#define DSAF_SUB_CTRL_DERESET_OFFSET        (0xa64)
#define DSAF_SUB_CTRL_RESET_STATUS_OFFSET   (0x5a30)
#define DSAF_SUB_CTRL_RESET_VALUE           (0x7ffff)
#define DSAF_SUB_CTRL_DERESET_VALUE         (0x7ffff)

#define PCIE_SUBCTL_BASE                    (0xb0000000ull)
#define PCIE_SUBCTL_RANGE                   (0xffff)
#define PCIE_SUB_CTRL_RESET_OFFSET          (0xa18)
#define PCIE_SUB_CTRL_DERESET_OFFSET        (0xa1c)
#define PCIE_SUB_CTRL_RESET_STATUS_OFFSET   (0x5a0c)
#define PCIE_SUB_CTRL_RESET_VALUE           (0x7ffff)
#define PCIE_SUB_CTRL_DERESET_VALUE         (0x7ffff)

	if (0 == hisi_hba->id) {
		sub_ctrl_base = (void __iomem *)DSAF_SUBCTL_BASE;
		sub_ctrl_range = DSAF_SUBCTL_RANGE;
		reset_reg_addr = DSAF_SUB_CTRL_RESET_OFFSET;
		dereset_reg_addr = DSAF_SUB_CTRL_DERESET_OFFSET;
		reset_status_reg_addr = DSAF_SUB_CTRL_RESET_STATUS_OFFSET;
		reset_value = DSAF_SUB_CTRL_RESET_VALUE;
		dereset_value = DSAF_SUB_CTRL_DERESET_VALUE;
	} else {
		sub_ctrl_base = (void __iomem *)PCIE_SUBCTL_BASE;
		sub_ctrl_range = PCIE_SUBCTL_RANGE;
		reset_reg_addr = PCIE_SUB_CTRL_RESET_OFFSET;
		dereset_reg_addr = PCIE_SUB_CTRL_DERESET_OFFSET;
		reset_status_reg_addr = PCIE_SUB_CTRL_RESET_STATUS_OFFSET;
		reset_value = PCIE_SUB_CTRL_RESET_VALUE;
		dereset_value = PCIE_SUB_CTRL_DERESET_VALUE;
	}

	/* reset */
	sub_ctrl_base = (void __iomem *)ioremap(
			(unsigned long)sub_ctrl_base,
			sub_ctrl_range);
	writel(reset_value, sub_ctrl_base + reset_reg_addr);
	mdelay(1);
	reg_value = readl(sub_ctrl_base + reset_status_reg_addr);
	if (RESET_STATUS_RESET != (reg_value & RESET_STATUS_MSK)) {
		pr_err("%s card:%d sas reset failed", __func__, hisi_hba->id);
		return -1;
	}

	/* dereset */
	writel(dereset_value, sub_ctrl_base + dereset_reg_addr);
	mdelay(1);
	reg_value = readl(sub_ctrl_base + reset_status_reg_addr);
	if (RESET_STATUS_DERESET != (reg_value & RESET_STATUS_MSK)) {
		pr_err("%s card:%d sas dereset failed",
			__func__,
			hisi_hba->id);
		return -1; /* sorry i don't konw about the right errcode.. */
	}

	/* io unmap */
	iounmap(sub_ctrl_base);
	return 0;
}

static int hisi_sas_init_reg(struct hisi_hba *hisi_hba)
{
	int i;

	/* Global registers init*/
	hisi_sas_write32(hisi_hba, DLVRY_QUEUE_ENABLE_REG, 0xffffffff); //fixme j00310691
	hisi_sas_write32(hisi_hba, HGC_TRANS_TASK_CNT_LIMIT_REG, 0x11);
	hisi_sas_write32(hisi_hba, DEVICE_MSG_WORK_MODE_REG, 0x1);
//	hisi_sas_write32(hisi_hba, MAX_BURST_BYTES_REG, 0);
//	hisi_sas_write32(hisi_hba, SMP_TIMEOUT_TIMER_REG, 0);
//	hisi_sas_write32(hisi_hba, MAX_CON_TIME_LIMIT_TIME_REG, 0);
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
	hisi_sas_write32(hisi_hba, ENT_INT_SRC1_REG, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1_REG, 0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC2_REG, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK2_REG, 0);
	hisi_sas_write32(hisi_hba, SAS_ECC_INTR_MSK_REG, 0);
	hisi_sas_write32(hisi_hba, AXI_AHB_CLK_CFG_REG, 0x2);
	hisi_sas_write32(hisi_hba, CFG_SAS_CONFIG_REG, 0x22000000);

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/*phy registers init set 12G - see g_astPortRegConfig */
		hisi_sas_phy_write32(hisi_hba,
				i,
				PROG_PHY_LINK_RATE_REG,
				0x0000088a);
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
	}

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


static void hisi_sas_config_id_frame(struct hisi_hba *hisi_hba, int phy)
{
	struct sas_identify_frame identify_frame;
	u32 *identify_buffer;

	memset(&identify_frame, 0, sizeof(identify_frame));
	/*dev_type is [6-4]bit, frame_type is [3-0]bit
	 *according to IT code, the byte is set to 0x10 */
	/* l00293075 I found this one */
	identify_frame.dev_type = SAS_END_DEVICE;
	identify_frame.frame_type = 0;
	/*_un1 is the second byte,the byte is set to 0x1 in IT code*/
	identify_frame._un1 = 1;

	identify_frame.initiator_bits = SAS_PROTOCOL_ALL;
	identify_frame.target_bits = SAS_PROTOCOL_NONE;
	memcpy(&identify_frame._un4_11[0],
		hisi_hba->sas_addr,
		SAS_ADDR_SIZE);
	memcpy(&identify_frame.sas_addr[0],
		hisi_hba->sas_addr,
		SAS_ADDR_SIZE);
	identify_frame.phy_id = phy;

	identify_buffer = (u32 *)(&identify_frame);
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD0_REG,
		__swab32(identify_buffer[0]));
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD1_REG,
		identify_buffer[2]);
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD2_REG,
		identify_buffer[1]);
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD3_REG,
		identify_buffer[4]);
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD4_REG,
		identify_buffer[3]);
	hisi_sas_phy_write32(hisi_hba,
		phy,
		TX_ID_DWORD5_REG,
		__swab32(identify_buffer[5]));
}

static int hisi_sas_init_id_frame(struct hisi_hba *hisi_hba)
{
	int i;

	/*ifdef _LITTLE_ENDIAN_BITFIELD,
	*sas_identify_frame the same as the structure in IT code*/
	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_config_id_frame(hisi_hba, i);
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

	msleep(100);
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
	init_timer(&phy->serdes_timer);
	init_timer(&phy->dma_status_timer);
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
}

static void hisi_sas_port_notify_formed(struct asd_sas_phy *sas_phy, int lock)
{
	struct sas_ha_struct *sas_ha = sas_phy->ha;
	struct hisi_hba *hisi_hba = NULL; int i = 0, hi;
	struct hisi_sas_phy *phy = sas_phy->lldd_phy;
	struct asd_sas_port *sas_port = sas_phy->port;
	struct hisi_sas_port *port;
	unsigned long flags = 0;
	if (!sas_port)
		return;

	while (sas_ha->sas_phy[i]) {
		if (sas_ha->sas_phy[i] == sas_phy)
			break;
		i++;
	}
	hi = i/((struct hisi_hba_priv_info *)sas_ha->lldd_ha)->n_phy;
	hisi_hba = ((struct hisi_hba_priv_info *)sas_ha->lldd_ha)->hisi_hba[hi];
	if (i >= hisi_hba->n_phy) {
		port = &hisi_hba->port[i - 8]; /* j00310691 fixme for >2 controllers and >8 phys/core */
		i -= 8;
	} else {
		port = &hisi_hba->port[i];
	}
	if (lock)
		spin_lock_irqsave(&hisi_hba->lock, flags);
	port->port_attached = 1;
	phy->port = port;
	pr_info("%s port=%p should be non-null\n", __func__, port);
	sas_port->lldd_port = port;
	if (phy->phy_type & PORT_TYPE_SAS) {
		u32 port_state = hisi_sas_read32(hisi_hba, PORT_STATE_REG);
		u32 phy_port_dis_state = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA_REG);
		pr_info("%s phy=%d port_state=0x%x phy_port_dis_state=0x%x hisi_hba->id=%d\n",
			__func__,
			i,
			port_state,
			phy_port_dis_state,
			hisi_hba->id
			);
	}
	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
}

void hisi_sas_do_release_task(struct hisi_hba *hisi_hba,
		int phy_no, struct domain_device *dev)
{
	struct hisi_sas_phy *phy;
	struct hisi_sas_port *port;

	phy = &hisi_hba->phy[phy_no];
	port = phy->port;
	if (!port)
		return;

	/* j00310691 todo release active slots */
	pr_info("%s release active slots todo\n", __func__);
}

static void hisi_sas_port_notify_deformed(struct asd_sas_phy *sas_phy, int lock)
{
	struct domain_device *dev;
	struct hisi_sas_phy *phy = sas_phy->lldd_phy;
	struct hisi_hba *hisi_hba = phy->hisi_hba;
	struct asd_sas_port *port = sas_phy->port;
	int phy_no = 0;

	while (phy != &hisi_hba->phy[phy_no]) {
		phy_no++;
		if (phy_no >= HISI_SAS_MAX_PHYS)
			return;
	}
	list_for_each_entry(dev, &port->dev_list, dev_list_node)
		hisi_sas_do_release_task(phy->hisi_hba, phy_no, dev);
}

int hisi_sas_dev_found(struct domain_device *dev)
{
	return hisi_sas_dev_found_notify(dev, 1);
}

void hisi_sas_dev_gone(struct domain_device *dev)
{
	pr_debug("%s\n", __func__);
}

int hisi_sas_queue_command(struct sas_task *task, gfp_t gfp_flags)
{
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
	hisi_sas_port_notify_formed(sas_phy, 1);
}

void hisi_sas_port_deformed(struct asd_sas_phy *sas_phy)
{
	hisi_sas_port_notify_deformed(sas_phy, 1);
}

static void hisi_sas_phy_disconnected(struct hisi_sas_phy *phy)
{
	phy->phy_attached = 0;
	phy->att_dev_info = 0;
	phy->att_dev_sas_addr = 0;
	phy->phy_type = 0;
}

static u32 hisi_sas_is_phy_ready(struct hisi_hba *hisi_hba, int phy_no)
{
	u32 phy_state, port_state, phy_port_dis_state;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct hisi_sas_port *port = phy->port;

	/* j00310691 fimxe (check on phy rdy register) */
	pr_info("%s1 port=%p hisi_hba=%p\n", __func__, port, hisi_hba);
	port_state = hisi_sas_read32(hisi_hba, PORT_STATE_REG);
	phy_port_dis_state = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA_REG);

	if (port!=NULL)
	{
		pr_info("%s port_state=0x%x phy_port_dis_state=0x%x hisi_hba->id=%d",
			__func__,
			port_state,
			phy_port_dis_state,
			hisi_hba->id);
	} else {
		pr_info("%s port_state=0x%x phy_port_dis_state=0x%x hisi_hba->id=%d",
			__func__,
			port_state,
			phy_port_dis_state,
			hisi_hba->id);
	}

	phy_state = hisi_sas_read32(hisi_hba, PHY_STATE_REG);
	if (phy_state & (1 << phy_no)) {
		if (!port)
			phy->phy_attached = 1;
		return 1;
	}

	/* phy is not ready, so update port */
	if (port) {
		u32 wide_port_phymap = (hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA_REG) >> (phy_no * 4)) & 0xf;
		pr_info("%s4\n", __func__);
		if (phy->phy_type & PORT_TYPE_SAS) {
			pr_info("%s5 wide_port_phymap=0x%x\n", __func__, wide_port_phymap);
			if (wide_port_phymap == 0xf)
				port->port_attached = 0;
		} else if (phy->phy_type & PORT_TYPE_SATA) {
			/* j00310691 todo */
		}
		phy->port = NULL;
		phy->phy_attached = 0;
		phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
	}

	return 0;
}

void hisi_sas_update_phyinfo(struct hisi_hba *hisi_hba, int phy_no, int get_st)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct sas_identify_frame *id;
	id = (struct sas_identify_frame *)phy->frame_rcvd;


	if (get_st) {
		phy->phy_status = hisi_sas_is_phy_ready(hisi_hba, phy_no);
	}

	if (phy->phy_status) {
		int oob_done = 0;
		struct asd_sas_phy *sas_phy = &phy->sas_phy;
		oob_done = 1;

		/* j00310691 do as fix phy info */
		phy->att_dev_sas_addr = *(u64 *)id->sas_addr;
		if (phy->phy_type & PORT_TYPE_SATA) {
			pr_info("%s todo for SATA\n", __func__);
		} else if (phy->phy_type & PORT_TYPE_SAS) {
			phy->phy_attached = 1;

			phy->identify.device_type = SAS_END_DEVICE;

			if (phy->identify.device_type == SAS_END_DEVICE)
				phy->identify.target_port_protocols =
							SAS_PROTOCOL_SSP;
			else if (phy->identify.device_type != SAS_PHY_UNUSED)
				phy->identify.target_port_protocols =
							SAS_PROTOCOL_SMP;
			if (oob_done)
				sas_phy->oob_mode = SAS_OOB_MODE;
			phy->frame_rcvd_size =
			    sizeof(struct sas_identify_frame);
		}
		memcpy(sas_phy->attached_sas_addr,
			&phy->att_dev_sas_addr, SAS_ADDR_SIZE);

	}
	pr_debug("%s phy %d attach dev info is %llx\n", __func__,
		phy_no + hisi_hba->id * hisi_hba->n_phy, phy->att_dev_info);
	pr_debug("%s phy %d attach sas addr is %llx\n", __func__,
		phy_no + hisi_hba->id * hisi_hba->n_phy, phy->att_dev_sas_addr);
}

static irqreturn_t hisi_sas_int_phyup(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value, port_id, link_rate;
	struct hisi_sas_port *port;
	int i;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	u32 *frame_rcvd = (u32 *)sas_phy->frame_rcvd;
	struct sas_identify_frame *id = (struct sas_identify_frame *)frame_rcvd;
	irqreturn_t res = IRQ_NONE;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_SL_PHY_ENA_MSK)) {
		pr_debug("%s irq_value = %x not set enable bit\n", __func__, irq_value);
		goto end;
	}

	port_id = (hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA_REG) >> (4 * phy_no)) & 0xf;
	if (port_id == 0xf) {
		pr_err("%s phy = %d, invalid portid\n", __func__, phy_no);
		goto end;
	}

	/* j00310691 todo stop serdes fw timer */

	port = &hisi_hba->port[port_id];
	for (i = 0; i < 6; i++) {
		u32 tmp = hisi_sas_phy_read32(hisi_hba, phy_no, RX_IDAF_DWORD0_REG + (i * 4));
		frame_rcvd[i] = __swab32(tmp);
	}

	phy->frame_rcvd_size = sizeof(struct sas_identify_frame);
	phy->phy_attached = 1;

	if (id->dev_type == SAS_END_DEVICE) {
		u32 sl_control =  hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL_REG);
		sl_control |= SL_CONTROL_REG_NOTIFY_EN_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL_REG, sl_control);
		mdelay(1);
		sl_control =  hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL_REG);
		sl_control &= ~SL_CONTROL_REG_NOTIFY_EN_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL_REG, sl_control);
	}

	/* Get the linkrate */
	link_rate = (hisi_sas_read32(hisi_hba, PHY_CONN_RATE_REG) >> (phy_no * 4)) & 0xf;
	sas_phy->linkrate = link_rate;

	phy->phy_type |= PORT_TYPE_SAS; /* j00310691 todo check for SATA */

	hisi_sas_update_phyinfo(hisi_hba, phy_no, 1);
	hisi_sas_bytes_dmaed(hisi_hba, phy_no);

	res = IRQ_HANDLED;
end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_REG_SL_PHY_ENA_MSK);

	if (irq_value & CHL_INT2_REG_SL_PHY_ENA_MSK) {
		u32 val = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_REG);
		val &= ~1;
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_REG, val);
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, 0x003ce3ee);
	}


	return res;
}

void hisi_sas_config_serdes_12G_timer_handler(unsigned long arg)
{
    struct hisi_sas_phy *phy = (struct hisi_sas_phy *)arg;
    struct hisi_hba *hisi_hba = phy->hisi_hba;
    int phy_id = phy->phy_id;

    u32 val = hisi_sas_phy_read32(hisi_hba, phy_id, PHY_CONFIG2_REG);
    val |= (CFG_TX_TRAIN_COMP_MSK << CFG_TX_TRAIN_COMP_OFF);
    hisi_sas_phy_write32(hisi_hba, phy_id, PHY_CONFIG2_REG, val);
}

int hisi_sas_config_serdes_12G(struct hisi_hba *hisi_hba, int phy_id)
{
	u32 link_rate = 0;
	unsigned long end_time = jiffies +
        msecs_to_jiffies(NEGO_QUERY_WINDOW_12G);
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_id];
	int loop_count = 0;

	phy->phy_id = phy_id;

	while(!time_after(jiffies, end_time)) {
		/*check for phy down interrupt*/
		u32 val = hisi_sas_phy_read32(hisi_hba, phy_id, CHL_INT0_REG);

		if (val & CHL_INT0_REG_PHYCTRL_NOTRDY_MSK) {
			pr_debug("%s phy come while ctrl rdy for phy %d\n",
		            __func__, phy_id);
			hisi_sas_phy_write32(hisi_hba, phy_id,
				CHL_INT0_REG,
				CHL_INT0_REG_PHYCTRL_NOTRDY_MSK);
			return 0;
		}

		link_rate = (hisi_sas_phy_read32(hisi_hba, phy_id, HARD_PHY_LINK_RATE_REG) & HARD_PHY_LINK_RATE_REG_NEG_MSK) >> HARD_PHY_LINK_RATE_REG_NEG_OFF;

		if (link_rate == SAS_LINK_RATE_12_0_GBPS) {
			pr_debug("%s a link_rate = %d loop count = %d\n", __func__, link_rate, loop_count);
			mdelay(20);
			/*i will check whether need Higgs_SerdesEnableCTLEDFE*/
			break;
		}
		loop_count++;
		udelay(100);
	}

	if (link_rate == SAS_LINK_RATE_12_0_GBPS) {
		struct timer_list *timer = &phy->serdes_timer;
		if (!timer_pending(timer)) {
			init_timer(timer);
			timer->data = (unsigned long)phy;
			timer->expires = jiffies + msecs_to_jiffies(300);
			timer->function = hisi_sas_config_serdes_12G_timer_handler;
			add_timer(timer);
		} else {
			mod_timer(timer, jiffies + msecs_to_jiffies(300));
		}
	}

	return 0;
}

static irqreturn_t hisi_sas_int_ctrlrdy(int phy, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_CTRL_PHY_RDY_MSK)) {
		pr_debug("%s irq_value = %x not set enable bit", __func__, irq_value);
		hisi_sas_phy_write32(hisi_hba, phy, CHL_INT2_REG, CHL_INT2_REG_CTRL_PHY_RDY_MSK);
		return IRQ_NONE;
	}
	else
		pr_debug("%s phy = %d, irq_value = %x in phy_ctrlrdy\n", __func__, phy, irq_value);

	hisi_sas_config_serdes_12G(hisi_hba, phy);

	hisi_sas_phy_write32(hisi_hba, phy, CHL_INT2_REG, CHL_INT2_REG_CTRL_PHY_RDY_MSK);

	return IRQ_HANDLED;
}


static irqreturn_t hisi_sas_int_dmaerr(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_DMA_RESP_ERR_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in dma_resp_err\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_REG_DMA_RESP_ERR_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_hotplug(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_PHY_HP_TOUT_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in hotplug_tout\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_REG_PHY_HP_TOUT_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_bcast(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_SL_RX_BC_ACK_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in sl_rx_bcast_ack\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_REG_SL_RX_BC_ACK_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_oobrst(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_REG_OOB_RESTART_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phyctrl_oob_restart_ci\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_REG_OOB_RESTART_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_hardrst(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_SL_RX_HARDRST_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in sl_rx_hardrst\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_SL_RX_HARDRST_MSK);

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_statuscg(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);

	if (!(irq_value & CHL_INT2_PHY_STATUS_CHG_MSK))
		pr_err("%s irq_value = %x not set enable bit", __func__, irq_value);

	pr_info("%s phy = %d, irq_value = %x in phyctrl_status_chg\n", __func__, phy_no, irq_value);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_REG, CHL_INT2_PHY_STATUS_CHG_MSK);

	return IRQ_HANDLED;
}

static void hisi_sas_phy_down(struct hisi_hba *hisi_hba, int phy_no)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha = hisi_hba->sas;
	u32 irq_value;

	/* j00310691 fixme maybe we can't trust this register */
	u32 phy_state = hisi_sas_read32(hisi_hba, PHY_STATE_REG); //guojian quotes reg@0x30
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_REG);
	pr_info("%s phy%d phy_state=0x%x irq_value=0x%x\n", __func__, phy_no, phy_state, irq_value);

	if (irq_value & CHL_INT2_REG_CTRL_PHY_RDY_MSK) {
		/* Phy down but ready */
		pr_debug("%s phy %d down and ready\n", __func__, phy_no);
		hisi_sas_update_phyinfo(hisi_hba, phy_no, 0);
		hisi_sas_bytes_dmaed(hisi_hba, phy_no);
		hisi_sas_port_notify_formed(sas_phy, 0);
		pr_info("phy%d Attached Device\n", phy_no);
	} else {
		/* Phy down and not ready */
		pr_info("phy%d Removed Device\n", phy_no);
		sas_phy_disconnected(sas_phy);
		hisi_sas_phy_disconnected(phy);
		sas_ha->notify_phy_event(sas_phy, PHYE_LOSS_OF_SIGNAL);
	}
}

static irqreturn_t hisi_sas_int_abnormal(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;
	u32 irq_mask_old;

	/* mask_int0 */
	irq_mask_old = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_MSK_REG);
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, 0x003FFFFF);

	/* read int0 */
	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT0_REG);

	if (irq_value & CHL_INT0_REG_PHYCTRL_NOTRDY_MSK) {
		u32 val = hisi_sas_phy_read32(hisi_hba, phy_no, PHY_CFG_REG);

		if (val & PHY_CFG_REG_ENA_MSK) {
			/* Enabled */
			/* Stop serdes fw timer */
			/* serdes lane reset */
			/* todo */

			hisi_sas_phy_down(hisi_hba, phy_no);
		} else {
			/* Disabled */
			/* Ignore phydown event if disabled */
			pr_warn("%s phy = %d phydown event and already disabled\n", __func__, phy_no);
		}

	} else if (irq_value & CHL_INT0_REG_ID_TIMEOUT_MSK) {
		pr_info("%s phy = %d identify timeout todo\n", __func__, phy_no);
	} else {
		if (irq_value & CHL_INT0_REG_DWS_LOST_MSK) {
			pr_info("%s phy = %d dws lost\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_REG_SN_FAIL_NGR_MSK) {
			pr_info("%s phy = %d sn fail ngr\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_REG_SL_IDAF_FAIL_MSK ||
			irq_value & CHL_INT0_REG_SL_OPAF_FAIL_MSK) {
			pr_info("%s phy = %d check address frame err\n", __func__, phy_no);
		}

		if (irq_value & CHL_INT0_REG_SL_PS_FAIL_OFF) {
			pr_debug("%s phy = %d ps req fail\n", __func__, phy_no);
		}
	}

	/* write to zero */
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_REG, irq_value);

	if (irq_value & CHL_INT0_REG_PHYCTRL_NOTRDY_MSK) {
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, 0x003FFFFF & ~1);
	} else {
		hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0_MSK_REG, irq_mask_old);
	}

	return IRQ_HANDLED;
}

static irqreturn_t hisi_sas_int_int1(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_value;

	irq_value = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT1_REG);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT1_REG, irq_value);

	return IRQ_HANDLED;
}

/* Interrupts */
irqreturn_t hisi_sas_cq_interrupt(int queue, void *p)
{
	struct hisi_hba *hisi_hba = p;
	struct hisi_sas_slot *slot;
	struct hisi_sas_complete_hdr *complete_queue = hisi_hba->complete_hdr[queue];
	u32 irq_value;
	u32 rd_point, wr_point;

	irq_value = hisi_sas_read32(hisi_hba, OQ_INT_SRC_REG);

	hisi_sas_write32(hisi_hba, OQ_INT_SRC_REG, 1 << queue);

	rd_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_RD_PTR + 20 * queue);
	wr_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_WR_PTR + 20 * queue);

	while (rd_point != wr_point) {
		struct hisi_sas_complete_hdr *complete_hdr;
		int iptt;

		complete_hdr = &complete_queue[rd_point];
		iptt = complete_hdr->iptt;

		slot = &hisi_hba->slot_info[iptt];

		hisi_sas_slot_complete(hisi_hba, slot, 0);

		if (++rd_point >= HISI_SAS_QUEUE_SLOTS)
			rd_point = 0;
	}

	/* update rd_point */
	hisi_sas_write32(hisi_hba, COMPL_Q_0_RD_PTR + 20 * queue, rd_point);
	return IRQ_HANDLED;
}

irqreturn_t hisi_sas_fatal_ecc_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;

	pr_info("%s core = %d, irq = %d\n", __func__, hisi_hba->id, irq);

	return IRQ_HANDLED;
}

irqreturn_t hisi_sas_fatal_axi_int(int irq, void *p)
{
	struct hisi_hba *hisi_hba = p;

	pr_info("%s core = %d, irq = %d\n", __func__, hisi_hba->id, irq);

	return IRQ_HANDLED;
}

#define DECLARE_INT_HANDLER(handler, idx)\
irqreturn_t handler##idx(int irq, void *p)\
{\
	return	handler(idx, p);\
}

#define INT_HANDLER_NAME(handler, idx)\
	handler##idx

#define DECLARE_PHY_INT_HANDLER_GROUP(phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_ctrlrdy, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_dmaerr, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_hotplug, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_bcast, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_oobrst, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_hardrst, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_statuscg, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_phyup, phy)\
	DECLARE_INT_HANDLER(hisi_sas_int_abnormal, phy)\
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
	INT_HANDLER_NAME(hisi_sas_int_abnormal, phy),\
	INT_HANDLER_NAME(hisi_sas_int_int1, phy),

DECLARE_PHY_INT_HANDLER_GROUP(0)
DECLARE_PHY_INT_HANDLER_GROUP(1)
DECLARE_PHY_INT_HANDLER_GROUP(2)
DECLARE_PHY_INT_HANDLER_GROUP(3)
DECLARE_PHY_INT_HANDLER_GROUP(4)
DECLARE_PHY_INT_HANDLER_GROUP(5)
DECLARE_PHY_INT_HANDLER_GROUP(6)
DECLARE_PHY_INT_HANDLER_GROUP(7)
DECLARE_PHY_INT_HANDLER_GROUP(8)

static const char phy_int_names[HISI_SAS_PHY_INT_NR][32] = {
	{"CTRL Rdy"},
	{"DMA Err"},
	{"HotPlug"},
	{"Bcast"},
	{"OOBRst"},
	{"HardRst"},
	{"StatusCG"},
	{"Phy Up"},
	{"Abnormal"},
	{"Int1"}
};

static const char cq_int_name[32] = "cq";
static const char fatal_int_name[HISI_SAS_FATAL_INT_NR][32] = {
	"fatal ecc",
	"fatal axi"
};

irq_handler_t phy_interrupt_handlers[HISI_SAS_MAX_PHYS][HISI_SAS_PHY_INT_NR] = {
	{DECLARE_PHY_INT_GROUP_PTR(0)},
	{DECLARE_PHY_INT_GROUP_PTR(1)},
	{DECLARE_PHY_INT_GROUP_PTR(2)},
	{DECLARE_PHY_INT_GROUP_PTR(3)},
	{DECLARE_PHY_INT_GROUP_PTR(4)},
	{DECLARE_PHY_INT_GROUP_PTR(5)},
	{DECLARE_PHY_INT_GROUP_PTR(6)},
	{DECLARE_PHY_INT_GROUP_PTR(7)},
	{DECLARE_PHY_INT_GROUP_PTR(8)},
};

DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 0)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 1)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 2)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 3)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 4)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 5)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 6)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 7)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 8)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 9)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 10)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 11)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 12)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 13)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 14)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 15)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 16)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 17)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 18)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 19)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 20)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 21)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 22)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 23)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 24)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 25)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 26)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 27)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 28)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 29)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 30)
DECLARE_INT_HANDLER(hisi_sas_cq_interrupt, 31)

irq_handler_t cq_interrupt_handlers[HISI_SAS_MAX_QUEUES] = {
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 0),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 1),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 2),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 3),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 4),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 5),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 6),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 7),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 8),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 9),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 10),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 11),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 12),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 13),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 14),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 15),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 16),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 17),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 18),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 19),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 20),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 21),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 22),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 23),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 24),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 25),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 26),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 27),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 28),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 29),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 30),
	INT_HANDLER_NAME(hisi_sas_cq_interrupt, 31)
};

irq_handler_t fatal_interrupt_handlers[HISI_SAS_MAX_QUEUES] = {
	hisi_sas_fatal_ecc_int,
	hisi_sas_fatal_axi_int
};

int hisi_sas_interrupt_init(struct hisi_hba *hisi_hba)
{
	int i, j, irq, rc, id = hisi_hba->id;

	if (!hisi_hba->np)
		return -ENOENT;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		for (j = 0; j < HISI_SAS_PHY_INT_NR; j++) {
			int idx = (i * HISI_SAS_PHY_INT_NR) + j;

			irq = irq_of_parse_and_map(hisi_hba->np, idx);
			if (!irq) {
				pr_err("%s [%d] could not map phy interrupt %d\n", __func__, hisi_hba->id, idx);
				return -ENOENT;
			}
			(void)snprintf(hisi_hba->int_names[idx], 32, DRV_NAME" %s [%d %d]", phy_int_names[j],  id, i);
			rc = request_irq(irq, phy_interrupt_handlers[i][j], 0, hisi_hba->int_names[idx], hisi_hba);
			if (rc) {
				pr_err("%s [%d] could not request interrupt %d, rc=%d\n", __func__, hisi_hba->id, irq, rc);
				return -ENOENT;
			}
		}
	}

	for (i = 0; i < hisi_hba->queue_count; i++) {
		int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) + i;

		irq = irq_of_parse_and_map(hisi_hba->np, idx);
		if (!irq) {
			pr_err("%s [%d] could not map cq interrupt %d\n", __func__, hisi_hba->id, idx);
			return -ENOENT;
		}
		(void)snprintf(hisi_hba->int_names[idx], 32, DRV_NAME" %s [%d %d]", cq_int_name,  id, i);
		rc = request_irq(irq, cq_interrupt_handlers[i], 0, hisi_hba->int_names[idx], hisi_hba);
		if (rc) {
			pr_err("%s [%d] could not request interrupt %d, rc=%d\n", __func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
		idx++;
	}

	for (i = 0; i < HISI_SAS_FATAL_INT_NR; i++) {
		int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) + hisi_hba->queue_count + i;

		irq = irq_of_parse_and_map(hisi_hba->np, idx);
		if (!irq) {
			pr_err("%s [%d] could not map fatal interrupt %d\n", __func__, hisi_hba->id, idx);
			return -ENOENT;
		}
		(void)snprintf(hisi_hba->int_names[idx], 32, DRV_NAME" %s [%d]", fatal_int_name[i], id);
		rc = request_irq(irq, fatal_interrupt_handlers[i], 0, hisi_hba->int_names[idx], hisi_hba);
		if (rc) {
			pr_err("%s [%d] could not request interrupt %d, rc=%d\n", __func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
		idx++;
	}

	return 0;
}

int hisi_sas_interrupt_openall(struct hisi_hba *hisi_hba)
{
	int i;
	u32 val;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/* Clear interrupt status */
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT0_REG);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_REG, val);
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT1_REG);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_REG, val);
		val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT2_REG);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_REG, val);

		/* Unmask interrupt */
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK_REG, 0x003ce3ee);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK_REG, 0x00017fff);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK_REG, 0x0000032a);

		/* bypass chip bug mask abnormal intr */
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK_REG, 0x003fffff & ~1);
	}

	return 0;
}

void hisi_sas_phys_up(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK_REG, 0x36a);
		hisi_sas_phy_read32(hisi_hba, i, CHL_INT2_MSK_REG);
	}
}

static void hisi_sas_config_phy_link_param(struct hisi_hba *hisi_hba, int phy, enum sas_linkrate linkrate)
{
	u32 rate = hisi_sas_phy_read32(hisi_hba, phy, PROG_PHY_LINK_RATE_REG);
	u32 pcn;

	rate &= ~PROG_PHY_LINK_RATE_REG_MAX_MSK;
	switch (linkrate) {
	case SAS_LINK_RATE_12_0_GBPS:
		rate |= SAS_LINK_RATE_6_0_GBPS << PROG_PHY_LINK_RATE_REG_MAX_OFF;
		pcn = 0x80aa0001;
		break;

	default:
		pr_warn("%s unsupported linkrate, %d", __func__, linkrate);
		return;
	}

	rate &= ~PROG_PHY_LINK_RATE_REG_OOB_MSK;
	rate |= SAS_LINK_RATE_1_5_GBPS << PROG_PHY_LINK_RATE_REG_OOB_OFF;
	rate &= ~PROG_PHY_LINK_RATE_REG_MIN_MSK;
	rate |= SAS_LINK_RATE_1_5_GBPS << PROG_PHY_LINK_RATE_REG_MIN_OFF;
	hisi_sas_phy_write32(hisi_hba, phy, PROG_PHY_LINK_RATE_REG, rate);
	hisi_sas_phy_write32(hisi_hba, phy, PHY_PCN_REG, pcn);
}

static void hisi_sas_config_phy_opt_mode(struct hisi_hba *hisi_hba, int phy)
{
	/* j00310691 assume not optical cable for now */
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG_REG);
	cfg &= ~PHY_CFG_REG_DC_OPT_MSK;
	cfg |= 1 << PHY_CFG_REG_DC_OPT_OFF;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG_REG, cfg);
}

static void hisi_sas_config_tx_tfe_autoneg(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CONFIG2_REG);
	cfg &= ~PHY_CONFIG2_REG_RXCLTEPRES_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CONFIG2_REG, cfg);
}

static void hisi_sas_enable_phy(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG_REG);
	cfg |= PHY_CFG_REG_ENA_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG_REG, cfg);
}

static void hisi_sas_start_phy(struct hisi_hba *hisi_hba, int phy)
{
	hisi_sas_config_id_frame(hisi_hba, phy);
	hisi_sas_config_phy_link_param(hisi_hba, phy, SAS_LINK_RATE_12_0_GBPS);
	hisi_sas_config_phy_opt_mode(hisi_hba, phy);
	hisi_sas_config_tx_tfe_autoneg(hisi_hba, phy);
	hisi_sas_enable_phy(hisi_hba, phy);
}

static void hisi_sas_start_phys(unsigned long data)
{
	struct hisi_hba *hisi_hba = (struct hisi_hba *)data;
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK_REG, 0x0000032a);
		hisi_sas_start_phy(hisi_hba, i);
	}

}

int hisi_sas_start_phy_layer(struct hisi_hba *hisi_hba)
{
	struct timer_list *timer = NULL;

	timer = vmalloc(sizeof(*timer)); /* j00310691 memory leak? Is this timer even needed? */
	if (!timer)
		return -ENOMEM;

	init_timer(timer);
	timer->data = (unsigned long)hisi_hba;
	timer->expires = jiffies + msecs_to_jiffies(1000);
	timer->function = hisi_sas_start_phys;

	add_timer(timer);

	return 0;
}

/*****************************************************
					for debug fs
*****************************************************/
static struct dentry *dbg_dir;

#define FILE_PATH           "/tmp"
#define DBG_BUF_SIZE        (128)
#define MAX_DBG_FILE_LEN    (256)
#define MAX_CACHED_IOST     (64)
#define MAX_PARA_NUM        (16)
#define ERR                 (-1)

#define HISI_SAS_IOST_IPTT_MASK     (0x1fff)
#define HISI_SAS_IOST_IO_VAL_MASK   (0x80000)
#define HISI_SAS_IOST_IO_VAL_OFF    (15)
#define HISI_SAS_IOST_IO_STA_MASK   (0xff0000)
#define HISI_SAS_IOST_IO_STA_OFF    (16)

struct hisi_sas_debugfs_data {
	struct hisi_hba *hisi_hba;
	char *pFileFullName[HISI_SAS_MAX_QUEUES];
	mm_segment_t OldFs[HISI_SAS_MAX_QUEUES];
	struct file *FileRt[HISI_SAS_MAX_QUEUES];
};

int str_to_ll(char *buf, long long *val, unsigned int *num)
{
	int i;
	int rc;
	char *p = NULL;
	char delim[] = ",";
	int idx = 0;
	long long attr[MAX_PARA_NUM] = {0};

	p = strsep(&buf, delim);
	while (p) {
		rc = kstrtoll(p, 0, &attr[idx++]);
		if (rc)
			return -EINVAL;

		if (idx > MAX_PARA_NUM) {
			pr_err("paramenter too much.idx = %d, limit %d\n",
					idx, MAX_PARA_NUM);
			return -EINVAL;
		}
		p = strsep(&buf, delim);
	}

	for (i = 0; i < idx; i++)
		*(val + i) = attr[i];

	*num = idx;
	return 0;
}

int hisi_sas_dump_single_que(
	u32 Que,
	struct file *FileRt,
	void *BaseAddr,
	u32 Num,
	u32 size)
{
	char BufWr[DBG_BUF_SIZE] = {0};
	u32 EleNo = 0;
	u32 loop = 0;
	int Ret = 0;
	u32 *MemAddr = NULL;

	/* dump header */
	snprintf(BufWr,
			sizeof(BufWr),
			"Queue %02d: BaseAddr=0x%p, EntryNum=%d, EntrySize=%d\n",
			Que,
			(void *)BaseAddr,
			Num,
			size
			);

	Ret = FileRt->f_op->write(FileRt,
				BufWr, strlen(BufWr), &FileRt->f_pos);
	if (Ret < 0) {
		pr_err("write queue info fail.\n");
		return -ENOMEM;
	}

	/* dump the queue entry one by one*/
	for (EleNo = 0; EleNo < Num; EleNo++) {
		/* write the header of the queue entry*/
		snprintf(BufWr,	sizeof(BufWr), "[0x%03x] : {[",	EleNo);
		Ret = FileRt->f_op->write(FileRt,
				BufWr, strlen(BufWr), &FileRt->f_pos);
		if (Ret < 0) {
			pr_err("write queue header fail.\n");
			return -ENOMEM;
		}
		MemAddr = (u32 *)(u64)((u8 *)BaseAddr + EleNo * size);

		/* write the whole entry*/
		for (loop = 0; loop < size; loop += sizeof(u32)) {

			snprintf(BufWr,	sizeof(BufWr), " 0x%08x,", *MemAddr);
			Ret = FileRt->f_op->write(FileRt,
					BufWr, strlen(BufWr), &FileRt->f_pos);
			if (Ret < 0) {
				pr_err("write queue entry fail.\n");
				return -ENOMEM;
			}
			MemAddr++;
		}

		/* write the tail of the queue entry*/
		snprintf(BufWr, sizeof(BufWr), "]}\n");
		Ret = FileRt->f_op->write(FileRt,
				BufWr, strlen(BufWr), &FileRt->f_pos);
		if (Ret < 0) {
			pr_err("write queue info fail.\n");
			return -ENOMEM;
		}
	}
	return Ret;
}

static int hisi_sas_dq_open(struct inode *inode, struct file *file)
{
	int que_idx;
	u32 SubSys = 0;
	char Fname[MAX_DBG_FILE_LEN] = {0};
	char *dump_file = "dq";
	struct file *Fp = NULL;
	struct hisi_sas_debugfs_data *hsdd;
	struct hisi_hba *hisi_hba;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);

	if (inode->i_private) {
		hsdd = inode->i_private;
		hisi_hba = hsdd->hisi_hba;
		SubSys = hisi_hba->id;
	}

	for (que_idx = 0; que_idx < HISI_SAS_MAX_QUEUES; que_idx++) {
		/*open file*/
		snprintf(Fname,
				sizeof(Fname),
				"%s/c%d-%s-%02d.txt",
				FILE_PATH,
				SubSys,
				dump_file,
				que_idx);
		hsdd->pFileFullName[que_idx] = Fname;

		Fp = filp_open(Fname, (O_WRONLY | O_CREAT), 0777);
		if (IS_ERR(Fp)) {
			pr_err("open file fail.\n");
			return -ENOMEM;
		}

		hsdd->OldFs[que_idx] = get_fs();
		set_fs(KERNEL_DS);
		hsdd->FileRt[que_idx] = Fp;
	}
	file->private_data = hsdd;
	return 0;
}

static ssize_t hisi_sas_dq_read(struct file *file,
		char __user *buf, size_t count, loff_t *ppos)
{
	int i;
	int ret = 0;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	u32 MaxDqNum = HISI_SAS_MAX_QUEUES;
	void *DqBaseAddr = 0;
	u32 DqSize = sizeof(struct hisi_sas_cmd_hdr);
	u32 DqDepth = 0;

	/*dump all of the DQ*/
	for (i = 0; i < MaxDqNum; i++) {
		DqBaseAddr = hisi_hba->cmd_hdr[i];
		DqDepth = hisi_sas_read32(hisi_hba,
				DLVRY_Q_0_WR_PTR + (i * 0x14));
		pr_info("i = %d, BA = 0x%p, size = 0x%08x, dep = 0x%08x.\n",
				i, DqBaseAddr, DqSize, DqDepth);

		ret = hisi_sas_dump_single_que(i, hsdd->FileRt[i],
				DqBaseAddr, DqDepth, DqSize);
		if (ret < 0) {
			pr_err("dump DQ %d file fail.\n", i);
			return ret;
		}
	}

	return 0;
}

static ssize_t hisi_sas_dq_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos)
{
	u32 que_idx;
	u32 entry_num;
	int rt = 0;
	u32 num = 0;
	char info[MAX_DBG_FILE_LEN] = {0};
	unsigned long long val[MAX_PARA_NUM] = {0};
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct hisi_sas_cmd_hdr *dq = NULL;
	u32 MaxDqNum = HISI_SAS_MAX_QUEUES;
	u32 DqSize = sizeof(struct hisi_sas_cmd_hdr);
	void *DqBaseAddr = 0;
	u32 DqDepth = 0;

	/*copy data from user, check the dq index.*/
	rt = copy_from_user(info, buf, count);
	if (rt) {
		pr_err("copy_from_user fail.\n");
		return -ENOMEM;
	}

	/* separate input para*/
	rt = str_to_ll(info, val, &num);
	if (rt) {
		pr_err("separate input paramenter fail.");
		return -EINVAL;
	}
	que_idx = val[0];
	entry_num = val[1];
	pr_info("que_idx = %d, entry_num = %d\n", que_idx, entry_num);

	/* get basic info of delivery queue*/
	DqBaseAddr = hisi_hba->cmd_hdr[que_idx];
	DqDepth = hisi_sas_read32(hisi_hba,
			DLVRY_Q_0_WR_PTR + (que_idx * 0x14));
	pr_info("queue = %d, BA = 0x%p, size = 0x%08x, dep = 0x%08x.\n",
			que_idx, DqBaseAddr, DqSize, DqDepth);

	if (que_idx > MaxDqNum) {
		pr_err("que idx err.\n");
		return -EINVAL;
	}

	if (entry_num > DqDepth) {
		pr_err("dq num err.\n");
		return -EINVAL;
	}

	/* dump the dq info*/
	dq = (struct hisi_sas_cmd_hdr *)
		(u64)(DqBaseAddr + entry_num * DqSize);
	pr_info("***** delivery queue %d entry %d show below:*****\n",
			que_idx, entry_num);
	pr_info("DW0 : abort_flag = 0x%x.\n", dq->abort_flag);
	pr_info("      t10_flds_pres = 0x%x.\n", dq->t10_flds_pres);
	pr_info("      resp_report = 0x%x.\n", dq->resp_report);
	pr_info("      tlr_ctrl = 0x%x.\n", dq->tlr_ctrl);
	pr_info("      phy_id = 0x%x.\n", dq->phy_id);
	pr_info("      force_phy = 0x%x.\n", dq->force_phy);
	pr_info("      port = 0x%x.\n", dq->port);
	pr_info("      sata_reg_set = 0x%x.\n", dq->sata_reg_set);
	pr_info("      priority = 0x%x.\n", dq->priority);
	pr_info("      mode = 0x%x.\n", dq->mode);
	pr_info("      cmd = 0x%x.\n\n", dq->cmd);

	pr_info("DW1 : port_multiplier = 0x%x.\n",
			dq->port_multiplier);
	pr_info("      bist_activate = 0x%x.\n", dq->bist_activate);
	pr_info("      atapi = 0x%x.\n", dq->atapi);
	pr_info("      first_part_dma = 0x%x.\n", dq->first_part_dma);
	pr_info("      reset = 0x%x.\n", dq->reset);
	pr_info("      pir_pres = 0x%x.\n", dq->pir_pres);
	pr_info("      enable_tlr = 0x%x.\n", dq->enable_tlr);
	pr_info("      verify_dtl = 0x%x.\n", dq->verify_dtl);
	pr_info("      ssp_pass_through = 0x%x.\n",
			dq->ssp_pass_through);
	pr_info("      ssp_frame_type = 0x%x.\n", dq->ssp_frame_type);
	pr_info("      device_id = 0x%x.\n\n", dq->device_id);

	pr_info("DW2 : cmd_frame_len = 0x%x.\n", dq->cmd_frame_len);
	pr_info("      leave_affil_open = 0x%x.\n",
			dq->leave_affil_open);
	pr_info("      max_resp_frame_len = 0x%x.\n",
			dq->max_resp_frame_len);
	pr_info("      sg_mode = 0x%x.\n", dq->sg_mode);
	pr_info("      first_burst = 0x%x.\n\n", dq->first_burst);

	pr_info("DW3 : iptt = 0x%x.\n", dq->iptt);
	pr_info("      iptt = 0x%x.\n\n", dq->iptt);

	pr_info("DW4 : data_transfer_len = 0x%x.\n\n",
			dq->data_transfer_len);

	pr_info("DW5 : first_burst_num = 0x%x.\n\n", dq->first_burst_num);

	pr_info("DW6 : dif_sg_len = 0x%x.\n", dq->dif_sg_len);
	pr_info("      data_sg_len = 0x%x.\n\n", dq->data_sg_len);

	pr_info("DW7 : double_mode = 0x%x.\n", dq->double_mode);
	pr_info("      abort_iptt = 0x%x.\n\n", dq->abort_iptt);

	pr_info("DW8 : cmd_table_addr_lo = 0x%x.\n\n",
			dq->cmd_table_addr_lo);

	pr_info("DW9 : cmd_table_addr_hi = 0x%x.\n\n",
			dq->cmd_table_addr_hi);

	pr_info("DW10: sts_buffer_addr_lo = 0x%x.\n\n",
			dq->sts_buffer_addr_lo);

	pr_info("DW11: sts_buffer_addr_hi = 0x%x.\n\n",
			dq->sts_buffer_addr_hi);

	pr_info("DW12: prd_table_addr_lo = 0x%x.\n\n",
			dq->prd_table_addr_lo);

	pr_info("DW13: prd_table_addr_hi = 0x%x.\n\n",
			dq->prd_table_addr_hi);

	pr_info("DW14: dif_prd_table_addr_lo = 0x%x.\n\n",
			dq->dif_prd_table_addr_lo);

	pr_info("DW15: dif_prd_table_addr_hi = 0x%x.\n\n",
			dq->dif_prd_table_addr_hi);

	pr_info("**************************************************\n");

	return count;
}

static int hisi_sas_dq_release(struct inode *inode, struct file *file)
{
	int que_idx;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;

	for (que_idx = 0; que_idx < HISI_SAS_MAX_QUEUES; que_idx++) {
		set_fs(hsdd->OldFs[que_idx]);
		filp_close(hsdd->FileRt[que_idx], NULL);
	}
	return 0;
}

static const struct file_operations dq_operations = {
	.owner   = THIS_MODULE,
	.open    = hisi_sas_dq_open,
	.read    = hisi_sas_dq_read,
	.write   = hisi_sas_dq_write,
	.llseek  = NULL,
	.release = hisi_sas_dq_release,
};

static int hisi_sas_cq_open(struct inode *inode, struct file *file)
{
	int i;
	u32 SubSys = 0;
	char Fname[MAX_DBG_FILE_LEN] = {0};
	struct file *Fp = NULL;
	char *dump_file = "cq";
	struct hisi_sas_debugfs_data *hsdd;
	struct hisi_hba *hisi_hba;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);

	if (inode->i_private) {
		hsdd = inode->i_private;
		hisi_hba = hsdd->hisi_hba;
		SubSys = hisi_hba->id;
	}

	for (i = 0; i < HISI_SAS_MAX_QUEUES; i++) {
		/*open file*/
		snprintf(Fname,
				sizeof(Fname),
				"%s/c%d-%s-%02d.txt",
				FILE_PATH,
				SubSys,
				dump_file,
				i);

		hsdd->pFileFullName[i] = Fname;
		Fp = filp_open(Fname, (O_WRONLY | O_CREAT), 777);
		if (IS_ERR(Fp)) {
			pr_err("open file fail.\n");
			return -1;
		}

		hsdd->OldFs[i] = get_fs();
		set_fs(KERNEL_DS);
		hsdd->FileRt[i] = Fp;
	}
	file->private_data = hsdd;

	return 0;
}

static ssize_t hisi_sas_cq_read(struct file *file,
		char __user *buf, size_t count, loff_t *ppos)
{
	int i;
	int ret = 0;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	u32 MaxCqNum = HISI_SAS_MAX_QUEUES;
	u32 CqSize = sizeof(struct hisi_sas_complete_hdr);
	void *CqBaseAddr = 0;
	u32 CqDepth = 0;

	/*dump all of the CQ*/
	for (i = 0; i < MaxCqNum; i++) {
		CqBaseAddr = hisi_hba->complete_hdr[i];
		CqDepth = hisi_sas_read32(hisi_hba,
				COMPL_Q_0_WR_PTR + (i * 20));
		pr_info("i = %d, BA = 0x%p, size = 0x%08x, dep = 0x%08x.\n",
				i, CqBaseAddr, CqSize, CqDepth);

		ret = hisi_sas_dump_single_que(i,
				hsdd->FileRt[i], CqBaseAddr, CqDepth, CqSize);
		if (ret < 0) {
			pr_err("dump CQ %d file fail. ret=%d\n", i, ret);
			return ret;
		}
	}
	return 0;
}

static ssize_t hisi_sas_cq_write(struct file *file,
		const char __user *buf,	size_t count, loff_t *ppos)
{
	u32 que_idx;
	u32 entry_num;
	int rt = 0;
	u32 num = 0;
	char info[MAX_DBG_FILE_LEN] = {0};
	unsigned long long val[MAX_PARA_NUM] = {0};
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct hisi_sas_complete_hdr *cq = NULL;
	u32 MaxCqNum = HISI_SAS_MAX_QUEUES;
	u32 CqSize = sizeof(struct hisi_sas_complete_hdr);
	void *CqBaseAddr = 0;
	u32 CqDepth = 0;

	/*copy data from user*/
	rt = copy_from_user(info, buf, count);
	if (rt) {
		pr_err("copy_from_user fail.\n");
		return -ENOMEM;
	}

	/* separate input para*/
	rt = str_to_ll(info, val, &num);
	if (rt) {
		pr_err("seprate input paramenter fail.");
		return -EINVAL;
	}
	que_idx = val[0];
	entry_num = val[1];
	pr_info("que_idx = %d, entry_num = %d\n", que_idx, entry_num);

	/* get basic info of complete queue*/
	CqBaseAddr = hisi_hba->complete_hdr[que_idx];
	CqDepth = hisi_sas_read32(hisi_hba, COMPL_Q_0_WR_PTR + (que_idx * 20));
	pr_info("queue = %d, BA = 0x%p, size = 0x%08x, dep = 0x%08x.\n",
			que_idx, CqBaseAddr, CqSize, CqDepth);

	if (que_idx > MaxCqNum) {
		pr_err("que idx err.\n");
		return -EINVAL;
	}

	if (entry_num > CqDepth) {
		pr_err("cq num err.\n");
		return -EINVAL;
	}

	/* dump the cq info*/
	cq = (struct hisi_sas_complete_hdr *)
		(u64)(CqBaseAddr + entry_num * CqSize);
	pr_info("***** complete queue %d entry %d show below:*****\n",
			que_idx, entry_num);
	pr_info("DW0 : iptt = 0x%x.\n", cq->iptt);
	pr_info("      cmd_complt = 0x%x.\n", cq->cmd_complt);
	pr_info("      err_rcrd_xfrd = 0x%x.\n", cq->err_rcrd_xfrd);
	pr_info("      rspns_xfrd = 0x%x.\n", cq->rspns_xfrd);
	pr_info("      attention = 0x%x.\n", cq->attention);
	pr_info("      cmd_rcvd = 0x%x.\n", cq->cmd_rcvd);
	pr_info("      slot_rst_cmplt = 0x%x.\n", cq->slot_rst_cmplt);
	pr_info("      rspns_good = 0x%x.\n", cq->rspns_good);
	pr_info("      abort_status = 0x%x.\n", cq->abort_status);
	pr_info("      io_cfg_err = 0x%x.\n", cq->io_cfg_err);
	pr_info("**************************************************\n");
	return count;
}

static int hisi_sas_cq_release(struct inode *inode, struct file *file)
{
	int i;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;

	for (i = 0; i < HISI_SAS_MAX_QUEUES; i++) {
		set_fs(hsdd->OldFs[i]);
		filp_close(hsdd->FileRt[i], NULL);
	}
	return 0;
}

static const struct file_operations cq_operations = {
	.open    = hisi_sas_cq_open,
	.read    = hisi_sas_cq_read,
	.write   = hisi_sas_cq_write,
	.llseek  = NULL,
	.release = hisi_sas_cq_release,
};

static int hisi_sas_itct_open(struct inode *inode, struct file *file)
{
	u32 SubSys = 0;
	char Fname[MAX_DBG_FILE_LEN] = {0};
	struct file *Fp = NULL;
	char *dump_file = "itct";
	struct hisi_sas_debugfs_data *hsdd;
	struct hisi_hba *hisi_hba;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);

	if (inode->i_private) {
		hsdd = inode->i_private;
		hisi_hba = hsdd->hisi_hba;
		SubSys = hisi_hba->id;
	}

	/*open file*/
	snprintf(Fname,
			sizeof(Fname),
			"%s/c%d-%s.txt",
			FILE_PATH,
			SubSys,
			dump_file);
	hsdd->pFileFullName[0] = Fname;

	Fp = filp_open(Fname, (O_WRONLY | O_CREAT),  777);
	if (IS_ERR(Fp)) {
		pr_err("open itct file fail.\n");
		return -ENOMEM;
	}

	hsdd->OldFs[0] = get_fs();
	set_fs(KERNEL_DS);
	hsdd->FileRt[0] = Fp;

	file->private_data = hsdd;

	return 0;
}

static ssize_t hisi_sas_itct_read(struct file *file,
		char __user *buf, size_t count, loff_t *ppos)
{
	int i, j;
	int Ret = 0;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct file *Fp = hsdd->FileRt[0];
	void *ItctBaseAddr = hisi_hba->itct;
	u32 ItctSize = sizeof(struct hisi_sas_itct);
	u32 ItctDepth = HISI_SAS_MAX_ITCT_ENTRIES;
	char BufWr[DBG_BUF_SIZE] = {0};
	u32 *MemAddr = NULL;

	pr_info("BA = 0x%p, size = 0x%08x, itct num = 0x%08x.\n",
			ItctBaseAddr, ItctSize, ItctDepth);

#ifdef DUMP_BY_STRUCT
	/* loop for itct dump*/
	for (i = 0; i < HISI_SAS_MAX_ITCT_ENTRIES; i++) {
		MemAddr = (void *)&(((struct hisi_sas_itct *)ItctBaseAddr)[i]);
		Ret = Fp->f_op->write(Fp, MemAddr, ItctSize, &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write ITCT file fail.\n");
			return -ENOMEM;
		}
	}
#else
	/* write the itct table, the whole num is 4096 */
	for (i = 0; i < HISI_SAS_MAX_ITCT_ENTRIES; i++) {
		snprintf(BufWr,	sizeof(BufWr), "[0x%03x] : {[",	i);
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write ITCT header fail.\n");
			return -ENOMEM;
		}
		MemAddr = (u32 *)(u64)((u8 *)ItctBaseAddr + i * ItctSize);

		/* write the whole itct, by word. totally 16 words. */
		for (j = 0; j < ItctSize; j += sizeof(u32)) {

			snprintf(BufWr,	sizeof(BufWr), " 0x%08x,", *MemAddr);
			Ret = Fp->f_op->write(Fp,
					BufWr, strlen(BufWr), &Fp->f_pos);
			if (Ret < 0) {
				pr_err("write ITCT entry fail.\n");
				return -ENOMEM;
			}
			MemAddr++;
		}

		/* write the tail of the queue entry*/
		snprintf(BufWr, sizeof(BufWr), "]}\n");
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write ITCT info fail.\n");
			return -ENOMEM;
		}
	}
#endif
	return 0;
}

static ssize_t hisi_sas_itct_write(struct file *file,
		const char __user *buf,	size_t count, loff_t *ppos)
{
	int rt, i;
	u32 num;
	char info[MAX_DBG_FILE_LEN] = {0};
	unsigned long long val[MAX_PARA_NUM] = {0};
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct hisi_sas_itct *itct = NULL;
	void *ItctBaseAddr = hisi_hba->itct;
	u32 ItctSize = sizeof(struct hisi_sas_itct);
	u32 ItctIdx = 0;

	/*copy data from user, check the itct index.*/
	rt = copy_from_user(info, buf, count);
	if (rt) {
		pr_err("copy_from_user fail.\n");
		return -ENOMEM;
	}

	/* separate input para*/
	rt = str_to_ll(info, val, &num);
	if (rt) {
		pr_err("separate input paramenter fail.");
		return -EINVAL;
	}
	ItctIdx = val[0];
	pr_info("itct_idx = %d\n", ItctIdx);

	if (ItctIdx < HISI_SAS_MAX_ITCT_ENTRIES) {
		itct = (struct hisi_sas_itct *)
				(u64)((u8 *)ItctBaseAddr + ItctIdx * ItctSize);
		pr_info("***** ITCT %d show below:*****\n", ItctIdx);
		pr_info("DW0 : dev_type = 0x%x.\n", itct->dev_type);
		pr_info("      valid = 0x%x.\n", itct->valid);
		pr_info("      brak_reply_ena = 0x%x.\n",
				itct->break_reply_ena);
		pr_info("      awt_control = 0x%x.\n", itct->awt_control);
		pr_info("      max_conn_rate = 0x%x.\n",
				itct->max_conn_rate);
		pr_info("      valid_link_number = 0x%x.\n",
				itct->valid_link_number);
		pr_info("      port_id = 0x%x.\n", itct->port_id);
		pr_info("      smp_timeout = 0x%x.\n", itct->smp_timeout);
		pr_info("      max_burst_byte = 0x%x.\n\n",
				itct->max_burst_byte);

		pr_info("DW1 : sas_addr = 0x%llx.\n\n", itct->sas_addr);

		pr_info("DW2 : IT_nexus_loss_time = 0x%x.\n",
				itct->IT_nexus_loss_time);
		pr_info("      bus_inactive_time_limit = 0x%x.\n",
				itct->bus_inactive_time_limit);
		pr_info("      max_conn_time_limit = 0x%x.\n",
				itct->max_conn_time_limit);
		pr_info("      reject_open_time_limit = 0x%x.\n\n",
				itct->reject_open_time_limit);

		pr_info("DW3 : curr_pathway_blk_cnt = 0x%x.\n",
				itct->curr_pathway_blk_cnt);
		pr_info("      curr_transmit_dir = 0x%x.\n",
				itct->curr_transmit_dir);
		pr_info("      tx_pri = 0x%x.\n", itct->tx_pri);
		pr_info("      awt_cont = 0x%x.\n", itct->awt_cont);
		pr_info("      curr_awt = 0x%x.\n", itct->curr_awt);
		pr_info("      curr_IT_nexus_loss_val = 0x%x.\n",
				itct->curr_IT_nexus_loss_val);
		pr_info("      tlr_enable = 0x%x.\n", itct->tlr_enable);
		pr_info("      catap = 0x%x.\n", itct->catap);
		pr_info("      curr_ncq_tag = 0x%x.\n", itct->curr_ncq_tag);
		pr_info("      cpn = 0x%x.\n", itct->cpn);
		pr_info("      cb = 0x%x.\n\n", itct->cb);

		pr_info("DW4 : sata_active_reg = 0x%x.\n",
				itct->sata_active_reg);
		pr_info("      ata_status = 0x%x.\n", itct->ata_status);
		pr_info("      eb = 0x%x.\n", itct->eb);
		pr_info("      rpn = 0x%x.\n", itct->rpn);
		pr_info("      rb = 0x%x.\n", itct->rb);
		pr_info("      sata_tx_ata_p = 0x%x.\n",
				itct->sata_tx_ata_p);
		pr_info("      tpn = 0x%x.\n", itct->tpn);
		pr_info("      tb = 0x%x.\n\n", itct->tb);

		for (i = 0; i < 8; i++) {
			pr_info("DW%02d: ncq_tag[%02d] = 0x%x.\n",
					(5 + i), 4 * i, itct->ncq_tag[4 * i]);
			pr_info("      ncq_tag[%02d] = 0x%x.\n", 4 * i + 1,
					itct->ncq_tag[4 * i + 1]);
			pr_info("      ncq_tag[%02d] = 0x%x.\n", 4 * i + 2,
					itct->ncq_tag[4 * i + 2]);
			pr_info("      ncq_tag[%02d] = 0x%x.\n\n", 4 * i + 3,
					itct->ncq_tag[4 * i + 3]);
		}
		pr_info("DW13: non_ncq_iptt = 0x%x.\n\n", itct->non_ncq_iptt);

		pr_info("******************************\n");
	} else {
		pr_err("ITCT idx err.\n");
	}

	return count;
}

static int hisi_sas_itct_release(struct inode *inode, struct file *file)
{
	struct hisi_sas_debugfs_data *hsdd = file->private_data;

	set_fs(hsdd->OldFs[0]);
	filp_close(hsdd->FileRt[0], NULL);

	return 0;
}

static const struct file_operations itct_operations = {
	.open    = hisi_sas_itct_open,
	.read    = hisi_sas_itct_read,
	.write   = hisi_sas_itct_write,
	.llseek  = NULL,
	.release = hisi_sas_itct_release,
};

static int hisi_sas_iost_open(struct inode *inode, struct file *file)
{
	u32 SubSys = 0;
	char Fname[MAX_DBG_FILE_LEN] = {0};
	struct file *Fp = NULL;
	char *dump_file = "iost";
	struct hisi_sas_debugfs_data *hsdd;
	struct hisi_hba *hisi_hba;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);

	if (inode->i_private) {
		hsdd = inode->i_private;
		hisi_hba = hsdd->hisi_hba;
		SubSys = hisi_hba->id;
	}

	/*open file*/
	snprintf(Fname,
			sizeof(Fname),
			"%s/c%d-%s.txt",
			FILE_PATH,
			SubSys,
			dump_file);
	hsdd->pFileFullName[0] = Fname;
	Fp = filp_open(Fname, (O_WRONLY | O_CREAT), 777);
	if (IS_ERR(Fp)) {
		pr_err("open iost file fail.\n");
		return -ENOMEM;
	}
	hsdd->OldFs[0] = get_fs();
	set_fs(KERNEL_DS);
	hsdd->FileRt[0] = Fp;
	file->private_data = hsdd;
	return 0;
}

#define IOST_IO_VAL_OFF     (GLOBAL_BASE_REG + 0x14c)

static ssize_t hisi_sas_iost_read(struct file *file,
		char __user *buf, size_t count, loff_t *ppos)
{
	int i, j;
	int Ret = 0;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct file *Fp = hsdd->FileRt[0];
	u32 val, iptt, flag, status;
	void *IostBaseAddr = hisi_hba->iost;
	u32 IostSize = sizeof(struct hisi_sas_iost);
	u32 IostDepth = HISI_SAS_COMMAND_ENTRIES;
	u32 *MemAddr = NULL;
	char BufWr[DBG_BUF_SIZE] = {0};

	pr_info("BA = 0x%p, size = 0x%08x, dep = 0x%08x.\n",
			IostBaseAddr, IostSize, IostDepth);

	/* loop for iost cached in ram*/
	for (i = 0; i < MAX_CACHED_IOST; i++) {
		val =  hisi_sas_read32(hisi_hba, IOST_IO_VAL_OFF);
		iptt = val & HISI_SAS_IOST_IPTT_MASK;
		flag = (val & HISI_SAS_IOST_IO_VAL_MASK)
			>> HISI_SAS_IOST_IO_VAL_OFF;
		status = (val & HISI_SAS_IOST_IO_STA_MASK)
			>>  HISI_SAS_IOST_IO_STA_OFF;

		snprintf(BufWr, sizeof(BufWr),
			"IPTT:0x%02d, flag:0x%1d, status:0x%08x, val:0x%08x\n",
			iptt, flag, status, val);

		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write cached IOST file fail.\n");
			return -ENOMEM;
		}
	}

#ifdef DUMP_BY_STRUCT
	/* loop for iost stored in ddr*/
	for (i = 0; i < IostDepth; i++) {
		MemAddr = (void *)&(((struct hisi_sas_iost *)IostBaseAddr)[i]);
		Ret = Fp->f_op->write(Fp, BufWr, IostSize, &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write IOST file fail.\n");
			return -ENOMEM;
		}
		MemAddr++;
	}
#else
	/* write the iost table, the whole num is 8192 */
	for (i = 0; i < HISI_SAS_COMMAND_ENTRIES; i++) {
		snprintf(BufWr,	sizeof(BufWr), "[0x%03x] : {[",	i);
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write IOST header fail.\n");
			return -ENOMEM;
		}
		MemAddr = (u32 *)(u64)((u8 *)IostBaseAddr + i * IostSize);

		/* write the whole iost, by word. totally 4 words.*/
		for (j = 0; j < IostSize; j += sizeof(u32)) {
			snprintf(BufWr,	sizeof(BufWr), " 0x%08x,", *MemAddr);
			Ret = Fp->f_op->write(Fp,
					BufWr, strlen(BufWr), &Fp->f_pos);
			if (Ret < 0) {
				pr_err("write IOST entry fail.\n");
				return -ENOMEM;
			}
			MemAddr++;
		}

		/* write the tail of the iost entry*/
		snprintf(BufWr, sizeof(BufWr), "]}\n");
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write IOST info fail.\n");
			return -ENOMEM;
		}
	}
#endif
	return 0;
}

static ssize_t hisi_sas_iost_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos)
{
	int rt = 0;
	u32 num;
	char info[MAX_DBG_FILE_LEN] = {0};
	unsigned long long val[MAX_PARA_NUM] = {0};
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	void *IostBaseAddr = hisi_hba->iost;
	struct hisi_sas_iost *iost = NULL;
	u32 IostSize = sizeof(struct hisi_sas_iost);
	u32 IostIdx = 0;

	/* copy data from user, check the iost index.*/
	rt = copy_from_user(info, buf, count);
	if (rt) {
		pr_err("copy_from_user fail.\n");
		return -ENOMEM;
	}

	/* separate input para*/
	rt = str_to_ll(info, val, &num);
	if (rt) {
		pr_err("separate input paramenter fail.");
		return -EINVAL;
	}
	IostIdx = val[0];
	pr_info("iost_idx = %d\n", IostIdx);

	if (IostIdx < HISI_SAS_COMMAND_ENTRIES) {
		iost = (struct hisi_sas_iost *)(u64)
			((u8 *)IostBaseAddr + IostIdx * IostSize);
		pr_info("***** IOST %d show below:*****\n", IostIdx);

		pr_info("DW0 : io_type = 0x%x.\n", iost->io_type);
		pr_info("      io_dir = 0x%x.\n", iost->io_dir);
		pr_info("      cmd_tlr = 0x%x.\n", iost->cmd_tlr);
		pr_info("      send_rpt = 0x%x.\n", iost->send_rpt);
		pr_info("      phy_id = 0x%x.\n", iost->phy_id);
		pr_info("      target_ict = 0x%x.\n", iost->target_ict);
		pr_info("      force_phy = 0x%x.\n", iost->force_phy);
		pr_info("      tlr_cnt = 0x%x.\n", iost->tlr_cnt);
		pr_info("      io_retry_cnt = 0x%x.\n", iost->io_retry_cnt);
		pr_info("      dir_fmt = 0x%x.\n", iost->dir_fmt);
		pr_info("      prd_dif_src = 0x%x.\n", iost->prd_dif_src);
		pr_info("      sgl_mode = 0x%x.\n", iost->sgl_mode);
		pr_info("      pir_present = 0x%x.\n", iost->pir_present);
		pr_info("      first_burst = 0x%x.\n", iost->first_burst);
		pr_info("      spp_pass_through = 0x%x.\n",
				iost->spp_pass_through);
		pr_info("      io_slot_number = 0x%x.\n\n",
				iost->io_slot_number);

		pr_info("DW1 : io_status = 0x%x.\n", iost->io_status);
		pr_info("      io_ts = 0x%x.\n", iost->io_ts);
		pr_info("      io_rs = 0x%x.\n", iost->io_rs);
		pr_info("      io_ct = 0x%x.\n", iost->io_ct);
		pr_info("      max_resp_frame_len = 0x%x.\n",
				iost->max_resp_frame_len);
		pr_info("      chk_len = 0x%x.\n", iost->chk_len);
		pr_info("      xfer_tptt = 0x%x.\n", iost->xfer_tptt);
		pr_info("      io_rt = 0x%x.\n", iost->io_rt);
		pr_info("      io_rd = 0x%x.\n", iost->io_rd);
		pr_info("      mis_cnt = 0x%x.\n\n", iost->mis_cnt);

		pr_info("DW2 : xfer_Offset = 0x%x.\n", iost->xfer_Offset);
		pr_info("      xfer_len = 0x%x.\n\n", iost->xfer_len);

		pr_info("DW3 : status_buffer_address = 0x%llx.\n\n",
				iost->status_buffer_address);

		pr_info("******************************\n");
	} else {
	    pr_err("IOST idx err.\n");
	}

	return count;
}

static int hisi_sas_iost_release(struct inode *inode, struct file *file)
{
	struct hisi_sas_debugfs_data *hsdd = file->private_data;

	set_fs(hsdd->OldFs[0]);
	filp_close(hsdd->FileRt[0], NULL);
	return 0;
}

static const struct file_operations iost_operations = {
	.open    = hisi_sas_iost_open,
	.read    = hisi_sas_iost_read,
	.write   = hisi_sas_iost_write,
	.llseek  = NULL,
	.release = hisi_sas_iost_release,
};

static int hisi_sas_bkpt_open(struct inode *inode, struct file *file)
{
	u32 SubSys = 0;
	char Fname[MAX_DBG_FILE_LEN] = {0};
	struct file *Fp = NULL;
	char *dump_file = "bkpt";
	struct hisi_sas_debugfs_data *hsdd;
	struct hisi_hba *hisi_hba;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);

	if (inode->i_private) {
		hsdd = inode->i_private;
		hisi_hba = hsdd->hisi_hba;
		SubSys = hisi_hba->id;
	}

	/*open file*/
	snprintf(Fname,
			sizeof(Fname),
			"%s/c%d-%s.txt",
			FILE_PATH,
			SubSys,
			dump_file);
	hsdd->pFileFullName[0] = Fname;

	Fp = filp_open(Fname, (O_WRONLY | O_CREAT), 777);
	if (IS_ERR(Fp)) {
		pr_err("open bkpt file fail.\n");
		return -ENOMEM;
	}

	hsdd->OldFs[0] = get_fs();
	set_fs(KERNEL_DS);
	hsdd->FileRt[0] = Fp;
	file->private_data = hsdd;
	return 0;
}

static ssize_t hisi_sas_bkpt_read(struct file *file,
		char __user *buf, size_t count, loff_t *ppos)
{
	int i, j;
	int Ret = 0;
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct file *Fp = hsdd->FileRt[0];
	void *BkptBaseAddr = hsdd->hisi_hba->breakpoint;
	u32 BkptSize = sizeof(struct hisi_sas_breakpoint);
	u32 BkptDepth = HISI_SAS_COMMAND_ENTRIES * BkptSize;
	char BufWr[DBG_BUF_SIZE] = {0};
	u32 *MemAddr = NULL;

	pr_info("BA = 0x%p, size = 0x%08x, total size = 0x%08x.\n",
			BkptBaseAddr, BkptSize, BkptDepth);

#ifdef DUMP_BY_STRUCT
	/* loop for breakpoint dump*/
	MemAddr = (void *)(BkptBaseAddr);
	Ret = Fp->f_op->write(Fp, MemAddr, BkptDepth, &Fp->f_pos);
	if (Ret < 0) {
		pr_err("write breakpoint file fail.\n");
		return -ENOMEM;
	}
#else
	/* write the breakpoint table, the whole num is 8192 */
	for (i = 0; i < HISI_SAS_COMMAND_ENTRIES; i++) {
		snprintf(BufWr,	sizeof(BufWr), "[0x%03x] : {[",	i);
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write IOST header fail.\n");
			return -ENOMEM;
		}
		MemAddr = (u32 *)(u64)((u8 *)BkptBaseAddr + i * BkptSize);

		/* write the whole iost, by word. totally 32 words.*/
		for (j = 0; j < BkptSize; j += sizeof(u32)) {
			snprintf(BufWr,	sizeof(BufWr), " 0x%08x,", *MemAddr);
			Ret = Fp->f_op->write(Fp,
					BufWr, strlen(BufWr), &Fp->f_pos);
			if (Ret < 0) {
				pr_err("write IOST entry fail.\n");
				return -ENOMEM;
			}
			MemAddr++;
		}

		/* write the tail of the iost entry*/
		snprintf(BufWr, sizeof(BufWr), "]}\n");
		Ret = Fp->f_op->write(Fp, BufWr, strlen(BufWr), &Fp->f_pos);
		if (Ret < 0) {
			pr_err("write IOST info fail.\n");
			return -ENOMEM;
		}
	}
#endif
	return 0;
}

static ssize_t hisi_sas_bkpt_write(struct file *file,
		const char __user *buf,	size_t count, loff_t *ppos)
{
	int rt, i;
	u32 num;
	char info[MAX_DBG_FILE_LEN] = {0};
	unsigned long long val[MAX_PARA_NUM] = {0};
	struct hisi_sas_debugfs_data *hsdd = file->private_data;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;
	struct hisi_sas_breakpoint *bkpt = NULL;
	void *BkptBaseAddr = hisi_hba->breakpoint;
	u32 BkptSize = sizeof(struct hisi_sas_breakpoint);
	u32 BkptIdx = 0;

	/* copy data from user, check the breakpoint index.*/
	rt = copy_from_user(info, buf, count);
	if (rt) {
		pr_err("copy_from_user fail.\n");
		return -ENOMEM;
	}

	/* separate input para */
	rt = str_to_ll(info, val, &num);
	if (rt) {
		pr_err("separate input paramenter fail.");
		return -EINVAL;
	}
	BkptIdx = val[0];
	pr_info("breakpoint idx = %d\n", BkptIdx);

	if (BkptIdx < HISI_SAS_COMMAND_ENTRIES) {
		bkpt = (struct hisi_sas_breakpoint *)
			(u64)((u8 *)BkptBaseAddr + BkptIdx * BkptSize);
	    pr_info("***** BREAKPOINT %d show below:*****\n", BkptIdx);
		for (i = 0; i < 32; i++) {
			pr_info("DW%02d: data[%02d] = 0x%x.\n",
					i, 4 * i, bkpt->data[4 * i]);
			pr_info("      data[%02d] = 0x%x.\n",
					4 * i + 1, bkpt->data[4 * i + 1]);
			pr_info("      data[%02d] = 0x%x.\n",
					4 * i + 2, bkpt->data[4 * i + 2]);
			pr_info("      data[%02d] = 0x%x.\n\n",
					4 * i + 3, bkpt->data[4 * i + 3]);
	    }
		pr_info("***********************************\n");
	} else {
		pr_err("BREAKPOINT idx err.\n");
	}
	return count;
}

static int hisi_sas_bkpt_release(struct inode *inode, struct file *file)
{
	struct hisi_sas_debugfs_data *hsdd = file->private_data;

	set_fs(hsdd->OldFs[0]);
	filp_close(hsdd->FileRt[0], NULL);
	return 0;
}

static const struct file_operations bkpt_operations = {
	.open    = hisi_sas_bkpt_open,
	.read    = hisi_sas_bkpt_read,
	.write   = hisi_sas_bkpt_write,
	.llseek  = NULL,
	.release = hisi_sas_bkpt_release,
};

#define  HARD_PHY_STATE_REG    (PORT_BASE_REG + 0x8)
#define  PHY_SUMBER_TIME_REG   (PORT_BASE_REG + 0x10)
#define  PHY_CTRL_REG          (PORT_BASE_REG + 0x14)
#define  PHY_TRAIN_TIME_REG    (PORT_BASE_REG + 0x24)
#define  PHY_RATE_CHG_REG      (PORT_BASE_REG + 0x28)
#define  PHY_TIMER_SET_REG     (PORT_BASE_REG + 0x2c)
#define  PHY_CTRL_STATUS_REG   (PORT_BASE_REG + 0x40)

static int hisi_sas_phy_reg_show(struct seq_file *seq, void *v)
{
	int phy = 0;
	struct hisi_sas_debugfs_data *hsdd = seq->private;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;

    /* show the reg of phy. to be continued */
	pr_info("%s PHY REG show :\n ", __func__);
	for (phy = 0; phy < HISI_SAS_MAX_PHYS; phy++) {
		pr_info("\n************** phy %d reg cfg **********\n", phy);
		pr_info("PHY cfg        : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, 0));
		pr_info("hard link rate : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, HARD_PHY_LINK_RATE_REG));
		pr_info("state          : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, HARD_PHY_STATE_REG));
		pr_info("prog link rate : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PROG_PHY_LINK_RATE_REG));
		pr_info("sumber time    : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_SUMBER_TIME_REG));
		pr_info("ctrl           : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_CTRL_REG));
		pr_info("train time     : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_TRAIN_TIME_REG));
		pr_info("rate chg       : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_RATE_CHG_REG));
		pr_info("timer set      : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_TIMER_SET_REG));
		pr_info("rate nego      : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_RATE_NEGO_REG));
		pr_info("ctrl status    : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_CTRL_STATUS_REG));
		pr_info("pcn            : 0x%x\n", hisi_sas_phy_read32
				(hisi_hba, phy, PHY_PCN_REG));
	}
	return 0;
}

static int hisi_sas_phy_cfg_open(struct inode *inode, struct file *file)
{
	return single_open(file, hisi_sas_phy_reg_show, inode->i_private);
}

static const struct file_operations phy_cfg_operations = {
	.open    = hisi_sas_phy_cfg_open,
	.read    = seq_read,
	.write   = NULL,
	.llseek  = seq_lseek,
	.release = single_release,
};

#define  GLOBAL_ARB_TRES_REG    (GLOBAL_BASE_REG + 0x4)
#define  GLOBAL_PHY_CONTEXT_REG (GLOBAL_BASE_REG + 0x20)
#define  GLOBAL_PHY_STATE_REG   (GLOBAL_BASE_REG + 0x24)
#define  PHY_PORT_STATE         (GLOBAL_BASE_REG + 0x2c)
#define  HGC_CON_TIME_REG       (GLOBAL_BASE_REG + 0x34)

static int hisi_sas_global_reg_show(struct seq_file *seq, void *v)
{
	struct hisi_sas_debugfs_data *hsdd = seq->private;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;

	/* show the reg of global config. to be continued */
	pr_info("%s GLOBAL CONFIG REG show :\n ", __func__);
	pr_info("DQ enalbe    : 0x%x\n",
			hisi_sas_read32(hisi_hba, GLOBAL_BASE_REG));
	pr_info("arb_thres    : 0x%x\n",
			hisi_sas_read32(hisi_hba, GLOBAL_ARB_TRES_REG));
	pr_info("iost_base_l  : 0x%x\n",
			hisi_sas_read32(hisi_hba, IOST_BASE_ADDR_LO));
	pr_info("iost_base_h  : 0x%x\n",
			hisi_sas_read32(hisi_hba, IOST_BASE_ADDR_HI));
	pr_info("itct_base_l  : 0x%x\n",
			hisi_sas_read32(hisi_hba, ITCT_BASE_ADDR_LO));
	pr_info("itct_base_h  : 0x%x\n",
			hisi_sas_read32(hisi_hba, ITCT_BASE_ADDR_HI));
	pr_info("bkpt_base_l  : 0x%x\n",
			hisi_sas_read32(hisi_hba, BROKEN_MSG_ADDR_LO));
	pr_info("bkpt_base_h  : 0x%x\n",
			hisi_sas_read32(hisi_hba, BROKEN_MSG_ADDR_HI));
	pr_info("phy_context  : 0x%x\n",
			hisi_sas_read32(hisi_hba, GLOBAL_PHY_CONTEXT_REG));
	pr_info("phy_state    : 0x%x\n",
			hisi_sas_read32(hisi_hba, GLOBAL_PHY_STATE_REG));
	pr_info("port_num_ma  : 0x%x\n",
			hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA_REG));
	pr_info("port_state   : 0x%x\n",
			hisi_sas_read32(hisi_hba, PHY_PORT_STATE));
	pr_info("phy_con_rate : 0x%x\n",
			hisi_sas_read32(hisi_hba, PHY_CONN_RATE_REG));
	pr_info("hgc_con_time : 0x%x\n",
			hisi_sas_read32(hisi_hba, HGC_CON_TIME_REG));
	return 0;
}

static int hisi_sas_global_cfg_open(struct inode *inode, struct file *file)
{
	return single_open(file, hisi_sas_global_reg_show, inode->i_private);
}

static const struct file_operations global_cfg_operations = {
	.open    = hisi_sas_global_cfg_open,
	.read    = seq_read,
	.write   = NULL,
	.llseek  = seq_lseek,
	.release = single_release,
};

/* debugfs initiliazation */
int hisi_sas_debugfs_init(struct hisi_hba *hisi_hba)
{
	int ret = 0;
	struct dentry *dq_dentry;
	struct dentry *cq_dentry;
	struct dentry *itct_dentry;
	struct dentry *iost_dentry;
	struct dentry *bkpt_dentry;
	struct dentry *phycfg_dentry;
	struct dentry *portcfg_dentry;
	struct hisi_sas_debugfs_data *hsdd;

	hsdd = kzalloc(sizeof(struct hisi_sas_debugfs_data), GFP_KERNEL);
	hsdd->hisi_hba = hisi_hba;

	/* create root dir, 0 for dsaf subsys, 1 for pcie subsys*/
	if (hisi_hba->id)
		dbg_dir = debugfs_create_dir("hs_sas_p", NULL);
	else
		dbg_dir = debugfs_create_dir("hs_sas_d", NULL);

	if (!dbg_dir) {
		pr_err("create debugfs dir fail.\n");
		goto out_remove;
	}
	pr_info("create root dir ok.\n");

	/* create file dq*/
	dq_dentry = debugfs_create_file("delivery_queue",
			S_IRUSR, dbg_dir, hsdd, &dq_operations);
	if (!dq_dentry) {
		pr_err("fail to create dq file\n");
		goto out_remove_root;
	}
	pr_info("create dq dbf ok.\n");

	/* create file cq*/
	cq_dentry = debugfs_create_file("complete_queue",
			S_IRUSR, dbg_dir, hsdd, &cq_operations);
	if (!cq_dentry) {
		pr_err("fail to create cq file.\n");
		goto out_remove_dq;
	}
	pr_info("create cq dbf ok.\n");

	/* create file itct*/
	itct_dentry = debugfs_create_file("itct",
			S_IRUSR, dbg_dir, hsdd, &itct_operations);
	if (!itct_dentry) {
		pr_err("fail to create itct file.\n");
		goto out_remove_cq;
	}
	pr_info("create itct dbf ok.\n");

	/* create file iost*/
	iost_dentry = debugfs_create_file("iost",
			S_IRUSR, dbg_dir, hsdd, &iost_operations);
	if (!iost_dentry) {
		pr_err("fail to create iost file.\n");
		goto out_remove_itct;
	}
	pr_info("create iost dbf ok.\n");

	/* create file breakpoint*/
	bkpt_dentry = debugfs_create_file("breakpoint",
			S_IRUSR, dbg_dir, hsdd, &bkpt_operations);
	if (!bkpt_dentry) {
		pr_err("fail to create breakpoint file.\n");
		goto out_remove_iost;
	}
	pr_info("create bkpt dbf ok.\n");

	/* create file phy config*/
	phycfg_dentry = debugfs_create_file("phy_cfg",
			S_IRUSR, dbg_dir, hsdd, &phy_cfg_operations);
	if (!phycfg_dentry) {
		pr_err("fail to create phy_config file.\n");
		goto out_remove_bkpt;
	}
	pr_info("create phy dbf ok.\n");

	/* create file port config*/
	portcfg_dentry = debugfs_create_file("global_cfg",
			S_IRUSR, dbg_dir, hsdd, &global_cfg_operations);
	if (!portcfg_dentry) {
		pr_err("fail to create port_config file.\n");
		goto out_remove_phycfg;
	}
	pr_info("create port dbf ok.\n");
	return ret;

out_remove_phycfg:
	debugfs_remove(phycfg_dentry);
out_remove_bkpt:
	debugfs_remove(bkpt_dentry);
out_remove_iost:
	debugfs_remove(iost_dentry);
out_remove_itct:
	debugfs_remove(itct_dentry);
out_remove_cq:
	debugfs_remove(cq_dentry);
out_remove_dq:
	debugfs_remove(dq_dentry);
out_remove_root:
	debugfs_remove(dbg_dir);
out_remove:
	return -ENOMEM;
}
