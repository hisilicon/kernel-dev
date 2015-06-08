#include "hisi_sas.h"

#define DEV_IS_EXPANDER(type) \
	((type == SAS_EDGE_EXPANDER_DEVICE) || (type == SAS_FANOUT_EXPANDER_DEVICE))

#define DEV_IS_GONE(dev) \
	((!dev) || (dev->dev_type == SAS_PHY_UNUSED))


#define DMA_ADDR_LO(addr) ((u32)(addr&0xffffffff))
#define DMA_ADDR_HI(addr) ((u32)(addr>>32))

/* registers */
#define BASE_REG		(0x800)
#define PHY_CFG_REG		(BASE_REG + 0x0)
#define PHY_CFG_REG_RESET_OFF	0
#define PHY_CFG_REG_RESET_MASK	1
#define PHY_CTRL_REG		(BASE_REG + 0x14)

static inline void hisi_sas_write32(struct hisi_hba *hisi_hba, int phy, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	writel(val, regs);
}

static inline u32 hisi_sas_read32(struct hisi_hba *hisi_hba, int phy, u32 off)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;
	return readl(regs);
}

void hisi_sas_tag_clear(struct hisi_hba *hisi_hba, int tag)
{
	void *bitmap = hisi_hba->tags;
	clear_bit(tag, bitmap);
}

void hisi_sas_tag_free(struct hisi_hba *hisi_hba, int tag)
{
	hisi_sas_tag_clear(hisi_hba, tag);
}

void hisi_sas_tag_set(struct hisi_hba *hisi_hba, int tag)
{
	void *bitmap = hisi_hba->tags;
	set_bit(tag, bitmap);
}

int hisi_sas_tag_alloc(struct hisi_hba *hisi_hba, int *tag)
{
	return 0;
}

void hisi_sas_tag_init(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->tags_num; ++i)
		hisi_sas_tag_clear(hisi_hba, i);
}

static int hisi_sas_get_free_slot(struct hisi_hba *hisi_hba, int *q, int *s)
{
	return 0;
}

static int hisi_sas_task_prep_smp(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct sas_phy *sphy = dev->phy;
	struct scatterlist *sg_req, *sg_resp;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	dma_addr_t req_dma_addr;
	unsigned int req_len, resp_len;
	int elem, rc, queue = tei->queue, queue_slot = tei->queue_slot;
	struct hisi_sas_slot_info *slot = &hisi_hba->slot_info[queue][queue_slot];

	/*
	* DMA-map SMP request, response buffers
	*/

	sg_req = &task->smp_task.smp_req; /* this is the request frame - see alloc_smp_req() */
	elem = dma_map_sg(hisi_hba->dev, sg_req, 1, DMA_TO_DEVICE); /* map to dma address */
	if (!elem)
		return -ENOMEM;
	req_len = sg_dma_len(sg_req);
	req_dma_addr = sg_dma_address(sg_req);
	pr_info("%s sq_req=%p elem=%d req_len=%d\n", __func__, sg_req, elem, req_len);

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
	hdr->t10_flds_pres = 0; /* not sure */
	/* hdr->resp_report, ->tlr_ctrl for STP */
	hdr->phy_id = 1 << sphy->number; /* double-checl */
	hdr->force_phy = 0; /* due not force ordering in phy */
	hdr->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	hdr->priority = 0; /* ordinary priority */
	hdr->mode = 1; /* ini/host mode */
	hdr->cmd = 2; /* smp */

	/* hdr->port_multiplier, ->bist_active, ->atapi */
	/* ->first_party_dma, ->reset only applies to stp */
	/* hdr->pir_pres, ->enable_tlr, ->ssp->pass_through */
	/* ->spp_frame_type only applicable to ssp */

	hdr->device_id = hisi_sas_dev->device_id; /* map itct entry */

	hdr->cmd_frame_len = req_len/4;
	/* hdr->leave_affil_open only applicable to stp */
	hdr->max_resp_frame_len = resp_len/4;
	/* hdr->sg_mode, ->first_burst not applicable to smp */

	/* hdr->iptt, ->tptt not applicable to smp */

	/* hdr->data_transfer_len not applicable to smp */

	/* hdr->first_burst_num not applicable to smp */

	/* hdr->dif_prd_table_len, ->prd_table_len not applicable to smp */

	/* hdr->double_mode, ->abort_iptt not applicable to smp */

	hdr->cmd_frame_addr_lo  = DMA_ADDR_LO(req_dma_addr);
	hdr->cmd_frame_addr_hi  = DMA_ADDR_HI(req_dma_addr);

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

static int hisi_sas_task_prep_ssp(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei, int is_tmf,
		struct hisi_sas_tmf_task *tmf)
{
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
	struct hisi_sas_slot_info *slot;
	int queue_slot = -1, queue = -1, n_elem = 0, rc = 0, tag = -1;

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
			pr_info("SATA/STP port %d does not attach"
					"device.\n", dev->port->id);
			ts->resp = SAS_TASK_COMPLETE;
			ts->stat = SAS_PHY_DOWN;

			task->task_done(task);

		} else {
			struct task_status_struct *ts = &task->task_status;
			pr_info("SAS port %d does not attach"
				"device.\n", dev->port->id);
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

	rc = hisi_sas_tag_alloc(hisi_hba, &tag);
	if (rc)
		goto err_out;

	rc = hisi_sas_get_free_slot(hisi_hba, &queue, &queue_slot);
	if (rc)
		goto err_out;

	slot = &hisi_hba->slot_info[queue][queue_slot];

	task->lldd_task = NULL;
	slot->n_elem = n_elem;
	slot->queue = queue;
	slot->queue_slot = queue_slot;

	slot->status_buffer = dma_pool_alloc(hisi_hba->status_dma_pool, GFP_ATOMIC,
				&slot->status_buffer_dma);
	if (!slot->status_buffer)
		goto err_out;
	memset(slot->status_buffer, 0, HISI_SAS_STATUS_BUF_SZ);

	tei.task = task;
	tei.hdr = &hisi_hba->slot_hdr[queue*HISI_SAS_QUEUES+queue_slot];
	tei.queue = queue;
	tei.queue_slot = queue_slot;
	tei.n_elem = n_elem;
	tei.tag = tag;
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
		dev_printk(KERN_ERR, hisi_hba->dev,
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

	hisi_sas_dev->running_req++;
	++(*pass);

	return rc;

err_out:
	// Add proper labels j00310691
	return rc;
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
		dev_printk(KERN_ERR, hisi_hba->dev, "hisi_sas exec failed[%d]!\n", rc);

	//if (likely(pass))
	//		MVS_CHIP_DISP->start_delivery(mvi, (mvi->tx_prod - 1) &
	//			(MVS_CHIP_SLOT_SZ - 1));
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
		//sphy->maximum_linkrate_hw = MVS_CHIP_DISP->phy_max_link_rate();
	}

	if (phy->phy_type & PORT_TYPE_SAS) {
		struct sas_identify_frame *id;

		id = (struct sas_identify_frame *)phy->frame_rcvd;
		id->dev_type = phy->identify.device_type;
		id->initiator_bits = SAS_PROTOCOL_ALL;
		id->target_bits = phy->identify.target_port_protocols;

		/* direct attached SAS device */
		if (phy->att_dev_info & PORT_SSP_TRGT_MASK) {
			//MVS_CHIP_DISP->write_port_cfg_addr(mvi, i, PHYR_PHY_STAT);
			//MVS_CHIP_DISP->write_port_cfg_data(mvi, i, 0x00);
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
	pr_info("%s\n", __func__);
	return NULL;
}

void hisi_sas_setup_itct(struct hisi_sas_device *hisi_sas_device)
{

}

int hisi_sas_dev_found_notify(struct domain_device *dev, int lock)
{
	unsigned long flags = 0;
	int res = 0;
	struct hisi_hba *hisi_hba = NULL;
	struct domain_device  *parent_dev = dev->parent;
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

	hisi_sas_setup_itct(hisi_sas_device);

	if (parent_dev && DEV_IS_EXPANDER(parent_dev->dev_type)) {

	}

found_out:
	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
	return res;
}

static void hisi_sas_reset_hw(struct hisi_hba *hisi_hba)
{
	int i;

	/* inline Higgs_PrepareResetHw j00310691 */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 phy_ctrl = hisi_sas_read32(hisi_hba, i, PHY_CTRL_REG);

		phy_ctrl |= (PHY_CFG_REG_RESET_MASK << PHY_CFG_REG_RESET_OFF);
		hisi_sas_write32(hisi_hba, i, PHY_CTRL_REG, phy_ctrl);
	}
	udelay(50);
}

static void hisi_sas_init_reg(struct hisi_hba *hisi_hba)
{

}

static void hisi_sas_init_id_frame(struct hisi_hba *hisi_hba)
{

}


void hisi_sas_hw_init(struct hisi_hba *hisi_hba)
{
	hisi_sas_reset_hw(hisi_hba);
	hisi_sas_init_reg(hisi_hba);
	/* maybe init serdes param j00310691 */
	hisi_sas_init_id_frame(hisi_hba);
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

