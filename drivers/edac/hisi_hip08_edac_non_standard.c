// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Hisilicon Limited.
 * Author: Shiju Jose <shiju.jose@huawei.com>
 */

/*
 * HISI HIP08 EDAC for Non Standard Errors
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/edac.h>
#include <linux/bitops.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/uuid.h>
#include <linux/pci.h>

#include "edac_module.h"
#include "edac_mc.h"
#include "linux/edac_non_standard.h"

#define HISI_HIP08_NS "HISI_HIP08_NON_STANDARD"

/* HISI OEM error definitions */
/* HISI PCIe Local error definitions */
#define HISI_PCIE_SUB_MODULE_ID_AP	0
#define HISI_PCIE_SUB_MODULE_ID_TL	1
#define HISI_PCIE_SUB_MODULE_ID_MAC	2
#define HISI_PCIE_SUB_MODULE_ID_DL	3
#define HISI_PCIE_SUB_MODULE_ID_SDI	4

#define HISI_PCIE_LOCAL_VALID_VERSION		BIT(0)
#define HISI_PCIE_LOCAL_VALID_SOC_ID		BIT(1)
#define HISI_PCIE_LOCAL_VALID_SOCKET_ID		BIT(2)
#define HISI_PCIE_LOCAL_VALID_NIMBUS_ID		BIT(3)
#define HISI_PCIE_LOCAL_VALID_SUB_MODULE_ID	BIT(4)
#define HISI_PCIE_LOCAL_VALID_CORE_ID		BIT(5)
#define HISI_PCIE_LOCAL_VALID_PORT_ID		BIT(6)
#define HISI_PCIE_LOCAL_VALID_ERR_TYPE		BIT(7)
#define HISI_PCIE_LOCAL_VALID_ERR_SEVERITY	BIT(8)
#define HISI_PCIE_LOCAL_VALID_ERR_MISC		9

#define HISI_ERR_SEV_RECOVERABLE	0
#define HISI_ERR_SEV_FATAL		1
#define HISI_ERR_SEV_CORRECTED		2

struct hisi_pcie_local_err_sec {
	uint64_t   val_bits;
	uint8_t    version;
	uint8_t    soc_id;
	uint8_t    socket_id;
	uint8_t    nimbus_id;
	uint8_t    sub_module_id;
	uint8_t    core_id;
	uint8_t    port_id;
	uint8_t    err_severity;
	uint16_t   err_type;
	uint8_t    reserv[2];
	uint32_t   err_misc[33];
	struct {
		u16	vendor_id;
		u16	device_id;
		u8	class_code[3];
		u8	function;
		u8	device;
		u16	segment;
		u8	bus;
		u8	secondary_bus;
		u16	slot;
		u8	reserved;
	}	device_id;
};

/* helper functions */
static char *err_severity(uint8_t err_sev)
{
	switch (err_sev) {
	case 0: return "recoverable";
	case 1: return "fatal";
	case 2: return "corrected";
	case 3: return "none";
	}
	return "unknown";
}

static char *pcie_local_sub_module_name(uint8_t id)
{
	switch (id) {
	case HISI_PCIE_SUB_MODULE_ID_AP: return "AP Layer";
	case HISI_PCIE_SUB_MODULE_ID_TL: return "TL Layer";
	case HISI_PCIE_SUB_MODULE_ID_MAC: return "MAC Layer";
	case HISI_PCIE_SUB_MODULE_ID_DL: return "DL Layer";
	case HISI_PCIE_SUB_MODULE_ID_SDI: return "SDI Layer";
	}
	return "unknown";
}

/* error data decoding functions */
static int hip08_pcie_local_error_handle(const struct edac_ns_err_info *ns_err)
{
	const struct hisi_pcie_local_err_sec *err = ns_err->err_data;
	struct pci_dev *pdev;
	unsigned int devfn;
	char buf[1024];
	char *p = buf;
	uint32_t i;
	int rc;

	if (err->val_bits == 0) {
		pr_info("%s: no valid error information\n", __func__);
		return -1;
	}

	p += sprintf(p, "[ ");
	p += sprintf(p, "Table version=%d ", err->version);
	if (err->val_bits & HISI_PCIE_LOCAL_VALID_SOC_ID)
		p += sprintf(p, "SOC ID=%d ", err->soc_id);

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_SOCKET_ID)
		p += sprintf(p, "socket ID=%d ", err->socket_id);

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_NIMBUS_ID)
		p += sprintf(p, "nimbus ID=%d ", err->nimbus_id);

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_SUB_MODULE_ID)
		p += sprintf(p, "sub module=%s ",
			     pcie_local_sub_module_name(err->sub_module_id));

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_CORE_ID)
		p += sprintf(p, "core ID=core%d ", err->core_id);

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_PORT_ID)
		p += sprintf(p, "port ID=port%d ", err->port_id);

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_ERR_SEVERITY)
		p += sprintf(p, "error severity=%s ",
			     err_severity(err->err_severity));

	if (err->val_bits & HISI_PCIE_LOCAL_VALID_ERR_TYPE)
		p += sprintf(p, "error type=0x%x ", err->err_type);
	p += sprintf(p, "]");
	pr_info("\nHISI HIP08: PCIe local error\n");
	pr_info("%s\n", buf);

	pr_info("Reg Dump:\n");
	for (i = 0; i < 33; i++) {
		if (err->val_bits & BIT(HISI_PCIE_LOCAL_VALID_ERR_MISC + i))
			pr_info("ERR_MISC_%d=0x%x\n", i, err->err_misc[i]);
	}

	devfn = PCI_DEVFN(err->device_id.device, err->device_id.function);
	pdev = pci_get_domain_bus_and_slot(err->device_id.segment,
					   err->device_id.bus,
					   devfn);
	if (!pdev) {
		pr_err("Can not find pci_dev for %04x:%02x:%02x:%x\n",
			err->device_id.segment, err->device_id.bus,
			PCI_SLOT(devfn), PCI_FUNC(devfn));
		return 0;
	}

	if (err->err_severity == HISI_ERR_SEV_FATAL ||
	    err->err_severity == HISI_ERR_SEV_RECOVERABLE) {
		/* try reset PCI bus for the error recovery */
		rc = pci_reset_bus(pdev);
		if (!rc)
			pr_info("pci_reset_bus for %04x:%02x:%02x:%x\n",
				err->device_id.segment, err->device_id.bus,
				PCI_SLOT(devfn), PCI_FUNC(devfn));
		else {
			pr_err("fail pci_reset_bus for %04x:%02x:%02x:%x\n",
				err->device_id.segment, err->device_id.bus,
				PCI_SLOT(devfn), PCI_FUNC(devfn));
			pci_uevent_ers(pdev, PCI_ERS_RESULT_DISCONNECT);
		}
	}

	return 0;
}

struct edac_ns_handler hip08_ns_handle_tab[] = {
	{
		.id = GUID_INIT(0xB2889FC9, 0xE7D7, 0x4F9D, 0xA8, 0x67, \
				  0xAF, 0x42, 0xE9, 0x8B, 0xE7, 0x72),
		.notify = hip08_pcie_local_error_handle,
	},
	{ /* sentinel */ }
};

static int hisi_hip08_ns_probe(void)
{
	int i;

	for (i = 0; hip08_ns_handle_tab[i].notify; i++) {
		if (edac_non_standard_event_notify_register(
						&hip08_ns_handle_tab[i]) < 0) {
			pr_warn("%s edac_ns_event_notify_register fail\n",
			       __func__);
			goto err;
		}
	}

	return 0;

err:
	for (i = 0; hip08_ns_handle_tab[i].notify; i++) {
		edac_non_standard_event_notify_unregister(
						&hip08_ns_handle_tab[i]);
	}

	return 0;
}

static int hisi_hip08_ns_remove(void)
{
	int i;

	for (i = 0; hip08_ns_handle_tab[i].notify; i++) {
		edac_non_standard_event_notify_unregister(
						&hip08_ns_handle_tab[i]);
	}

	return 0;
}

static int __init hisi_hip08_ns_edac_init(void)
{
	int rc;

	rc = hisi_hip08_ns_probe();
	if (rc >= 0) {
		pr_info("Loading driver\n");
		return 0;
	}

	pr_warn("Failed to register hisi_hip08_ns with error= %d\n", rc);

	return rc;
}

static void __exit hisi_hip08_ns_edac_exit(void)
{
	hisi_hip08_ns_remove();
}

module_init(hisi_hip08_ns_edac_init);
module_exit(hisi_hip08_ns_edac_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shiju Jose <shiju.jose@huawei.com>");
MODULE_DESCRIPTION("HiSilicon HIP08 EDAC Driver for Non-Standard Errors");
