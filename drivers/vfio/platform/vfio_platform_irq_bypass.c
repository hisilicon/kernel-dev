/*
 * VFIO platform device irqbypass callback implementation for DEOI
 *
 * Copyright (C) 2017 Red Hat, Inc.  All rights reserved.
 * Author: Eric Auger <eric.auger@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/err.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/irqbypass.h>
#include "vfio_platform_private.h"

#ifdef CONFIG_VFIO_PLATFORM_IRQ_BYPASS_DEOI

static void irq_bypass_deoi_start(struct irq_bypass_producer *prod)
{
	enable_irq(prod->irq);
}

static void irq_bypass_deoi_stop(struct irq_bypass_producer *prod)
{
	disable_irq(prod->irq);
}

/**
 * irq_bypass_deoi_add_consumer - turns irq direct EOI on
 *
 * The linux irq is disabled when the function is called.
 * The operation succeeds only if the irq is not active at irqchip level
 * and the irq is not automasked at VFIO level, meaning the IRQ is under
 * injection into the guest.
 */
static int irq_bypass_deoi_add_consumer(struct irq_bypass_producer *prod,
					struct irq_bypass_consumer *cons)
{
	struct vfio_platform_irq *irq_ctx =
		container_of(prod, struct vfio_platform_irq, producer);
	unsigned long flags;
	bool active;
	int ret;

	spin_lock_irqsave(&irq_ctx->lock, flags);

	ret = irq_get_irqchip_state(irq_ctx->hwirq, IRQCHIP_STATE_ACTIVE,
				    &active);
	if (ret)
		goto out;

	if (active || irq_ctx->automasked) {
		ret = -EAGAIN;
		goto out;
	}

	/*
	 * Direct EOI is enabled for edge sensitive interrupts without any
	 * change with respect to the physical interrupt handler
	 */
	if (!(irq_get_trigger_type(irq_ctx->hwirq) & IRQ_TYPE_LEVEL_MASK))
		goto out;

	ret = vfio_platform_set_deoi(irq_ctx, true);
out:
	spin_unlock_irqrestore(&irq_ctx->lock, flags);
	return ret;
}

static void irq_bypass_deoi_del_consumer(struct irq_bypass_producer *prod,
					 struct irq_bypass_consumer *cons)
{
	struct vfio_platform_irq *irq_ctx =
		container_of(prod, struct vfio_platform_irq, producer);
	unsigned long flags;

	spin_lock_irqsave(&irq_ctx->lock, flags);
	if (irq_get_trigger_type(irq_ctx->hwirq) & IRQ_TYPE_LEVEL_MASK)
		vfio_platform_set_deoi(irq_ctx, false);
	spin_unlock_irqrestore(&irq_ctx->lock, flags);
}

bool vfio_platform_has_deoi(void)
{
	return true;
}

void vfio_platform_register_deoi_producer(struct vfio_platform_device *vdev,
					  struct vfio_platform_irq *irq,
					  struct eventfd_ctx *trigger,
					  unsigned int host_irq)
{
	struct irq_bypass_producer *prod = &irq->producer;
	int ret;

	prod->token =		trigger;
	prod->irq =		host_irq;
	prod->add_consumer =	irq_bypass_deoi_add_consumer;
	prod->del_consumer =	irq_bypass_deoi_del_consumer;
	prod->stop =		irq_bypass_deoi_stop;
	prod->start =		irq_bypass_deoi_start;

	ret = irq_bypass_register_producer(prod);
	if (unlikely(ret))
		dev_info(vdev->device,
			 "irq bypass producer (token %p) registration fails: %d\n",
			 prod->token, ret);
}

#endif

