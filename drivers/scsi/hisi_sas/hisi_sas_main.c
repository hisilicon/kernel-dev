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

#define DEV_IS_EXPANDER(type) \
	((type == SAS_EDGE_EXPANDER_DEVICE) || \
	(type == SAS_FANOUT_EXPANDER_DEVICE))

#define DEV_IS_GONE(dev) \
	((!dev) || (dev->dev_type == SAS_PHY_UNUSED))

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

int hisi_sas_get_ncq_tag(struct sas_task *task, u32 *hdr_tag)
{
	return 0;
}

void hisi_sas_slot_task_free(struct hisi_hba *hisi_hba, struct sas_task *task,
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

static int hisi_sas_task_prep_smp(struct hisi_hba *hisi_hba,
				struct hisi_sas_tei *tei)
{
	return HISI_SAS_DISP->prep_smp(hisi_hba, tei);
}

static int hisi_sas_task_prep_ssp(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei, int is_tmf,
		struct hisi_sas_tmf_task *tmf)
{
	return HISI_SAS_DISP->prep_ssp(hisi_hba, tei, is_tmf, tmf);
}

static int hisi_sas_task_prep_ata(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	return HISI_SAS_DISP->prep_stp(hisi_hba, tei);
}

static int hisi_sas_task_prep(struct sas_task *task, struct hisi_hba *hisi_hba,
				int is_tmf, struct hisi_sas_tmf_task *tmf,
				int *pass)
{
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_tei tei;
	struct hisi_sas_slot *slot;
	struct hisi_sas_cmd_hdr	*cmd_hdr_base;
	int queue_slot, queue, n_elem = 0, rc, iptt;

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
		return 0;
	}

	if (DEV_IS_GONE(hisi_sas_dev)) {
		if (hisi_sas_dev)
			dev_info(hisi_hba->dev, "device %llu not ready.\n",
				 hisi_sas_dev->device_id);
		else
			dev_info(hisi_hba->dev, "device %016llx not ready.\n",
				 SAS_ADDR(dev->sas_addr));

		rc = SAS_PHY_DOWN;
		return rc;
	}
	tei.port = dev->port->lldd_port;
	if (tei.port && !tei.port->port_attached && !tmf) {
		if (sas_protocol_ata(task->task_proto)) {
			struct task_status_struct *ts = &task->task_status;

			dev_info(hisi_hba->dev, "SATA/STP port %d not attach device.\n",
				 dev->port->id);
			ts->resp = SAS_TASK_COMPLETE;
			ts->stat = SAS_PHY_DOWN;
			task->task_done(task);
		} else {
			struct task_status_struct *ts = &task->task_status;

			dev_info(hisi_hba->dev, "SAS port %d does not attach device.\n",
				 dev->port->id);
			ts->resp = SAS_TASK_UNDELIVERED;
			ts->stat = SAS_PHY_DOWN;
			task->task_done(task);
		}
		return 0;
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
	rc = HISI_SAS_DISP->get_free_slot(hisi_hba, &queue, &queue_slot);
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

	slot->status_buffer = dma_pool_alloc(hisi_hba->status_buffer_pool,
					GFP_ATOMIC, &slot->status_buffer_dma);
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

static int hisi_sas_task_exec(struct sas_task *task,
	gfp_t gfp_flags,
	struct completion *completion,
	int is_tmf,
	struct hisi_sas_tmf_task *tmf)
{
	struct hisi_hba *hisi_hba;
	u32 rc;
	u32 pass = 0;
	unsigned long flags = 0;

	hisi_hba = ((struct hisi_sas_device *)task->dev->lldd_dev)->hisi_hba;

	spin_lock_irqsave(&hisi_hba->lock, flags);
	rc = hisi_sas_task_prep(task, hisi_hba, is_tmf, tmf, &pass);
	if (rc)
		dev_err(hisi_hba->dev, "hisi_sas exec failed[%d]!\n", rc);

	if (likely(pass))
		HISI_SAS_DISP->start_delivery(hisi_hba);
	spin_unlock_irqrestore(&hisi_hba->lock, flags);

	return rc;
}

void hisi_sas_bytes_dmaed(struct hisi_hba *hisi_hba, int phy_no)
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
	}

	if (phy->phy_type & PORT_TYPE_SAS) {
		struct sas_identify_frame *id;

		id = (struct sas_identify_frame *)phy->frame_rcvd;
		id->dev_type = phy->identify.device_type;
		id->initiator_bits = SAS_PROTOCOL_ALL;
		id->target_bits = phy->identify.target_port_protocols;
	} else if (phy->phy_type & PORT_TYPE_SATA) {
		/*Nothing*/
	}

	sas_phy->frame_rcvd_size = phy->frame_rcvd_size;

	hisi_hba->sas->notify_port_event(sas_phy, PORTE_BYTES_DMAED);
}

struct hisi_hba *hisi_sas_find_dev_hba(struct domain_device *dev)
{
	unsigned long i = 0, j = 0, hi;
	struct sas_ha_struct *sha = dev->port->ha;
	struct hisi_hba *hisi_hba;
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

	hi = j / ((struct hisi_hba_priv *)sha->lldd_ha)->n_phy;
	hisi_hba = ((struct hisi_hba_priv *)sha->lldd_ha)->hisi_hba[hi];

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

	dev_err(hisi_hba->dev, "%s: max support %d devices - could not alloc\n",
		__func__, HISI_SAS_MAX_DEVICES);

	return NULL;
}

void hisi_sas_setup_itct(struct hisi_hba *hisi_hba, struct hisi_sas_device *device)
{
	struct domain_device *dev = device->sas_device;
	u32 device_id = device->device_id;
	struct hisi_sas_itct *itct = &hisi_hba->itct[device_id];

	memset(itct, 0, sizeof(*itct));

	/* qw0 */
	switch (dev->dev_type) {
	case SAS_END_DEVICE:
	case SAS_EDGE_EXPANDER_DEVICE:
	case SAS_FANOUT_EXPANDER_DEVICE:
		itct->dev_type = HISI_SAS_DEV_TYPE_SSP;
		break;
	default:
		dev_warn(hisi_hba->dev, "%s unsupported dev type (%d)\n", __func__, dev->dev_type);
	}

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
	struct hisi_hba *hisi_hba;
	struct domain_device *parent_dev = dev->parent;
	struct hisi_sas_device *hisi_sas_device;

	hisi_hba = hisi_sas_find_dev_hba(dev);

	if (lock)
		spin_lock_irqsave(&hisi_hba->lock, flags);

	hisi_sas_device = hisi_sas_alloc_dev(hisi_hba);
	if (!hisi_sas_device) {
		res = -EINVAL;
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
			dev_info(hisi_hba->dev, "%s Error: no attached dev:%016llx at ex:%016llx.\n",
				__func__, SAS_ADDR(dev->sas_addr),
				SAS_ADDR(parent_dev->sas_addr));
			res = -EINVAL;
		}
	}

found_out:
	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
	return res;
}

int hisi_sas_scan_finished(struct Scsi_Host *shost, unsigned long time)
{
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	struct hisi_hba_priv *hisi_hba_priv = sha->lldd_ha;

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
	struct hisi_hba_priv *hisi_hba_priv = sha->lldd_ha;

	core_nr = hisi_hba_priv->n_core;

	for (j = 0; j < core_nr; j++) {
		hisi_hba = ((struct hisi_hba_priv *)sha->lldd_ha)->hisi_hba[j];
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

void hisi_sas_port_notify_formed(struct asd_sas_phy *sas_phy, int lock)
{
	struct sas_ha_struct *sas_ha = sas_phy->ha;
	struct hisi_hba *hisi_hba = NULL;
	int i = 0, j = 0, hi = 0;
	struct hisi_sas_phy *phy = sas_phy->lldd_phy;
	struct asd_sas_port *sas_port = sas_phy->port;
	struct hisi_sas_port *port;
	unsigned long flags = 0;

	if (!sas_port)
		return;

	while (sas_ha->sas_phy[i]) {
		if (sas_ha->sas_phy[i] == sas_phy) {
			hisi_hba = ((struct hisi_hba_priv *)sas_ha->lldd_ha)
					->hisi_hba[hi];
			port = &hisi_hba->port[j];
			break;
		}
		i++; j++;
		if (j == ((struct hisi_hba_priv *)sas_ha->lldd_ha)
				->hisi_hba[hi]->n_phy) {
			j = 0;
			hi++;
		}
	}

	if (hisi_hba == NULL) {
		pr_err("%s could not find hba\n", __func__);
		return;
	}

	if (lock)
		spin_lock_irqsave(&hisi_hba->lock, flags);
	port->port_attached = 1;
	phy->port = port;
	sas_port->lldd_port = port;

	if (lock)
		spin_unlock_irqrestore(&hisi_hba->lock, flags);
}

void hisi_sas_do_release_task(struct hisi_hba *hisi_hba,
		int phy_no, struct domain_device *dev)
{
	struct hisi_sas_phy *phy;
	struct hisi_sas_port *port;
	struct hisi_sas_slot *slot, *slot2;

	phy = &hisi_hba->phy[phy_no];
	port = phy->port;
	if (!port)
		return;

	list_for_each_entry_safe(slot, slot2, &port->list, entry) {
		struct sas_task *task;

		task = slot->task;
		if (dev && task->dev != dev)
			continue;

		dev_info(hisi_hba->dev, "Release slot [%x:%x], task [%p]:\n",
			slot->queue, slot->queue_slot, task);

		HISI_SAS_DISP->slot_complete(hisi_hba, slot, 1);
	}
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

		if (phy_no >= hisi_hba->n_phy)
			return;
	}
	list_for_each_entry(dev, &port->dev_list, dev_list_node)
		hisi_sas_do_release_task(phy->hisi_hba, phy_no, dev);
}

int hisi_sas_dev_found(struct domain_device *dev)
{
	return hisi_sas_dev_found_notify(dev, 1);
}

int hisi_sas_find_dev_phyno(struct domain_device *dev, int *phyno)
{
	int i = 0, j = 0, num = 0, n = 0;
	struct hisi_sas_device *hisi_sas_dev = (struct hisi_sas_device *)dev->lldd_dev;
	struct hisi_hba *hisi_hba = hisi_sas_dev->hisi_hba;
	struct sas_ha_struct *sha = dev->port->ha;

	while (sha->sas_port[i]) {
		if (sha->sas_port[i] == dev->port) {
			struct asd_sas_phy *phy;

			list_for_each_entry(phy,
				&sha->sas_port[i]->phy_list, port_phy_el) {
				j = 0;

				while (sha->sas_phy[j]) {
					if (sha->sas_phy[j] == phy)
						break;
					j++;
				}

				BUG(); /* j00310691 fixme as I cannot generate this path */
				num++;
				n++;
			}
			break;
		}
		i++;
	}
	return num;
}

static void hisi_sas_release_task(struct hisi_hba *hisi_hba,
			struct domain_device *dev)
{
	int i, phyno[4], num;

	num = hisi_sas_find_dev_phyno(dev, phyno);
	for (i = 0; i < num; i++)
		hisi_sas_do_release_task(hisi_hba, phyno[i], dev);
}

void hisi_sas_free_dev(struct hisi_sas_device *dev)
{
	u32 id = dev->device_id;

	memset(dev, 0, sizeof(*dev));
	dev->device_id = id;
	dev->dev_type = SAS_PHY_UNUSED;
}

static void hisi_sas_dev_gone_notify(struct domain_device *dev)
{
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_hba *hisi_hba;

	if (!hisi_sas_dev) {
		pr_info("found dev has gone.\n");
		return;
	}

	hisi_hba = hisi_sas_dev->hisi_hba;

	dev_info(hisi_hba->dev, "found dev[%lld:%x] is gone.\n",
		hisi_sas_dev->device_id, hisi_sas_dev->dev_type);
	hisi_sas_release_task(hisi_hba, dev);
	hisi_sas_free_dev(hisi_sas_dev);

	dev->lldd_dev = NULL;
	hisi_sas_dev->sas_device = NULL;
}

void hisi_sas_dev_gone(struct domain_device *dev)
{
	hisi_sas_dev_gone_notify(dev);
}

int hisi_sas_queue_command(struct sas_task *task, gfp_t gfp_flags)
{
	return hisi_sas_task_exec(task, gfp_flags, NULL, 0, NULL);
}

int hisi_sas_control_phy(struct asd_sas_phy *sas_phy,
			enum phy_func func,
			void *funcdata)
{
	pr_info("%s fixme\n", __func__);
	BUG();

	return 0;
}

int hisi_sas_abort_task(struct sas_task *task)
{
	pr_info("%s fixme\n", __func__);
	BUG();

	return 0;
}

int hisi_sas_abort_task_set(struct domain_device *dev, u8 *lun)
{
	pr_info("%s fixme\n", __func__);
	BUG();

	return 0;
}

int hisi_sas_clear_aca(struct domain_device *dev, u8 *lun)
{
	pr_info("%s\n", __func__);
	BUG();

	return 0;
}

int hisi_sas_clear_task_set(struct domain_device *dev, u8 *lun)
{
	pr_info("%s fixme\n", __func__);

	return 0;
}

int hisi_sas_I_T_nexus_reset(struct domain_device *dev)
{
	pr_info("%s\n", __func__);
	BUG();
	return 0;
}

int hisi_sas_lu_reset(struct domain_device *dev, u8 *lun)
{
	pr_info("%s fixme\n", __func__);
	BUG();

	return 0;
}

int hisi_sas_query_task(struct sas_task *task)
{
	pr_info("%s fixme\n", __func__);

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

void hisi_sas_phy_down(struct hisi_hba *hisi_hba, int phy_no, int rdy)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha = hisi_hba->sas;

	if (rdy) {
		/* Phy down but ready */
		pr_debug("phy%d down and ready\n", phy_no);
		hisi_sas_bytes_dmaed(hisi_hba, phy_no);
		hisi_sas_port_notify_formed(sas_phy, 0);
		pr_info("phy%d Attached Device\n", phy_no);
	} else {
		/* Phy down and not ready */
		pr_info("phy%d Removed Device\n", phy_no);
		phy->phy_attached = 0;
		sas_phy_disconnected(sas_phy);
		hisi_sas_phy_disconnected(phy);
		sas_ha->notify_phy_event(sas_phy, PHYE_LOSS_OF_SIGNAL);
	}
}

#ifdef CONFIG_DEBUG_FS
/*****************************************************
					for debug fs
*****************************************************/
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

	hsdd = kzalloc(sizeof(*hsdd), GFP_KERNEL);

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

	hsdd = kzalloc(sizeof(*hsdd), GFP_KERNEL);

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

	hsdd = kzalloc(sizeof(*hsdd), GFP_KERNEL);

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

	hsdd = kzalloc(sizeof(*hsdd), GFP_KERNEL);

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

#define IOST_IO_VAL_OFF     0x14c

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

	hsdd = kzalloc(sizeof(*hsdd), GFP_KERNEL);

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

#define  GLOBAL_ARB_TRES_REG    0x4
#define  GLOBAL_PHY_CONTEXT_REG 0x20
#define  GLOBAL_PHY_STATE_REG   0x24
#define  PHY_PORT_STATE         0x2c
#define  HGC_CON_TIME_REG       0x34

static int hisi_sas_global_reg_show(struct seq_file *seq, void *v)
{
	struct hisi_sas_debugfs_data *hsdd = seq->private;
	struct hisi_hba *hisi_hba = hsdd->hisi_hba;

	/* show the reg of global config. to be continued */
	pr_info("%s GLOBAL CONFIG REG show :\n ", __func__);
	pr_info("DQ enalbe    : 0x%x\n",
			hisi_sas_read32(hisi_hba, DLVRY_QUEUE_ENABLE_REG));
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

void hisi_sas_debugfs_free(struct hisi_hba *hisi_hba)
{
	debugfs_remove_recursive(hisi_hba->dbg_dir);
}

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
	struct dentry *dbg_dir;

	hsdd = devm_kzalloc(hisi_hba->dev, sizeof(*hsdd), GFP_KERNEL);
	if (!hsdd)
		return -ENOMEM;

	hsdd->hisi_hba = hisi_hba;

	/* create root dir, 0 for dsaf subsys, 1 for pcie subsys*/
	if (hisi_hba->id)
		dbg_dir = debugfs_create_dir("hs_sas_p", NULL);
	else
		dbg_dir = debugfs_create_dir("hs_sas_d", NULL);

	if (!dbg_dir) {
		pr_err("create debugfs dir fail.\n");
		return -ENOMEM;
	}
	pr_info("create root dir ok.\n");

	hisi_hba->dbg_dir = dbg_dir;
	/* create file dq*/
	dq_dentry = debugfs_create_file("delivery_queue",
			S_IRUSR, dbg_dir, hsdd, &dq_operations);
	if (!dq_dentry) {
		pr_err("fail to create dq file\n");
		goto err;
	}
	pr_info("create dq dbf ok.\n");

	/* create file cq*/
	cq_dentry = debugfs_create_file("complete_queue",
			S_IRUSR, dbg_dir, hsdd, &cq_operations);
	if (!cq_dentry) {
		pr_err("fail to create cq file.\n");
		goto err;
	}
	pr_info("create cq dbf ok.\n");

	/* create file itct*/
	itct_dentry = debugfs_create_file("itct",
			S_IRUSR, dbg_dir, hsdd, &itct_operations);
	if (!itct_dentry) {
		pr_err("fail to create itct file.\n");
		goto err;
	}
	pr_info("create itct dbf ok.\n");

	/* create file iost*/
	iost_dentry = debugfs_create_file("iost",
			S_IRUSR, dbg_dir, hsdd, &iost_operations);
	if (!iost_dentry) {
		pr_err("fail to create iost file.\n");
		goto err;
	}
	pr_info("create iost dbf ok.\n");

	/* create file breakpoint*/
	bkpt_dentry = debugfs_create_file("breakpoint",
			S_IRUSR, dbg_dir, hsdd, &bkpt_operations);
	if (!bkpt_dentry) {
		pr_err("fail to create breakpoint file.\n");
		goto err;
	}
	pr_info("create bkpt dbf ok.\n");

	/* create file phy config*/
	phycfg_dentry = debugfs_create_file("phy_cfg",
			S_IRUSR, dbg_dir, hsdd, &phy_cfg_operations);
	if (!phycfg_dentry) {
		pr_err("fail to create phy_config file.\n");
		goto err;
	}
	pr_info("create phy dbf ok.\n");

	/* create file port config*/
	portcfg_dentry = debugfs_create_file("global_cfg",
			S_IRUSR, dbg_dir, hsdd, &global_cfg_operations);
	if (!portcfg_dentry) {
		pr_err("fail to create port_config file.\n");
		goto err;
	}
	pr_info("create port dbf ok.\n");
	return ret;

err:
	debugfs_remove_recursive(dbg_dir);
	return -ENOMEM;
}
#endif /* CONFIG_DEBUG_FS */

