// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Hisilicon Limited.
 * Author: Shiju Jose <shiju.jose@huawei.com>
 */

/*
 * EDAC - Non Standard Error Reporting
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include "linux/edac_non_standard.h"

#include "edac_mc.h"

#define EDAC_NS "NON_STANDARD"

static DEFINE_MUTEX(edac_ns_mutex);
static LIST_HEAD(edac_ns_handler_list);

int edac_non_standard_event_notify_register(const struct edac_ns_handler *h)
{
	struct edac_ns_notify_info *ns_notify;

	mutex_lock(&edac_ns_mutex);

	ns_notify = kzalloc(sizeof(*ns_notify), GFP_KERNEL);
	if (!ns_notify)
		return -ENOMEM;

	ns_notify->notify  = h->notify;
	guid_copy(&ns_notify->id, &h->id);
	ns_notify->pvt_data  = h->pvt_data;
	list_add_tail(&ns_notify->list, &edac_ns_handler_list);

	mutex_unlock(&edac_ns_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(edac_non_standard_event_notify_register);

void edac_non_standard_event_notify_unregister(const struct edac_ns_handler *h)
{
	struct edac_ns_notify_info *ns_notify;

	mutex_lock(&edac_ns_mutex);
	list_for_each_entry(ns_notify, &edac_ns_handler_list, list) {
		if (guid_equal(&ns_notify->id, &(h->id))) {
			list_del(&ns_notify->list);
			kfree(ns_notify);
			break;
		}
	}
	mutex_unlock(&edac_ns_mutex);
}
EXPORT_SYMBOL_GPL(edac_non_standard_event_notify_unregister);

#define EDAC_NS_RECOVER_RING_SIZE           16

static DEFINE_KFIFO(edac_ns_recover_ring, struct edac_ns_err_info,
		    EDAC_NS_RECOVER_RING_SIZE);
/*
 * Mutual exclusion for writers of edc_ns_recover_ring, reader side don't
 * need lock, because there is only one reader and lock is not needed
 * between reader and writer.
 */
static DEFINE_SPINLOCK(edac_ns_recover_ring_lock);

static void edac_ns_error_recover_work_func(struct work_struct *work)
{
	struct edac_ns_err_info ns_err_entry;
	struct edac_ns_notify_info *edac_ns_notify;

	/* run the registered non-standard error handlers */
	while (kfifo_get(&edac_ns_recover_ring, &ns_err_entry)) {
		list_for_each_entry(edac_ns_notify,
				    &edac_ns_handler_list, list) {
			if (guid_equal(&edac_ns_notify->id,
				       &ns_err_entry.id))
				edac_ns_notify->notify(&ns_err_entry);
		}
		kfree(ns_err_entry.err_data);
		kfree(ns_err_entry.fru_text);
	}
}

static DECLARE_WORK(edac_ns_recover_work, edac_ns_error_recover_work_func);

void edac_report_non_standard_error(const guid_t *sec_type,
				    const guid_t *fru_id,
				    const char *fru_text, const u8 sev,
				    const u8 *err, const u32 len)
{
	struct edac_ns_err_info ns_err;

	ns_err.fru_text =  kzalloc((strlen(fru_text) + 1), GFP_KERNEL);
	if (!ns_err.fru_text)
		return;

	ns_err.err_data = kzalloc(len, GFP_KERNEL);
	if (!ns_err.err_data) {
		kfree(ns_err.fru_text);
		return;
	}

	guid_copy(&ns_err.id, sec_type);
	ns_err.err_sev = sev;
	guid_copy(&ns_err.fru_id, fru_id);
	strcpy(ns_err.fru_text, fru_text);
	memcpy(ns_err.err_data, err, len);
	ns_err.err_data_len = len;

	if (kfifo_in_spinlocked(&edac_ns_recover_ring, &ns_err, 1,
				&edac_ns_recover_ring_lock))
		schedule_work(&edac_ns_recover_work);
	else
		pr_warn("Buffer overflow when recovering NS for %pU\n",
			&ns_err.id);
}
EXPORT_SYMBOL_GPL(edac_report_non_standard_error);

