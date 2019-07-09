/* SPDX-License-Identifier: GPL-2.0+ */
/*  Copyright (c) 2019 Hisilicon Limited. */

#ifndef _EDAC_NON_STANDARD_H_
#define _EDAC_NON_STANDARD_H_

#include <linux/list.h>
#include <linux/types.h>

struct edac_ns_err_info {
	guid_t id;
	guid_t fru_id;
	char *fru_text;
	u8 err_sev;
	u32 err_data_len;
	void *err_data;
};

struct edac_ns_handler {
	guid_t id;  /* guid of the vendor non-standard error table */
	void *pvt_data;/* pointer to the driver's private data */
	int (*notify)(const struct edac_ns_err_info *ns_err);
};

/*
 * edac_non_standard error notify info structure
 *
 */
struct edac_ns_notify_info {
	struct list_head list;
	guid_t id; /* guid of the vendor non-standard error table */
	/* pointer to edac handler func for the non-standard errors */
	int (*notify)(const struct edac_ns_err_info *ns_err);
	void *pvt_data; /* pointer to the driver's private data */
};

/*
 * edac_non_standard APIs
 */

/**
 * edac_non_standard_event_notify_register:
 *	This function help to register the non-standard event handler/s.
 *
 * @h: struct edac_ns_handler
 *
 * Return: 0 on success; negative otherwise.
 */
int edac_non_standard_event_notify_register(const struct edac_ns_handler *h);

/**
 * edac_non_standard_event_notify_unregister:
 *	This function help to unregister the non-standard event handler/s.
 *
 * @h: struct edac_ns_handler
 *
 * Return: none.
 */
void edac_non_standard_event_notify_unregister(const struct edac_ns_handler *h);


/**
 * edac_non_standard_event_notify_unregister:
 *      This function help to unregister the non-standard event handler/s.
 *
 * @ns_err: struct edac_ns_err_info
 *
 * Return: none.
 */
void edac_report_non_standard_error(const guid_t *sec_type,
				    const guid_t *fru_id,
				    const char *fru_text, const u8 sev,
				    const u8 *err, const u32 len);
#endif
