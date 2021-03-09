// SPDX-License-Identifier: GPL-2.0-only
/*
 * VFIO PCI driver for POWER9 NPU support (NVLink2 host bus adapter).
 *
 * Copyright (c) 2020, Mellanox Technologies, Ltd.  All rights reserved.
 *
 * Copyright (C) 2018 IBM Corp.  All rights reserved.
 *     Author: Alexey Kardashevskiy <aik@ozlabs.ru>
 *
 * Register an on-GPU RAM region for cacheable access.
 *
 * Derived from original vfio_pci_igd.c:
 * Copyright (C) 2016 Red Hat, Inc.  All rights reserved.
 *	Author: Alex Williamson <alex.williamson@redhat.com>
 */

#include <linux/io.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/vfio.h>
#include <linux/sched/mm.h>
#include <linux/mmu_context.h>
#include <asm/kvm_ppc.h>

#include "vfio_pci_core.h"

#define CREATE_TRACE_POINTS
#include "npu2_trace.h"

EXPORT_TRACEPOINT_SYMBOL_GPL(vfio_pci_npu2_mmap);

struct vfio_pci_npu2_data {
	void *base; /* ATSD register virtual address, for emulated access */
	unsigned long mmio_atsd; /* ATSD physical address */
	unsigned long gpu_tgt; /* TGT address of corresponding GPU RAM */
	unsigned int link_speed; /* The link speed from DT's ibm,nvlink-speed */
};

static size_t vfio_pci_npu2_rw(struct vfio_pci_core_device *vdev,
		char __user *buf, size_t count, loff_t *ppos, bool iswrite)
{
	unsigned int i = VFIO_PCI_OFFSET_TO_INDEX(*ppos) - VFIO_PCI_NUM_REGIONS;
	struct vfio_pci_npu2_data *data = vdev->region[i].data;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;

	if (pos >= vdev->region[i].size)
		return -EINVAL;

	count = min(count, (size_t)(vdev->region[i].size - pos));

	if (iswrite) {
		if (copy_from_user(data->base + pos, buf, count))
			return -EFAULT;
	} else {
		if (copy_to_user(buf, data->base + pos, count))
			return -EFAULT;
	}
	*ppos += count;

	return count;
}

static int vfio_pci_npu2_mmap(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region, struct vm_area_struct *vma)
{
	int ret;
	struct vfio_pci_npu2_data *data = region->data;
	unsigned long req_len = vma->vm_end - vma->vm_start;

	if (req_len != PAGE_SIZE)
		return -EINVAL;

	vma->vm_flags |= VM_PFNMAP;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	ret = remap_pfn_range(vma, vma->vm_start, data->mmio_atsd >> PAGE_SHIFT,
			req_len, vma->vm_page_prot);
	trace_vfio_pci_npu2_mmap(vdev->pdev, data->mmio_atsd, vma->vm_start,
			vma->vm_end - vma->vm_start, ret);

	return ret;
}

static void vfio_pci_npu2_release(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region)
{
	struct vfio_pci_npu2_data *data = region->data;

	memunmap(data->base);
	kfree(data);
}

static int vfio_pci_npu2_add_capability(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region, struct vfio_info_cap *caps)
{
	struct vfio_pci_npu2_data *data = region->data;
	struct vfio_region_info_cap_nvlink2_ssatgt captgt = {
		.header.id = VFIO_REGION_INFO_CAP_NVLINK2_SSATGT,
		.header.version = 1,
		.tgt = data->gpu_tgt
	};
	struct vfio_region_info_cap_nvlink2_lnkspd capspd = {
		.header.id = VFIO_REGION_INFO_CAP_NVLINK2_LNKSPD,
		.header.version = 1,
		.link_speed = data->link_speed
	};
	int ret;

	ret = vfio_info_add_capability(caps, &captgt.header, sizeof(captgt));
	if (ret)
		return ret;

	return vfio_info_add_capability(caps, &capspd.header, sizeof(capspd));
}

static const struct vfio_pci_regops vfio_pci_npu2_regops = {
	.rw = vfio_pci_npu2_rw,
	.mmap = vfio_pci_npu2_mmap,
	.release = vfio_pci_npu2_release,
	.add_capability = vfio_pci_npu2_add_capability,
};

int vfio_pci_ibm_npu2_init(struct vfio_pci_core_device *vdev)
{
	int ret;
	struct vfio_pci_npu2_data *data;
	struct device_node *nvlink_dn;
	u32 nvlink_index = 0, mem_phandle = 0;
	struct pci_dev *npdev = vdev->pdev;
	struct device_node *npu_node = pci_device_to_OF_node(npdev);
	struct pci_controller *hose = pci_bus_to_host(npdev->bus);
	u64 mmio_atsd = 0;
	u64 tgt = 0;
	u32 link_speed = 0xff;

	/*
	 * PCI config space does not tell us about NVLink presense but
	 * platform does, use this.
	 */
	if (!pnv_pci_get_gpu_dev(vdev->pdev))
		return -ENODEV;

	if (of_property_read_u32(npu_node, "memory-region", &mem_phandle))
		return -ENODEV;

	/*
	 * NPU2 normally has 8 ATSD registers (for concurrency) and 6 links
	 * so we can allocate one register per link, using nvlink index as
	 * a key.
	 * There is always at least one ATSD register so as long as at least
	 * NVLink bridge #0 is passed to the guest, ATSD will be available.
	 */
	nvlink_dn = of_parse_phandle(npdev->dev.of_node, "ibm,nvlink", 0);
	if (WARN_ON(of_property_read_u32(nvlink_dn, "ibm,npu-link-index",
			&nvlink_index)))
		return -ENODEV;

	if (of_property_read_u64_index(hose->dn, "ibm,mmio-atsd", nvlink_index,
			&mmio_atsd)) {
		if (of_property_read_u64_index(hose->dn, "ibm,mmio-atsd", 0,
				&mmio_atsd)) {
			dev_warn(&vdev->pdev->dev, "No available ATSD found\n");
			mmio_atsd = 0;
		} else {
			dev_warn(&vdev->pdev->dev,
				 "Using fallback ibm,mmio-atsd[0] for ATSD.\n");
		}
	}

	if (of_property_read_u64(npu_node, "ibm,device-tgt-addr", &tgt)) {
		dev_warn(&vdev->pdev->dev, "No ibm,device-tgt-addr found\n");
		return -EFAULT;
	}

	if (of_property_read_u32(npu_node, "ibm,nvlink-speed", &link_speed)) {
		dev_warn(&vdev->pdev->dev, "No ibm,nvlink-speed found\n");
		return -EFAULT;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->mmio_atsd = mmio_atsd;
	data->gpu_tgt = tgt;
	data->link_speed = link_speed;
	if (data->mmio_atsd) {
		data->base = memremap(data->mmio_atsd, SZ_64K, MEMREMAP_WT);
		if (!data->base) {
			ret = -ENOMEM;
			goto free_exit;
		}
	}

	/*
	 * We want to expose the capability even if this specific NVLink
	 * did not get its own ATSD register because capabilities
	 * belong to VFIO regions and normally there will be ATSD register
	 * assigned to the NVLink bridge.
	 */
	ret = vfio_pci_register_dev_region(vdev,
			PCI_VENDOR_ID_IBM |
			VFIO_REGION_TYPE_PCI_VENDOR_TYPE,
			VFIO_REGION_SUBTYPE_IBM_NVLINK2_ATSD,
			&vfio_pci_npu2_regops,
			data->mmio_atsd ? PAGE_SIZE : 0,
			VFIO_REGION_INFO_FLAG_READ |
			VFIO_REGION_INFO_FLAG_WRITE |
			VFIO_REGION_INFO_FLAG_MMAP,
			data);
	if (ret)
		goto free_exit;

	return 0;

free_exit:
	if (data->base)
		memunmap(data->base);
	kfree(data);

	return ret;
}
