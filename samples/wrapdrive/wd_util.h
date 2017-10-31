/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* the common drv header define the unified interface for wd */
#ifndef __WD_UTIL_H__
#define __WD_UTIL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "../../include/uapi/linux/vfio.h"
#include "wd.h"


#ifndef WD_ERR
#define WD_ERR(format, args...) printf(format, ##args)
#endif

int wd_write_sysfs_file(const char *path, char *buf, int size);

static inline void wd_reg_write(void *reg_addr, uint32_t value)
{
	*((volatile uint32_t *)reg_addr) = value;
}

static inline uint32_t wd_reg_read(void *reg_addr)
{
	uint32_t temp;

	temp = *((volatile uint32_t *)reg_addr);

	return temp;
}

static inline int _get_attr_value(const char *path, const char *attr_name)
{
	char attr_path[PATH_STR_SIZE];
	int fd, ret;
	char value[SYS_VAL_SIZE];

	(void)sprintf(attr_path, "%s/%s", path, attr_name);
	fd = open(attr_path, O_RDONLY);
	if (fd < 0)
		return fd;
	memset(value, 0, SYS_VAL_SIZE);
	ret = read(fd, value, SYS_VAL_SIZE);
	if (ret > 0) {
		close(fd);
		return atoi(value);
	}
	close(fd);

	return ret;
}

static inline int _set_attr_value(const char *path, const char *attr_name, char *value)
{
	char attr_path[PATH_STR_SIZE];
	int fd, ret;

	(void)sprintf(attr_path, "%s/%s",  path, attr_name);
	fd = open(attr_path, O_WRONLY);
	if (fd < 0)
		return fd;

	ret = write(fd, value, SYS_VAL_SIZE);
	if (ret >= 0) {
		close(fd);
		return 0;
	}
	close(fd);

	return -EFAULT;
}
static inline void wd_kill_mdev(char *dev_path)
{
	char buf[PATH_STR_SIZE];
	FILE *f;

	strncpy(buf, dev_path, PATH_STR_SIZE);
	strcat(buf, "/remove");
	f = fopen(buf, "w");
	if (!f) {
		WD_ERR("wrapdrive kill mdev failt: open %s fail\n", buf);
		return;
	}
	fwrite("1", 1, 1, f);
	fclose(f);
}

#endif
