/*
 * Copyright (C) 2015 Hisilicon Limited, All Rights Reserved.
 * Author: Jun Ma <majun258@huawei.com>
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

#include <linux/interrupt.h>
#include <linux/irqchip.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

/* The maximum IRQ pin number of mbigen chip(start from 0) */
#define MAXIMUM_IRQ_PIN_NUM		640

/**
 * In mbigen vector register
 * bit[31:16]:	device id
 * bit[15:0]:	event id value
 */
#define IRQ_EVENT_ID_MASK		0xffff

/* offset of vector register in mbigen node */
#define REG_MBIGEN_VEC_OFFSET		0x300
#define REG_MBIGEN_EXT_VEC_OFFSET		0x320

/**
 * offset of clear register in mbigen node
 * This register is used to clear the status
 * of interrupt
 */
#define REG_MBIGEN_CLEAR_OFFSET		0x100

/**
 * offset of interrupt type register
 * This register is used to configure interrupt
 * trigger type
 */
#define REG_MBIGEN_TYPE_OFFSET		0x0

/**
 * struct mbigen_device - holds the information of mbigen device.
 *
 * @pdev:		pointer to the platform device structure of mbigen chip.
 * @base:		mapped address of this mbigen chip.
 */
struct mbigen_device {
	struct platform_device	*pdev;
	void __iomem		*base;
};

static int get_mbigen_nid(unsigned int offset)
{
	int nid = 0;

	if (offset < 256)
		nid = offset / 64;
	else if (offset < 384)
		nid = 4;
	else if (offset < 640)
		nid = 5;

	return nid;
}

static inline unsigned int get_mbigen_vec_reg(irq_hw_number_t hwirq)
{
	unsigned int nid;

	nid = get_mbigen_nid(hwirq);

	if (nid < 4)
		return (nid * 4) + REG_MBIGEN_VEC_OFFSET;
	else
		return (nid - 4) * 4 + REG_MBIGEN_EXT_VEC_OFFSET;
}

static inline void get_mbigen_type_reg(irq_hw_number_t hwirq,
					u32 *mask, u32 *addr)
{
	int ofst;

	ofst = hwirq / 32 * 4;
	*mask = 1 << (hwirq % 32);

	*addr = ofst + REG_MBIGEN_TYPE_OFFSET;
}

static inline void get_mbigen_clear_reg(irq_hw_number_t hwirq,
					u32 *mask, u32 *addr)
{
	unsigned int ofst;

	ofst = hwirq / 32 * 4;

	*mask = 1 << (hwirq % 32);
	*addr = ofst + REG_MBIGEN_CLEAR_OFFSET;
}

static void mbigen_eoi_irq(struct irq_data *data)
{
	void __iomem *base = data->chip_data;
	u32 mask, addr;

	get_mbigen_clear_reg(data->hwirq, &mask, &addr);

	writel_relaxed(mask, base + addr);

	irq_chip_eoi_parent(data);
}

static int mbigen_set_type(struct irq_data *data, unsigned int type)
{
	void __iomem *base = data->chip_data;
	u32 mask, addr, val;

	if (type != IRQ_TYPE_LEVEL_HIGH && type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	get_mbigen_type_reg(data->hwirq, &mask, &addr);

	val = readl_relaxed(base + addr);

	if (type == IRQ_TYPE_LEVEL_HIGH)
		val |= mask;
	else
		val &= ~mask;

	writel_relaxed(val, base + addr);

	return 0;
}

static struct irq_chip mbigen_irq_chip = {
	.name =			"mbigen-v1",
	.irq_mask =		irq_chip_mask_parent,
	.irq_unmask =		irq_chip_unmask_parent,
	.irq_eoi =		mbigen_eoi_irq,
	.irq_set_type =		mbigen_set_type,
	.irq_set_affinity =	irq_chip_set_affinity_parent,
};

static void mbigen_write_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	struct irq_data *d = irq_get_irq_data(desc->irq);
	void __iomem *base = d->chip_data;
	u32 val;

	base += get_mbigen_vec_reg(d->hwirq);
	val = readl_relaxed(base);

	val &= ~IRQ_EVENT_ID_MASK;
	val |= msg->data;

	/* The address of doorbell is encoded in mbigen register by default
	 * So,we don't need to program the doorbell address at here
	 */
	writel_relaxed(val, base);
}

static int mbigen_domain_translate(struct irq_domain *d,
				    struct irq_fwspec *fwspec,
				    unsigned long *hwirq,
				    unsigned int *type)
{
	if (is_of_node(fwspec->fwnode)) {
		if (fwspec->param_count != 2)
			return -EINVAL;

		if (fwspec->param[0] > MAXIMUM_IRQ_PIN_NUM)
			return -EINVAL;

		*hwirq = fwspec->param[0];

		/* If there is no valid irq type, just use the default type */
		if ((fwspec->param[1] == IRQ_TYPE_EDGE_RISING) ||
			(fwspec->param[1] == IRQ_TYPE_LEVEL_HIGH))
			*type = fwspec->param[1];
		else
			return -EINVAL;

		return 0;
	}
	return -EINVAL;
}

static int mbigen_irq_domain_alloc(struct irq_domain *domain,
					unsigned int virq,
					unsigned int nr_irqs,
					void *args)
{
	struct irq_fwspec *fwspec = args;
	irq_hw_number_t hwirq;
	unsigned int type;
	struct mbigen_device *mgn_chip;
	int i, err;

	err = mbigen_domain_translate(domain, fwspec, &hwirq, &type);
	if (err)
		return err;

	err = platform_msi_domain_alloc(domain, virq, nr_irqs);
	if (err)
		return err;

	mgn_chip = platform_msi_get_host_data(domain);

	for (i = 0; i < nr_irqs; i++)
		irq_domain_set_hwirq_and_chip(domain, virq + i, hwirq + i,
				      &mbigen_irq_chip, mgn_chip->base);

	return 0;
}

static struct irq_domain_ops mbigen_domain_ops = {
	.translate	= mbigen_domain_translate,
	.alloc		= mbigen_irq_domain_alloc,
	.free		= irq_domain_free_irqs_common,
};

static int mbigen_device_probe(struct platform_device *pdev)
{
	struct mbigen_device *mgn_chip;
	struct resource *res;
	struct irq_domain *domain;
	u32 num_pins;

	mgn_chip = devm_kzalloc(&pdev->dev, sizeof(*mgn_chip), GFP_KERNEL);
	if (!mgn_chip)
		return -ENOMEM;

	mgn_chip->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mgn_chip->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mgn_chip->base))
		return PTR_ERR(mgn_chip->base);

	if (of_property_read_u32(pdev->dev.of_node, "num-pins", &num_pins) < 0) {
		dev_err(&pdev->dev, "No num-pins property\n");
		return -EINVAL;
	}

	domain = platform_msi_create_device_domain(&pdev->dev, num_pins,
							mbigen_write_msg,
							&mbigen_domain_ops,
							mgn_chip);

	if (!domain)
		return -ENOMEM;

	platform_set_drvdata(pdev, mgn_chip);

	dev_info(&pdev->dev, "Allocated %d MSIs\n", num_pins);

	return 0;
}

static const struct of_device_id mbigen_of_match[] = {
	{ .compatible = "hisilicon,mbigen-v1" },
	{ /* END */ }
};
MODULE_DEVICE_TABLE(of, mbigen_of_match);

static struct platform_driver mbigen_platform_driver = {
	.driver = {
		.name		= "Hisilicon MBIGEN-V1",
		.owner		= THIS_MODULE,
		.of_match_table	= mbigen_of_match,
	},
	.probe			= mbigen_device_probe,
};

module_platform_driver(mbigen_platform_driver);

MODULE_AUTHOR("Jun Ma <majun258@huawei.com>");
MODULE_AUTHOR("Yun Wu <wuyun.wu@huawei.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon MBI Generator driver");
