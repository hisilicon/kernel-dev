/**
 * Copyright (C) 2014, Yun Wu <wuyun.wu@huawei.com>
 *
 * This file contains the generic interfaces for Message Based
 * Interrupts (MBI).
 */

#include <linux/interrupt.h>
#include <linux/mbi.h>
#include <linux/slab.h>
#include <linux/of.h>

/**
 * mbi_alloc_desc() - allocate an MBI descriptor
 * @dev:	the device owned the MBI
 * @ops:	config operations of @dev
 * @msg_id:	identifier of the message group
 * @lines:	max number of interrupts supported by the message register
 * @offset:	hardware pin offset of @irq
 * @data:	message specific data
 */
struct mbi_desc *mbi_alloc_desc(struct device *dev, struct mbi_ops *ops,
				int msg_id, unsigned int lines,
				unsigned int offset, void *data)
{
	struct mbi_desc *desc;

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return NULL;

	desc->mbi.dev	= dev;
	desc->mbi.ops	= ops;
	desc->mbi.msg_id = msg_id;
	desc->mbi.lines	= lines;
	desc->offset	= offset;
	desc->data	= data;

	return desc;
}

/**
 * mbi_config_irq() - config one MBI interrupt
 * @data:	irq_data of the MBI interrupt
 * @mask:	type of configuration, mask if true, otherwise unmask
 */
static void mbi_config_irq(struct irq_data *data, int mask)
{
	struct mbi_desc *desc = irq_data_get_mbi(data);
	struct mbi_ops *ops = desc ? desc->mbi.ops : NULL;
	void (*fn)(struct mbi_desc *) = NULL;

	if (ops)
		fn = mask ? ops->mask_irq : ops->unmask_irq;
	if (fn)
		fn(desc);
}

static void mbi_mask_irq(struct irq_data *data)
{
	mbi_config_irq(data, 1);
	irq_chip_mask_parent(data);
}

static void mbi_unmask_irq(struct irq_data *data)
{
	mbi_config_irq(data, 0);
	irq_chip_unmask_parent(data);
}

static void mbi_write_msg(struct irq_data *data, struct mbi_msg *msg, bool enable)
{
	struct mbi_desc *desc = irq_data_get_mbi(data);
	struct mbi_ops *ops = desc->mbi.ops;

	if (ops && ops->write_msg)
		ops->write_msg(desc, msg, enable);
}

static int mbi_set_affinity(struct irq_data *data, const struct cpumask *mask,
			    bool force)
{
	struct mbi_msg msg;
	int ret;

	ret = irq_chip_set_affinity_parent(data, mask, force);
	if (ret >= 0 && ret != IRQ_SET_MASK_OK_DONE) {
		BUG_ON(irq_compose_mbi_msg(data, &msg));
		mbi_write_msg(data, &msg, false);
	}

	return ret;
}

static struct irq_chip mbi_irq_chip = {
	.name			= "MBI",
	.irq_unmask		= mbi_unmask_irq,
	.irq_mask		= mbi_mask_irq,
	.irq_ack		= irq_chip_ack_parent,
	.irq_eoi		= irq_chip_eoi_parent,
	.irq_set_affinity	= mbi_set_affinity,
	.irq_retrigger		= irq_chip_retrigger_hierarchy,
	.flags			= IRQCHIP_SKIP_SET_WAKE,
};

static void mbi_domain_activate(struct irq_domain *domain, struct irq_data *data)
{
	struct mbi_msg msg;

	WARN_ON(domain != data->domain);
	BUG_ON(irq_compose_mbi_msg(data, &msg));
	mbi_write_msg(data, &msg, true);
}

static void mbi_domain_deactivate(struct irq_domain *domain, struct irq_data *data)
{
	struct mbi_msg msg;

	WARN_ON(domain != data->domain);
	memset(&msg, 0, sizeof(msg));
	mbi_write_msg(data, &msg, false);
}

static int mbi_domain_alloc(struct irq_domain *domain, unsigned int virq,
			    unsigned int nr_irqs, void *arg)
{
	struct mbi_desc *desc = arg;
	int ret, i;

	WARN_ON(!desc);
	desc->irq = virq;
	desc->nvec = nr_irqs;

	ret = irq_set_mbi_desc_range(virq, desc, nr_irqs);
	if (unlikely(ret))
		return ret;

	for (i = 0; i < nr_irqs; i++)
		irq_domain_set_hwirq_and_chip(domain, virq + i, virq + i,
					      &mbi_irq_chip, desc);

	ret = irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, arg);
	if (ret) {
		irq_set_mbi_desc_range(desc->irq, NULL, desc->nvec);
		irq_domain_free_irqs_top(domain, virq, nr_irqs);
		return ret;
	}

	return 0;
}

static void mbi_domain_free(struct irq_domain *domain, unsigned int virq,
			    unsigned int nr_irqs)
{
	struct mbi_desc *desc = irq_get_mbi_desc(virq);

	irq_domain_free_irqs_top(domain, virq, nr_irqs);
	desc->irq = 0;
	desc->nvec = 0;
}

static struct irq_domain_ops mbi_domain_ops = {
	.alloc		= mbi_domain_alloc,
	.free		= mbi_domain_free,
	.activate	= mbi_domain_activate,
	.deactivate	= mbi_domain_deactivate,
};

struct irq_domain *mbi_create_irq_domain(struct device_node *np,
					 struct irq_domain *parent, void *arg)
{
	return irq_domain_add_hierarchy(parent, 0, 0, np, &mbi_domain_ops, arg);
}

int mbi_parse_irqs(struct device *dev, struct mbi_ops *ops)
{
	struct device_node *node = dev->of_node;
	struct device_node *mbic;
	struct irq_domain *parent;
	struct mbi_desc *desc;
	const __be32 *label;
	unsigned int len;
	unsigned int msg_id, lines, offset, nvec;

	mbic = of_parse_phandle(node, "msi-parent", 0);
	if (!mbic)
		return -EINVAL;

	parent = irq_find_host(mbic);
	if (!parent) {
		pr_warn("MBI: invalid MSI parent for %s!\n",
			node->full_name);
		return -EINVAL;
	}

	label = of_get_property(node, "mbi", &len);
	if (!label) {
		pr_warn("MBI: no mbi description on %s!\n",
			node->full_name);
		return -EINVAL;
	}

	len /= sizeof(*label);
	if (len < 4) {
		pr_warn("MBI: bad MBI label detected on %s!\n",
			node->full_name);
		return -EINVAL;
	}

	msg_id	= be32_to_cpup(label++);
	lines	= be32_to_cpup(label++);
	offset	= be32_to_cpup(label++);
	nvec	= be32_to_cpup(label++);

	desc = mbi_alloc_desc(dev, ops, msg_id, lines, offset, NULL);
	if (!desc)
		return -ENOMEM;

	return irq_domain_alloc_irqs(parent, nvec, dev_to_node(dev), desc);
}
EXPORT_SYMBOL(mbi_parse_irqs);

void mbi_free_irqs(struct device *dev, unsigned int virq, unsigned int nvec)
{
	struct mbi_desc *desc;
	int start = virq, left = nvec;

again:
	desc = irq_get_mbi_desc(start);
	if (WARN_ON(!desc)			||
	    WARN_ON(desc->mbi.dev != dev)	||
	    WARN_ON(desc->irq != start)		||
	    WARN_ON(desc->nvec > left))
		return;

	start += desc->nvec;
	left -= desc->nvec;

	irq_domain_free_irqs(desc->irq, desc->nvec);
	kfree(desc);

	if (left > 0)
		goto again;

	WARN_ON(left < 0);
}
EXPORT_SYMBOL(mbi_free_irqs);
