#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/dmapool.h>
#include <linux/iopoll.h>
#include <linux/lcm.h>
#include <linux/libata.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/timer.h>
#include <scsi/sas_ata.h>
#include <scsi/libsas.h>
#include <linux/kthread.h>

static int ways = 64;
module_param(ways, int, S_IRUGO);

static int seconds = 4;
module_param(seconds, int, S_IRUGO);

static int completions = 20;
module_param(completions, int, S_IRUGO);


unsigned long long mappings[NR_CPUS];
struct semaphore sem[NR_CPUS];

struct test_data {
	struct semaphore *sem;
	struct device *dev;
};

extern struct device *get_zip_dev(void);

#define COMPLETIONS_SIZE 200

static noinline dma_addr_t test_mapsingle(struct device *dev, void *buf, int size)
{
	dma_addr_t dma_addr = dma_map_single(dev, buf, size, DMA_TO_DEVICE);
	return dma_addr;
}

static noinline void test_unmapsingle(struct device *dev, void *buf, int size, dma_addr_t dma_addr)
{
	dma_unmap_single(dev, dma_addr, size, DMA_TO_DEVICE);
}

static noinline void test_memcpy(void *out, void *in, int size)
{  
	memcpy(out, in, size);
}


static int testthread(void *data)
{  
	unsigned long stop = jiffies +seconds*HZ;
	char *inputs[COMPLETIONS_SIZE];
	char *outputs[COMPLETIONS_SIZE];
	dma_addr_t dma_addr[COMPLETIONS_SIZE];
	int i, cpu = smp_processor_id();
	struct test_data *tdata = data;
	struct semaphore *sem = tdata->sem;
	struct device *dev = tdata->dev;

	for (i = 0; i < completions; i++) {
		inputs[i] = kzalloc(4096, GFP_KERNEL);
		if (!inputs[i])
			return -ENOMEM;
	}

	for (i = 0; i < completions; i++) {
		outputs[i] = kzalloc(4096, GFP_KERNEL);
		if (!outputs[i])
			return -ENOMEM;
	}

	while (time_before(jiffies, stop)) {
		for (i = 0; i < completions; i++) {
			dma_addr[i] = test_mapsingle(dev, inputs[i], 4096);
			test_memcpy(outputs[i], inputs[i], 4096);
		}
		for (i = 0; i < completions; i++) {
			test_unmapsingle(dev, inputs[i], 4096, dma_addr[i]);
		}
		mappings[cpu] += completions;
	}

	for (i = 0; i < completions; i++) {
		kfree(outputs[i]);
		kfree(inputs[i]);
	}

	up(sem);

	return 0;
}  

int smmu_test;


extern ktime_t arm_smmu_cmdq_get_average_time(void);
extern void arm_smmu_cmdq_zero_times(void);
extern void arm_smmu_cmdq_zero_cmpxchg(void);
extern u64 arm_smmu_cmdq_get_tries(void);
extern u64 arm_smmu_cmdq_get_cmpxcgh_fails(void);


void smmu_test_core(int cpus)
{
	struct task_struct *tsk;
	int i;
	unsigned long long total_mappings = 0;
	struct test_data gtdata[NR_CPUS];
	struct device *dev = NULL;
	struct pci_dev *pdev = NULL;
	smmu_test = 1;
	for_each_pci_dev(pdev) {
		struct device *_dev;
		_dev =&pdev->dev;
		if ((pci_pcie_type(pdev) == PCI_EXP_TYPE_RC_END) && (_dev->iommu_group ||_dev->iommu) ) {
			dev = _dev;
			dev_err(dev, "%s found\n", __func__);
			break;
		}
	}

	if (!dev) {
		pr_err("%s could not find dev\n", __func__);
	}


	dev_err(dev, "%s cpus=%d dev=%pS\n", __func__, cpus, dev);


	ways = cpus;
	arm_smmu_cmdq_zero_times();
	arm_smmu_cmdq_zero_cmpxchg();

	if (ways > num_possible_cpus()) {
		ways = num_possible_cpus();
		pr_err("limiting ways to %d\n", ways);
	}

	if (completions > COMPLETIONS_SIZE) {
		completions = COMPLETIONS_SIZE;
		pr_err("limiting completions to %d\n", completions);
	}

	for(i=0;i<ways;i++) {
		struct test_data *tdata = &gtdata[i];
		tdata->sem = &sem[i];
		tdata->dev = dev;
		mappings[i] = 0;
		tsk = kthread_create_on_cpu(testthread, tdata, i,  "map_test");

		if (IS_ERR(tsk))
			printk(KERN_ERR "create test thread failed\n");
		wake_up_process(tsk);
	}

	for(i=0;i<ways;i++) {
		down(&sem[i]);
		total_mappings += mappings[i];
	}
	smmu_test = 0;

	printk(KERN_ERR "finished total_mappings=%llu (per way=%llu) (rate=%llu per second per cpu) ways=%d average=%lld tries=%lld cmpxcgh tries=%lld\n", 
	total_mappings, total_mappings / ways, total_mappings / (seconds* ways), ways,
	arm_smmu_cmdq_get_average_time(),
	arm_smmu_cmdq_get_tries(),
	arm_smmu_cmdq_get_cmpxcgh_fails());

}
EXPORT_SYMBOL(smmu_test_core);


static int __init test_init(void)
{
	int i;

	for(i=0;i<NR_CPUS;i++)
		sema_init(&sem[i], 0);

	return 0;
}
  
static void __exit test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
