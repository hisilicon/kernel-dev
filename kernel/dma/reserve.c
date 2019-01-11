// SPDX-License-Identifier: GPL-2.0+

#include <linux/dma-reserve.h>
#include <linux/dma-mapping.h>
#include <linux/memblock.h>

static phys_addr_t noiommu_size;
static phys_addr_t noiommu_base;
static phys_addr_t each_size = 1;
static DEFINE_IDR(mem_idr);

static int __init early_noiommu(char *p)
{
	phys_addr_t noiommu_size_cmdline = -1;
	phys_addr_t each_size_cmdline = -1;
	if (!p)
		return -EINVAL;

	noiommu_size_cmdline = memparse(p, &p);
	if (noiommu_size_cmdline != -1)
		noiommu_size = noiommu_size_cmdline;
	if (*p != '/')
		return 0;
	each_size_cmdline = memparse(p + 1, &p);
	if (each_size_cmdline != -1)
		each_size = each_size_cmdline;
	return 0;
}
early_param("noiommu", early_noiommu);

static phys_addr_t __init noiommu_mem_reserve(phys_addr_t size)
{
	phys_addr_t phys;

	phys = memblock_alloc_base(size, PAGE_SIZE, MEMBLOCK_ALLOC_ANYWHERE);
	memblock_free(phys, size);
	memblock_remove(phys, size);

	return phys;
}

void __init dma_memory_reserve(void)
{
	if (noiommu_size) {
		noiommu_base = noiommu_mem_reserve(noiommu_size);
		printk(KERN_INFO "NOIOMMU Reserved %ld MiB at %pa\n",
		       (unsigned long)noiommu_size / SZ_1M, &noiommu_base);
	}
}

int dma_memory_declear(struct device *dev)
{
	int id;
	int max = noiommu_size / each_size;

	id = idr_alloc(&mem_idr, NULL, 0, 0, GFP_KERNEL);
	if (id < 0)
		return id;

	if (id < max) {
		phys_addr_t phys =  noiommu_base + id * each_size;
		dma_declare_coherent_memory(dev, phys, phys,
					    each_size,
					    DMA_MEMORY_EXCLUSIVE);
	}
	return id;

}
EXPORT_SYMBOL_GPL(dma_memory_declear);

void dma_memory_release(struct device *dev, int id)
{
	int max = noiommu_size / each_size;

	if (id < max) {
		dma_release_declared_memory(dev);
		idr_remove(&mem_idr, id);
	}
}
EXPORT_SYMBOL_GPL(dma_memory_release);
