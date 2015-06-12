/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd														 */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfTdmmW1lpew9jFMPkqw+y7uUX7KIxXxZdg6VP3raNKR0kDz+Vr7WsVcjvgY25lxd4htn
gcqD80lhlKSAFyPAZnzduMs/17bj91E3Kkw6y+u8bhaDwZ588c6wregkZtNlaw==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon network interface controller driver
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:TBD

*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/slab.h>

/* #include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_error.h"
#include "iware_nic_main.h"

#ifndef SYSFS_PAGE_SIZE
#define SYSFS_PAGE_SIZE  4096
#endif

static ssize_t nic_sys_show(struct kobject *kobj, struct attribute *attr,
	char *buf);
static void nic_show_all_sw_stat_by_port(struct device *dev);
static void nic_show_all_reg_by_port(struct device *dev);

struct nic_sys_attr {
	struct attribute attr;

	void (*show)(struct device *dev);
	void (*store)(struct device *dev, const char *buf);
};

static struct nic_sys_attr g_ppe_stat_attr = {
	{
	 .name = "ppe_stat",
	 .mode = S_IRUGO | S_IWUGO,
	 },
	.show = ppe_show_stat_by_port,
	.store = NULL,
};

static struct nic_sys_attr g_rcb_stat_attr = {
	{
	 .name = "rcb_rxtx",
	 .mode = S_IRUGO | S_IWUGO,
	 },
	.show = rcb_show_stat_by_port,
	.store = NULL,
};

static struct nic_sys_attr g_mac_stat_attr = {
	{
	 .name = "mac_static",
	 .mode = S_IRUGO | S_IWUGO,
	 },
	.show = mac_show_stat_by_port,
	.store = NULL,
};

static struct nic_sys_attr g_nic_sw_info_all_attr = {
	{
	 .name = "nic_sw_info_all",
	 .mode = S_IRUGO | S_IWUGO,
	 },
	.show = nic_show_all_sw_stat_by_port,
	.store = NULL,
};

static struct nic_sys_attr g_nic_show_all_reg_attr = {
	{
	 .name = "nic_dump_reg",
	 .mode = S_IRUGO | S_IWUGO,
	 },
	.show = nic_show_all_reg_by_port,
	.store = NULL,
};

static struct attribute *g_nic_sys_attrs_list[] = {
	&g_ppe_stat_attr.attr,
	&g_rcb_stat_attr.attr,
	&g_mac_stat_attr.attr,
	&g_nic_sw_info_all_attr.attr,
	&g_nic_show_all_reg_attr.attr,
	NULL
};

static const struct sysfs_ops g_nic_sys_ops = {
	.show = nic_sys_show,
	.store = NULL,
};

static struct kobj_type g_nic_sys_ktype = {
	.release = NULL,
	.sysfs_ops = &g_nic_sys_ops,
	.default_attrs = g_nic_sys_attrs_list,
};

struct nic_dev_kobj {
	struct device *dev;
	struct kobject kobj;
};
static struct nic_dev_kobj g_nic_kobj[DSAF_MAX_CHIP_NUM][NIC_MAX_NUM_PER_CHIP];

/**
 * nic_show_all_reg_by_port - show all register by port
 * @dev: device
 */
static void nic_show_all_reg_by_port(struct device *dev)
{
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;
	struct ppe_device *ppe_device = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	struct nic_ring_pair *ring = NULL;
	struct mac_device *mac_dev = NULL;
	int i;

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}

	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(dev, "mac_dev is NULL!\n");
		return;
	}

	(void)mac_dev->get_dump_regs(mac_dev);

	ppe_device = nic_dev->ppe_device;
	if ((NULL != ppe_device) && (NULL != ppe_device->ops.dump_regs))
		ppe_device->ops.dump_regs(ppe_device);

	rcb_common = nic_dev->rcb_common;
	if ((NULL != rcb_common) && (NULL != rcb_common->ops.dump_regs))
		rcb_common->ops.dump_regs(rcb_common);

	for (i = 0; i < nic_dev->ring_pair_num; i++) {
		ring = nic_dev->ring[i];
		if ((NULL != ring) && (NULL != ring->rcb_dev.ops.dump_reg))
			ring->rcb_dev.ops.dump_reg(ring);
	}
}

/**
 * nic_show_all_sw_stat_by_port - show all software status by port
 * @dev: device
 */
static void nic_show_all_sw_stat_by_port(struct device *dev)
{
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;
	struct ppe_device *ppe_device = NULL;
	int i;

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}
	nic_show_sw_stat_by_port(netdev);

	ppe_common_show_sw_info(nic_dev->ppe_common);

	ppe_device = nic_dev->ppe_device;
	while (ppe_device) {
		ppe_show_sw_stat_by_port(ppe_device);
		ppe_device = ppe_device->next;
	}

	rcb_common_show_sw_info(nic_dev->rcb_common);

	for (i = 0; i < nic_dev->ring_pair_num; i++)
		rcb_show_sw_stat_by_ring(nic_dev->ring[i]);
}

static ssize_t nic_sys_show(struct kobject *kobj, struct attribute *attr,
	char *buf)
{
	struct nic_sys_attr *sys_attr
	    = container_of(attr, struct nic_sys_attr, attr);
	struct nic_dev_kobj *nic_kobj
	    = container_of(kobj, struct nic_dev_kobj, kobj);
	struct device *dev = nic_kobj->dev;

	if (sys_attr->show) {
		sys_attr->show(dev);
		return strlen(buf);
	}
	return -ENOSYS;
}

int nic_add_sysfs(struct device *dev)
{
	int ret;
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;
	u32 chip_id;
	u32 nic_idx;

	log_dbg(dev, "func begin\n");

	if (NULL == netdev) {
		log_err(dev, "netdev is NULL!\n");
		return -ENODEV;
	}

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return -ENODEV;
	}
	chip_id = nic_dev->chip_id;
	nic_idx = nic_dev->index;
	log_dbg(dev, "nic_idx%d\n", nic_dev->gidx);

	memset(&g_nic_kobj[chip_id][nic_idx], 0, sizeof(struct nic_dev_kobj));

	ret = kobject_init_and_add(&(g_nic_kobj[chip_id][nic_idx].kobj),
				   &g_nic_sys_ktype, &dev->kobj,
				   "user_control");
	if (ret) {
		log_err(dev, "kobject_init_and_add failed! ret=%d\n", ret);
		return ret;
	}

	g_nic_kobj[chip_id][nic_idx].dev = dev;

	return 0;
}

void nic_del_sysfs(struct device *dev)
{
	u32 chip_id;
	u32 nic_idx;
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;

	log_dbg(dev, "func begin\n");

	if (NULL == netdev) {
		log_err(dev, "netdev is NULL!\n");
		return;
	}

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}

	chip_id = nic_dev->chip_id;
	nic_idx = nic_dev->index;

	kobject_del(&(g_nic_kobj[chip_id][nic_idx].kobj));
	memset(&g_nic_kobj[chip_id][nic_idx], 0, sizeof(struct nic_dev_kobj));
}
