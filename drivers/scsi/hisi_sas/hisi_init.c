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

static struct scsi_transport_template *hisi_sas_stt;

static struct scsi_host_template hisi_sas_sht = {
	.module			= THIS_MODULE,
	.name			= DRV_NAME,
	.queuecommand		= sas_queuecommand,
	.target_alloc		= sas_target_alloc,
	.slave_configure	= sas_slave_configure,
	.scan_finished		= hisi_sas_scan_finished,
	.scan_start		= hisi_sas_scan_start,
	.change_queue_depth	= sas_change_queue_depth,
	.bios_param		= sas_bios_param,
	.can_queue		= 1,
	.cmd_per_lun		= 1,
	.this_id		= -1,
	.sg_tablesize		= SG_ALL,
	.max_sectors		= SCSI_DEFAULT_MAX_SECTORS,
	.use_clustering		= ENABLE_CLUSTERING,
	.eh_device_reset_handler = sas_eh_device_reset_handler,
	.eh_bus_reset_handler	= sas_eh_bus_reset_handler,
	.target_destroy		= sas_target_destroy,
	.ioctl			= sas_ioctl,
};

static struct sas_domain_function_template hisi_sas_transport_ops = {
	.lldd_dev_found		= hisi_sas_dev_found,
	.lldd_dev_gone		= hisi_sas_dev_gone,
	.lldd_execute_task	= hisi_sas_queue_command,
	.lldd_control_phy	= hisi_sas_control_phy,
	.lldd_abort_task	= hisi_sas_abort_task,
	.lldd_abort_task_set	= hisi_sas_abort_task_set,
	.lldd_clear_aca		= hisi_sas_clear_aca,
	.lldd_I_T_nexus_reset	= hisi_sas_I_T_nexus_reset,
	.lldd_lu_reset		= hisi_sas_lu_reset,
	.lldd_query_task	= hisi_sas_query_task,
	.lldd_abort_task_set	= hisi_sas_abort_task_set,
	.lldd_port_formed	= hisi_sas_port_formed,
	.lldd_port_deformed	= hisi_sas_port_deformed,
};

static int hisi_sas_prep_ha_init(struct device *dev, struct Scsi_Host *shost,
				 int n_core)
{
	int phy_nr, port_nr;
	struct asd_sas_phy **arr_phy;
	struct asd_sas_port **arr_port;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);

	phy_nr = HISI_SAS_MAX_PHYS * n_core;
	port_nr = phy_nr;

	arr_phy = devm_kcalloc(dev, phy_nr, sizeof(void *), GFP_KERNEL);
	arr_port = devm_kcalloc(dev, port_nr, sizeof(void *), GFP_KERNEL);
	if (!arr_phy || !arr_port)
		return -ENOMEM;

	sha->sas_phy = arr_phy;
	sha->sas_port = arr_port;
	sha->core.shost = shost;

	sha->lldd_ha = devm_kzalloc(dev, sizeof(struct hisi_hba_priv),
				    GFP_KERNEL);
	if (!sha->lldd_ha)
		return -ENOMEM;

	((struct hisi_hba_priv *)sha->lldd_ha)->n_core = n_core;

	shost->transportt = hisi_sas_stt;
	shost->max_id = HISI_SAS_MAX_DEVICES;
	shost->max_lun = ~0;
	shost->max_channel = 1;
	shost->max_cmd_len = 16;

	return 0;
}

static int hisi_sas_alloc(struct hisi_hba *hisi_hba, struct Scsi_Host *shost)
{
	int i, s;
	char pool_name[32];
	struct device *dev = hisi_hba->dev;

	spin_lock_init(&hisi_hba->lock);
	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_sas_phy_init(hisi_hba, i);
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

	for (i = 0; i < hisi_hba->queue_count; i++) {
		/* Delivery queue */
		s = sizeof(struct hisi_sas_cmd_hdr) * HISI_SAS_QUEUE_SLOTS;
		hisi_hba->cmd_hdr[i] = dma_alloc_coherent(dev, s,
					&hisi_hba->cmd_hdr_dma[i], GFP_KERNEL);
		if (!hisi_hba->cmd_hdr[i])
			goto err_out;
		memset(hisi_hba->cmd_hdr[i], 0, s);

		/* Completion queue */
		s = sizeof(struct hisi_sas_complete_hdr) * HISI_SAS_QUEUE_SLOTS;
		hisi_hba->complete_hdr[i] = dma_alloc_coherent(dev, s,
				&hisi_hba->complete_hdr_dma[i], GFP_KERNEL);
		if (!hisi_hba->complete_hdr[i])
			goto err_out;
		memset(hisi_hba->complete_hdr[i], 0, s);
	}

	sprintf(pool_name, "%s%d", "hisi_sas_status_buffer_pool", hisi_hba->id);
	s = HISI_SAS_STATUS_BUF_SZ;
	hisi_hba->status_buffer_pool = dma_pool_create(pool_name, dev, s, 16, 0);
	if (!hisi_hba->status_buffer_pool)
		goto err_out;

	sprintf(pool_name, "%s%d", "hisi_sas_command_table_pool", hisi_hba->id);
	s = HISI_SAS_COMMAND_TABLE_SZ;
	hisi_hba->command_table_pool = dma_pool_create(pool_name, dev, s, 16, 0);
	if (!hisi_hba->command_table_pool)
		goto err_out;

	s = HISI_SAS_MAX_ITCT_ENTRIES * HISI_SAS_ITCT_ENTRY_SZ;
	hisi_hba->itct = dma_alloc_coherent(dev, s, &hisi_hba->itct_dma,
					    GFP_KERNEL);
	if (!hisi_hba->itct)
		goto err_out;

	memset(hisi_hba->itct, 0, s);

	hisi_hba->slot_info = devm_kcalloc(dev, HISI_SAS_COMMAND_ENTRIES,
					   sizeof(struct hisi_sas_slot),
					   GFP_KERNEL);
	if (!hisi_hba->slot_info)
		goto err_out;

	s = HISI_SAS_COMMAND_ENTRIES * HISI_SAS_IOST_ENTRY_SZ;
	hisi_hba->iost = dma_alloc_coherent(dev, s, &hisi_hba->iost_dma,
					    GFP_KERNEL);
	if (!hisi_hba->iost)
		goto err_out;

	memset(hisi_hba->iost, 0, s);

	s = HISI_SAS_COMMAND_ENTRIES * HISI_SAS_BREAKPOINT_ENTRY_SZ;
	hisi_hba->breakpoint = dma_alloc_coherent(dev, s,
				&hisi_hba->breakpoint_dma, GFP_KERNEL);
	if (!hisi_hba->breakpoint)
		goto err_out;

	memset(hisi_hba->breakpoint, 0, s);

	hisi_hba->iptt_count = HISI_SAS_COMMAND_ENTRIES;
	s = hisi_hba->iptt_count / sizeof(unsigned long);
	hisi_hba->iptt_tags = devm_kzalloc(dev, s, GFP_KERNEL);
	if (!hisi_hba->iptt_tags)
		goto err_out;

	sprintf(pool_name, "%s%d", "hisi_sas_status_sge_pool", hisi_hba->id);
	hisi_hba->sge_page_pool = dma_pool_create(pool_name, hisi_hba->dev,
				sizeof(struct hisi_sas_sge_page), 16, 0);
	if (!hisi_hba->sge_page_pool)
		goto err_out;

	s = sizeof(struct hisi_sas_initial_fis) * HISI_SAS_MAX_PHYS;
	hisi_hba->initial_fis = dma_alloc_coherent(dev, s,
				&hisi_hba->initial_fis_dma, GFP_KERNEL);
	if (!hisi_hba->initial_fis)
		goto err_out;
	memset(hisi_hba->initial_fis, 0, s);

	s = HISI_SAS_COMMAND_ENTRIES * HISI_SAS_BREAKPOINT_ENTRY_SZ * 2;
	hisi_hba->sata_breakpoint = dma_alloc_coherent(dev, s,
				&hisi_hba->sata_breakpoint_dma, GFP_KERNEL);
	if (!hisi_hba->sata_breakpoint)
		goto err_out;
	memset(hisi_hba->sata_breakpoint, 0, s);

	hisi_sas_iptt_init(hisi_hba);

	return 0;
err_out:
	return -ENOMEM;
}

static void hisi_sas_free(struct hisi_hba *hisi_hba)
{
	int i, s;

	for (i = 0; i < hisi_hba->queue_count; i++) {
		s = sizeof(struct hisi_sas_cmd_hdr) * HISI_SAS_QUEUE_SLOTS;
		if (hisi_hba->cmd_hdr[i])
			dma_free_coherent(hisi_hba->dev, s,
					  hisi_hba->cmd_hdr[i],
					  hisi_hba->cmd_hdr_dma[i]);

		s = sizeof(struct hisi_sas_complete_hdr) * HISI_SAS_QUEUE_SLOTS;
		if (hisi_hba->complete_hdr[i])
			dma_free_coherent(hisi_hba->dev, s,
					  hisi_hba->complete_hdr[i],
					  hisi_hba->complete_hdr_dma[i]);
	}

	s = HISI_SAS_MAX_ITCT_ENTRIES * HISI_SAS_ITCT_ENTRY_SZ;
	if (hisi_hba->itct)
		dma_free_coherent(hisi_hba->dev, s,
				  hisi_hba->itct, hisi_hba->itct_dma);

	s = HISI_SAS_COMMAND_ENTRIES * HISI_SAS_IOST_ENTRY_SZ;
	if (hisi_hba->iost)
		dma_free_coherent(hisi_hba->dev, s,
				  hisi_hba->iost, hisi_hba->iost_dma);

	s = HISI_SAS_COMMAND_ENTRIES * HISI_SAS_BREAKPOINT_ENTRY_SZ;
	if (hisi_hba->breakpoint)
		dma_free_coherent(hisi_hba->dev, s,
				  hisi_hba->breakpoint,
				  hisi_hba->breakpoint_dma);

	if (hisi_hba->status_buffer_pool)
		dma_pool_destroy(hisi_hba->status_buffer_pool);

	if (hisi_hba->command_table_pool)
		dma_pool_destroy(hisi_hba->command_table_pool);

	if (hisi_hba->sge_page_pool)
		dma_pool_destroy(hisi_hba->sge_page_pool);
}

int hisi_sas_ioremap(struct hisi_hba *hisi_hba)
{
	struct platform_device *pdev = hisi_hba->pdev;
	struct resource res;
	int err;

	err = of_address_to_resource(hisi_hba->np, 0, &res);
	if (err)
		return err;

	hisi_hba->regs = devm_ioremap_resource(&pdev->dev, &res);
	if (IS_ERR(hisi_hba->regs))
		return PTR_ERR(hisi_hba->regs);

	return 0;
}

static const struct of_device_id sas_core_of_match[] = {
	{ .compatible = "hisilicon,p660-sas-core",
	.data = &hisi_sas_p660_dispatch},
	{},
};

static struct hisi_hba *hisi_sas_platform_dev_alloc(
			struct platform_device *pdev,
			struct Scsi_Host *shost,
			struct device_node *np)
{
	int interrupt_count;
	struct hisi_hba *hisi_hba;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	const struct of_device_id *match = of_match_node(sas_core_of_match, np);

	if (!match)
		goto err_out;

	hisi_hba = devm_kzalloc(&pdev->dev, sizeof(*hisi_hba), GFP_KERNEL);
	if (!hisi_hba)
		goto err_out;

	hisi_hba->pdev = pdev;
	hisi_hba->dev = &pdev->dev;
	hisi_hba->np = np;

	if (of_property_read_u32(np, "phy-count", &hisi_hba->n_phy))
		goto err_out;

	if (of_property_read_u32(np, "queue-count", &hisi_hba->queue_count))
		goto err_out;

	if (of_property_read_u32(np, "core-id", &hisi_hba->id))
		goto err_out;

	interrupt_count = of_property_count_u32_elems(np, "interrupts");
	if (interrupt_count < 0)
		goto err_out;

	hisi_hba->int_names = devm_kcalloc(&pdev->dev, interrupt_count / 2,
				HISI_SAS_INT_NAME_LENGTH,
				GFP_KERNEL);
	if (!hisi_hba->int_names)
		goto err_out;

	hisi_hba->dispatch = match->data;

	INIT_LIST_HEAD(&hisi_hba->wq_list);

	((struct hisi_hba_priv *)sha->lldd_ha)->hisi_hba[hisi_hba->id] = hisi_hba;
	((struct hisi_hba_priv *)sha->lldd_ha)->n_phy = hisi_hba->n_phy;

	hisi_hba->sas = sha;
	hisi_hba->shost = shost;

	if (hisi_sas_ioremap(hisi_hba))
		goto err_out;

	if (hisi_sas_alloc(hisi_hba, shost)) {
		hisi_sas_free(hisi_hba);
		goto err_out;
	}

	return hisi_hba;
err_out:
	dev_err(&pdev->dev, "platform_dev_alloc failed\n");
	return NULL;
}

static void hisi_sas_init_add(struct hisi_hba *hisi_hba)
{
	u8 i;

	/* Huawei IEEE id (001882) */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		hisi_hba->phy[i].dev_sas_addr = 0x5001882016072015ULL;
		hisi_hba->phy[i].dev_sas_addr =
			cpu_to_be64((u64)(*(u64 *)&hisi_hba->phy[i].dev_sas_addr));
	}
	memcpy(hisi_hba->sas_addr, &hisi_hba->phy[0].dev_sas_addr, SAS_ADDR_SIZE);
}

static void hisi_sas_post_ha_init(struct Scsi_Host *shost, int n_core)
{
	int i, j;
	struct hisi_hba *hisi_hba = NULL;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);
	int n_phy = 0;

	for (j = 0; j < n_core; j++) {
		hisi_hba = ((struct hisi_hba_priv *)sha->lldd_ha)->hisi_hba[j];
		for (i = 0; i < hisi_hba->n_phy; i++) {

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

	sha->num_phys = n_phy;

	shost->sg_tablesize = min_t(u16, SG_ALL, HISI_SAS_SGE_PAGE_CNT); // SG_ALL=SCSI_MAX_SG_SEGMENTS=128
	shost->can_queue = HISI_SAS_COMMAND_ENTRIES;
	shost->cmd_per_lun = HISI_SAS_COMMAND_ENTRIES;
	sha->core.shost = hisi_hba->shost;
}

static int hisi_sas_probe(struct platform_device *pdev)
{
	struct Scsi_Host *shost;
	struct hisi_hba *hisi_hba;
	struct device_node *node = pdev->dev.of_node, *np;
	int rc, n_core;

	if (of_property_read_u32(node, "core-count", &n_core) || (n_core == 0))
		return -ENOMEM;

	shost = scsi_host_alloc(&hisi_sas_sht, sizeof(void *));
	if (!shost)
		return -ENOMEM;

	SHOST_TO_SAS_HA(shost) = devm_kcalloc(&pdev->dev, 1,
					      sizeof(struct sas_ha_struct),
					      GFP_KERNEL);
	if (!SHOST_TO_SAS_HA(shost)) {
		rc = -ENOMEM;
		goto err_out_ha;
	}
	platform_set_drvdata(pdev, SHOST_TO_SAS_HA(shost));

	rc = hisi_sas_prep_ha_init(&pdev->dev, shost, n_core);
	if (rc)
		goto err_out_ha;

	for (np = of_find_matching_node(node, sas_core_of_match); np;
		np = of_find_matching_node(np, sas_core_of_match)) {
		hisi_hba = hisi_sas_platform_dev_alloc(pdev, shost, np);
		if (!hisi_hba) {
			rc = -ENOMEM;
			goto err_out_ha;
		}

		memset(&hisi_hba->hba_param, 0xFF, sizeof(struct hba_info_page));

		hisi_sas_init_add(hisi_hba);

		rc = HISI_SAS_DISP->hw_init(hisi_hba);
		if (rc)
			goto err_out_ha;

		rc = HISI_SAS_DISP->interrupt_init(hisi_hba);
		if (rc)
			goto err_out_ha;

		rc = HISI_SAS_DISP->interrupt_openall(hisi_hba);
		if (rc)
			goto err_out_ha;

		HISI_SAS_DISP->phys_init(hisi_hba);

#ifdef CONFIG_DEBUG_FS
		rc = hisi_sas_debugfs_init(hisi_hba);
		if (rc)
			goto err_out_ha;
#endif
	}

	hisi_sas_post_ha_init(shost, n_core);

	rc = scsi_add_host(shost, &pdev->dev);
	if (rc)
		goto err_out_ha;

	rc = sas_register_ha(SHOST_TO_SAS_HA(shost));
	if (rc)
		goto err_out_register_ha;

	scsi_scan_host(shost);

	return 0;

err_out_register_ha:
	scsi_remove_host(shost);

err_out_ha:
	kfree(shost);
	return rc;
}

static int hisi_sas_remove(struct platform_device *pdev)
{
	struct sas_ha_struct *sha = platform_get_drvdata(pdev);
	struct hisi_hba *hisi_hba;
	u32 n_core, i;

	n_core = ((struct hisi_hba_priv *)sha->lldd_ha)->n_core;
	sas_unregister_ha(sha);
	sas_remove_host(sha->core.shost);
	scsi_remove_host(sha->core.shost);

	for (i = 0; i < n_core; i++) {
		hisi_hba = ((struct hisi_hba_priv *)sha->lldd_ha)->hisi_hba[i];
#ifdef CONFIG_DEBUG_FS
		hisi_sas_debugfs_free(hisi_hba);
#endif
		hisi_sas_free(hisi_hba);
	}
	return 0;
}

static const struct of_device_id sas_of_match[] = {
	{ .compatible = "hisilicon,p660-sas",},
	{},
};

static struct platform_driver hisi_sas_driver = {
	.probe = hisi_sas_probe,
	.remove = hisi_sas_remove,
	.driver = {
		.name = "hisi_sas_controller",
		.of_match_table = sas_of_match,
	},
};

static __init int hisi_sas_init(void)
{
	pr_info("hisi_sas: driver version %s\n", DRV_VERSION);

	hisi_sas_stt = sas_domain_attach_transport(&hisi_sas_transport_ops);
	if (!hisi_sas_stt)
		return -ENOMEM;

	return platform_driver_register(&hisi_sas_driver);
}

static __exit void hisi_sas_exit(void)
{
	platform_driver_unregister(&hisi_sas_driver);
	sas_release_transport(hisi_sas_stt);
}

module_init(hisi_sas_init);
module_exit(hisi_sas_exit);

MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HISILICON P660 SAS/SATA controller driver");
MODULE_ALIAS("platform:hisi_sas_controller");
