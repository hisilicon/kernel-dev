/*
 * Copyright (C) 2013-2015 ARM Limited, All Rights Reserved.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
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

#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/iort.h>
#include <linux/msi.h>
#include <linux/of.h>
#include <linux/of_irq.h>

static struct irq_chip its_pmsi_irq_chip = {
	.name			= "ITS-pMSI",
};

static int its_pmsi_prepare(struct irq_domain *domain, struct device *dev,
			    int nvec, msi_alloc_info_t *info)
{
	struct msi_domain_info *msi_info;
	u32 dev_id;
	int ret, index = 0;

	msi_info = msi_get_domain_info(domain->parent);

	/* Suck the DeviceID out of the msi-parent property */
	do {
		struct of_phandle_args args;

		ret = of_parse_phandle_with_args(dev->of_node,
						 "msi-parent", "#msi-cells",
						 index, &args);
		if (!ret) {
			if (args.np == irq_domain_get_of_node(domain)) {
				if (WARN_ON(args.args_count != 1))
					return -EINVAL;
				dev_id = args.args[0];
				break;
			}
		}
	} while (!ret);

	if (ret)
		ret = iort_find_platform_dev_id(dev, &dev_id);

	if (ret)
		return ret;

	/* ITS specific DeviceID, as the core ITS ignores dev. */
	info->scratchpad[0].ul = dev_id;

	return msi_info->ops->msi_prepare(domain->parent,
					  dev, nvec, info);
}

static struct msi_domain_ops its_pmsi_ops = {
	.msi_prepare	= its_pmsi_prepare,
};

static struct msi_domain_info its_pmsi_domain_info = {
	.flags	= (MSI_FLAG_USE_DEF_DOM_OPS | MSI_FLAG_USE_DEF_CHIP_OPS),
	.ops	= &its_pmsi_ops,
	.chip	= &its_pmsi_irq_chip,
};

static struct of_device_id its_device_id[] = {
	{	.compatible	= "arm,gic-v3-its",	},
	{},
};

static int __init its_pmsi_init_one(struct fwnode_handle *fwnode)
{
	struct irq_domain *parent;
	const char *name = irq_domain_get_irqchip_fwnode_name(fwnode);

	parent = irq_find_matching_fwnode(fwnode, DOMAIN_BUS_NEXUS);
	if (!parent || !msi_get_domain_info(parent)) {
		pr_err("%s: unable to locate ITS domain\n", name);
		return -ENXIO;
	}

	if (!platform_msi_create_irq_domain(fwnode, &its_pmsi_domain_info,
					    parent)) {
		pr_err("%s: unable to create platform domain\n", name);
		return -ENXIO;
	}

	pr_info("Platform MSI: %s domain created\n", name);
	return 0;
}

#ifdef CONFIG_ACPI
static int __init
its_pmsi_parse_madt(struct acpi_subtable_header *header,
			const unsigned long end)
{
	struct acpi_madt_generic_translator *its_entry;
	struct fwnode_handle *domain_handle;

	its_entry = (struct acpi_madt_generic_translator *)header;
	domain_handle = iort_find_its_domain_token(its_entry->translation_id);
	if (!domain_handle) {
		pr_err("ITS@0x%lx: Unable to locate ITS domain handle\n",
			(long)its_entry->base_address);
		return 0;
	}

	if (its_pmsi_init_one(domain_handle))
		return 0;

	platform_msi_register_fwnode_provider(&iort_find_platform_dev_domain_token);
	return 0;
}

static void __init its_acpi_pmsi_init(void)
{
	acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_TRANSLATOR,
				its_pmsi_parse_madt, 0);
}
#else
static inline void its_acpi_pmsi_init(void) { }
#endif

static int __init its_pmsi_init(void)
{
	struct device_node *np;

	for (np = of_find_matching_node(NULL, its_device_id); np;
	     np = of_find_matching_node(np, its_device_id)) {
		if (!of_property_read_bool(np, "msi-controller"))
			continue;

		if (its_pmsi_init_one(of_node_to_fwnode(np)))
			continue;
	}

	its_acpi_pmsi_init();
	return 0;
}
early_initcall(its_pmsi_init);
