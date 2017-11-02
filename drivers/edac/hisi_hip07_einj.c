/*
 * Copyright (c) 2017 Hisilicon Limited.
 * Error injection facility for testing non-standard errors
 * on Hisilicon HIP07 SoC.
 * This driver should be built as module so that it can be
 * loaded on production kernels for testing purposes.
 *
 * This file may be distributed under the terms of the GNU
 * General Public License version 2.
 */

#include <linux/kobject.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/delay.h>

/* SAS definitions */
#define SAS_REG_BASE		0xa2000000
#define SAS_REG_MAP_SIZE	0x0270

#define SAS_ECC_ERR_MASK0	0x01f0
#define SAS_CFG_ECC_EINJ0_EN	0x0200

/* HNS definitions */
#define HNS_REG_BASE		0xc5080000
#define HNS_REG_MAP_SIZE	0x1400

#define HNS_SRAM_ECC_CHK_EN	0x0428
#define HNS_SRAM_ECC_CHK0	0x042C

/* Prevent EINJ run simultaneously. */
static DEFINE_MUTEX(hip07_einj_mutex);

struct hisi_hw_error {
	u32 val;
	u32 reg_ecc_einj_en_val;
	const char *type;
	void __iomem *regbase;
};

static struct dentry *dfs_inj;
static u32 sas_error_type;
static u32 hns_error_type;

static struct hisi_hw_error sas_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x01,
		.type = "hgc_dqe_ecc\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x20,
		.type = "hgc_iost_ecc\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x200,
		.type = "hgc_itct_ecc\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x200000,
		.type = "hgc_iostl_ecc\n",
	},
	{
		.val = 0x5,
		.reg_ecc_einj_en_val = 0x20000,
		.type = "hgc_itctl_ecc\n",
	},
	{
		.val = 0x6,
		.reg_ecc_einj_en_val = 0x10,
		.type = "hgc_cqe_ecc\n",
	},
	{
		.val = 0x7,
		.reg_ecc_einj_en_val = 0x2000,
		.type = "rxm_mem0_ecc\n",
	},
	{
		.val = 0x8,
		.reg_ecc_einj_en_val = 0x4000,
		.type = "rxm_mem1_ecc\n",
	},
	{
		.val = 0x9,
		.reg_ecc_einj_en_val = 0x8000,
		.type = "rxm_mem2_ecc\n",
	},
	{
		.val = 0xA,
		.reg_ecc_einj_en_val = 0x10000,
		.type = "rxm_mem3_ecc\n",
	},
};

static struct hisi_hw_error hns_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x01,
		.type = "rcb_rx_ebd_sram_ecc\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x02,
		.type = "rcb_rx_ring_sram_ecc\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x04,
		.type = "rcb_tx_ring_sram_ecc\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x08,
		.type = "rcb_tx_fbd_sram_ecc\n",
	},
};

/* common functions */

/* SAS functions */
static int sas_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sas_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   sas_errors[i].val,
			   sas_errors[i].type);
	return 0;
}

static int sas_available_error_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, sas_available_error_type_show, NULL);
}

static const struct file_operations sas_avl_err_type_fops = {
	.open		= sas_available_error_type_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int sas_error_type_get(void *data, u64 *val)
{
	*val = sas_error_type;

	return 0;
}

static int sas_error_type_set(void *data, u64 val)
{
	if (val < 0x1 && val > 0xA) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	sas_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sas_error_type_fops, sas_error_type_get,
			sas_error_type_set, "0x%llx\n");

static int sas_error_inject_set(void *data, u64 val)
{
	if (!sas_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}
	mutex_lock(&hip07_einj_mutex);
	val = sas_errors[sas_error_type - 1].reg_ecc_einj_en_val;
	writel(val, sas_errors[0].regbase + SAS_CFG_ECC_EINJ0_EN);
	writel(0x101, sas_errors[0].regbase + SAS_ECC_ERR_MASK0);
	mutex_unlock(&hip07_einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sas_error_inject_fops, NULL,
			sas_error_inject_set, "%llu\n");

/* HNS functions */
static int hns_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hns_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   hns_errors[i].val,
			   hns_errors[i].type);
		return 0;
}

static int hns_available_error_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, hns_available_error_type_show, NULL);
}

static const struct file_operations hns_avl_err_type_fops = {
	.open           = hns_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int hns_error_type_get(void *data, u64 *val)
{
	*val = hns_error_type;
	return 0;
}

static int hns_error_type_set(void *data, u64 val)
{
	if (val < 0x1 && val > 0xB) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}
	hns_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hns_error_type_fops, hns_error_type_get,
			hns_error_type_set, "0x%llx\n");

static int hns_error_inject_set(void *data, u64 val)
{
	if (!hns_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}
	mutex_lock(&hip07_einj_mutex);
	val = hns_errors[hns_error_type - 1].reg_ecc_einj_en_val;
	writel(val, hns_errors[0].regbase + HNS_SRAM_ECC_CHK_EN);
	writel(0x11, hns_errors[0].regbase + HNS_SRAM_ECC_CHK0);
	mutex_unlock(&hip07_einj_mutex);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hns_error_inject_fops, NULL,
			hns_error_inject_set, "%llu\n");

/* common structures and functions */
struct dfs_node {
	char *name;
	struct dentry *d;
	const struct file_operations *avl_err_types_fops;
	const struct file_operations *einj_type_fops;
	const struct file_operations *einj_fops;
};

static struct dfs_node dfs_dirs[] = {
	{
		.name = "sas",
		.avl_err_types_fops = &sas_avl_err_type_fops,
		.einj_type_fops = &sas_error_type_fops,
		.einj_fops = &sas_error_inject_fops,
	},
	{
		.name = "hns",
		.avl_err_types_fops = &hns_avl_err_type_fops,
		.einj_type_fops = &hns_error_type_fops,
		.einj_fops = &hns_error_inject_fops,
	},
};

static int __init init_hip07_einj(void)
{
	int i;
	struct dentry *fentry;

	/* map hw register base */
	sas_errors[0].regbase = ioremap_nocache(SAS_REG_BASE,
						SAS_REG_MAP_SIZE);
	if (sas_errors[0].regbase == NULL) {
		pr_err("%s: cannot remap SAS registers\n", __func__);
		return -ENXIO;
	}

	hns_errors[0].regbase = ioremap_nocache(HNS_REG_BASE, HNS_REG_MAP_SIZE);
	if (hns_errors[0].regbase == NULL) {
		pr_err("%s: cannot remap HNS registers\n", __func__);
		return -ENXIO;
	}

	dfs_inj = debugfs_create_dir("hip07-einj", NULL);
	if (!dfs_inj) {
		iounmap(hns_errors[0].regbase);
		iounmap(sas_errors[0].regbase);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(dfs_dirs); i++) {
		dfs_dirs[i].d = debugfs_create_dir(dfs_dirs[i].name, dfs_inj);
		if (dfs_dirs[i].d == NULL)
			goto cleanup;

		fentry = debugfs_create_file("available_error_type", 0400,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].avl_err_types_fops);
		if (!fentry)
			goto cleanup;

		fentry = debugfs_create_file("error_type", 0600,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].einj_type_fops);
		if (!fentry)
			goto cleanup;

		fentry = debugfs_create_file("error_inject", 0200,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].einj_fops);
		if (!fentry)
			goto cleanup;
	}
	return 0;

cleanup:
	pr_err("%s failed\n", __func__);
	debugfs_remove_recursive(dfs_inj);
	dfs_inj = NULL;

	iounmap(hns_errors[0].regbase);
	iounmap(sas_errors[0].regbase);
	return -ENOMEM;
}

static void __exit exit_hip07_einj(void)
{
	debugfs_remove_recursive(dfs_inj);
	dfs_inj = NULL;

	iounmap(hns_errors[0].regbase);
	iounmap(sas_errors[0].regbase);
}
module_init(init_hip07_einj);
module_exit(exit_hip07_einj);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shiju Jose <shiju.jose@huawei.com>");
MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("HISI HIP07 error injection facility for testing non-standard errors");
