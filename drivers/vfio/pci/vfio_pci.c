// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, NVIDIA CORPORATION & AFFILIATES. All rights reserved
 *
 * Copyright (C) 2012 Red Hat, Inc.  All rights reserved.
 *     Author: Alex Williamson <alex.williamson@redhat.com>
 *
 * Derived from original vfio:
 * Copyright 2010 Cisco Systems, Inc.  All rights reserved.
 * Author: Tom Lyon, pugs@cisco.com
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/device.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/circ_buf.h>

#include <linux/vfio_pci_core.h>

#define DRIVER_AUTHOR   "Alex Williamson <alex.williamson@redhat.com>"
#define DRIVER_DESC     "VFIO PCI - User Level meta-driver"

static char ids[1024] __initdata;
module_param_string(ids, ids, sizeof(ids), 0);
MODULE_PARM_DESC(ids, "Initial PCI IDs to add to the vfio driver, format is \"vendor:device[:subvendor[:subdevice[:class[:class_mask]]]]\" and multiple comma separated entries can be specified");

static bool nointxmask;
module_param_named(nointxmask, nointxmask, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nointxmask,
		  "Disable support for PCI 2.3 style INTx masking.  If this resolves problems for specific devices, report lspci -vvvxxx to linux-pci@vger.kernel.org so the device can be fixed automatically via the broken_intx_masking flag.");

#ifdef CONFIG_VFIO_PCI_VGA
static bool disable_vga;
module_param(disable_vga, bool, S_IRUGO);
MODULE_PARM_DESC(disable_vga, "Disable VGA resource access through vfio-pci");
#endif

static bool disable_idle_d3;
module_param(disable_idle_d3, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(disable_idle_d3,
		 "Disable using the PCI D3 low power state for idle, unused devices");

static bool enable_sriov;
#ifdef CONFIG_PCI_IOV
module_param(enable_sriov, bool, 0644);
MODULE_PARM_DESC(enable_sriov, "Enable support for SR-IOV configuration.  Enabling SR-IOV on a PF typically requires support of the userspace PF driver, enabling VFs without such support may result in non-functional VFs or PF.");
#endif

static bool disable_denylist;
module_param(disable_denylist, bool, 0444);
MODULE_PARM_DESC(disable_denylist, "Disable use of device denylist. Disabling the denylist allows binding to devices with known errata that may lead to exploitable stability or security issues when accessed by untrusted users.");

static bool vfio_pci_dev_in_denylist(struct pci_dev *pdev)
{
	switch (pdev->vendor) {
	case PCI_VENDOR_ID_INTEL:
		switch (pdev->device) {
		case PCI_DEVICE_ID_INTEL_QAT_C3XXX:
		case PCI_DEVICE_ID_INTEL_QAT_C3XXX_VF:
		case PCI_DEVICE_ID_INTEL_QAT_C62X:
		case PCI_DEVICE_ID_INTEL_QAT_C62X_VF:
		case PCI_DEVICE_ID_INTEL_QAT_DH895XCC:
		case PCI_DEVICE_ID_INTEL_QAT_DH895XCC_VF:
			return true;
		default:
			return false;
		}
	}

	return false;
}

static bool vfio_pci_is_denylisted(struct pci_dev *pdev)
{
	if (!vfio_pci_dev_in_denylist(pdev))
		return false;

	if (disable_denylist) {
		pci_warn(pdev,
			 "device denylist disabled - allowing device %04x:%04x.\n",
			 pdev->vendor, pdev->device);
		return false;
	}

	pci_warn(pdev, "%04x:%04x exists in vfio-pci device denylist, driver probing disallowed.\n",
		 pdev->vendor, pdev->device);

	return true;
}

static void vfio_pci_dma_fault_release(struct vfio_pci_core_device *vdev,
				       struct vfio_pci_region *region)
{
	kfree(vdev->fault_pages);
}

static void
vfio_pci_dma_fault_response_release(struct vfio_pci_core_device *vdev,
				    struct vfio_pci_region *region)
{
	if (vdev->dma_fault_response_wq)
		destroy_workqueue(vdev->dma_fault_response_wq);
	kfree(vdev->fault_response_pages);
	vdev->fault_response_pages = NULL;
}

static int __vfio_pci_dma_fault_mmap(struct vfio_pci_core_device *vdev,
				   struct vfio_pci_region *region,
				   struct vm_area_struct *vma,
				   u8 *pages)
{
	u64 phys_len, req_len, pgoff, req_start;
	unsigned long long addr;
	unsigned int ret;

	phys_len = region->size;

	req_len = vma->vm_end - vma->vm_start;
	pgoff = vma->vm_pgoff &
		((1U << (VFIO_PCI_OFFSET_SHIFT - PAGE_SHIFT)) - 1);
	req_start = pgoff << PAGE_SHIFT;

	/* only the second page of the fault region is mmappable */
	if (req_start < PAGE_SIZE)
		return -EINVAL;

	if (req_start + req_len > phys_len)
		return -EINVAL;

	addr = virt_to_phys(pages);
	vma->vm_private_data = vdev;
	vma->vm_pgoff = (addr >> PAGE_SHIFT) + pgoff;

	ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			      req_len, vma->vm_page_prot);
	return ret;
}

static int vfio_pci_dma_fault_mmap(struct vfio_pci_core_device *vdev,
				   struct vfio_pci_region *region,
				   struct vm_area_struct *vma)
{
	return __vfio_pci_dma_fault_mmap(vdev, region, vma, vdev->fault_pages);
}

static int
vfio_pci_dma_fault_response_mmap(struct vfio_pci_core_device *vdev,
				struct vfio_pci_region *region,
				struct vm_area_struct *vma)
{
	return __vfio_pci_dma_fault_mmap(vdev, region, vma, vdev->fault_response_pages);
}

static int __vfio_pci_dma_fault_add_capability(struct vfio_pci_core_device *vdev,
					     struct vfio_pci_region *region,
					     struct vfio_info_cap *caps,
					     u32 cap_id)
{
	struct vfio_region_info_cap_sparse_mmap *sparse = NULL;
	struct vfio_region_info_cap_fault cap = {
		.header.id = cap_id,
		.header.version = 1,
		.version = 1,
	};
	size_t size = sizeof(*sparse) + sizeof(*sparse->areas);
	int ret;

	ret = vfio_info_add_capability(caps, &cap.header, sizeof(cap));
	if (ret)
		return ret;

	sparse = kzalloc(size, GFP_KERNEL);
	if (!sparse)
		return -ENOMEM;

	sparse->header.id = VFIO_REGION_INFO_CAP_SPARSE_MMAP;
	sparse->header.version = 1;
	sparse->nr_areas = 1;
	sparse->areas[0].offset = PAGE_SIZE;
	sparse->areas[0].size = region->size - PAGE_SIZE;

	ret = vfio_info_add_capability(caps, &sparse->header, size);
	if (ret)
		kfree(sparse);

	return ret;
}

static int vfio_pci_dma_fault_add_capability(struct vfio_pci_core_device *vdev,
					     struct vfio_pci_region *region,
					     struct vfio_info_cap *caps)
{
	return __vfio_pci_dma_fault_add_capability(vdev, region, caps,
						   VFIO_REGION_INFO_CAP_DMA_FAULT);
}

static int
vfio_pci_dma_fault_response_add_capability(struct vfio_pci_core_device *vdev,
					   struct vfio_pci_region *region,
					   struct vfio_info_cap *caps)
{
	return __vfio_pci_dma_fault_add_capability(vdev, region, caps,
						   VFIO_REGION_INFO_CAP_DMA_FAULT_RESPONSE);
}

static const struct vfio_pci_regops vfio_pci_dma_fault_regops = {
	.rw		= vfio_pci_dma_fault_rw,
	.release	= vfio_pci_dma_fault_release,
	.mmap		= vfio_pci_dma_fault_mmap,
	.add_capability = vfio_pci_dma_fault_add_capability,
};

static const struct vfio_pci_regops vfio_pci_dma_fault_response_regops = {
	.rw		= vfio_pci_dma_fault_response_rw,
	.release	= vfio_pci_dma_fault_response_release,
	.mmap		= vfio_pci_dma_fault_response_mmap,
	.add_capability = vfio_pci_dma_fault_response_add_capability,
};

static int
vfio_pci_iommu_dev_fault_handler(struct iommu_fault *fault, void *data)
{
	struct vfio_pci_core_device *vdev = (struct vfio_pci_core_device *)data;
	struct vfio_region_dma_fault *reg =
		(struct vfio_region_dma_fault *)vdev->fault_pages;
	struct iommu_fault *new;
	u32 head, tail, size;
	int ext_irq_index;
	int ret = -EINVAL;

	if (WARN_ON(!reg))
		return ret;

	mutex_lock(&vdev->fault_queue_lock);

	head = reg->head;
	tail = reg->tail;
	size = reg->nb_entries;

	new = (struct iommu_fault *)(vdev->fault_pages + reg->offset +
				     head * reg->entry_size);

	if (CIRC_SPACE(head, tail, size) < 1) {
		ret = -ENOSPC;
		goto unlock;
	}

	*new = *fault;
	reg->head = (head + 1) % size;
	ret = 0;
unlock:
	mutex_unlock(&vdev->fault_queue_lock);
	if (ret)
		return ret;

	ext_irq_index = vfio_pci_get_ext_irq_index(vdev, VFIO_IRQ_TYPE_NESTED,
						   VFIO_IRQ_SUBTYPE_DMA_FAULT);
	if (ext_irq_index < 0)
		return -EINVAL;

	mutex_lock(&vdev->igate);
	if (vdev->ext_irqs[ext_irq_index].trigger)
		eventfd_signal(vdev->ext_irqs[ext_irq_index].trigger, 1);
	mutex_unlock(&vdev->igate);
	return 0;
}

#define DMA_FAULT_RING_LENGTH 512

static int vfio_pci_dma_fault_init(struct vfio_pci_core_device *vdev)
{
	struct vfio_region_dma_fault *header;
	struct iommu_domain *domain;
	size_t size;
	int ret;

	domain = iommu_get_domain_for_dev(&vdev->pdev->dev);
	if (!domain)
		return 0;

	if (!domain->nested)
		return 0;

	mutex_init(&vdev->fault_queue_lock);

	/*
	 * We provision 1 page for the header and space for
	 * DMA_FAULT_RING_LENGTH fault records in the ring buffer.
	 */
	size = ALIGN(sizeof(struct iommu_fault) *
		     DMA_FAULT_RING_LENGTH, PAGE_SIZE) + PAGE_SIZE;

	vdev->fault_pages = kzalloc(size, GFP_KERNEL);
	if (!vdev->fault_pages)
		return -ENOMEM;

	ret = vfio_pci_register_dev_region(vdev,
		VFIO_REGION_TYPE_NESTED,
		VFIO_REGION_SUBTYPE_NESTED_DMA_FAULT,
		&vfio_pci_dma_fault_regops, size,
		VFIO_REGION_INFO_FLAG_READ | VFIO_REGION_INFO_FLAG_WRITE |
		VFIO_REGION_INFO_FLAG_MMAP,
		vdev->fault_pages);
	if (ret)
		goto out;

	header = (struct vfio_region_dma_fault *)vdev->fault_pages;
	header->entry_size = sizeof(struct iommu_fault);
	header->nb_entries = DMA_FAULT_RING_LENGTH;
	header->offset = PAGE_SIZE;

	ret = iommu_register_device_fault_handler(&vdev->pdev->dev,
					vfio_pci_iommu_dev_fault_handler,
					vdev);
	if (ret) /* the dma fault region is freed in vfio_pci_disable() */
		goto out;

	ret = vfio_pci_register_irq(vdev, VFIO_IRQ_TYPE_NESTED,
				    VFIO_IRQ_SUBTYPE_DMA_FAULT,
				    VFIO_IRQ_INFO_EVENTFD);
	if (ret) /* the fault handler is also freed in vfio_pci_disable() */
		goto out;

	return 0;
out:
	kfree(vdev->fault_pages);
	vdev->fault_pages = NULL;
	return ret;
}

static void dma_response_inject(struct work_struct *work)
{
	struct vfio_pci_dma_fault_response_work *rwork =
		container_of(work, struct vfio_pci_dma_fault_response_work, inject);
	struct vfio_region_dma_fault_response *header = rwork->header;
	struct vfio_pci_core_device *vdev = rwork->vdev;
	struct iommu_page_response *resp;
	u32 tail, head, size;

	mutex_lock(&vdev->fault_response_queue_lock);

	tail = header->tail;
	head = header->head;
	size = header->nb_entries;

	while (CIRC_CNT(head, tail, size) >= 1) {
		resp = (struct iommu_page_response *)(vdev->fault_response_pages + header->offset +
						tail * header->entry_size);

		/* TODO: properly handle the return value */
		iommu_page_response(&vdev->pdev->dev, resp);
		header->tail = tail = (tail + 1) % size;
	}
	mutex_unlock(&vdev->fault_response_queue_lock);
}

#define DMA_FAULT_RESPONSE_RING_LENGTH 512

static int vfio_pci_dma_fault_response_init(struct vfio_pci_core_device *vdev)
{
	struct vfio_region_dma_fault_response *header;
	struct iommu_domain *domain;
	int ret;
	size_t size;

	domain = iommu_get_domain_for_dev(&vdev->pdev->dev);
	if (!domain)
		return 0;

	if (!domain->nested)
		return 0;

	mutex_init(&vdev->fault_response_queue_lock);

	/*
	 * We provision 1 page for the header and space for
	 * DMA_FAULT_RING_LENGTH fault records in the ring buffer.
	 */
	size = ALIGN(sizeof(struct iommu_page_response) *
		     DMA_FAULT_RESPONSE_RING_LENGTH, PAGE_SIZE) + PAGE_SIZE;

	vdev->fault_response_pages = kzalloc(size, GFP_KERNEL);
	if (!vdev->fault_response_pages)
		return -ENOMEM;

	ret = vfio_pci_register_dev_region(vdev,
		VFIO_REGION_TYPE_NESTED,
		VFIO_REGION_SUBTYPE_NESTED_DMA_FAULT_RESPONSE,
		&vfio_pci_dma_fault_response_regops, size,
		VFIO_REGION_INFO_FLAG_READ | VFIO_REGION_INFO_FLAG_WRITE |
		VFIO_REGION_INFO_FLAG_MMAP,
		vdev->fault_response_pages);
	if (ret)
		goto out;

	header = (struct vfio_region_dma_fault_response *)vdev->fault_response_pages;
	header->entry_size = sizeof(struct iommu_page_response);
	header->nb_entries = DMA_FAULT_RESPONSE_RING_LENGTH;
	header->offset = PAGE_SIZE;

	vdev->response_work = kzalloc(sizeof(*vdev->response_work), GFP_KERNEL);
	if (!vdev->response_work)
		goto out;
	vdev->response_work->header = header;
	vdev->response_work->vdev = vdev;

	/* launch the thread that will extract the response */
	INIT_WORK(&vdev->response_work->inject, dma_response_inject);
	vdev->dma_fault_response_wq =
		create_singlethread_workqueue("vfio-dma-fault-response");
	if (!vdev->dma_fault_response_wq)
		return -ENOMEM;

	return 0;
out:
	kfree(vdev->fault_response_pages);
	vdev->fault_response_pages = NULL;
	return ret;
}

static int vfio_pci_open_device(struct vfio_device *core_vdev)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);
	struct pci_dev *pdev = vdev->pdev;
	int ret;

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	if (vfio_pci_is_vga(pdev) &&
	    pdev->vendor == PCI_VENDOR_ID_INTEL &&
	    IS_ENABLED(CONFIG_VFIO_PCI_IGD)) {
		ret = vfio_pci_igd_init(vdev);
		if (ret && ret != -ENODEV) {
			pci_warn(pdev, "Failed to setup Intel IGD regions\n");
			vfio_pci_core_disable(vdev);
			return ret;
		}
	}

	ret = vfio_pci_dma_fault_init(vdev);
	if (ret)
		return ret;

	ret = vfio_pci_dma_fault_response_init(vdev);
	if (ret)
		return ret;

	vfio_pci_core_finish_enable(vdev);

	return 0;
}

static const struct vfio_device_ops vfio_pci_ops = {
	.name		= "vfio-pci",
	.open_device	= vfio_pci_open_device,
	.close_device	= vfio_pci_core_close_device,
	.ioctl		= vfio_pci_core_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
};

static int vfio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct vfio_pci_core_device *vdev;
	int ret;

	if (vfio_pci_is_denylisted(pdev))
		return -EINVAL;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev)
		return -ENOMEM;
	vfio_pci_core_init_device(vdev, pdev, &vfio_pci_ops);

	ret = vfio_pci_core_register_device(vdev);
	if (ret)
		goto out_free;
	dev_set_drvdata(&pdev->dev, vdev);
	return 0;

out_free:
	vfio_pci_core_uninit_device(vdev);
	kfree(vdev);
	return ret;
}

static void vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_pci_core_device *vdev = dev_get_drvdata(&pdev->dev);

	vfio_pci_core_unregister_device(vdev);
	vfio_pci_core_uninit_device(vdev);
	kfree(vdev);
}

static int vfio_pci_sriov_configure(struct pci_dev *pdev, int nr_virtfn)
{
	if (!enable_sriov)
		return -ENOENT;

	return vfio_pci_core_sriov_configure(pdev, nr_virtfn);
}

static const struct pci_device_id vfio_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_ANY_ID, PCI_ANY_ID) }, /* match all by default */
	{}
};

MODULE_DEVICE_TABLE(pci, vfio_pci_table);

static struct pci_driver vfio_pci_driver = {
	.name			= "vfio-pci",
	.id_table		= vfio_pci_table,
	.probe			= vfio_pci_probe,
	.remove			= vfio_pci_remove,
	.sriov_configure	= vfio_pci_sriov_configure,
	.err_handler		= &vfio_pci_core_err_handlers,
};

static void __init vfio_pci_fill_ids(void)
{
	char *p, *id;
	int rc;

	/* no ids passed actually */
	if (ids[0] == '\0')
		return;

	/* add ids specified in the module parameter */
	p = ids;
	while ((id = strsep(&p, ","))) {
		unsigned int vendor, device, subvendor = PCI_ANY_ID,
			subdevice = PCI_ANY_ID, class = 0, class_mask = 0;
		int fields;

		if (!strlen(id))
			continue;

		fields = sscanf(id, "%x:%x:%x:%x:%x:%x",
				&vendor, &device, &subvendor, &subdevice,
				&class, &class_mask);

		if (fields < 2) {
			pr_warn("invalid id string \"%s\"\n", id);
			continue;
		}

		rc = pci_add_dynid(&vfio_pci_driver, vendor, device,
				   subvendor, subdevice, class, class_mask, 0);
		if (rc)
			pr_warn("failed to add dynamic id [%04x:%04x[%04x:%04x]] class %#08x/%08x (%d)\n",
				vendor, device, subvendor, subdevice,
				class, class_mask, rc);
		else
			pr_info("add [%04x:%04x[%04x:%04x]] class %#08x/%08x\n",
				vendor, device, subvendor, subdevice,
				class, class_mask);
	}
}

static int __init vfio_pci_init(void)
{
	int ret;
	bool is_disable_vga = true;

#ifdef CONFIG_VFIO_PCI_VGA
	is_disable_vga = disable_vga;
#endif

	vfio_pci_core_set_params(nointxmask, is_disable_vga, disable_idle_d3);

	/* Register and scan for devices */
	ret = pci_register_driver(&vfio_pci_driver);
	if (ret)
		return ret;

	vfio_pci_fill_ids();

	if (disable_denylist)
		pr_warn("device denylist disabled.\n");

	return 0;
}
module_init(vfio_pci_init);

static void __exit vfio_pci_cleanup(void)
{
	pci_unregister_driver(&vfio_pci_driver);
}
module_exit(vfio_pci_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
