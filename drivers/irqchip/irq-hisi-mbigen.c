/*
 * Copyright (C) 2014 Hisilicon Limited, All Rights Reserved.
 * Author: Yun Wu <wuyun.wu@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/mbi.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include "irqchip.h"

/* Register offsets */
#define MG_IRQ_TYPE		0x0
#define MG_IRQ_CLEAR		0x100
#define MG_IRQ_STATUS		0x200
#define MG_MSG_DATA		0x300

/* The gap between normal and extended pin region */
#define MG_EXT_OFST		0x10

/* Max number of interrupts supported */
#define MG_NR_IRQS		640

/* Number of mbigens supported in one chip */
#define MG_NR			6

/*compose the device id. This value should not be 0*/
#define MBIGEN_DEVID(nid)  (((nid)+1) << 4)

struct mbigen_node {
	struct list_head	entry;
	struct mbigen		*mbigen;
	struct device_node	*source;
	unsigned int		irq;
	unsigned int		nr_irqs;
};

struct mbigen {
	raw_spinlock_t		lock;
	struct list_head	entry;
	struct mbigen_chip	*chip;
	unsigned int		nid;
	unsigned int		irqs_used;
	struct list_head	nodes;
};

struct mbigen_chip {
	raw_spinlock_t		lock;
	struct list_head	entry;
	struct device		*dev;
	struct device_node	*node;
	void __iomem		*base;
	struct irq_domain	*domain;
	struct list_head	nodes;
};

static LIST_HEAD(mbigen_chips);
static DEFINE_SPINLOCK(mbigen_lock);

static inline unsigned int mbigen_get_nr_irqs(unsigned int nid)
{
	return 1 << (max_t(unsigned int, nid, 3) + 3);
}

static unsigned int mbigen_get_nid(unsigned long hwirq)
{
	unsigned int nid = min_t(unsigned long, hwirq >> 6, 3);

	if (hwirq >> 8)
		nid += min_t(unsigned long, hwirq >> 7, 3) - 1;

	return nid;
}

static unsigned long mbigen_get_hwirq_base(unsigned int nid)
{
	return (nid < 5) ? (nid << 6) : ((nid + 1) << 6);
}

static void mbigen_free_node(struct mbigen_node *mgn)
{
	raw_spin_lock(&mgn->mbigen->lock);
	list_del(&mgn->entry);
	raw_spin_unlock(&mgn->mbigen->lock);
	kfree(mgn);
}

static struct mbigen_node *mbigen_create_node(struct mbigen *mbigen,
					      struct device_node *node,
					      unsigned int virq,
					      unsigned int nr_irqs)
{
	struct mbigen_node *mgn;

	mgn = kzalloc(sizeof(*mgn), GFP_KERNEL);
	if (!mgn)
		return NULL;

	INIT_LIST_HEAD(&mgn->entry);
	mgn->mbigen = mbigen;
	mgn->source = node;
	mgn->irq = virq;
	mgn->nr_irqs = nr_irqs;

	raw_spin_lock(&mbigen->lock);
	list_add(&mgn->entry, &mbigen->nodes);
	raw_spin_unlock(&mbigen->lock);

	return mgn;
}

static void mbigen_free(struct mbigen *mbigen)
{
	struct mbigen_node *mgn, *tmp;

	list_for_each_entry_safe(mgn, tmp, &mbigen->nodes, entry)
		mbigen_free_node(mgn);

	kfree(mbigen);
}

static struct mbigen *mbigen_get_device(struct mbigen_chip *chip,
											unsigned int nid)
{
	struct mbigen *tmp, *mbigen;
	bool found = false;

	if (nid >= MG_NR) {
		pr_warn("MBIGEN: Device ID exceeds max number!\n");
		return NULL;
	}

	/*
	 * Stop working if no memory available, even if we could
	 * get what we want.
	 */
	tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
	if (!tmp)
		return NULL;

	raw_spin_lock(&chip->lock);

	list_for_each_entry(mbigen, &chip->nodes, entry) {
		if (mbigen->nid == nid) {
			found = true;
			break;
		}
	}

	if (!found) {
		tmp->chip = chip;
		tmp->nid = nid;
		raw_spin_lock_init(&tmp->lock);
		INIT_LIST_HEAD(&tmp->entry);
		INIT_LIST_HEAD(&tmp->nodes);

		list_add(&tmp->entry, &chip->nodes);

		mbigen = tmp;
		tmp = NULL;
	}

	raw_spin_unlock(&chip->lock);
	kfree(tmp);

	return mbigen;
}

/*
 * MBI operations
 */

static void mbigen_write_msg(struct mbi_desc *desc, struct mbi_msg *msg, bool enable)
{
	struct mbigen_node *mgn = desc->data;
	struct mbigen *mbigen = mgn->mbigen;
	void __iomem *addr;
	u32 value, dev_id, dev_id_old;

	if (!enable)
		return;

	dev_id = (msg->data >> 16) & 0xffff;

	if (!dev_id) {
		pr_warn("Warning: Mebigen device id should not be 0\n");
		return;
	}

	addr = mbigen->chip->base + MG_MSG_DATA + mbigen->nid * 4;
	if (mbigen->nid > 3)
		addr += MG_EXT_OFST;

	value = readl_relaxed(addr);
	dev_id_old = (value >> 16) & 0xffff;

	if (dev_id_old && (dev_id_old != dev_id))
		pr_warn("Warning: Mbigen device id already exist.Old:0x%x,new:0x%x\n"
			, dev_id_old, dev_id);

	writel_relaxed(msg->data & ~0xffff, addr);

}

static struct mbi_ops mbigen_mbi_ops = {
	.write_msg	= mbigen_write_msg,
};

/*
 * Interrupt controller operations
 */

static void mbigen_ack_irq(struct irq_data *d)
{
	struct mbigen_chip *chip = d->domain->host_data;
	u32 ofst = d->hwirq / 32 * 4;
	u32 mask = 1 << (d->hwirq % 32);

	writel_relaxed(mask, chip->base + MG_IRQ_CLEAR + ofst);
}

static int mbigen_set_type(struct irq_data *d, unsigned int type)
{
	struct mbigen_chip *chip = d->domain->host_data;
	u32 ofst = d->hwirq / 32 * 4;
	u32 mask = 1 << (d->hwirq % 32);
	u32 val;

	if (type != IRQ_TYPE_LEVEL_HIGH && type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	raw_spin_lock(&chip->lock);
	val = readl_relaxed(chip->base + MG_IRQ_TYPE + ofst);

	if (type == IRQ_TYPE_LEVEL_HIGH)
		val |= mask;
	else if (type == IRQ_TYPE_EDGE_RISING)
		val &= ~mask;

	writel_relaxed(val, chip->base + MG_IRQ_TYPE + ofst);
	raw_spin_unlock(&chip->lock);

	return 0;
}

static int mbigen_set_affinity(struct irq_data *data,
			       const struct cpumask *mask,
			       bool force)
{
	int ret;

	ret = irq_chip_set_affinity_parent(data, mask, force);
	return ret;
}

void mbigen_mask_irq(struct irq_data *data)
{
	irq_chip_mask_parent(data);
}

void mbigen_unmask_irq(struct irq_data *data)
{
	mbigen_ack_irq(data);
	irq_chip_unmask_parent(data);
}

static struct irq_chip mbigen_chip = {
	.name			= "Hisilicon MBIGEN",
	.irq_mask		= mbigen_mask_irq,
	.irq_unmask		= mbigen_unmask_irq,
	.irq_ack		= mbigen_ack_irq,
	.irq_eoi		= irq_chip_eoi_parent,
	.irq_set_affinity	= mbigen_set_affinity,
	.irq_set_type		= mbigen_set_type,
};

/*
 * Interrupt domain operations
 */

static int mbigen_domain_xlate(struct irq_domain *d,
			       struct device_node *controller,
			       const u32 *intspec, unsigned int intsize,
			       unsigned long *out_hwirq,
			       unsigned int *out_type)
{
	if (d->of_node != controller)
		return -EINVAL;

	if (intsize < 2)
		return -EINVAL;

	*out_hwirq = intspec[0];
	*out_type = intspec[1] & IRQ_TYPE_SENSE_MASK;

	return 0;
}

static int mbigen_domain_alloc(struct irq_domain *domain, unsigned int virq,
			       unsigned int nr_irqs, void *arg)
{
	struct of_phandle_args *irq_data = arg;
	irq_hw_number_t hwirq = irq_data->args[0];
	struct mbigen_chip *chip = domain->host_data;
	unsigned int nid = mbigen_get_nid(hwirq);
	struct mbigen *mbigen;
	struct mbigen_node *mgn;
	struct mbi_desc *mbi;
	unsigned dev_id, type;
	int ret, i;

	/* OF style allocation, one interrupt at a time */
	WARN_ON(nr_irqs != 1);

	mbigen = mbigen_get_device(chip, nid);
	if (!mbigen)
		return -ENODEV;

	mgn = mbigen_create_node(mbigen, irq_data->np, virq, nr_irqs);
	if (!mgn)
		return -ENOMEM;

	dev_id = MBIGEN_DEVID(nid);
	mbi = mbi_alloc_desc(chip->dev, &mbigen_mbi_ops, dev_id, mbigen_get_nr_irqs(nid),
			     hwirq - mbigen_get_hwirq_base(nid), mgn);
	if (!mbi) {
		mbigen_free_node(mgn);
		return -ENOMEM;
	}

	irq_domain_set_hwirq_and_chip(domain, virq, hwirq, &mbigen_chip, mgn);

	ret = irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, mbi);
	if (ret < 0)
		return ret;

	type = irq_data->args[1] & IRQ_TYPE_SENSE_MASK;

	if (type == IRQ_TYPE_LEVEL_HIGH) {
		for (i = 0; i < nr_irqs; i++) {
			irq_set_handler((virq + i), handle_level_irq);
		}
	}

	return 0;
}

static void mbigen_domain_free(struct irq_domain *domain, unsigned int virq,
			       unsigned int nr_irqs)
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct mbigen_node *mgn = irq_data_get_irq_chip_data(d);

	WARN_ON(virq != mgn->irq);
	WARN_ON(nr_irqs != mgn->nr_irqs);
	mbigen_free_node(mgn);
	irq_domain_free_irqs_parent(domain, virq, nr_irqs);
}

static struct irq_domain_ops mbigen_domain_ops = {
	.xlate		= mbigen_domain_xlate,
	.alloc		= mbigen_domain_alloc,
	.free		= mbigen_domain_free,
};

/*
 * Early initialization as an interrupt controller
 */

static int __init mbigen_of_init(struct device_node *node,
				 struct device_node *parent)
{
	struct mbigen_chip *chip;
	struct device_node *msi_parent;
	int err;

	msi_parent = of_parse_phandle(node, "msi-parent", 0);
	if (msi_parent)
		parent = msi_parent;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->base = of_iomap(node, 0);
	if (!chip->base) {
		pr_err("%s: Registers not found.\n", node->full_name);
		err = -ENXIO;
		goto free_chip;
	}

	chip->domain = irq_domain_add_hierarchy(irq_find_host(parent),
						0, MG_NR_IRQS, node,
						&mbigen_domain_ops, chip);
	if (!chip->domain) {
		err = -ENOMEM;
		goto unmap_reg;
	}

	chip->node = node;
	raw_spin_lock_init(&chip->lock);
	INIT_LIST_HEAD(&chip->entry);
	INIT_LIST_HEAD(&chip->nodes);

	pr_info("MBIGEN: %s\n", node->full_name);

	spin_lock(&mbigen_lock);
	list_add(&chip->entry, &mbigen_chips);
	spin_unlock(&mbigen_lock);

	return 0;

unmap_reg:
	iounmap(chip->base);
free_chip:
	kfree(chip);
	pr_info("MBIGEN: failed probing %s\n", node->full_name);
	return err;
}
IRQCHIP_DECLARE(hisi_mbigen, "hisilicon,mbi-gen", mbigen_of_init);

/*
 * Late initialization as a platform device
 */

static int mbigen_probe(struct platform_device *pdev)
{
	struct mbigen_chip *tmp, *chip = NULL;
	struct device *dev = &pdev->dev;

	spin_lock(&mbigen_lock);
	list_for_each_entry(tmp, &mbigen_chips, entry) {
		if (tmp->node == dev->of_node) {
			chip = tmp;
			break;
		}
	}
	spin_unlock(&mbigen_lock);

	if (!chip)
		return -ENODEV;

	chip->dev = dev;
	platform_set_drvdata(pdev, chip);

	return 0;
}

static int mbigen_remove(struct platform_device *pdev)
{
	struct mbigen_chip *chip = platform_get_drvdata(pdev);
	struct mbigen *mbigen, *tmp;

	spin_lock(&mbigen_lock);
	list_del(&chip->entry);
	spin_unlock(&mbigen_lock);

	list_for_each_entry_safe(mbigen, tmp, &chip->nodes, entry) {
		list_del(&mbigen->entry);
		mbigen_free(mbigen);
	}

	irq_domain_remove(chip->domain);
	iounmap(chip->base);
	kfree(chip);

	return 0;
}

static const struct of_device_id mbigen_of_match[] = {
	{ .compatible = "hisilicon,mbi-gen" },
	{ /* END */ }
};
MODULE_DEVICE_TABLE(of, mbigen_of_match);

static struct platform_driver mbigen_platform_driver = {
	.driver = {
		.name		= "Hisilicon MBIGEN",
		.owner		= THIS_MODULE,
		.of_match_table	= mbigen_of_match,
	},
	.probe			= mbigen_probe,
	.remove			= mbigen_remove,
};

module_platform_driver(mbigen_platform_driver);

MODULE_AUTHOR("Yun Wu <wuyun.wu@huawei.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon MBI Generator driver");
