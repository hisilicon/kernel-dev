/*
 * ACPI GSI IRQ layer
 *
 * Copyright (C) 2015 ARM Ltd.
 * Author: Lorenzo Pieralisi <lorenzo.pieralisi@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/acpi.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>

enum acpi_irq_model_id acpi_irq_model;

static struct fwnode_handle *acpi_gsi_domain_id;

/**
 * acpi_get_irq_source_fwhandle() - Retrieve the fwhandle of the given
 *                                  acpi_resource_source which is used
 *                                  to be used as an IRQ domain id
 * @source: acpi_resource_source to use for the lookup
 *
 * Returns: The appropriate IRQ fwhandle domain id
 *          NULL on failure
 */
struct fwnode_handle *
acpi_get_irq_source_fwhandle(const struct acpi_resource_source *source)
{
	struct fwnode_handle *result;
	struct acpi_device *device;
	acpi_handle handle;
	acpi_status status;

	if (!source->string_length)
		return acpi_gsi_domain_id;

	status = acpi_get_handle(NULL, source->string_ptr, &handle);
	if (ACPI_FAILURE(status)) {
		pr_warn("Could not find handle for %s\n", source->string_ptr);
		return NULL;
	}

	device = acpi_bus_get_acpi_device(handle);
	if (!device) {
		pr_warn("Could not get device for %s\n", source->string_ptr);
		return NULL;
	}

	result = &device->fwnode;
	acpi_bus_put_acpi_device(device);

	return result;
}
EXPORT_SYMBOL_GPL(acpi_get_irq_source_fwhandle);

/**
 * acpi_gsi_to_irq() - Retrieve the linux irq number for a given GSI
 * @gsi: GSI IRQ number to map
 * @irq: pointer where linux IRQ number is stored
 *
 * irq location updated with irq value [>0 on success, 0 on failure]
 *
 * Returns: linux IRQ number on success (>0)
 *          -EINVAL on failure
 */
int acpi_gsi_to_irq(u32 gsi, unsigned int *irq)
{
	struct irq_domain *d = irq_find_matching_fwnode(acpi_gsi_domain_id,
							DOMAIN_BUS_ANY);

	*irq = irq_find_mapping(d, gsi);
	/*
	 * *irq == 0 means no mapping, that should
	 * be reported as a failure
	 */
	return (*irq > 0) ? *irq : -EINVAL;
}
EXPORT_SYMBOL_GPL(acpi_gsi_to_irq);

/**
 * acpi_register_irq() - Map a hardware to a linux IRQ number
 * @source: IRQ source
 * @hwirq: Hardware IRQ number
 * @trigger: trigger type of the IRQ number to be mapped
 * @polarity: polarity of the IRQ to be mapped
 *
 * Returns: a valid linux IRQ number on success
 *          -EINVAL on failure
 */
int acpi_register_irq(struct fwnode_handle *source, u32 hwirq, int trigger,
		      int polarity)
{
	struct irq_fwspec fwspec;

	if (!source)
		return -EINVAL;

	if (irq_find_matching_fwnode(source, DOMAIN_BUS_ANY) == NULL)
		return -EPROBE_DEFER;

	fwspec.fwnode = source;
	fwspec.param[0] = hwirq;
	fwspec.param[1] = acpi_dev_get_irq_type(trigger, polarity);
	fwspec.param_count = 2;

	return irq_create_fwspec_mapping(&fwspec);
}
EXPORT_SYMBOL_GPL(acpi_register_irq);

/**
 * acpi_unregister_irq() - Free a Hardware IRQ<->linux IRQ number mapping
 * @hwirq: Hardware IRQ number
 */
void acpi_unregister_irq(struct fwnode_handle *source, u32 hwirq)
{
	struct irq_domain *d = irq_find_matching_fwnode(source,
							DOMAIN_BUS_ANY);
	int irq = irq_find_mapping(d, hwirq);

	irq_dispose_mapping(irq);
}
EXPORT_SYMBOL_GPL(acpi_unregister_irq);

/**
 * acpi_register_gsi() - Map a GSI to a linux IRQ number
 * @dev: device for which IRQ has to be mapped
 * @gsi: GSI IRQ number
 * @trigger: trigger type of the GSI number to be mapped
 * @polarity: polarity of the GSI to be mapped
 *
 * Returns: a valid linux IRQ number on success
 *          -EINVAL on failure
 */
int acpi_register_gsi(struct device *dev, u32 gsi, int trigger,
		      int polarity)
{
	if (WARN_ON(!acpi_gsi_domain_id)) {
		pr_warn("GSI: No registered irqchip, giving up\n");
		return -EINVAL;
	}

	return acpi_register_irq(acpi_gsi_domain_id, gsi, trigger, polarity);
}
EXPORT_SYMBOL_GPL(acpi_register_gsi);

/**
 * acpi_unregister_gsi() - Free a GSI<->linux IRQ number mapping
 * @gsi: GSI IRQ number
 */
void acpi_unregister_gsi(u32 gsi)
{
	acpi_unregister_irq(acpi_gsi_domain_id, gsi);
}
EXPORT_SYMBOL_GPL(acpi_unregister_gsi);

/**
 * acpi_set_irq_model - Setup the GSI irqdomain information
 * @model: the value assigned to acpi_irq_model
 * @fwnode: the irq_domain identifier for mapping and looking up
 *          GSI interrupts
 */
void __init acpi_set_irq_model(enum acpi_irq_model_id model,
			       struct fwnode_handle *fwnode)
{
	acpi_irq_model = model;
	acpi_gsi_domain_id = fwnode;
}
