#include "hisi_sas.h"

static struct scsi_transport_template *hisi_sas_stt;
struct kmem_cache *hisi_sas_task_list_cache;

static struct scsi_host_template hisi_sas_sht = {
	.module = THIS_MODULE,
	.name = DRV_NAME,
	.queuecommand = sas_queuecommand,
	.target_alloc = sas_target_alloc,
	.slave_configure = sas_slave_configure,
	.scan_finished = hisi_sas_scan_finished,
	.scan_start = hisi_sas_scan_start,
	.change_queue_depth = sas_change_queue_depth,
	.bios_param = sas_bios_param,
	.can_queue = 1,
	.cmd_per_lun = 1,
	.this_id = -1,
	.sg_tablesize = SG_ALL,
	.max_sectors = SCSI_DEFAULT_MAX_SECTORS,
	.use_clustering = ENABLE_CLUSTERING,
	.eh_device_reset_handler = sas_eh_device_reset_handler,
	.eh_bus_reset_handler = sas_eh_bus_reset_handler,
	.target_destroy = sas_target_destroy,
	.ioctl = sas_ioctl,
};

static struct sas_domain_function_template hisi_sas_transport_ops = {
	.lldd_dev_found = hisi_sas_dev_found,
	.lldd_dev_gone	= hisi_sas_dev_gone,
	.lldd_execute_task	= hisi_sas_queue_command,
	.lldd_control_phy	= hisi_sas_control_phy,
	.lldd_abort_task	= hisi_sas_abort_task,
	.lldd_abort_task_set	= hisi_sas_abort_task_set,
	.lldd_clear_aca	= hisi_sas_clear_aca,
	.lldd_I_T_nexus_reset	= hisi_sas_I_T_nexus_reset,
	.lldd_lu_reset	= hisi_sas_lu_reset,
	.lldd_query_task	= hisi_sas_query_task,
	.lldd_abort_task_set	= hisi_sas_abort_task_set,
	.lldd_port_formed	= hisi_sas_port_formed,
	.lldd_port_deformed	= hisi_sas_port_deformed,
};

static int hisi_sas_prep_ha_init(struct Scsi_Host *shost, int n_core)
{
	int phy_nr, port_nr;
	struct asd_sas_phy **arr_phy;
	struct asd_sas_port **arr_port;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);

	phy_nr = HISI_SAS_MAX_PHYS * n_core;
	port_nr = phy_nr;

	memset(sha, 0x00, sizeof(struct sas_ha_struct));

	arr_phy  = kcalloc(phy_nr, sizeof(void *), GFP_KERNEL);
	arr_port = kcalloc(port_nr, sizeof(void *), GFP_KERNEL);
	if (!arr_phy || !arr_port)
		goto exit_free;

	sha->sas_phy = arr_phy;
	sha->sas_port = arr_port;
	sha->core.shost = shost;

	sha->lldd_ha = kzalloc(sizeof(struct hisi_hba_priv_info), GFP_KERNEL);
	if (!sha->lldd_ha)
		goto exit_free;

	//((struct mvs_prv_info *)sha->lldd_ha)->n_host = core_nr;

	shost->transportt = hisi_sas_stt;
	shost->max_id = HISI_SAS_MAX_DEVICES;
	shost->max_lun = ~0;
	shost->max_channel = 1;
	shost->max_cmd_len = 16;

	return 0;
exit_free:
	kfree(arr_phy);
	kfree(arr_port);
	return -1;
}

static int hisi_sas_alloc(struct hisi_hba *hisi_hba,
			struct Scsi_Host *shost)
{
	int i = 0, queue_slot_nr;
	char pool_name[32];

	queue_slot_nr = HISI_SAS_QUEUE_SLOTS * HISI_SAS_QUEUES;

	spin_lock_init(&hisi_hba->lock);
	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_init(hisi_hba, i);
		hisi_hba->port[i].wide_port_phymap = 0;
		hisi_hba->port[i].port_attached = 0;
		INIT_LIST_HEAD(&hisi_hba->port[i].list);
	}
	for (i = 0; i < HISI_SAS_MAX_DEVICES; i++) {
		hisi_hba->devices[i].taskfileset = HISI_SAS_ID_NOT_MAPPED;
		hisi_hba->devices[i].dev_type = SAS_PHY_UNUSED;
		hisi_hba->devices[i].device_id = i;
		hisi_hba->devices[i].dev_status = HISI_SAS_DEV_NORMAL;
		init_timer(&hisi_hba->devices[i].timer);
	}

	hisi_hba->slot_hdr = dma_alloc_coherent(hisi_hba->dev,
				sizeof(*hisi_hba->slot_hdr) * queue_slot_nr,
				&hisi_hba->slot_dma, GFP_KERNEL);
	if (!hisi_hba->slot_hdr)
		goto err_out;
	memset(hisi_hba->slot_hdr, 0, sizeof(*hisi_hba->slot_hdr) * queue_slot_nr);

	sprintf(pool_name, "%s%d", "hisi_sas_status_dma_pool", hisi_hba->id);
	hisi_hba->status_dma_pool = dma_pool_create(pool_name,
					hisi_hba->dev,
					HISI_SAS_STATUS_BUF_SZ,
					8, 0);

	if (!hisi_hba->status_dma_pool)
		goto err_out;

	hisi_hba->itct = dma_alloc_coherent(hisi_hba->dev,
				HISI_SAS_MAX_ITCT_ENTRIES *
				HISI_SAS_ITCT_ENTRY_SZ,
				&hisi_hba->itct_dma,
				GFP_KERNEL);
	if (!hisi_hba->itct)
		goto err_out;

	hisi_hba->tags_num = HISI_SAS_COMMAND_ENTRIES;

	hisi_sas_tag_init(hisi_hba);

	return 0;

err_out:
	return 1;
}

static void hisi_sas_free(struct hisi_hba *hisi_hba)
{
	pr_info("%s hisi_hba=%p\n", __func__, hisi_hba);
}

int hisi_sas_ioremap(struct hisi_hba *hisi_hba)
{
	struct platform_device *pdev = hisi_hba->pdev;
	struct device *dev = &pdev->dev;
	struct resource res;
	int err;

	err = of_address_to_resource(hisi_hba->np, 0, &res);
	if (err)
		goto err_out;

	if (!res.start)
		goto err_out;

	hisi_hba->regs = devm_ioremap(dev, res.start, resource_size(&res));
	if (!hisi_hba->regs)
		goto err_out;

	return 0;
err_out:
	return 1;
}

void hisi_sas_iounmap(void __iomem *regs)
{
	iounmap(regs);
}

static const struct of_device_id sas_core_of_match[] = {
	{ .compatible = "hisilicon,p660-sas-core",},
	{},
};

static struct hisi_hba *hisi_sas_platform_dev_alloc(
			struct platform_device *pdev,
			struct Scsi_Host *shost,
			struct device_node *np)
{
	struct hisi_hba *hisi_hba = NULL;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);

	hisi_hba = kzalloc(sizeof(*hisi_hba), GFP_KERNEL);
	if (!hisi_hba)
		return NULL;

	hisi_hba->pdev = pdev;
	hisi_hba->dev = &pdev->dev;
	hisi_hba->np = np;

	if (of_property_read_u32(np,
		"phy-count",
		&hisi_hba->n_phy))
		goto err_out;

	if (of_property_read_u32(np,
		"core-id",
		&hisi_hba->id))
		goto err_out;

	INIT_LIST_HEAD(&hisi_hba->wq_list);

	((struct hisi_hba_priv_info *)sha->lldd_ha)->hisi_hba[hisi_hba->id] = hisi_hba;
	((struct hisi_hba_priv_info *)sha->lldd_ha)->n_phy = hisi_hba->n_phy;

	hisi_hba->sas = sha;
	hisi_hba->shost = shost;

	hisi_hba->tags = kzalloc(HISI_SAS_COMMAND_ENTRIES/sizeof(unsigned long), GFP_KERNEL);
	if (!hisi_hba->tags)
		goto err_out;

	if (of_property_read_u32(np,
		"core-id",
		&hisi_hba->id))
		goto err_out;

	if (hisi_sas_ioremap(hisi_hba))
		goto err_out;

	if (!hisi_sas_alloc(hisi_hba, shost))
		return hisi_hba;
err_out:
	hisi_sas_free(hisi_hba);
	return NULL;
}

static void hisi_sas_init_add(struct hisi_hba *hisi_hba)
{
	u8 i;

	for (i=0; i < hisi_hba->n_phy; i++) {
		// copied from mvsas, j00310691
		hisi_hba->phy[i].dev_sas_addr = 0x5005043011ab0000ULL;
		hisi_hba->phy[i].dev_sas_addr =
			cpu_to_be64((u64)(*(u64 *)&hisi_hba->phy[i].dev_sas_addr));
	}
	memcpy(hisi_hba->sas_addr, &hisi_hba->phy[0].dev_sas_addr, SAS_ADDR_SIZE);
}

int hisi_sas_interrupt_ini(struct hisi_hba *hisi_hba)
{
	return 0;
}

void hisi_sas_platform_dev_free(struct hisi_hba *hisi_hba)
{
	pr_info("%s\n", __func__);
}

static void hisi_sas_post_ha_init(struct Scsi_Host *shost, int n_core)
{
	int can_queue, i, j;
	struct hisi_hba *hisi_hba = NULL;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	int n_phy = 0;

	for (j = 0; j < n_core; j++) {
		hisi_hba = ((struct hisi_hba_priv_info *)sha->lldd_ha)->hisi_hba[j];
		for (i = 0; i < hisi_hba->n_phy; i++) {
			pr_info("hisi_sas_post_ha_init hisi_hba=%p j=%d i=%d\n", hisi_hba, j, i);
			sha->sas_phy[n_phy] =
				&hisi_hba->phy[i].sas_phy;
			sha->sas_port[n_phy] =
				&hisi_hba->port[i].sas_port;
			n_phy++;
		}
	}

	sha->sas_ha_name = DRV_NAME;
	sha->dev = hisi_hba->dev;
	sha->lldd_module = THIS_MODULE;
	sha->sas_addr = &hisi_hba->sas_addr[0];

	sha->num_phys = n_phy; // fixme j00310691

	can_queue = 1; // fixme j00310691

	shost->sg_tablesize = min_t(u16, SG_ALL, HISI_SAS_MAX_SG);
	shost->can_queue = can_queue;
	hisi_hba->shost->cmd_per_lun = 1; // fixme j00310691
	sha->core.shost = hisi_hba->shost;
}

static int hisi_sas_probe(struct platform_device *pdev)
{
	struct Scsi_Host *shost = NULL;
	struct hisi_hba *hisi_hba = NULL;
	struct device_node *node = pdev->dev.of_node, *np;
	int rc, n_core = -1;

	if (of_property_read_u32(node,
		"core-count",
		&n_core) || (n_core == 0)) {
		rc = -ENOMEM;
		goto err_out_host;
	}

	shost = scsi_host_alloc(&hisi_sas_sht, sizeof(void *));
	if (!shost) {
		rc = -ENOMEM;
		goto err_out_host;
	}

	SHOST_TO_SAS_HA(shost) =
		kcalloc(1, sizeof(struct sas_ha_struct), GFP_KERNEL);
	if (!SHOST_TO_SAS_HA(shost)) {
		rc = -ENOMEM;
		goto err_out_ha;
	}

	rc = hisi_sas_prep_ha_init(shost, n_core);
	if (rc) {
		rc = -ENOMEM;
		goto err_out_prep_ha;
	}

	for (np = of_find_matching_node(node, sas_core_of_match); np;
		np = of_find_matching_node(np, sas_core_of_match)) {
		hisi_hba = hisi_sas_platform_dev_alloc(pdev, shost, np);
		if (!hisi_hba) {
			rc = -ENOMEM;
			goto err_out_dev_alloc;
		}

		memset(&hisi_hba->hba_param, 0xFF,
			sizeof(struct hba_info_page));

		hisi_sas_init_add(hisi_hba);

		hisi_sas_hw_init(hisi_hba);

		rc = hisi_sas_interrupt_ini(hisi_hba); // fixme j00310691
		if (rc)
			goto err_out_interrupt_ini;
	}

	hisi_sas_post_ha_init(shost, n_core);

	rc = scsi_add_host(shost, &pdev->dev);
	if (rc)
		goto err_out_add_host;

	rc = sas_register_ha(SHOST_TO_SAS_HA(shost));
	if (rc)
		goto err_out_register_ha;

	scsi_scan_host(hisi_hba->shost);

	return 0;

err_out_interrupt_ini:
	sas_unregister_ha(SHOST_TO_SAS_HA(shost));
err_out_register_ha:
	scsi_remove_host(shost);
err_out_add_host:
err_out_dev_alloc:
	hisi_sas_platform_dev_free(hisi_hba);
err_out_prep_ha:
err_out_ha:
	kfree(shost);
err_out_host:
	return rc;
}

static int hisi_sas_remove(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	return 0;
}

static const struct of_device_id sas_of_match[] = {
	{ .compatible = "hisilicon,p660-sas",},
	{},
};

static struct platform_driver sas_driver = {
	.probe = hisi_sas_probe,
	.remove = hisi_sas_remove,
	.driver = {
		.name = "hisi_sas_controller",
		.owner = THIS_MODULE,
		.of_match_table = sas_of_match,
	},
};



static __init int hisi_sas_init(void)
{
	int rc;

	pr_info("hisi_sas: driver version %s\n", DRV_VERSION);

	hisi_sas_stt = sas_domain_attach_transport(&hisi_sas_transport_ops);
	if (!hisi_sas_stt)
		return -ENOMEM;

	rc = platform_driver_register(&sas_driver);
	if (rc)
		return rc;

	return 0;
}

static __exit void hisi_sas_exit(void)
{

}

module_init(hisi_sas_init);
module_exit(hisi_sas_exit);

MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
