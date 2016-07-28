/*
 * Driver for Hisilicon djtag r/w via System Controller.
 *
 * Copyright (C) 2016-2017 Hisilicon Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HISI_DJTAG_H
#define __HISI_DJTAG_H

#define DJATG_CLIENT_NAME_LEN 24

struct hisi_djtag_client {
	struct hisi_djtag_host *host;
	char name[DJATG_CLIENT_NAME_LEN];
	struct device dev;
};

struct hisi_djtag_driver {
	struct device_driver driver;
	int (*probe)(struct hisi_djtag_client *);
};

extern struct bus_type hisi_djtag_bus;

int hisi_djtag_register_driver(struct module *, struct hisi_djtag_driver *);
int hisi_djtag_readl(struct hisi_djtag_client *, u32, u32, int, u32 *);
int hisi_djtag_writel(struct hisi_djtag_client *, u32, u32, u32, u32);
#endif /* __HISI_DJTAG_H */
