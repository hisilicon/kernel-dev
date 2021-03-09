// SPDX-License-Identifier: GPL-2.0-only
/*
 * VFIO PCI NVIDIA NVLink2 GPUs support.
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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/vfio.h>
#include <linux/sched/mm.h>
#include <linux/mmu_context.h>
#include <asm/kvm_ppc.h>

#include "vfio_pci_core.h"
#include "nvlink2gpu_vfio_pci.h"

#define CREATE_TRACE_POINTS
#include "nvlink2gpu_trace.h"

#define DRIVER_VERSION  "0.1"
#define DRIVER_AUTHOR   "Alexey Kardashevskiy <aik@ozlabs.ru>"
#define DRIVER_DESC     "NVLINK2GPU VFIO PCI - User Level meta-driver for NVIDIA NVLink2 GPUs"

EXPORT_TRACEPOINT_SYMBOL_GPL(vfio_pci_nvgpu_mmap_fault);
EXPORT_TRACEPOINT_SYMBOL_GPL(vfio_pci_nvgpu_mmap);

struct vfio_pci_nvgpu_data {
	unsigned long gpu_hpa; /* GPU RAM physical address */
	unsigned long gpu_tgt; /* TGT address of corresponding GPU RAM */
	unsigned long useraddr; /* GPU RAM userspace address */
	unsigned long size; /* Size of the GPU RAM window (usually 128GB) */
	struct mm_struct *mm;
	struct mm_iommu_table_group_mem_t *mem; /* Pre-registered RAM descr. */
	struct pci_dev *gpdev;
	struct notifier_block group_notifier;
};

struct nv_vfio_pci_device {
	struct vfio_pci_core_device	vdev;
};

static size_t vfio_pci_nvgpu_rw(struct vfio_pci_core_device *vdev,
		char __user *buf, size_t count, loff_t *ppos, bool iswrite)
{
	unsigned int i = VFIO_PCI_OFFSET_TO_INDEX(*ppos) - VFIO_PCI_NUM_REGIONS;
	struct vfio_pci_nvgpu_data *data = vdev->region[i].data;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;
	loff_t posaligned = pos & PAGE_MASK, posoff = pos & ~PAGE_MASK;
	size_t sizealigned;
	void __iomem *ptr;

	if (pos >= vdev->region[i].size)
		return -EINVAL;

	count = min(count, (size_t)(vdev->region[i].size - pos));

	/*
	 * We map only a bit of GPU RAM for a short time instead of mapping it
	 * for the guest lifetime as:
	 *
	 * 1) we do not know GPU RAM size, only aperture which is 4-8 times
	 *    bigger than actual RAM size (16/32GB RAM vs. 128GB aperture);
	 * 2) mapping GPU RAM allows CPU to prefetch and if this happens
	 *    before NVLink bridge is reset (which fences GPU RAM),
	 *    hardware management interrupts (HMI) might happen, this
	 *    will freeze NVLink bridge.
	 *
	 * This is not fast path anyway.
	 */
	sizealigned = ALIGN(posoff + count, PAGE_SIZE);
	ptr = ioremap_cache(data->gpu_hpa + posaligned, sizealigned);
	if (!ptr)
		return -EFAULT;

	if (iswrite) {
		if (copy_from_user(ptr + posoff, buf, count))
			count = -EFAULT;
		else
			*ppos += count;
	} else {
		if (copy_to_user(buf, ptr + posoff, count))
			count = -EFAULT;
		else
			*ppos += count;
	}

	iounmap(ptr);

	return count;
}

static void vfio_pci_nvgpu_release(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region)
{
	struct vfio_pci_nvgpu_data *data = region->data;
	long ret;

	/* If there were any mappings at all... */
	if (data->mm) {
		if (data->mem) {
			ret = mm_iommu_put(data->mm, data->mem);
			WARN_ON(ret);
		}

		mmdrop(data->mm);
	}

	vfio_unregister_notifier(&data->gpdev->dev, VFIO_GROUP_NOTIFY,
			&data->group_notifier);

	pnv_npu2_unmap_lpar_dev(data->gpdev);

	kfree(data);
}

static vm_fault_t vfio_pci_nvgpu_mmap_fault(struct vm_fault *vmf)
{
	vm_fault_t ret;
	struct vm_area_struct *vma = vmf->vma;
	struct vfio_pci_region *region = vma->vm_private_data;
	struct vfio_pci_nvgpu_data *data = region->data;
	unsigned long vmf_off = (vmf->address - vma->vm_start) >> PAGE_SHIFT;
	unsigned long nv2pg = data->gpu_hpa >> PAGE_SHIFT;
	unsigned long vm_pgoff = vma->vm_pgoff &
		((1U << (VFIO_PCI_OFFSET_SHIFT - PAGE_SHIFT)) - 1);
	unsigned long pfn = nv2pg + vm_pgoff + vmf_off;

	ret = vmf_insert_pfn(vma, vmf->address, pfn);
	trace_vfio_pci_nvgpu_mmap_fault(data->gpdev, pfn << PAGE_SHIFT,
			vmf->address, ret);

	return ret;
}

static const struct vm_operations_struct vfio_pci_nvgpu_mmap_vmops = {
	.fault = vfio_pci_nvgpu_mmap_fault,
};

static int vfio_pci_nvgpu_mmap(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region, struct vm_area_struct *vma)
{
	int ret;
	struct vfio_pci_nvgpu_data *data = region->data;

	if (data->useraddr)
		return -EPERM;

	if (vma->vm_end - vma->vm_start > data->size)
		return -EINVAL;

	vma->vm_private_data = region;
	vma->vm_flags |= VM_PFNMAP;
	vma->vm_ops = &vfio_pci_nvgpu_mmap_vmops;

	/*
	 * Calling mm_iommu_newdev() here once as the region is not
	 * registered yet and therefore right initialization will happen now.
	 * Other places will use mm_iommu_find() which returns
	 * registered @mem and does not go gup().
	 */
	data->useraddr = vma->vm_start;
	data->mm = current->mm;

	mmgrab(data->mm);
	ret = (int) mm_iommu_newdev(data->mm, data->useraddr,
			vma_pages(vma), data->gpu_hpa, &data->mem);

	trace_vfio_pci_nvgpu_mmap(vdev->pdev, data->gpu_hpa, data->useraddr,
			vma->vm_end - vma->vm_start, ret);

	return ret;
}

static int vfio_pci_nvgpu_add_capability(struct vfio_pci_core_device *vdev,
		struct vfio_pci_region *region, struct vfio_info_cap *caps)
{
	struct vfio_pci_nvgpu_data *data = region->data;
	struct vfio_region_info_cap_nvlink2_ssatgt cap = {
		.header.id = VFIO_REGION_INFO_CAP_NVLINK2_SSATGT,
		.header.version = 1,
		.tgt = data->gpu_tgt
	};

	return vfio_info_add_capability(caps, &cap.header, sizeof(cap));
}

static const struct vfio_pci_regops vfio_pci_nvgpu_regops = {
	.rw = vfio_pci_nvgpu_rw,
	.release = vfio_pci_nvgpu_release,
	.mmap = vfio_pci_nvgpu_mmap,
	.add_capability = vfio_pci_nvgpu_add_capability,
};

static int vfio_pci_nvgpu_group_notifier(struct notifier_block *nb,
		unsigned long action, void *opaque)
{
	struct kvm *kvm = opaque;
	struct vfio_pci_nvgpu_data *data = container_of(nb,
			struct vfio_pci_nvgpu_data,
			group_notifier);

	if (action == VFIO_GROUP_NOTIFY_SET_KVM && kvm &&
			pnv_npu2_map_lpar_dev(data->gpdev,
				kvm->arch.lpid, MSR_DR | MSR_PR))
		return NOTIFY_BAD;

	return NOTIFY_OK;
}

static int
vfio_pci_nvidia_v100_nvlink2_init(struct vfio_pci_core_device *vdev)
{
	int ret;
	u64 reg[2];
	u64 tgt = 0;
	struct device_node *npu_node, *mem_node;
	struct pci_dev *npu_dev;
	struct vfio_pci_nvgpu_data *data;
	uint32_t mem_phandle = 0;
	unsigned long events = VFIO_GROUP_NOTIFY_SET_KVM;

	/*
	 * PCI config space does not tell us about NVLink presense but
	 * platform does, use this.
	 */
	npu_dev = pnv_pci_get_npu_dev(vdev->pdev, 0);
	if (!npu_dev)
		return -ENODEV;

	npu_node = pci_device_to_OF_node(npu_dev);
	if (!npu_node)
		return -EINVAL;

	if (of_property_read_u32(npu_node, "memory-region", &mem_phandle))
		return -ENODEV;

	mem_node = of_find_node_by_phandle(mem_phandle);
	if (!mem_node)
		return -EINVAL;

	if (of_property_read_variable_u64_array(mem_node, "reg", reg,
				ARRAY_SIZE(reg), ARRAY_SIZE(reg)) !=
			ARRAY_SIZE(reg))
		return -EINVAL;

	if (of_property_read_u64(npu_node, "ibm,device-tgt-addr", &tgt)) {
		dev_warn(&vdev->pdev->dev, "No ibm,device-tgt-addr found\n");
		return -EFAULT;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->gpu_hpa = reg[0];
	data->gpu_tgt = tgt;
	data->size = reg[1];

	dev_dbg(&vdev->pdev->dev, "%lx..%lx\n", data->gpu_hpa,
			data->gpu_hpa + data->size - 1);

	data->gpdev = vdev->pdev;
	data->group_notifier.notifier_call = vfio_pci_nvgpu_group_notifier;

	ret = vfio_register_notifier(&data->gpdev->dev, VFIO_GROUP_NOTIFY,
			&events, &data->group_notifier);
	if (ret)
		goto free_exit;

	/*
	 * We have just set KVM, we do not need the listener anymore.
	 * Also, keeping it registered means that if more than one GPU is
	 * assigned, we will get several similar notifiers notifying about
	 * the same device again which does not help with anything.
	 */
	vfio_unregister_notifier(&data->gpdev->dev, VFIO_GROUP_NOTIFY,
			&data->group_notifier);

	ret = vfio_pci_register_dev_region(vdev,
			PCI_VENDOR_ID_NVIDIA | VFIO_REGION_TYPE_PCI_VENDOR_TYPE,
			VFIO_REGION_SUBTYPE_NVIDIA_NVLINK2_RAM,
			&vfio_pci_nvgpu_regops,
			data->size,
			VFIO_REGION_INFO_FLAG_READ |
			VFIO_REGION_INFO_FLAG_WRITE |
			VFIO_REGION_INFO_FLAG_MMAP,
			data);
	if (ret)
		goto free_exit;

	return 0;
free_exit:
	kfree(data);

	return ret;
}

static void nvlink2gpu_vfio_pci_release(void *device_data)
{
	struct vfio_pci_core_device *vdev = device_data;

	mutex_lock(&vdev->reflck->lock);
	if (!(--vdev->refcnt)) {
		vfio_pci_vf_token_user_add(vdev, -1);
		vfio_pci_core_spapr_eeh_release(vdev);
		vfio_pci_core_disable(vdev);
	}
	mutex_unlock(&vdev->reflck->lock);

	module_put(THIS_MODULE);
}

static int nvlink2gpu_vfio_pci_open(void *device_data)
{
	struct vfio_pci_core_device *vdev = device_data;
	int ret = 0;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	mutex_lock(&vdev->reflck->lock);

	if (!vdev->refcnt) {
		ret = vfio_pci_core_enable(vdev);
		if (ret)
			goto error;

		ret = vfio_pci_nvidia_v100_nvlink2_init(vdev);
		if (ret && ret != -ENODEV) {
			pci_warn(vdev->pdev,
				 "Failed to setup NVIDIA NV2 RAM region\n");
			vfio_pci_core_disable(vdev);
			goto error;
		}
		ret = 0;
		vfio_pci_probe_mmaps(vdev);
		vfio_pci_core_spapr_eeh_open(vdev);
		vfio_pci_vf_token_user_add(vdev, 1);
	}
	vdev->refcnt++;
error:
	mutex_unlock(&vdev->reflck->lock);
	if (ret)
		module_put(THIS_MODULE);
	return ret;
}

static const struct vfio_device_ops nvlink2gpu_vfio_pci_ops = {
	.name		= "nvlink2gpu-vfio-pci",
	.open		= nvlink2gpu_vfio_pci_open,
	.release	= nvlink2gpu_vfio_pci_release,
	.ioctl		= vfio_pci_core_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
};

static int nvlink2gpu_vfio_pci_probe(struct pci_dev *pdev,
		const struct pci_device_id *id)
{
	struct nv_vfio_pci_device *nvdev;
	int ret;

	nvdev = kzalloc(sizeof(*nvdev), GFP_KERNEL);
	if (!nvdev)
		return -ENOMEM;

	ret = vfio_pci_core_register_device(&nvdev->vdev, pdev,
			&nvlink2gpu_vfio_pci_ops);
	if (ret)
		goto out_free;

	return 0;

out_free:
	kfree(nvdev);
	return ret;
}

static void nvlink2gpu_vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_device *vdev = dev_get_drvdata(&pdev->dev);
	struct vfio_pci_core_device *core_vpdev = vfio_device_data(vdev);
	struct nv_vfio_pci_device *nvdev;

	nvdev = container_of(core_vpdev, struct nv_vfio_pci_device, vdev);

	vfio_pci_core_unregister_device(core_vpdev);
	kfree(nvdev);
}

static const struct pci_device_id nvlink2gpu_vfio_pci_table[] = {
	{ PCI_VDEVICE(NVIDIA, 0x1DB1) }, /* GV100GL-A NVIDIA Tesla V100-SXM2-16GB */
	{ PCI_VDEVICE(NVIDIA, 0x1DB5) }, /* GV100GL-A NVIDIA Tesla V100-SXM2-32GB */
	{ PCI_VDEVICE(NVIDIA, 0x1DB8) }, /* GV100GL-A NVIDIA Tesla V100-SXM3-32GB */
	{ PCI_VDEVICE(NVIDIA, 0x1DF5) }, /* GV100GL-B NVIDIA Tesla V100-SXM2-16GB */
	{ 0, }
};

static struct pci_driver nvlink2gpu_vfio_pci_driver = {
	.name			= "nvlink2gpu-vfio-pci",
	.id_table		= nvlink2gpu_vfio_pci_table,
	.probe			= nvlink2gpu_vfio_pci_probe,
	.remove			= nvlink2gpu_vfio_pci_remove,
#ifdef CONFIG_PCI_IOV
	.sriov_configure	= vfio_pci_core_sriov_configure,
#endif
	.err_handler		= &vfio_pci_core_err_handlers,
};

#ifdef CONFIG_VFIO_PCI_DRIVER_COMPAT
struct pci_driver *get_nvlink2gpu_vfio_pci_driver(struct pci_dev *pdev)
{
	if (pci_match_id(nvlink2gpu_vfio_pci_driver.id_table, pdev))
		return &nvlink2gpu_vfio_pci_driver;
	return NULL;
}
EXPORT_SYMBOL_GPL(get_nvlink2gpu_vfio_pci_driver);
#endif

module_pci_driver(nvlink2gpu_vfio_pci_driver);

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
