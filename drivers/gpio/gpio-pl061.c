// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2008, 2009 Provigent Ltd.
 *
 * Author: Baruch Siach <baruch@tkos.co.il>
 *
 * Driver for the ARM PrimeCell(tm) General Purpose Input/Output (PL061)
 *
 * Data sheet: ARM DDI 0190B, September 2000
 */
#include <linux/amba/bus.h>
#include <linux/bitops.h>
#include <linux/cleanup.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio/regmap.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/regmap.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>

#define PL061_GPIO_NR	8
#define PL061_DATA_OFFSET	2

#define PL061_DIR		0x400
#define PL061_IS		0x404
#define PL061_IBE		0x408
#define PL061_IEV		0x40c
#define PL061_IE		0x410
#define PL061_MIS		0x418
#define PL061_IC		0x41c

#define AMBARELLA_GPIO_NR	32
#define AMBARELLA_DAT		0x00
#define AMBARELLA_DIR		0x04
#define AMBARELLA_IS		0x08
#define AMBARELLA_IBE		0x0c
#define AMBARELLA_IEV		0x10
#define AMBARELLA_IE		0x14
#define AMBARELLA_MIS		0x20
#define AMBARELLA_IC		0x24
#define AMBARELLA_MASK		0x28
#define AMBARELLA_ENABLE	0x2c

struct pl061_regs {
	unsigned int dat;
	unsigned int dir;
	unsigned int is;
	unsigned int ibe;
	unsigned int iev;
	unsigned int ie;
	unsigned int mis;
	unsigned int ic;
	unsigned int mask;
	unsigned int enable;
};

struct pl061_drvdata {
	const struct regmap_config *regmap_config;
	const struct pl061_regs *regs;
	unsigned int ngpio;
	unsigned long quirks;
	bool clear_irq_on_type;
	bool pm;
	int (*reg_mask_xlate)(struct gpio_regmap *gpio, unsigned int base,
			      unsigned int offset, unsigned int *reg,
			      unsigned int *mask);
};

struct pl061 {
	raw_spinlock_t		lock;

	struct device		*dev;
	struct regmap		*regmap;
	const struct pl061_drvdata *data;
	struct irq_domain	*irq_domain;
	int			parent_irq;

	u32			saved_dat;
};

static unsigned int pl061_line_mask(struct pl061 *pl061, irq_hw_number_t hwirq)
{
	return BIT(hwirq % pl061->data->ngpio);
}

static int pl061_arm_reg_mask_xlate(struct gpio_regmap *gpio, unsigned int base,
				    unsigned int offset, unsigned int *reg,
				    unsigned int *mask)
{
	if (base == 0)
		*reg = BIT(offset + PL061_DATA_OFFSET);
	else
		*reg = base;
	*mask = BIT(offset);

	return 0;
}

static int pl061_irq_type(struct irq_data *d, unsigned int trigger)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(d);
	const struct pl061_regs *regs = pl061->data->regs;
	int offset = irqd_to_hwirq(d);
	unsigned int gpiois = 0, gpioibe = 0, gpioiev = 0;
	unsigned int bit = BIT(offset);

	if (offset < 0 || offset >= pl061->data->ngpio)
		return -EINVAL;

	if ((trigger & IRQ_TYPE_LEVEL_MASK) && (trigger & IRQ_TYPE_EDGE_BOTH)) {
		dev_err(pl061->dev,
			"trying to configure line %d for both level and edge "
			"detection, choose one!\n",
			offset);
		return -EINVAL;
	}

	guard(raw_spinlock_irqsave)(&pl061->lock);

	regmap_read(pl061->regmap, regs->iev, &gpioiev);
	regmap_read(pl061->regmap, regs->is, &gpiois);
	regmap_read(pl061->regmap, regs->ibe, &gpioibe);

	if (trigger & IRQ_TYPE_LEVEL_MASK) {
		bool polarity = trigger & IRQ_TYPE_LEVEL_HIGH;

		/* Disable edge detection */
		gpioibe &= ~bit;
		/* Enable level detection */
		gpiois |= bit;
		/* Select polarity */
		if (polarity)
			gpioiev |= bit;
		else
			gpioiev &= ~bit;
		irq_set_handler_locked(d, handle_level_irq);
		dev_dbg(pl061->dev, "line %d: IRQ on %s level\n",
			offset,
			polarity ? "HIGH" : "LOW");
	} else if ((trigger & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH) {
		/* Disable level detection */
		gpiois &= ~bit;
		/* Select both edges, setting this makes GPIOEV be ignored */
		gpioibe |= bit;
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(pl061->dev, "line %d: IRQ on both edges\n", offset);
	} else if ((trigger & IRQ_TYPE_EDGE_RISING) ||
		   (trigger & IRQ_TYPE_EDGE_FALLING)) {
		bool rising = trigger & IRQ_TYPE_EDGE_RISING;

		/* Disable level detection */
		gpiois &= ~bit;
		/* Clear detection on both edges */
		gpioibe &= ~bit;
		/* Select edge */
		if (rising)
			gpioiev |= bit;
		else
			gpioiev &= ~bit;
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(pl061->dev, "line %d: IRQ on %s edge\n",
			offset,
			rising ? "RISING" : "FALLING");
	} else {
		/* No trigger: disable everything */
		gpiois &= ~bit;
		gpioibe &= ~bit;
		gpioiev &= ~bit;
		irq_set_handler_locked(d, handle_bad_irq);
		dev_warn(pl061->dev, "no trigger selected for line %d\n",
			 offset);
	}

	regmap_write(pl061->regmap, regs->is, gpiois);
	regmap_write(pl061->regmap, regs->ibe, gpioibe);
	regmap_write(pl061->regmap, regs->iev, gpioiev);
	if (pl061->data->clear_irq_on_type)
		regmap_write(pl061->regmap, regs->ic, bit);

	return 0;
}

static void pl061_irq_handler(struct irq_desc *desc)
{
	unsigned long pending;
	unsigned int mis = 0;
	int offset, ret;
	struct pl061 *pl061 = irq_desc_get_handler_data(desc);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);

	chained_irq_enter(irqchip, desc);

	ret = regmap_read(pl061->regmap, pl061->data->regs->mis, &mis);
	if (ret)
		goto out;

	if (mis) {
		pending = mis;
		for_each_set_bit(offset, &pending, pl061->data->ngpio)
			generic_handle_domain_irq(pl061->irq_domain, offset);
	}

out:
	chained_irq_exit(irqchip, desc);
}

static void pl061_irq_mask(struct irq_data *d)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(d);
	unsigned int mask = pl061_line_mask(pl061, irqd_to_hwirq(d));

	regmap_update_bits(pl061->regmap, pl061->data->regs->ie, mask, 0);
}

static void pl061_irq_unmask(struct irq_data *d)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(d);
	unsigned int mask = pl061_line_mask(pl061, irqd_to_hwirq(d));

	regmap_update_bits(pl061->regmap, pl061->data->regs->ie, mask, mask);
}

/**
 * pl061_irq_ack() - ACK an edge IRQ
 * @d: IRQ data for this IRQ
 *
 * This gets called from the edge IRQ handler to ACK the edge IRQ
 * in the GPIOIC (interrupt-clear) register. For level IRQs this is
 * not needed: these go away when the level signal goes away.
 */
static void pl061_irq_ack(struct irq_data *d)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(d);
	unsigned int mask = pl061_line_mask(pl061, irqd_to_hwirq(d));

	regmap_write(pl061->regmap, pl061->data->regs->ic, mask);
}

static int pl061_irq_set_wake(struct irq_data *d, unsigned int state)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(d);

	return irq_set_irq_wake(pl061->parent_irq, state);
}

static void pl061_irq_print_chip(struct irq_data *data, struct seq_file *p)
{
	struct pl061 *pl061 = irq_data_get_irq_chip_data(data);

	seq_puts(p, dev_name(pl061->dev));
}

static const struct irq_chip pl061_irqchip = {
	.name			= "pl061",
	.irq_ack		= pl061_irq_ack,
	.irq_mask		= pl061_irq_mask,
	.irq_unmask		= pl061_irq_unmask,
	.irq_set_type		= pl061_irq_type,
	.irq_set_wake		= pl061_irq_set_wake,
	.irq_print_chip		= pl061_irq_print_chip,
	.flags			= IRQCHIP_IMMUTABLE,
};

static int pl061_irq_domain_map(struct irq_domain *d, unsigned int virq,
				irq_hw_number_t hwirq)
{
	struct pl061 *pl061 = d->host_data;

	irq_set_chip_data(virq, pl061);
	irq_set_chip_and_handler(virq, &pl061_irqchip, handle_bad_irq);
	irq_set_noprobe(virq);
	irq_set_parent(virq, pl061->parent_irq);

	return 0;
}

static void pl061_irq_domain_unmap(struct irq_domain *d, unsigned int virq)
{
	irq_set_chip_and_handler(virq, NULL, NULL);
	irq_set_chip_data(virq, NULL);
}

static const struct irq_domain_ops pl061_irq_domain_ops = {
	.map	= pl061_irq_domain_map,
	.unmap	= pl061_irq_domain_unmap,
	.xlate	= irq_domain_xlate_twothreecell,
};

static void pl061_remove_irq(void *data)
{
	struct pl061 *pl061 = data;

	irq_set_chained_handler_and_data(pl061->parent_irq, NULL, NULL);

	for (unsigned int i = 0; i < pl061->data->ngpio; i++) {
		unsigned int virq;

		virq = irq_find_mapping(pl061->irq_domain, i);
		if (virq)
			irq_dispose_mapping(virq);
	}

	irq_domain_remove(pl061->irq_domain);
}

static bool pl061_arm_is_data_reg(unsigned int reg)
{
	return reg >= BIT(PL061_DATA_OFFSET) &&
	       reg <= BIT(PL061_DATA_OFFSET + PL061_GPIO_NR - 1) &&
	       is_power_of_2(reg);
}

static bool pl061_arm_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case PL061_MIS:
	case PL061_IC:
		return true;
	default:
		return pl061_arm_is_data_reg(reg);
	}
}

static bool pl061_ambarella_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case AMBARELLA_DAT:
	case AMBARELLA_MIS:
	case AMBARELLA_IC:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config pl061_arm_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.reg_stride = 4,
	.max_register = PL061_IC,
	.volatile_reg = pl061_arm_volatile_reg,
	.cache_type = REGCACHE_FLAT_S,
	.fast_io = true,
	.use_raw_spinlock = true,
};

static const struct regmap_config pl061_ambarella_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.max_register = AMBARELLA_ENABLE,
	.volatile_reg = pl061_ambarella_volatile_reg,
	.fast_io = true,
	.use_raw_spinlock = true,
};

static const struct pl061_regs pl061_arm_regs = {
	.dir	= PL061_DIR,
	.is	= PL061_IS,
	.ibe	= PL061_IBE,
	.iev	= PL061_IEV,
	.ie	= PL061_IE,
	.mis	= PL061_MIS,
	.ic	= PL061_IC,
};

static const struct pl061_regs pl061_ambarella_regs = {
	.dat	= AMBARELLA_DAT,
	.dir	= AMBARELLA_DIR,
	.is	= AMBARELLA_IS,
	.ibe	= AMBARELLA_IBE,
	.iev	= AMBARELLA_IEV,
	.ie	= AMBARELLA_IE,
	.mis	= AMBARELLA_MIS,
	.ic	= AMBARELLA_IC,
	.mask	= AMBARELLA_MASK,
	.enable	= AMBARELLA_ENABLE,
};

static const struct pl061_drvdata pl061_arm = {
	.regmap_config		= &pl061_arm_regmap_config,
	.regs			= &pl061_arm_regs,
	.ngpio			= PL061_GPIO_NR,
	.quirks			= GPIO_REGMAP_QUIRK_SET_AFTER_DIR,
	.pm			= true,
	.reg_mask_xlate		= pl061_arm_reg_mask_xlate,
};

static const struct pl061_drvdata pl061_ambarella = {
	.regmap_config		= &pl061_ambarella_regmap_config,
	.regs			= &pl061_ambarella_regs,
	.ngpio			= AMBARELLA_GPIO_NR,
	.clear_irq_on_type	= true,
};

static int pl061_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct device *dev = &adev->dev;
	const struct pl061_drvdata *data;
	const struct pl061_regs *regs;
	struct gpio_regmap_config config = {};
	struct gpio_regmap *gpio_regmap;
	struct pl061 *pl061;
	void __iomem *base;
	int ret, irq;

	data = id->data;
	if (!data)
		return -ENODATA;

	regs = data->regs;

	pl061 = devm_kzalloc(dev, sizeof(*pl061), GFP_KERNEL);
	if (pl061 == NULL)
		return -ENOMEM;

	pl061->dev = dev;
	pl061->data = data;

	base = devm_ioremap_resource(dev, &adev->res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	pl061->regmap = devm_regmap_init_mmio(dev, base, data->regmap_config);
	if (IS_ERR(pl061->regmap))
		return PTR_ERR(pl061->regmap);

	raw_spin_lock_init(&pl061->lock);

	if (regs->enable) {
		ret = regmap_write(pl061->regmap, regs->enable, ~0U);
		if (ret)
			return ret;
	}
	if (regs->mask) {
		ret = regmap_write(pl061->regmap, regs->mask, ~0U);
		if (ret)
			return ret;
	}

	/*
	 * irq_chip support
	 */
	regmap_write(pl061->regmap, regs->ie, 0); /* disable irqs */

	irq = adev->irq[0];
	if (!irq)
		dev_warn(dev, "IRQ support disabled\n");
	pl061->parent_irq = irq;

	if (irq) {
		pl061->irq_domain = irq_domain_create_linear(dev_fwnode(dev),
							     data->ngpio,
							     &pl061_irq_domain_ops,
							     pl061);
		if (!pl061->irq_domain)
			return -ENOMEM;
	}

	config.parent = dev;
	config.regmap = pl061->regmap;
	config.ngpio = data->ngpio;
	config.reg_dat_base = GPIO_REGMAP_ADDR(regs->dat);
	config.reg_set_base = GPIO_REGMAP_ADDR(regs->dat);
	config.reg_dir_out_base = GPIO_REGMAP_ADDR(regs->dir);
	config.reg_mask_xlate = data->reg_mask_xlate;
	config.quirks = data->quirks;
	config.irq_domain = pl061->irq_domain;
	config.drvdata = pl061;

	gpio_regmap = devm_gpio_regmap_register(dev, &config);
	if (IS_ERR(gpio_regmap)) {
		if (pl061->irq_domain)
			irq_domain_remove(pl061->irq_domain);
		return PTR_ERR(gpio_regmap);
	}

	if (irq) {
		ret = devm_add_action_or_reset(dev, pl061_remove_irq, pl061);
		if (ret)
			return ret;

		irq_set_chained_handler_and_data(irq, pl061_irq_handler, pl061);
	}

	amba_set_drvdata(adev, pl061);
	dev_info(dev, "PL061 GPIO chip registered\n");

	return 0;
}

static void pl061_cache_ctrl_regs(struct pl061 *pl061, unsigned int *dir)
{
	const struct pl061_regs *regs = pl061->data->regs;
	unsigned int val;

	regmap_read(pl061->regmap, regs->dir, dir);
	regmap_read(pl061->regmap, regs->is, &val);
	regmap_read(pl061->regmap, regs->ibe, &val);
	regmap_read(pl061->regmap, regs->iev, &val);
	regmap_read(pl061->regmap, regs->ie, &val);
}

static int pl061_suspend(struct device *dev)
{
	struct pl061 *pl061 = dev_get_drvdata(dev);
	unsigned long dir_mask;
	unsigned int dir = 0, val;
	unsigned int offset;

	if (!pl061->data->pm)
		return 0;

	pl061_cache_ctrl_regs(pl061, &dir);

	pl061->saved_dat = 0;
	dir_mask = dir;
	for_each_set_bit(offset, &dir_mask, pl061->data->ngpio) {
		val = 0;
		regmap_read_bypassed(pl061->regmap,
				     BIT(offset + PL061_DATA_OFFSET),
				     &val);
		pl061->saved_dat |= val;
	}

	return 0;
}

static int pl061_resume(struct device *dev)
{
	struct pl061 *pl061 = dev_get_drvdata(dev);
	const struct pl061_regs *regs;
	unsigned long dir_mask;
	unsigned int dir = 0, offset;

	if (!pl061->data->pm)
		return 0;

	regs = pl061->data->regs;

	regmap_read(pl061->regmap, regs->dir, &dir);

	regcache_mark_dirty(pl061->regmap);
	regcache_sync_region(pl061->regmap, regs->dir, regs->dir);

	dir_mask = dir;
	for_each_set_bit(offset, &dir_mask, pl061->data->ngpio)
		regmap_write(pl061->regmap,
			     BIT(offset + PL061_DATA_OFFSET),
			     !!(pl061->saved_dat & BIT(offset)) << offset);

	return regcache_sync_region(pl061->regmap, regs->is, regs->ie);
}

static DEFINE_SIMPLE_DEV_PM_OPS(pl061_dev_pm_ops, pl061_suspend, pl061_resume);

static const struct amba_id pl061_ids[] = {
	{
		.id	= 0x00041061,
		.mask	= 0x000fffff,
		.data	= (void *)&pl061_arm,
	},
	{
		.id	= (AMBA_VENDOR_AMBARELLA << 12) | 0x061,
		.mask	= 0x000fffff,
		.data	= (void *)&pl061_ambarella,
	},
	{ 0, 0 },
};
MODULE_DEVICE_TABLE(amba, pl061_ids);

static struct amba_driver pl061_gpio_driver = {
	.drv = {
		.name	= "pl061_gpio",
		.pm	= pm_sleep_ptr(&pl061_dev_pm_ops),
	},
	.id_table	= pl061_ids,
	.probe		= pl061_probe,
};
module_amba_driver(pl061_gpio_driver);

MODULE_DESCRIPTION("Driver for the ARM PrimeCell(tm) General Purpose Input/Output (PL061)");
MODULE_LICENSE("GPL v2");
