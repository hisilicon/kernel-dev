/*
 * Copyright (C) 2016, Semihalf
 *	Author: Tomasz Nowicki <tn@semihalf.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * This file implements early detection/parsing of I/O mapping
 * reported to OS through firmware via I/O Remapping Table (IORT)
 * IORT document number: ARM DEN 0049A
 */

#define pr_fmt(fmt)	"ACPI: IORT: " fmt

#include <linux/export.h>
#include <linux/iort.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/pci.h>

struct iort_its_msi_chip {
	struct list_head	list;
	struct fwnode_handle	*fw_node;
	u32			translation_id;
};

typedef acpi_status (*iort_find_node_callback)
	(struct acpi_iort_node *node, void *context);

/* Root pointer to the mapped IORT table */
static struct acpi_table_header *iort_table;

static LIST_HEAD(iort_msi_chip_list);

/**
 * iort_register_domain_token() - register domain token and related ITS ID
 * 				  to the list from where we can get it back
 * 				  later on.
 * @translation_id: ITS ID
 * @token: domain token
 *
 * Returns: 0 on success, -ENOMEM if not memory when allocating list element.
 */
int iort_register_domain_token(int trans_id, struct fwnode_handle *fw_node)
{
	struct iort_its_msi_chip *its_msi_chip;

	its_msi_chip = kzalloc(sizeof(*its_msi_chip), GFP_KERNEL);
	if (!its_msi_chip)
		return -ENOMEM;

	its_msi_chip->fw_node = fw_node;
	its_msi_chip->translation_id = trans_id;

	list_add(&its_msi_chip->list, &iort_msi_chip_list);
	return 0;
}

/**
 * iort_unregister_domain_token() - unregister domain token based on ITS ID.
 * @translation_id: ITS ID
 *
 * Returns: none.
 */
void iort_deregister_domain_token(int trans_id)
{
	struct iort_its_msi_chip *its_msi_chip, *t;

	list_for_each_entry_safe(its_msi_chip, t, &iort_msi_chip_list, list) {
		if (its_msi_chip->translation_id == trans_id) {
			list_del(&its_msi_chip->list);
			kfree(its_msi_chip);
			break;
		}
	}
}

/**
 * iort_find_its_domain_token() - find domain token based on given ITS ID.
 * @translation_id: ITS ID
 *
 * Returns: domain token when find on the list, NULL otherwise.
 */
struct fwnode_handle *iort_its_find_domain_token(int trans_id)
{
	struct iort_its_msi_chip *its_msi_chip;

	list_for_each_entry(its_msi_chip, &iort_msi_chip_list, list) {
		if (its_msi_chip->translation_id == trans_id)
			return its_msi_chip->fw_node;
	}

	return NULL;
}

static struct acpi_iort_node *
iort_scan_node(enum acpi_iort_node_type type,
	       iort_find_node_callback callback, void *context)
{
	struct acpi_iort_node *iort_node, *iort_end;
	struct acpi_table_iort *iort;
	int i;

	if (!iort_table)
		return NULL;

	/*
	 * iort_table and iort both point to the start of IORT table, but
	 * have different struct types
	 */
	iort = (struct acpi_table_iort *)iort_table;

	/* Get the first IORT node */
	iort_node = ACPI_ADD_PTR(struct acpi_iort_node, iort,
				 iort->node_offset);
	iort_end = ACPI_ADD_PTR(struct acpi_iort_node, iort_table,
				iort_table->length);

	for (i = 0; i < iort->node_count; i++) {
		if (iort_node >= iort_end) {
			pr_err("iort node pointer overflows, bad table\n");
			return NULL;
		}

		if (iort_node->type == type) {
			if (ACPI_SUCCESS(callback(iort_node, context)))
				return iort_node;
		}

		iort_node = ACPI_ADD_PTR(struct acpi_iort_node, iort_node,
					 iort_node->length);
	}

	return NULL;
}

static acpi_status
iort_find_dev_callback(struct acpi_iort_node *node, void *context)
{
	struct acpi_buffer path = { ACPI_ALLOCATE_BUFFER, NULL };
	struct acpi_iort_root_complex *pci_rc;
	struct acpi_iort_named_component *node_dev;
	struct device *dev = context;
	struct acpi_device *adev;
	struct pci_bus *bus;

	switch (node->type) {
	case ACPI_IORT_NODE_PCI_ROOT_COMPLEX:
		bus = to_pci_bus(dev);
		pci_rc = (struct acpi_iort_root_complex *)node->node_data;

		/*
		 * It is assumed that PCI segment numbers maps one-to-one
		 * with root complexes. Each segment number can represent only
		 * one root complex.
		 */
		if (pci_rc->pci_segment_number == pci_domain_nr(bus))
			return AE_OK;

		break;
	case ACPI_IORT_NODE_NAMED_COMPONENT:
		adev = ACPI_COMPANION(dev);
		if (!adev)
			break;

		if (acpi_get_name(adev->handle, ACPI_FULL_PATHNAME, &path) != AE_OK)
			break;

		node_dev = (struct acpi_iort_named_component *)node->node_data;
		if (!strcmp(node_dev->device_name, (char *)path.pointer)) {
			kfree(path.pointer);
			return AE_OK;
		}
		kfree(path.pointer);
		break;
	}

	return AE_NOT_FOUND;
}

/**
 * iort_dev_find_its_id() - find the ITS identifier based on specified device.
 * @dev: device
 * @node_type: iort node type
 * @idx: index of the ITS identifier list
 * @its_id: ITS identifier
 *
 * Returns: 0 on success, appropriate error value otherwise
 */
static int
iort_dev_find_its_id(struct device *dev, int node_type,
			unsigned int idx, int *its_id)
{
	struct acpi_iort_its_group *its;
	struct acpi_iort_node *node;

	node = iort_scan_node(node_type, iort_find_dev_callback, dev);
	if (!node) {
		pr_err("can't find node related to %s device\n", dev_name(dev));
		return -ENXIO;
	}

	/* Go upstream until find its parent ITS node */
	while (node->type != ACPI_IORT_NODE_ITS_GROUP) {
		node = iort_find_parent_node(node);
		if (!node)
			return -ENXIO;
	}

	/* Move to ITS specific data */
	its = (struct acpi_iort_its_group *)node->node_data;
	if (idx > its->its_count) {
		pr_err("requested ITS ID index [%d] is greater than available ITS count [%d]\n",
		       idx, its->its_count);
		return -ENXIO;
	}

	*its_id = its->identifiers[idx];
	return 0;
}

/**
 * iort_find_dev_domain_token() - find registered domain token related to
 * 				  PCI and platform device
 * @dev: device
 *
 * Returns: domain token on success, NULL otherwise
 */
struct fwnode_handle *iort_find_dev_domain_token(struct device *dev, int node_type)
{
	int its_id;

	if (iort_dev_find_its_id(dev, node_type, 0, &its_id))
		return NULL;

	return iort_find_its_domain_token(its_id);
}

static struct acpi_iort_node *
iort_dev_map_rid(struct acpi_iort_node *node, u32 rid_in,
			    u32 *rid_out)
{

	if (!node)
		goto out;

	/* Go upstream */
	while (node->type != ACPI_IORT_NODE_ITS_GROUP) {
		struct acpi_iort_id_mapping *id;
		int i, found = 0;

		/* Exit when no mapping array */
		if (!node->mapping_offset || !node->mapping_count)
			return NULL;

		id = ACPI_ADD_PTR(struct acpi_iort_id_mapping, node,
				  node->mapping_offset);

		for (i = 0, found = 0; i < node->mapping_count; i++, id++) {
			/*
			 * Single mapping is not translation rule,
			 * lets move on for this case
			 */
			if (id->flags & ACPI_IORT_ID_SINGLE_MAPPING) {
				if (node->type != ACPI_IORT_NODE_SMMU) {
					rid_in = id->output_base;
					found = 1;
					break;
				}

				pr_warn(FW_BUG "[node %p type %d] SINGLE MAPPING flag not allowed for SMMU node, skipping ID map\n",
					node, node->type);
				continue;
			}

			if (rid_in < id->input_base ||
			    (rid_in > id->input_base + id->id_count))
				continue;

			rid_in = id->output_base + (rid_in - id->input_base);
			found = 1;
			break;
		}

		if (!found)
			return NULL;

		/* Firmware bug! */
		if (!id->output_reference) {
			pr_err(FW_BUG "[node %p type %d] ID map has NULL parent reference\n",
			       node, node->type);
			return NULL;
		}

		node = ACPI_ADD_PTR(struct acpi_iort_node, iort_table,
				    id->output_reference);
	}

out:
	if (rid_out)
		*rid_out = rid_in;
	return node;
}

static struct acpi_iort_node *
iort_its_find_node_and_map_rid(struct pci_dev *pdev, u32 req_id, u32 *dev_id)
{
	struct pci_bus *pbus = pdev->bus;
	struct acpi_iort_node *node;

	/* Find a PCI root bus */
	while (!pci_is_root_bus(pbus))
		pbus = pbus->parent;


	node = iort_scan_node(ACPI_IORT_NODE_PCI_ROOT_COMPLEX,
			      iort_find_dev_callback, &pbus->dev);
	if (!node) {
		dev_err(&pdev->dev, "can't find related IORT node\n");
		return NULL;
	}

	return iort_dev_map_rid(node, req_id, dev_id);
}

/**
 * iort_pci_domain_get_msi_rid() - find MSI RID based on PCI device ID
 * @pdev: The PCI device
 * @req_id: The PCI device requester ID
 *
 * Returns: MSI RID on success, input requester ID otherwise
 */
u32 iort_pci_get_msi_rid(struct pci_dev *pdev, u32 req_id)
{
	u32 dev_id;

	if (!iort_its_find_node_and_map_rid(pdev, req_id, &dev_id))
		return req_id;

	return dev_id;
}

/**
 * iort_pci_find_its_id() - find the ITS identifier based on specified device.
 * @pdev: The PCI device
 * @idx: Index of the ITS identifier list
 * @its_id: ITS identifier
 *
 * Returns: 0 on success, appropriate error value otherwise
 */
static int
iort_pci_find_its_id(struct pci_dev *pdev, u32 req_id, unsigned int idx,
		     int *its_id)
{
	struct acpi_iort_its_group *its;
	struct acpi_iort_node *node;

	node = iort_its_find_node_and_map_rid(pdev, req_id, NULL);
	if (!node)
		return -ENXIO;

	/* Move to ITS specific data */
	its = (struct acpi_iort_its_group *)node->node_data;
	if (idx > its->its_count) {
		dev_err(&pdev->dev, "requested ITS ID index [%d] is greater than available[%d]\n",
			idx, its->its_count);
		return -ENXIO;
	}

	*its_id = its->identifiers[idx];
	return 0;
}

/**
 * iort_pci_get_msi_domain_handle() - find registered domain token related to
 *                                    PCI device
 * @pdev:    The PCI device
 * @req_id:  The PCI device requester ID
 *
 * Returns: the MSI domain for this device, NULL otherwise
 */
struct irq_domain *
iort_pci_get_domain(struct pci_dev *pdev, u32 req_id)
{
	static struct fwnode_handle *handle;
	int its_id;

	if (iort_pci_find_its_id(pdev, req_id, 0, &its_id))
		return NULL;

	handle = iort_its_find_domain_token(its_id);
	if (!handle)
		return NULL;

	return irq_find_matching_fwnode(handle, DOMAIN_BUS_PCI_MSI);
}

static int __init iort_table_detect(void)
{
	acpi_status status;

	if (acpi_disabled)
		return -ENODEV;

	status = acpi_get_table(ACPI_SIG_IORT, 0, &iort_table);
	if (ACPI_FAILURE(status)) {
		const char *msg = acpi_format_exception(status);
		pr_err("Failed to get table, %s\n", msg);
		return -EINVAL;
	}

	return 0;
}
arch_initcall(iort_table_detect);
