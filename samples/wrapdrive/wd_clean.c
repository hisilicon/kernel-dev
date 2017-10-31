/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include "wd_util.h"


#define MDEV_BUS_DEV_DIR	"/sys/bus/mdev/devices"

int main(void)
{
	DIR *mdev_bus_devs;
	struct dirent *mdev;
	char rm_path[PATH_STR_SIZE];
	char para_path[PATH_STR_SIZE];
	int pid;
	char spid[PATH_STR_SIZE];

	mdev_bus_devs = opendir(MDEV_BUS_DEV_DIR);
	if (!mdev_bus_devs) {
		WD_ERR("WD framework is not enabled on this system!\n");
		return -ENODEV;
	}

	if (access("/proc/1", 0)) {
		closedir(mdev_bus_devs);
		printf("error: mount /proc fs first\n");
		return EXIT_FAILURE;
	}

	printf("clean all unused wd queue...\n");
	while ((mdev = readdir(mdev_bus_devs)) != NULL) {
		if (strncmp(mdev->d_name, ".", 1) == 0 ||
		    strncmp(mdev->d_name, "..", 2) == 0)
			continue;
		(void)strncpy(rm_path, MDEV_BUS_DEV_DIR"/", PATH_STR_SIZE);
		(void)strcat(rm_path, mdev->d_name);
		(void)strncpy(para_path, rm_path, PATH_STR_SIZE);
		(void)strcat(para_path, WD_QUEUE_PARAM_GRP_NAME);
		pid = _get_attr_value(rm_path,  "/pid");
		if (pid < 0) {
			printf("MDEV(%s) unknown ownver\n", mdev->d_name);
			continue;
		}
		memset(spid, 0, PATH_STR_SIZE);
		sprintf(spid, "/proc/%d", pid);
		if (!access(spid, 0)) {
			printf("(no user, kill device!)\n");
			wd_kill_mdev(rm_path);
		}
	}

	closedir(mdev_bus_devs);
	printf("done\n");

	return 0;
}
