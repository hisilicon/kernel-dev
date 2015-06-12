/*
 * Huawei driver
 *
 * Copyright (C)
 * Author: Huawei majun258@huawei.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/nmi.h>
#include <linux/cpu.h>
#include <asm/smp_plat.h>

static struct class *cp_class;
static struct device *cp_dev;

static int hisi_nmi_handler(void)
{
	pr_info("%s: cpu %d\n", __func__, smp_processor_id());
	return 0;
}

#ifndef CONFIG_HISI_AARCH64_NMI 
void touch_nmi_watchdog(void)
{
	/* add your function here to feed watchdog */
	touch_softlockup_watchdog();
}
EXPORT_SYMBOL(touch_nmi_watchdog);
#endif

static ssize_t cpu_nmi_state_show(struct device *pdev,
				  struct device_attribute *attr, char *buf)
{
	int cpu = pdev->id;

	return sprintf(buf, "The cpu work status is %s\n",
		       cpu_nmi[cpu].state ? "active" : "inactive");
}

/* This funciton is designed to trigger the CPU deadlock by inputing
* " echo test > /sys/class/cpu_panic_class/cpu_panic/cpu_status"
*/
static ssize_t cpu_nmi_state_set(struct device *pdev, struct device_attribute
			*attr, const char *buf, size_t count)
{
	int cpu = pdev->id;
	u64 cpu_hwid = cpu_logical_map(cpu);

	pr_info("cpu id: %llx, buf is %s,count: %zx\n", cpu_hwid, buf, count);

	if (!strncmp(buf, "on", 2)) {
		cpu_nmi[cpu].state = NMI_WATCHDOG_ON;
		nmi_set_active_state(cpu_hwid, cpu_nmi[cpu].state);
	} else if (!strncmp(buf, "off", 3)) {
		cpu_nmi[cpu].state = NMI_WATCHDOG_OFF;
		nmi_set_active_state(cpu_hwid, cpu_nmi[cpu].state);
	} else {
		pr_info("unsupported command %s\n", buf);
	}
	return count;
}

static DEVICE_ATTR(nmi_state, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
		   cpu_nmi_state_show, cpu_nmi_state_set);


static ssize_t cpu_nmi_timeout_show(struct device *pdev,
				    struct device_attribute *attr, char *buf)
{
	int cpu = pdev->id;

	return sprintf(buf, "The cpu %d nmi watchdog timeout is %d\n",
		       cpu, cpu_nmi[cpu].timeout);
}
/* This funciton is designed to trigger the CPU deadlock by inputing
* " echo test > /sys/class/cpu_panic_class/cpu_panic/cpu_status"
*/

static ssize_t cpu_nmi_timeout_set(struct device *pdev, struct device_attribute
			*attr, const char *buf, size_t count)
{
	int cpu = pdev->id;
	u64 cpu_hwid = cpu_logical_map(cpu);
	long timeout;

	pr_info("cpu id: %llx, buf is %s,count: %zx\n", cpu_hwid, buf, count);

	if (kstrtol(buf, 10, &timeout)) {
		pr_info("get buf error\n");
		return count;
	}

	cpu_nmi[cpu].timeout = timeout;
	nmi_set_timeout(cpu_hwid, timeout);

	return count;
}

static DEVICE_ATTR(nmi_timeout, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
		   cpu_nmi_timeout_show, cpu_nmi_timeout_set);

static int __init cpu_monitor_init(void)
{
	char name[20];
	int ret = -1, cpu;

	cp_class = class_create(THIS_MODULE, "cpu_nmi_class");
	if (IS_ERR(cp_class)) {
		ret = PTR_ERR(cp_class);
		BUG();
	}

	for_each_possible_cpu(cpu) {
		sprintf(name, "cpu%d", cpu);
		cp_dev = device_create(cp_class, NULL, MKDEV(225, cpu), NULL, name);
		if (IS_ERR(cp_dev)) {
			ret = PTR_ERR(cp_dev);
			BUG();
		}
		cp_dev->id = cpu;
		ret = device_create_file(cp_dev, &dev_attr_nmi_state);
		if (ret) {
			dev_err(cp_dev, "Failed to create sysfs for cpu monitor state\n");
			goto exit;
		}
		ret = device_create_file(cp_dev, &dev_attr_nmi_timeout);
		if (ret) {
			dev_err(cp_dev, "Failed to create sysfs for cpu monitor timeout\n");
			goto exit;
		}
		cpu_nmi[cpu].timeout = 0;
		cpu_nmi[cpu].state = NMI_WATCHDOG_OFF;
		cpu_nmi[cpu].nmi_handler = NULL;
		register_nmi_handler(cpu, hisi_nmi_handler);
	}
exit:
	return ret;
}

static void __exit cpu_monitor_exit(void)
{
	device_destroy(cp_class, MKDEV(225, 0));
	class_destroy(cp_class);
}

module_init(cpu_monitor_init);
module_exit(cpu_monitor_exit);


