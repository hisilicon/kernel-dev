// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2019 Arm Ltd.
// Based on arch/x86/kernel/cpu/intel_rdt.h

#ifndef __LINUX_RESCTRL_TYPES_H
#define __LINUX_RESCTRL_TYPES_H

enum resctrl_conf_type {
	CDP_BOTH = 0,
	CDP_CODE,
	CDP_DATA,
};
#define NUM_CDP_TYPES  CDP_DATA + 1

/*
 * Event IDs, the values match those used to program IA32_QM_EVTSEL before
 * reading IA32_QM_CTR on RDT systems.
 */
enum resctrl_event_id {
	QOS_L3_OCCUP_EVENT_ID           = 0x01,
	QOS_L3_MBM_TOTAL_EVENT_ID       = 0x02,
	QOS_L3_MBM_LOCAL_EVENT_ID       = 0x03,

	/* Must be the last */
	RESCTRL_NUM_EVENT_IDS,
};

enum resctrl_resource_level {
	RDT_RESOURCE_L3,
	RDT_RESOURCE_L2,
	RDT_RESOURCE_MBA,

	/* Must be the last */
	RDT_NUM_RESOURCES,
};

#endif /* __LINUX_RESCTRL_TYPES_H */
