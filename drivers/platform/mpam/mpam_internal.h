// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Arm Ltd.

#ifndef MPAM_INTERNAL_H
#define MPAM_INTERNAL_H

#include <linux/cpumask.h>
#include <linux/idr.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/resctrl.h>
#include <linux/sizes.h>

/*
 * how long we will schedule round the monitors waiting for their values to
 * settle.
 */
#define NRDY_TIMEOUT	1	/* second */

/* System wide properties */
struct mpam_sysprops {
	u16 max_partid;
	u8 max_pmg;
	u32 mpam_llc_size;
};
extern struct mpam_sysprops mpam_sysprops;

/*
 * When we compact the supported features, we don't care what they are.
 * Storing them as a bitmap makes life easy.
 */
typedef u16 mpam_features_t;

/* Bits for mpam_features_t */
enum mpam_device_features {
	mpam_feat_ccap_part = 0,
	mpam_feat_cpor_part,
	mpam_feat_mbw_part,
	mpam_feat_mbw_min,
	mpam_feat_mbw_max,
	mpam_feat_mbw_prop,
	mpam_feat_intpri_part,
	mpam_feat_intpri_part_0_low,
	mpam_feat_dspri_part,
	mpam_feat_dspri_part_0_low,
	mpam_feat_msmon,
	mpam_feat_msmon_csu,
	mpam_feat_msmon_csu_capture,
	mpam_feat_msmon_mbwu,
	mpam_feat_msmon_mbwu_capture,
	mpam_feat_msmon_capt,
	MPAM_FEATURE_LAST,
};

#define MPAM_ALL_FEATURES	((1<<MPAM_FEATURE_LAST) - 1)

static inline bool mpam_has_feature(enum mpam_device_features feat,
				    mpam_features_t supported)
{
	return (1<<feat) & supported;
}

static inline void mpam_set_feature(enum mpam_device_features feat,
				    mpam_features_t *supported)
{
	*supported |= (1<<feat);
}

static inline void mpam_clear_feature(enum mpam_device_features feat,
				      mpam_features_t *supported)
{
	*supported &= ~(1<<feat);
}

static inline bool mpam_has_part_sel(mpam_features_t supported)
{
	mpam_features_t mask = (1<<mpam_feat_ccap_part) |
		(1<<mpam_feat_cpor_part) | (1<<mpam_feat_mbw_part) |
		(1<<mpam_feat_mbw_max) | (1<<mpam_feat_intpri_part) |
		(1<<mpam_feat_dspri_part);
	/* or HAS_PARTID_NRW or HAS_IMPL_IDR */

	return supported & mask;
}

struct mpam_config {

	/*
	 * The biggest config we could pass around is 4K, but resctrl's max
	 * cbm is u32, so we only need the full-size config during reset.
	 * Just in case a cache with a >u32 bitmap is exported for another
	 * reason, we need to track which bits of the configuration are valid.
	 */
	mpam_features_t valid;

	u32     cpbm;
	u32     mbw_pbm;
	u16     mbw_max;
};
typedef struct mpam_config mpam_config_t;

static inline bool mpam_config_is_empty(mpam_config_t *cfg)
{
	return (!cfg->cpbm || !cfg->mbw_pbm || !cfg->mbw_max);
}

/*
 * An mpam_device corresponds to an MSC, an interface to a component's cache
 * or bandwidth controls. It is associated with a set of CPUs, and a component.
 * For resctrl the component is expected to be a well-known cache (e.g. L2).
 * We may have multiple interfaces per component, each for a set of CPUs that
 * share the same component.
 */
struct mpam_device
{
	/* member of mpam_component:devices */
	struct list_head        comp_list;
	struct mpam_component   *comp;

	/* member of mpam_all_devices */
	struct list_head        glbl_list;

	/* The affinity learn't from firmware */
	struct cpumask          fw_affinity;
	/* of which these cpus are online */
	struct cpumask          online_affinity;

	spinlock_t              lock;
	phys_addr_t             hwpage_address;
	void __iomem *          mapped_hwpage;
	bool			probed;
	mpam_features_t		features;
	u16			cmax_wd;
	u16			cpbm_wd;
	u16			mbw_pbm_bits;
	u16			bwa_wd;
	u16			intpri_wd;
	u16			dspri_wd;
	u16			num_csu_mon;
	u16			num_mbwu_mon;

	bool			enable_error_irq;
	u32                     error_irq;
	u32                     error_irq_flags;
	u32                     overflow_irq;
	u32                     overflow_irq_flags;
};

/*
 * A set of devices that share the same component. e.g. the MSCs that
 * make up the L2 cache. This may be 1:1. Exposed to user-space as a domain by
 * resctrl when the component is a well-known cache.
 */
struct mpam_component
{
	u32			comp_id;

	/* mpam_devices in this domain */
	struct list_head        devices;

	struct cpumask          fw_affinity;

	/* Array of configuration values, indexed by partid. */
	mpam_config_t		*cfg;

	/* member of mpam_class:components */
	struct list_head        class_list;
};

/*
 * All the components of the same type at a particular level,
 * e.g. all the L2 cache components. Exposed to user-space as a resource
 * by resctrl when the component is a well-known cache. We may have additional
 * classes such as system-caches, or internal components that are not exposed.
 */
struct mpam_class
{
	/*
	 * resctrl expects to see an empty domain list if all 'those' CPUs are
	 * offline. As we can't discover the cpu affinity of 'unknown' MSCs, we
	 * need a second list.
	 * mpam_components in this class.
	 */
	struct list_head        components;

	struct cpumask          fw_affinity;

	u8			level;
	enum mpam_class_types	type;

	/* Once enabled, the common features */
	mpam_features_t		features;

	/* The common values if the feature is supported */
	u16			cpbm_wd;
	u16			mbw_pbm_bits;
	u16			bwa_wd;
	u16			intpri_wd;
	u16			dspri_wd;
	u16			num_csu_mon;
	u16			num_mbwu_mon;

	struct mutex		lock;
	struct ida		ida_csu_mon;
	struct ida		ida_mbwu_mon;

	/* member of mpam_classes */
	struct list_head        classes_list;
};

typedef u32 resctrl_config_t;

struct mpam_resctrl_dom {
	struct mpam_component	*comp;
	struct rdt_domain	resctrl_dom;

	/*
	 * Array of resctrl provided configuration values, indexed by
	 * hw_closid.
	 */
	resctrl_config_t	*resctrl_cfg;
};

struct mpam_resctrl_res {
	struct mpam_class	*class;

	/*
	 * If this class is exported as resctrl:MBA, are we using mbw_max
	 * control or the mbw_part bitmap.
	 */
	bool			resctrl_mba_uses_mbw_part;

	struct rdt_resource	resctrl_res;
};

/* List of all classes */
extern struct list_head mpam_classes;
#ifdef CONFIG_LOCKDEP
void mpam_class_list_lock_held(void);
#else
static inline mpam_class_list_lock_held(void) { }
#endif

struct mpam_component_sync_args
{
	/* Caller specifies these values: */
	u16 partid;

	/* mpam_component_config_sync() expects these too: */
	u8      mon;
	bool    match_pmg;
	u8      pmg;
};

/*
 * Apply the describe configuration update to all online devices.
 * Call with cpuhp lock held. A NULL arg causes these devices to be reset.
 */
int mpam_component_config_sync(struct mpam_component *comp,
			       struct mpam_component_sync_args *sync_args);

/*
 * Like mpam_component_config_sync(), but for programing and reading a CSU
 * monitor.
 */
int mpam_component_configure_mon(struct mpam_component *comp,
				 struct mpam_component_sync_args *sync_args,
				 u64 *result);

static inline int mpam_alloc_csu_mon(struct mpam_class *class)
{
	int mon_id;

	if (!mpam_has_feature(mpam_feat_msmon_csu, class->features))
		return -EIO;

	mutex_lock(&class->lock);
	mon_id = ida_alloc_range(&class->ida_csu_mon, 0, class->num_csu_mon,
				 GFP_KERNEL);
	mutex_unlock(&class->lock);

	return mon_id;
}

static inline void mpam_free_csu_mon(struct mpam_class *class, u8 csu_mon)
{
	mutex_lock(&class->lock);
	ida_free(&class->ida_csu_mon, csu_mon);
	mutex_unlock(&class->lock);
}

static inline int mpam_alloc_mbwu_mon(struct mpam_class *class)
{
	int mon_id;

	if (!mpam_has_feature(mpam_feat_msmon_mbwu, class->features))
		return -EIO;

	mutex_lock(&class->lock);
	mon_id = ida_alloc_range(&class->ida_mbwu_mon, 0, class->num_mbwu_mon,
			       GFP_KERNEL);
	mutex_unlock(&class->lock);

	return mon_id;
}

static inline void mpam_free_mbwu_mon(struct mpam_class *class, u8 mbwu_mon)
{
	mutex_lock(&class->lock);
	ida_free(&class->ida_mbwu_mon, mbwu_mon);
	mutex_unlock(&class->lock);
}

int mpam_resctrl_cpu_online(unsigned int cpu);
int mpam_resctrl_cpu_offline(unsigned int cpu);

int mpam_resctrl_setup(void);

/* Size of the memory mapped registers: 4K of feature page then 2x 4K bitmap registers */
#define SZ_MPAM_DEVICE	(3 * SZ_4K)


/*
 * MPAM MSCs have the following register layout. See:
 * Arm Architecture Reference Manual Supplement - Memory System Resource
 * Partitioning and Monitoring (MPAM), for Armv8-A. DDI 0598A.a
 */
#define MPAM_ARCHITECTURE_V1    0x10

/* Memory mapped control pages: */
/* ID Register offsets in the memory mapped page */
#define MPAMF_IDR               0x0000  /* features id register */
#define MPAMF_MSMON_IDR         0x0080  /* performance monitoring features */
#define MPAMF_IMPL_IDR          0x0028  /* imp-def partitioning */
#define MPAMF_CPOR_IDR          0x0030  /* cache-portion partitioning */
#define MPAMF_CCAP_IDR          0x0038  /* cache-capacity partitioning */
#define MPAMF_MBW_IDR           0x0040  /* mem-bw partitioning */
#define MPAMF_PRI_IDR           0x0048  /* priority partitioning */
#define MPAMF_CSUMON_IDR        0x0088  /* cache-usage monitor */
#define MPAMF_MBWUMON_IDR       0x0090  /* mem-bw usage monitor */
#define MPAMF_PARTID_NRW_IDR    0x0050  /* partid-narrowing */
#define MPAMF_IIDR              0x0018  /* implementer id register */
#define MPAMF_AIDR              0x0020  /* architectural id register */

/* Configuration and Status Register offsets in the memory mapped page */
#define MPAMCFG_PART_SEL        0x0100  /* partid to configure: */
#define MPAMCFG_CPBM            0x1000  /* cache-portion config */
#define MPAMCFG_CMAX            0x0108  /* cache-capacity config */
#define MPAMCFG_MBW_MIN         0x0200  /* min mem-bw config */
#define MPAMCFG_MBW_MAX         0x0208  /* max mem-bw config */
#define MPAMCFG_MBW_WINWD       0x0220  /* mem-bw accounting window config */
#define MPAMCFG_MBW_PBM         0x2000  /* mem-bw portion bitmap config */
#define MPAMCFG_PRI             0x0400  /* priority partitioning config */
#define MPAMCFG_MBW_PROP        0x0500  /* mem-bw stride config */
#define MPAMCFG_INTPARTID       0x0600  /* partid-narrowing config */

#define MSMON_CFG_MON_SEL       0x0800  /* monitor selector */
#define MSMON_CFG_CSU_FLT       0x0810  /* cache-usage monitor filter */
#define MSMON_CFG_CSU_CLT       0x0818  /* cache-usage monitor config */
#define MSMON_CFG_MBWU_FLT      0x0820  /* mem-bw monitor filter */
#define MSMON_CFG_MBWU_CTL      0x0828  /* mem-bw monitor config */
#define MSMON_CSU               0x0840  /* current cache-usage */
#define MSMON_CSU_CAPTURE       0x0848  /* last cache-usage value captured */
#define MSMON_MBWU              0x0860  /* current mem-bw usage value */
#define MSMON_MBWU_CAPTURE      0x0868  /* last mem-bw value captured */
#define MSMON_CAPT_EVNT         0x0808  /* signal a capture event */
#define MPAMF_ESR               0x00F8  /* error status register */
#define MPAMF_ECR               0x00F0  /* error control register */

/* MPAMF_IDR - MPAM features ID register */
#define MPAMF_IDR_PARTID_MAX_MASK       GENMASK(15, 0)
#define MPAMF_IDR_PMG_MAX_MASK          GENMASK(23, 16)
#define MPAMF_IDR_PMG_MAX_SHIFT         16
#define MPAMF_IDR_HAS_CCAP_PART         BIT(24)
#define MPAMF_IDR_HAS_CPOR_PART         BIT(25)
#define MPAMF_IDR_HAS_MBW_PART          BIT(26)
#define MPAMF_IDR_HAS_PRI_PART          BIT(27)
#define MPAMF_IDR_HAS_IMPL_IDR          BIT(29)
#define MPAMF_IDR_HAS_MSMON             BIT(30)
#define MPAMF_IDR_HAS_PARTID_NRW        BIT(31)

/* MPAMF_MSMON_IDR - MPAM performance monitoring ID register */
#define MPAMF_MSMON_IDR_MSMON_CSU               BIT(16)
#define MPAMF_MSMON_IDR_MSMON_MBWU              BIT(17)
#define MPAMF_MSMON_IDR_HAS_LOCAL_CAPT_EVNT     BIT(31)

/* MPAMF_CPOR_IDR - MPAM features cache portion partitioning ID register */
#define MPAMF_CPOR_IDR_CPBM_WD                  GENMASK(15, 0)

/* MPAMF_CCAP_IDR - MPAM features cache capacity partitioning ID register */
#define MPAMF_CCAP_IDR_CMAX_WD                  GENMASK(5, 0)

/* MPAMF_MBW_IDR - MPAM features memory bandwidth partitioning ID register */
#define MPAMF_MBW_IDR_BWA_WD            GENMASK(5, 0)
#define MPAMF_MBW_IDR_HAS_MIN           BIT(10)
#define MPAMF_MBW_IDR_HAS_MAX           BIT(11)
#define MPAMF_MBW_IDR_HAS_PBM           BIT(12)
#define MPAMF_MBW_IDR_HAS_PROP          BIT(13)
#define MPAMF_MBW_IDR_WINDWR            BIT(14)
#define MPAMF_MBW_IDR_BWPBM_WD          GENMASK(28, 16)
#define MPAMF_MBW_IDR_BWPBM_WD_SHIFT    16

/* MPAMF_PRI_IDR - MPAM features priority partitioning ID register */
#define MPAMF_PRI_IDR_HAS_INTPRI        BIT(0)
#define MPAMF_PRI_IDR_INTPRI_0_IS_LOW   BIT(1)
#define MPAMF_PRI_IDR_INTPRI_WD_SHIFT	4
#define MPAMF_PRI_IDR_INTPRI_WD         GENMASK(9, 4)
#define MPAMF_PRI_IDR_HAS_DSPRI         BIT(16)
#define MPAMF_PRI_IDR_DSPRI_0_IS_LOW    BIT(17)
#define MPAMF_PRI_IDR_DSPRI_WD_SHIFT	20
#define MPAMF_PRI_IDR_DSPRI_WD          GENMASK(25, 20)

/* MPAMF_CSUMON_IDR - MPAM cache storage usage monitor ID register */
#define MPAMF_CSUMON_IDR_NUM_MON        GENMASK(15, 0)
#define MPAMF_CSUMON_IDR_HAS_CAPTURE    BIT(31)

/* MPAMF_MBWUMON_IDR - MPAM memory bandwidth usage monitor ID register */
#define MPAMF_MBWUMON_IDR_NUM_MON       GENMASK(15, 0)
#define MPAMF_MBWUMON_IDR_HAS_CAPTURE   BIT(31)

/* MPAMF_PARTID_NRW_IDR - MPAM PARTID narrowing ID register */
#define MPAMF_PARTID_NRW_IDR_INTPARTID_MAX      GENMASK(15, 0)

/* MPAMF_IIDR - MPAM implementation ID register */
#define MPAMF_IIDR_PRODUCTID    GENMASK(31, 20)
#define MPAMF_IIDR_VARIANT      GENMASK(19, 16)
#define MPAMF_IIDR_REVISON      GENMASK(15, 12)
#define MPAMF_IIDR_IMPLEMENTER  GENMASK(11, 0)

/* MPAMF_AIDR - MPAM architecture ID register */
#define MPAMF_AIDR_ARCH_MAJOR_REV       GENMASK(7, 4)
#define MPAMF_AIDR_ARCH_MINOR_REV       GENMASK(3, 0)

/* MPAMCFG_PART_SEL - MPAM partition configuration selection register */
#define MPAMCFG_PART_SEL_PARTID_SEL     GENMASK(15, 0)
#define MPAMCFG_PART_SEL_INTERNAL       BIT(16)

/* MPAMCFG_CMAX - MPAM cache portion bitmap partition configuration register */
#define MPAMCFG_CMAX_CMAX               GENMASK(15, 0)

/*
 * MPAMCFG_MBW_MIN - MPAM memory minimum bandwidth partitioning configuration
 *                   register
 */
#define MPAMCFG_MBW_MIN_MIN             GENMASK(15, 0)

/*
 * MPAMCFG_MBW_MAX - MPAM memory maximum bandwidth partitioning configuration
 *                   register
 */
#define MPAMCFG_MBW_MAX_MAX             GENMASK(15, 0)
#define MPAMCFG_MBW_MAX_HARDLIM         BIT(31)

/*
 * MPAMCFG_MBW_WINWD - MPAM memory bandwidth partitioning window width
 *                     register
 */
#define MPAMCFG_MBW_WINWD_US_FRAC       GENMASK(7, 0)
#define MPAMCFG_MBW_WINWD_US_INT        GENMASK(23, 8)


/* MPAMCFG_PRI - MPAM priority partitioning configuration register */
#define MPAMCFG_PRI_INTPRI              GENMASK(15, 0)
#define MPAMCFG_PRI_DSPRI_SHIFT		16
#define MPAMCFG_PRI_DSPRI               GENMASK(31, 16)

/*
 * MPAMCFG_MBW_PROP - Memory bandwidth proportional stride partitioning
 *                    configuration register
 */
#define MPAMCFG_MBW_PROP_STRIDEM1       GENMASK(15, 0)
#define MPAMCFG_MBW_PROP_EN             BIT(31)

/*
 * MPAMCFG_INTPARTID - MPAM internal partition narrowing configuration register
 */
#define MPAMCFG_INTPARTID_INTPARTID     GENMASK(15, 0)
#define MPAMCFG_INTPARTID_INTERNAL      BIT(16)

/* MSMON_CFG_MON_SEL - Memory system performance monitor selection register */
#define MSMON_CFG_MON_SEL_MON_SEL       GENMASK(7, 0)

/* MPAMF_ESR - MPAM Error Status Register */
#define MPAMF_ESR_PARTID_OR_MON GENMASK(15, 0)
#define MPAMF_ESR_PMG           GENMASK(23, 16)
#define MPAMF_ESR_ERRCODE       GENMASK(27, 24)
#define MPAMF_ESR_ERRCODE_SHIFT 24
#define MPAMF_ESR_OVRWR         BIT(31)

/* MPAMF_ECR - MPAM Error Control Register */
#define MPAMF_ECR_INTEN         BIT(0)

/* Error conditions in accessing memory mapped registers */
#define MPAM_ERRCODE_NONE                       0
#define MPAM_ERRCODE_PARTID_SEL_RANGE           1
#define MPAM_ERRCODE_REQ_PARTID_RANGE           2
#define MPAM_ERRCODE_MSMONCFG_ID_RANGE          3
#define MPAM_ERRCODE_REQ_PMG_RANGE              4
#define MPAM_ERRCODE_MONITOR_RANGE              5
#define MPAM_ERRCODE_INTPARTID_RANGE            6
#define MPAM_ERRCODE_UNEXPECTED_INTERNAL        7
#define _MPAM_NUM_ERRCODE			8

/*
 * MSMON_CFG_CSU_FLT - Memory system performance monitor configure cache storage
 *                    usage monitor filter register
 */
#define MSMON_CFG_CSU_FLT_PARTID       GENMASK(15, 0)
#define MSMON_CFG_CSU_FLT_PMG          GENMASK(23, 16)
#define MSMON_CFG_CSU_FLT_PMG_SHIFT    16

/*
 * MSMON_CFG_CSU_CTL - Memory system performance monitor configure cache storage
 *                    usage monitor control register
 * MSMON_CFG_MBWU_CTL - Memory system performance monitor configure memory
 *                     bandwidth usage monitor control register
 */
#define MSMON_CFG_x_CTL_TYPE           GENMASK(7, 0)
#define MSMON_CFG_x_CTL_MATCH_PARTID   BIT(16)
#define MSMON_CFG_x_CTL_MATCH_PMG      BIT(17)
#define MSMON_CFG_x_CTL_SUBTYPE        GENMASK(23, 20)
#define MSMON_CFG_x_CTL_SUBTYPE_SHIFT  20
#define MSMON_CFG_x_CTL_OFLOW_FRZ      BIT(24)
#define MSMON_CFG_x_CTL_OFLOW_INTR     BIT(25)
#define MSMON_CFG_x_CTL_OFLOW_STATUS   BIT(26)
#define MSMON_CFG_x_CTL_CAPT_RESET     BIT(27)
#define MSMON_CFG_x_CTL_CAPT_EVNT      GENMASK(30, 28)
#define MSMON_CFG_x_CTL_CAPT_EVNT_SHIFT        28
#define MSMON_CFG_x_CTL_EN             BIT(31)

#define MSMON_CFG_MBWU_CTL_SUBTYPE_NONE                 0
#define MSMON_CFG_MBWU_CTL_SUBTYPE_READ                 1
#define MSMON_CFG_MBWU_CTL_SUBTYPE_WRITE                2
#define MSMON_CFG_MBWU_CTL_SUBTYPE_BOTH                 3

#define MSMON_CFG_MBWU_CTL_SUBTYPE_MAX                  3
#define MSMON_CFG_MBWU_CTL_SUBTYPE_MASK                 0x3

/*
 * MSMON_CFG_MBWU_FLT - Memory system performance monitor configure memory
 *                     bandwidth usage monitor filter register
 */
#define MSMON_CFG_MBWU_FLT_PARTID               GENMASK(15, 0)
#define MSMON_CFG_MBWU_FLT_PMG_SHIFT		16
#define MSMON_CFG_MBWU_FLT_PMG                  GENMASK(23, 16)

/*
 * MSMON_CSU - Memory system performance monitor cache storage usage monitor
 *            register
 * MSMON_CSU_CAPTURE -  Memory system performance monitor cache storage usage
 *                     capture register
 * MSMON_MBWU  - Memory system performance monitor memory bandwidth usage
 *               monitor register
 * MSMON_MBWU_CAPTURE - Memory system performance monitor memory bandwidth usage
 *                     capture register
 */
#define MSMON___VALUE          GENMASK(30, 0)
#define MSMON___NRDY           BIT(31)

/*
 * MSMON_CAPT_EVNT - Memory system performance monitoring capture event
 *                  generation register
 */
#define MSMON_CAPT_EVNT_NOW    BIT(0)

#endif /* MPAM_INTERNAL_H */
