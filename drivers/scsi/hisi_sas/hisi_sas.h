#ifndef _HISI_SAS_H_
#define _HISI_SAS_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <scsi/scsi.h>
#include <scsi/libsas.h>

#define DRV_NAME "hisi_sas"
#define DRV_VERSION "v0.x"

#define HISI_SAS_MAX_CORE 3

#define HISI_SAS_MAX_PHYS	9
#define HISI_SAS_QUEUES 32
#define HISI_SAS_QUEUE_SLOTS 4096
// j00310691 should be 4096, but reducing as dma alloc fails
#define HISI_SAS_MAX_ITCT_ENTRIES 4096
#define HISI_SAS_MAX_DEVICES HISI_SAS_MAX_ITCT_ENTRIES
#define HISI_SAS_COMMAND_ENTRIES 8192

#define HISI_SAS_ID_NOT_MAPPED 0x7f

#define HISI_SAS_STATUS_BUF_SZ 1024
#define HISI_SAS_ITCT_ENTRY_SZ 128

#define HISI_SAS_MAX_SG 10

enum dev_status {
	HISI_SAS_DEV_NORMAL,
	HISI_SAS_DEV_EH
};

struct hba_info_page {
	int unused;
	// To be completed, j00310691
};

struct hisi_sas_phy {
	u8	port_attached;
	u64	dev_sas_addr;
	struct asd_sas_phy	sas_phy;
	// To be completed, j00310691
};

struct hisi_sas_port {
	struct asd_sas_port	sas_port;
	u8	port_attached;
	u8	wide_port_phymap;
	struct list_head	list;
};

struct hiti_sas_itct {
	// To be completed, j00310691
};

struct hisi_sas_device {
	struct list_head	dev_entry;
	enum sas_device_type	dev_type;
	struct hisi_hba 	*hisi_hba;
	struct domain_device	*sas_device;
	struct timer_list	timer;
	u32 attached_phy;
	u32 device_id;
	u32 running_req;
	struct hisi_sas_itct *itct;
	u8 taskfileset;
	u8 dev_status;
	u16 reserved;
};

struct hisi_sas_slot_info {
	struct list_head entry;
	// To be completed, j00310691
};

struct hisi_sas_cmd_hdr {
	u32	abort_flag;
	// To be completed, j00310691
};

struct hisi_hba {
	spinlock_t	lock;

	struct platform_device *pdev;
	struct device *dev;
	struct device_node *np;

	void __iomem *regs;

	u8 sas_addr[SAS_ADDR_SIZE];

	struct hisi_sas_cmd_hdr	*slot_hdr;
	dma_addr_t	slot_dma;

	int	n_phy;

	int tags_num;
	unsigned long *tags;

	/* SCSI/SAS glue */
	struct sas_ha_struct *sas;
	struct Scsi_Host *shost;

	struct hisi_sas_phy phy[HISI_SAS_MAX_PHYS];
	struct hisi_sas_port port[HISI_SAS_MAX_PHYS];
	struct list_head wq_list;

	u32	id;

	struct hba_info_page	hba_param;
	struct hisi_sas_device	devices[HISI_SAS_MAX_DEVICES];
	struct dma_pool *status_dma_pool;
	struct hisi_sas_itct *itct;
	dma_addr_t itct_dma;
	struct hisi_sas_slot_info	slot_info[HISI_SAS_QUEUES][0];
	// To be completed, j00310691
};


struct hisi_hba_priv_info {
	u8	n_phy;
	struct hisi_hba *hisi_hba[HISI_SAS_MAX_CORE];
	struct tasklet_struct *hisi_sas_tasklet;
	// To be completed, j00310691
};

int hisi_sas_scan_finished(struct Scsi_Host *shost, unsigned long time);
void hisi_sas_scan_start(struct Scsi_Host *shost);

void hisi_sas_tag_init(struct hisi_hba *hisi_hba);
void hisi_sas_phy_init(struct hisi_hba *hisi_hba, int i);

void hisi_sas_hw_init(struct hisi_hba *hisi_hba);

int hisi_sas_dev_found(struct domain_device *dev);
void hisi_sas_dev_gone(struct domain_device *dev);
int hisi_sas_queue_command(struct sas_task *task, gfp_t gfp_flags);
int hisi_sas_control_phy(struct asd_sas_phy *sas_phy,
			enum phy_func func,
			void *funcdata);
int hisi_sas_abort_task(struct sas_task *task);
int hisi_sas_abort_task_set(struct domain_device *dev, u8 *lun);
int hisi_sas_clear_aca(struct domain_device *dev, u8 *lun);
int hisi_sas_clear_task_set(struct domain_device *dev, u8 *lun);
int hisi_sas_I_T_nexus_reset(struct domain_device *dev);
int hisi_sas_lu_reset(struct domain_device *dev, u8 *lun);
int hisi_sas_query_task(struct sas_task *task);
void hisi_sas_port_formed(struct asd_sas_phy *sas_phy);
void hisi_sas_port_deformed(struct asd_sas_phy *sas_phy);


#endif
