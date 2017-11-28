/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __HCLGE_MBX_H
#define __HCLGE_MBX_H
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/types.h>

#define HCLGE_MBX_VF_MSG_DATA_NUM	16

enum HCLGE_MBX_OPCODE {
	HCLGE_MBX_RESET		= 0x01, /* VF requests reset */
	HCLGE_MBX_SET_UNICAST,   /* VF requests PF to set MAC addr */
	HCLGE_MBX_SET_MULTICAST, /* VF requests PF to set MC addr */
	HCLGE_MBX_SET_VLAN,      /* VF requests PF to set VLAN */
	HCLGE_MBX_MAP_RING_TO_VECTOR,
	HCLGE_MBX_UNMAP_RING_TO_VECTOR,
	HCLGE_MBX_SET_PROMISC_MODE,
	HCLGE_MBX_SET_MACVLAN,   /* VF requests PF for unicast filter */
	HCLGE_MBX_API_NEGOTIATE, /* Negotiate API version */
	HCLGE_MBX_GET_QINFO,      /* Get queue configuration */
	HCLGE_MBX_GET_TCINFO,	  /* Get TC configuration */
	HCLGE_MBX_GET_RETA,      /* VF request for RETA */
	HCLGE_MBX_GET_RSS_KEY,   /* Get RSS key */
	HCLGE_MBX_GET_MAC_ADDR,  /* VF requests PF to get MAC addr */
	HCLGE_MBX_PF_VF_RESP,    /* PF generate respone to vf */
	HCLGE_MBX_GET_BDNUM,     /* VF requests PF to get BD num */
	HCLGE_MBX_GET_BUFSIZE,	 /* VF requests PF to get buffer size */
	HCLGE_MBX_GET_STREAMID,	 /* VF requests PF to get stream id */
	HCLGE_MBX_SET_AESTART,	 /* VF requests PF to start ae */
	HCLGE_MBX_SET_TSOSTATS,	 /* VF requests PF to get tso stats */
	HCLGE_MBX_LINK_STAT_CHANGE, /* PF notify VF link stat changed */
	HCLGE_MBX_GET_BASE_CONFIG, /* VF requests PF to get config */
	HCLGE_MBX_BIND_FUNC_QUEUE, /* Request PF to bind function and queue */
	HCLGE_MBX_GET_LINK_STATUS, /* get link status from PF */
	HCLGE_MBX_QUEUE_RESET,	   /* VF request to PF to reset queue */
};

enum hclge_mbx_mac_vlan_subcode {
	/* VF request to modify UC mac addr */
	HCLGE_MBX_MAC_VLAN_UC_MODIFY = 0,
	/* VF request to add a new UC mac addr */
	HCLGE_MBX_MAC_VLAN_UC_ADD,
	/* VF request to remove a new UC mac addr */
	HCLGE_MBX_MAC_VLAN_UC_REMOVE,
	/* VF request to modify MC mac addr */
	HCLGE_MBX_MAC_VLAN_MC_MODIFY,
	/* VF request to add a new MC mac addr */
	HCLGE_MBX_MAC_VLAN_MC_ADD,
	/* VF request to remove a new MC mac addr */
	HCLGE_MAC_VLAN_MC_REMOVE,
	/* VF request to config func MTA enable */
	HCLGE_MBX_MAC_VLAN_MC_FUNC_MTA_ENABLE,
};

enum hclge_mbx_vlan_cfg_subcode {
	/* Request to set vlan filter for this vf */
	HCLGE_MBX_VLAN_FILTER = 0,
	/* Request to set tx side vlan offload for this vf */
	HCLGE_MBX_VLAN_TX_OFF_CFG,
	/* Request to set rx side vlan offload for this vf */
	HCLGE_MBX_VLAN_RX_OFF_CFG,
};

#define HCLGE_MBX_MAX_MSG_SIZE	16
#define HCLGE_MBX_MAX_RESP_DATA_SIZE	8

struct hclgevf_mbx_resp_status {
	struct mutex mbx_mutex; /* status queue lock */
	u32 origin_mbx_msg;
	bool received_resp;
	int resp_status;
	u8  addtional_info[HCLGE_MBX_MAX_RESP_DATA_SIZE];
};

struct hclge_mbx_vf_to_pf_cmd {
	u8 rsv;
	u8 mbx_src_vfid; /* Auto filled by IMP */
	u8 rsv1[2];
	u8 msg_len;
	u8 rsv2[3];
	u8 msg[HCLGE_MBX_MAX_MSG_SIZE];
};

struct hclge_mbx_pf_to_vf_cmd {
	u16 dest_vfid;
	u8 rsv[2];
	u8 msg_len;
	u8 rsv1[3];
	u16 msg[8];
};

#define hclge_mbx_ring_ptr_move_crq(crq, next_to_use) \
	(crq->next_to_use = (crq->next_to_use + 1) % crq->desc_num)
#endif
