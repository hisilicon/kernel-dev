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

#define DJTAG_CLIENT_NAME_LEN 32

struct hisi_djtag_client {
	int id; /* Unique identifier to sufix in client->dev.name */
	struct hisi_djtag_host *host;
	struct list_head next;
	char name[DJTAG_CLIENT_NAME_LEN];
	struct device dev;
};

struct hisi_djtag_driver {
	struct device_driver driver;
	int (*probe)(struct hisi_djtag_client *);
	int (*remove)(struct hisi_djtag_client *);
};

extern struct bus_type hisi_djtag_bus;

int hisi_djtag_register_driver(struct module *owner,
			       struct hisi_djtag_driver *driver);
void hisi_djtag_unregister_driver(struct hisi_djtag_driver *driver);
int hisi_djtag_readl(struct hisi_djtag_client *client, u32 offset,
		     u32 mod_sel, int chain_id, u32 *val);
int hisi_djtag_writel(struct hisi_djtag_client *client, u32 offset,
		      u32 mod_sel, u32 mod_mask, u32 val);
u32 hisi_djtag_get_sclid(struct hisi_djtag_client *client);
#endif /* __HISI_DJTAG_H */
