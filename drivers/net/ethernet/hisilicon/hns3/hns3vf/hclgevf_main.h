#ifndef __HCLGEVF_MAIN_H
#define __HCLGEVF_MAIN_H
#include <linux/fs.h>
#include <linux/types.h>
#include "hclge_mbx.h"
#include "hclgevf_cmd.h"
#include "hnae3.h"

#define HCLGEVF_MOD_VERSION "v1.0"
#define HCLGEVF_DRIVER_NAME "hclgevf"

#define HCLGEVF_ROCEE_VECTOR_NUM	0
#define HCLGEVF_MISC_VECTOR_NUM		0

#define HCLGEVF_INVALID_VPORT 0xffff

/* This number in actual depends upon the total number of VFs
 * created by physical function. But the maximum number of
 * possible vector-per-VF is {VFn(1-32), VECTn(32 + 1)}.
 */
#define HCLGEVF_MAX_VF_VECTOR_NUM	(32 + 1)

#define HCLGEVF_VECTOR_REG_BASE		0x20000
#define HCLGEVF_MISC_VECTOR_REG_BASE	0x20400
#define HCLGEVF_VECTOR_REG_OFFSET	0x4
#define HCLGEVF_VECTOR_VF_OFFSET	0x100000

/* Vector0 interrupt CMDQ event source register(RW) */
#define HCLGEVF_VECTOR0_CMDQ_SRC_REG	0x27100
/* CMDQ register bits for RX event(=MBX event) */
#define HCLGEVF_VECTOR0_RX_CMDQ_INT_B	1

#define HCLGEVF_TQP_RESET_TRY_TIMES	10

#define HCLGEVF_RSS_IND_TBL_SIZE		512
#define HCLGEVF_RSS_SET_BITMAP_MSK	0xffff
#define HCLGEVF_RSS_KEY_SIZE		40
#define HCLGEVF_RSS_HASH_ALGO_TOEPLITZ	0
#define HCLGEVF_RSS_HASH_ALGO_SIMPLE	1
#define HCLGEVF_RSS_HASH_ALGO_SYMMETRIC	2
#define HCLGEVF_RSS_HASH_ALGO_MASK	0xf
#define HCLGEVF_RSS_CFG_TBL_NUM \
	(HCLGEVF_RSS_IND_TBL_SIZE / HCLGEVF_RSS_CFG_TBL_SIZE)

#define HCLGEVF_RSS_TC_SIZE_0		1
#define HCLGEVF_RSS_TC_SIZE_1		2
#define HCLGEVF_RSS_TC_SIZE_2		4
#define HCLGEVF_RSS_TC_SIZE_3		8
#define HCLGEVF_RSS_TC_SIZE_4		16
#define HCLGEVF_RSS_TC_SIZE_5		32
#define HCLGEVF_RSS_TC_SIZE_6		64
#define HCLGEVF_RSS_TC_SIZE_7		128

/* states of hclgevf device & tasks */
enum hclgevf_states {
	/* device states */
	HCLGEVF_STATE_DOWN,
	HCLGEVF_STATE_DISABLED,
	/* task states */
	HCLGEVF_STATE_SERVICE_SCHED,
	HCLGEVF_STATE_MBX_SERVICE_SCHED,
	HCLGEVF_STATE_MBX_HANDLING,
};

#define HCLGEVF_MPF_ENBALE 1

struct hclgevf_mac {
	u8 mac_addr[ETH_ALEN];
	int link;
};

struct hclgevf_hw {
	void __iomem *io_base;
	int num_vec;
	struct hclgevf_cmq cmq;
	struct hclgevf_mac mac;
	void *back;
};

/* TQP stats */
struct hlcgevf_tqp_stats {
	/* query_tqp_tx_queue_statistics ,opcode id:  0x0B03 */
	u64 rcb_tx_ring_pktnum_rcd; /* 32bit */
	/* query_tqp_rx_queue_statistics ,opcode id:  0x0B13 */
	u64 rcb_rx_ring_pktnum_rcd; /* 32bit */
};

struct hclgevf_tqp {
	struct device *dev;	/* Device for DMA mapping */
	struct hnae3_queue q;
	struct hlcgevf_tqp_stats tqp_stats;
	u16 index;	/* Global index in a NIC controller */

	bool alloced;
};

enum hclgevf_fc_mode {
	HCLGEVF_FC_NONE = 0,
	HCLGEVF_FC_RX_PAUSE,
	HCLGEVF_FC_TX_PAUSE,
	HCLGEVF_FC_FULL,
	HCLGEVF_FC_PFC,
	HCLGEVF_FC_DEFAULT
};

struct hclgevf_cfg {
	u8 vmdq_vport_num;
	u8 tc_num;
	u16 tqp_desc_num;
	u16 rx_buf_len;
	u8 phy_addr;
	u8 media_type;
	u8 mac_addr[ETH_ALEN];
	u32 numa_node_map;
};

struct hclgevf_rss_cfg {
	u8  rss_hash_key[HCLGEVF_RSS_KEY_SIZE]; /* User configured hash keys */
	u32 hash_algo;
	u32 rss_size;
	u8 hw_tc_map;
	/* User configured lookup table entries */
	u8  rss_indirection_tbl[HCLGEVF_RSS_IND_TBL_SIZE];
};

/* VPort level vlan tag configuration for TX direction */
struct hclgevf_tx_vtag_cfg {
	bool accept_tag;	/* Whether accept tagged packet from host */
	bool accept_untag;	/* Whether accept untagged packet from host */
	bool insert_tag1_en;	/* Whether insert inner vlan tag */
	bool insert_tag2_en;	/* Whether insert outer vlan tag */
	u16  default_tag1;	/* The default inner vlan tag to insert */
	u16  default_tag2;	/* The default outer vlan tag to insert */
};

/* VPort level vlan tag configuration for RX direction */
struct hclgevf_rx_vtag_cfg {
	bool strip_tag1_en;	/* Whether strip inner vlan tag */
	bool strip_tag2_en;	/* Whether strip outer vlan tag */
	bool vlan1_vlan_prionly;/* Inner VLAN Tag up to descriptor Enable */
	bool vlan2_vlan_prionly;/* Outer VLAN Tag up to descriptor Enable */
};

struct hclgevf_dev {
	struct pci_dev *pdev;
	struct hnae3_ae_dev *ae_dev;
	struct hclgevf_hw hw;
	struct hclgevf_misc_vector misc_vector;
	struct hclgevf_rss_cfg rss_cfg;
	unsigned long state;

	struct hclgevf_tx_vtag_cfg  txvlan_cfg;
	struct hclgevf_rx_vtag_cfg  rxvlan_cfg;

	u32 fw_version;
	u16 num_tqps;			/* Num task queue pairs of this PF */

	/* Base task tqp physical id of this PF */
	u16 base_tqp_pid;
	u16 alloc_rss_size;		/* Allocated RSS task queue */
	u16 rss_size_max;		/* HW defined max RSS task queue */

	/* Num of guaranteed filters for this PF */
	u16 fdir_pf_filter_count;
	u16 num_alloc_vport;		/* Num vports this driver supports */
	u32 numa_node_mask;
	u16 rx_buf_len;
	u16 num_desc;
	u8 hw_tc_map;
	enum hclgevf_fc_mode fc_mode_last_time;
#define HCLGEVF_FLAG_TC_BASE_SCH_MODE		1
#define HCLGEVF_FLAG_VNET_BASE_SCH_MODE     2

	u8 default_up;

	struct ieee_ets *hclgevf_ieee_ets;
	struct ieee_pfc *hclgevf_ieee_pfc;

	u16 num_msi;
	u16 num_msi_left;
	u16 num_msi_used;
	u32 base_msi_vector;
	u16 *vector_status;
	int *vector_irq;

	u16 pending_udp_bitmap;

	u16 rx_itr_default;
	u16 tx_itr_default;

	bool accept_mta_mc; /* Whether accept mta filter multicast */
	struct hclgevf_mbx_resp_status mbx_resp; /* MBX response */

	u16 adminq_work_limit; /* Num of admin receive queue desc to process */
	unsigned long service_timer_period;
	unsigned long service_timer_previous;
	struct timer_list service_timer;
	struct work_struct service_task;
	struct work_struct mbx_service_task;

	bool cur_promisc;
	int num_alloc_vfs;	/* Actual number of VFs allocated */

	struct hclgevf_tqp *htqp;

	struct hnae3_handle nic;
	struct hnae3_handle roce;

	struct hnae3_client *nic_client;
	struct hnae3_client *roce_client;
	u32 flag;
};

int hclgevf_send_mbx_msg(struct hclgevf_dev *hdev, u16 code, u16 subcode,
			 const u8 *msg_data, u8 msg_len, bool need_resp,
			 u8 *resp_data, u16 resp_len);
void hclgevf_mbx_handler(struct hclgevf_dev *hdev);
void hclgevf_update_link_status(struct hclgevf_dev *hdev, int link_state);
#endif
