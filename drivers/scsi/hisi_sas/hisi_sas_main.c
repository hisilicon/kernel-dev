#include "hisi_sas.h"

#define DEV_IS_EXPANDER(type) \
	((type == SAS_EDGE_EXPANDER_DEVICE) || (type == SAS_FANOUT_EXPANDER_DEVICE))

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

int hisi_sas_tag_alloc(struct hisi_hba *hisi_hba, int tag)
{
	return 0;
}

void hisi_sas_tag_init(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->tags_num; ++i)
		hisi_sas_tag_clear(hisi_hba, i);
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


void hisi_sas_hw_init(struct hisi_hba *hisi_hba)
{

}

int hisi_sas_scan_finished(struct Scsi_Host *shost, unsigned long time)
{
	//pr_info("%s shost=%p time=%lu\n", __func__, shost, time);

	return 1;
}

void hisi_sas_scan_start(struct Scsi_Host *shost)
{
	//pr_info("%s shost=%p\n", __func__, shost);
}

void hisi_sas_phy_init(struct hisi_hba *hisi_hba, int i)
{

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
	pr_info("%s\n", __func__);
	return 0;
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

