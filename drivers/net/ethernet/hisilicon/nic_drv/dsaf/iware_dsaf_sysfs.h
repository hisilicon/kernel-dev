/*******************************************************************************

  Hisilicon dsa fabric driver
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

#ifndef __HRD_DSAF_SYS_H__
#define __HRD_DSAF_SYS_H__

/*file attr code-make micro */
#define sysyfs_dsaf_file_attr_def(file_name, func_show, func_store)\
static struct dsaf_sys_attr g_##func_show = {\
	 {\
		.name = #file_name,\
		.mode = S_IRUGO|S_IWUGO,\
	 },\
	 .show = func_show,\
	 .store = func_store,\
}

#define sysfs_dsaf_fun0_define(file_name, name, name1)\
void sysfs_##name(struct device *dev)\
{\
	struct dsaf_device *dsaf_dev =\
	(struct dsaf_device *)dev_get_drvdata(dev);\
	u32 chip_id = dsaf_dev->chip_id;\
	(void)name(chip_id);\
	return;\
} \
void sysfs_##name##name1(struct device *dev, const char *buf)\
{\
	return;\
} \
sysyfs_dsaf_file_attr_def(file_name, sysfs_##name, sysfs_##name##name1)

#define sysfs_dsaf_fun1_define(file_name, name, name1, p1)\
void sysfs_##name##p1(struct device *dev)\
{\
	struct dsaf_device *dsaf_dev =\
	(struct dsaf_device *)dev_get_drvdata(dev);\
	u32 chip_id = dsaf_dev->chip_id;\
	(void)name(chip_id, p1);\
	return;\
} \
void sysfs_##name##name1##p1(struct device *dev, const char *buf)\
{\
	return;\
} \
sysyfs_dsaf_file_attr_def(file_name, sysfs_##name##p1, sysfs_##name##name1##p1)

#define sysfs_dsaf_dev_fun0_define(file_name, name, name1)\
void sysfs_##name(struct device *dev)\
{\
	struct dsaf_device *dsaf_dev =\
	(struct dsaf_device *)dev_get_drvdata(dev);\
	(void)name(dsaf_dev);\
	return;\
} \
void sysfs_##name##name1(struct device *dev, const char *buf)\
{\
	return;\
} \
sysyfs_dsaf_file_attr_def(file_name, sysfs_##name, sysfs_##name##name1)

#define sysfs_dsaf_dev_fun1_define(file_name, name, name1, p1)\
void sysfs_##name##p1(struct device *dev)\
{\
	struct dsaf_device *dsaf_dev =\
	(struct dsaf_device *)dev_get_drvdata(dev);\
	(void)name(dsaf_dev, p1);\
	return;\
} \
void sysfs_##name##name1##p1(struct device *dev, const char *buf)\
{\
	return;\
} \
sysyfs_dsaf_file_attr_def(file_name, sysfs_##name##p1, sysfs_##name##name1##p1)

#define sysfs_dsaf_fun2_define(file_name, name, name1, p1, p2)\
void sysfs_##name##p1##p2(struct device *dev)\
{\
	struct dsaf_device *dsaf_dev =\
	(struct dsaf_device *)dev_get_drvdata(dev);\
	u32 chip_id = dsaf_dev->chip_id;\
	(void)name(chip_id, p1, p2);\
	return;\
} \
void sysfs_##name##name1##p1##p2(struct device *dev, const char *buf)\
{\
	return;\
} \
sysyfs_dsaf_file_attr_def(file_name,\
sysfs_##name##p1##p2, sysfs_##name##name1##p1##p2)

#ifndef SYSFS_PAGE_SIZE
#define SYSFS_PAGE_SIZE  4096 /* sysfs file size */
#endif

extern ssize_t dsaf_sys_show(
	struct kobject *kobj, struct attribute *attr, char *buf);

extern ssize_t dsaf_sys_store(
	struct kobject *kobj, struct attribute *attr,
	const char *buf, size_t count);

struct dsaf_sys_attr {
	struct attribute attr;

	void (*show)(struct device *dev);
	void (*store)(struct device *dev, const char *buf);
};

struct dsaf_sysfs_config {
	struct kobj_type obj_type;
	const char		*obj_name;
	unsigned char	 father_idx;
};

/*this struct is for saving dev and obj*/
struct dsaf_dev_kobj {
	struct device *dev;
	struct kobject kobj;
};

extern int dsaf_add_sysfs(struct device *dev);
extern void dsaf_del_sysfs(struct device *dev);

#endif
