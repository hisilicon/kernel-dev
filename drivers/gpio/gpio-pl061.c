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
#include <linux/errno.h>
#include <linux/gpio/driver.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm.h>
#include <linux/regmap.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define GPIODIR 0x400
#define GPIOIS  0x404
#define GPIOIBE 0x408
#define GPIOIEV 0x40C
#define GPIOIE  0x410
#define GPIORIS 0x414
#define GPIOMIS 0x418
#define GPIOIC  0x41C

#define PL061_GPIO_NR	8
#define PL061_DATA_OFFSET	2

struct pl061_context_save_regs {
	u8 gpio_data;
	u8 gpio_dir;
	u8 gpio_is;
	u8 gpio_ibe;
	u8 gpio_iev;
	u8 gpio_ie;
};

struct pl061 {
	raw_spinlock_t		lock;

	struct regmap		*regmap;
	struct gpio_chip	gc;
	int			parent_irq;

	struct pl061_context_save_regs csave_regs;
};

static bool pl061_is_data_reg(unsigned int reg)
{
	return is_power_of_2(reg) &&
	       reg >= BIT(PL061_DATA_OFFSET) &&
	       reg <= BIT(PL061_DATA_OFFSET + PL061_GPIO_NR - 1);
}

static bool pl061_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case GPIOMIS:
	case GPIOIC:
		return true;
	default:
		return pl061_is_data_reg(reg);
	}
}

static const struct regmap_config pl061_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.reg_stride = 4,
	.max_register = GPIOIC,
	.volatile_reg = pl061_volatile_reg,
	.fast_io = true,
	.use_raw_spinlock = true,
};

static int pl061_get_direction(struct gpio_chip *gc, unsigned offset)
{
	struct pl061 *pl061 = gpiochip_get_data(gc);
	unsigned int gpiodir = 0;

	regmap_read(pl061->regmap, GPIODIR, &gpiodir);

	if (gpiodir & BIT(offset))
		return GPIO_LINE_DIRECTION_OUT;

	return GPIO_LINE_DIRECTION_IN;
}

static int pl061_direction_input(struct gpio_chip *gc, unsigned offset)
{
	struct pl061 *pl061 = gpiochip_get_data(gc);

	guard(raw_spinlock_irqsave)(&pl061->lock);
	regmap_update_bits(pl061->regmap, GPIODIR, BIT(offset), 0);

	return 0;
}

static int pl061_direction_output(struct gpio_chip *gc, unsigned offset,
		int value)
{
	struct pl061 *pl061 = gpiochip_get_data(gc);
	unsigned int mask = BIT(offset);

	guard(raw_spinlock_irqsave)(&pl061->lock);
	regmap_write(pl061->regmap, BIT(offset + PL061_DATA_OFFSET),
		     !!value << offset);
	regmap_update_bits(pl061->regmap, GPIODIR, mask, mask);

	/*
	 * gpio value is set again, because pl061 doesn't allow to set value of
	 * a gpio pin before configuring it in OUT mode.
	 */
	regmap_write(pl061->regmap, BIT(offset + PL061_DATA_OFFSET),
		     !!value << offset);

	return 0;
}

static int pl061_get_value(struct gpio_chip *gc, unsigned offset)
{
	struct pl061 *pl061 = gpiochip_get_data(gc);
	unsigned int val = 0;

	regmap_read(pl061->regmap, BIT(offset + PL061_DATA_OFFSET), &val);

	return !!val;
}

static int pl061_set_value(struct gpio_chip *gc, unsigned int offset, int value)
{
	struct pl061 *pl061 = gpiochip_get_data(gc);

	regmap_write(pl061->regmap, BIT(offset + PL061_DATA_OFFSET),
		     !!value << offset);

	return 0;
}

static int pl061_irq_type(struct irq_data *d, unsigned trigger)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct pl061 *pl061 = gpiochip_get_data(gc);
	int offset = irqd_to_hwirq(d);
	unsigned int gpiois = 0, gpioibe = 0, gpioiev = 0;
	unsigned int bit = BIT(offset);

	if (offset < 0 || offset >= PL061_GPIO_NR)
		return -EINVAL;

	if ((trigger & IRQ_TYPE_LEVEL_MASK) && (trigger & IRQ_TYPE_EDGE_BOTH)) {
		dev_err(gc->parent,
			"trying to configure line %d for both level and edge "
			"detection, choose one!\n",
			offset);
		return -EINVAL;
	}


	guard(raw_spinlock_irqsave)(&pl061->lock);

	regmap_read(pl061->regmap, GPIOIEV, &gpioiev);
	regmap_read(pl061->regmap, GPIOIS, &gpiois);
	regmap_read(pl061->regmap, GPIOIBE, &gpioibe);

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
		dev_dbg(gc->parent, "line %d: IRQ on %s level\n",
			offset,
			polarity ? "HIGH" : "LOW");
	} else if ((trigger & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH) {
		/* Disable level detection */
		gpiois &= ~bit;
		/* Select both edges, setting this makes GPIOEV be ignored */
		gpioibe |= bit;
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on both edges\n", offset);
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
		dev_dbg(gc->parent, "line %d: IRQ on %s edge\n",
			offset,
			rising ? "RISING" : "FALLING");
	} else {
		/* No trigger: disable everything */
		gpiois &= ~bit;
		gpioibe &= ~bit;
		gpioiev &= ~bit;
		irq_set_handler_locked(d, handle_bad_irq);
		dev_warn(gc->parent, "no trigger selected for line %d\n",
			 offset);
	}

	regmap_write(pl061->regmap, GPIOIS, gpiois);
	regmap_write(pl061->regmap, GPIOIBE, gpioibe);
	regmap_write(pl061->regmap, GPIOIEV, gpioiev);


	return 0;
}

static void pl061_irq_handler(struct irq_desc *desc)
{
	unsigned long pending;
	unsigned int mis = 0;
	int offset;
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	struct pl061 *pl061 = gpiochip_get_data(gc);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);

	chained_irq_enter(irqchip, desc);

	regmap_read(pl061->regmap, GPIOMIS, &mis);
	if (mis) {
		pending = mis;
		for_each_set_bit(offset, &pending, PL061_GPIO_NR)
			generic_handle_domain_irq(gc->irq.domain,
						  offset);
	}

	chained_irq_exit(irqchip, desc);
}

static void pl061_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct pl061 *pl061 = gpiochip_get_data(gc);
	u8 mask = BIT(irqd_to_hwirq(d) % PL061_GPIO_NR);

	guard(raw_spinlock)(&pl061->lock);
	regmap_update_bits(pl061->regmap, GPIOIE, mask, 0);

	gpiochip_disable_irq(gc, d->hwirq);
}

static void pl061_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct pl061 *pl061 = gpiochip_get_data(gc);
	u8 mask = BIT(irqd_to_hwirq(d) % PL061_GPIO_NR);

	gpiochip_enable_irq(gc, d->hwirq);

	guard(raw_spinlock)(&pl061->lock);
	regmap_update_bits(pl061->regmap, GPIOIE, mask, mask);
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
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct pl061 *pl061 = gpiochip_get_data(gc);
	u8 mask = BIT(irqd_to_hwirq(d) % PL061_GPIO_NR);

	regmap_write(pl061->regmap, GPIOIC, mask);
}

static int pl061_irq_set_wake(struct irq_data *d, unsigned int state)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct pl061 *pl061 = gpiochip_get_data(gc);

	return irq_set_irq_wake(pl061->parent_irq, state);
}

static void pl061_irq_print_chip(struct irq_data *data, struct seq_file *p)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);

	seq_puts(p, dev_name(gc->parent));
}

static const struct irq_chip pl061_irq_chip = {
	.irq_ack		= pl061_irq_ack,
	.irq_mask		= pl061_irq_mask,
	.irq_unmask		= pl061_irq_unmask,
	.irq_set_type		= pl061_irq_type,
	.irq_set_wake		= pl061_irq_set_wake,
	.irq_print_chip		= pl061_irq_print_chip,
	.flags			= IRQCHIP_IMMUTABLE,
	GPIOCHIP_IRQ_RESOURCE_HELPERS,
};

static int pl061_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct device *dev = &adev->dev;
	struct pl061 *pl061;
	struct gpio_irq_chip *girq;
	void __iomem *base;
	int ret, irq;

	pl061 = devm_kzalloc(dev, sizeof(*pl061), GFP_KERNEL);
	if (pl061 == NULL)
		return -ENOMEM;

	base = devm_ioremap_resource(dev, &adev->res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	pl061->regmap = devm_regmap_init_mmio(dev, base, &pl061_regmap_config);
	if (IS_ERR(pl061->regmap))
		return PTR_ERR(pl061->regmap);

	raw_spin_lock_init(&pl061->lock);
	pl061->gc.request = gpiochip_generic_request;
	pl061->gc.free = gpiochip_generic_free;
	pl061->gc.base = -1;
	pl061->gc.get_direction = pl061_get_direction;
	pl061->gc.direction_input = pl061_direction_input;
	pl061->gc.direction_output = pl061_direction_output;
	pl061->gc.get = pl061_get_value;
	pl061->gc.set = pl061_set_value;
	pl061->gc.ngpio = PL061_GPIO_NR;
	pl061->gc.label = dev_name(dev);
	pl061->gc.parent = dev;
	pl061->gc.owner = THIS_MODULE;

	/*
	 * irq_chip support
	 */
	regmap_write(pl061->regmap, GPIOIE, 0); /* disable irqs */
	irq = adev->irq[0];
	if (!irq)
		dev_warn(&adev->dev, "IRQ support disabled\n");
	pl061->parent_irq = irq;

	girq = &pl061->gc.irq;
	gpio_irq_chip_set_chip(girq, &pl061_irq_chip);
	girq->parent_handler = pl061_irq_handler;
	girq->num_parents = 1;
	girq->parents = devm_kcalloc(dev, 1, sizeof(*girq->parents),
				     GFP_KERNEL);
	if (!girq->parents)
		return -ENOMEM;
	girq->parents[0] = irq;
	girq->default_type = IRQ_TYPE_NONE;
	girq->handler = handle_bad_irq;

	ret = devm_gpiochip_add_data(dev, &pl061->gc, pl061);
	if (ret)
		return ret;

	amba_set_drvdata(adev, pl061);
	dev_info(dev, "PL061 GPIO chip registered\n");

	return 0;
}

static int pl061_suspend(struct device *dev)
{
	struct pl061 *pl061 = dev_get_drvdata(dev);
	unsigned int val = 0;
	int offset;

	pl061->csave_regs.gpio_data = 0;
	regmap_read(pl061->regmap, GPIODIR, &val);
	pl061->csave_regs.gpio_dir = val;
	regmap_read(pl061->regmap, GPIOIS, &val);
	pl061->csave_regs.gpio_is = val;
	regmap_read(pl061->regmap, GPIOIBE, &val);
	pl061->csave_regs.gpio_ibe = val;
	regmap_read(pl061->regmap, GPIOIEV, &val);
	pl061->csave_regs.gpio_iev = val;
	regmap_read(pl061->regmap, GPIOIE, &val);
	pl061->csave_regs.gpio_ie = val;

	for (offset = 0; offset < PL061_GPIO_NR; offset++) {
		if (pl061->csave_regs.gpio_dir & (BIT(offset)))
			pl061->csave_regs.gpio_data |=
				pl061_get_value(&pl061->gc, offset) << offset;
	}

	return 0;
}

static int pl061_resume(struct device *dev)
{
	struct pl061 *pl061 = dev_get_drvdata(dev);
	int offset;

	for (offset = 0; offset < PL061_GPIO_NR; offset++) {
		if (pl061->csave_regs.gpio_dir & (BIT(offset)))
			pl061_direction_output(&pl061->gc, offset,
					pl061->csave_regs.gpio_data &
					(BIT(offset)));
		else
			pl061_direction_input(&pl061->gc, offset);
	}

	regmap_write(pl061->regmap, GPIOIS, pl061->csave_regs.gpio_is);
	regmap_write(pl061->regmap, GPIOIBE, pl061->csave_regs.gpio_ibe);
	regmap_write(pl061->regmap, GPIOIEV, pl061->csave_regs.gpio_iev);
	regmap_write(pl061->regmap, GPIOIE, pl061->csave_regs.gpio_ie);

	return 0;
}

static DEFINE_SIMPLE_DEV_PM_OPS(pl061_dev_pm_ops, pl061_suspend, pl061_resume);

static const struct amba_id pl061_ids[] = {
	{
		.id	= 0x00041061,
		.mask	= 0x000fffff,
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
