/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_RESCTRL_INTERNAL_H
#define _ASM_X86_RESCTRL_INTERNAL_H

#include <linux/resctrl.h>
#include <linux/sched.h>
#include <linux/kernfs.h>
#include <linux/fs_context.h>
#include <linux/jump_label.h>

#define MSR_IA32_L3_QOS_CFG		0xc81
#define MSR_IA32_L2_QOS_CFG		0xc82
#define MSR_IA32_L3_CBM_BASE		0xc90
#define MSR_IA32_L2_CBM_BASE		0xd10
#define MSR_IA32_MBA_THRTL_BASE		0xd50
#define MSR_IA32_MBA_BW_BASE		0xc0000200

#define MSR_IA32_QM_CTR			0x0c8e
#define MSR_IA32_QM_EVTSEL		0x0c8d

#define L3_QOS_CDP_ENABLE		0x01ULL

#define L2_QOS_CDP_ENABLE		0x01ULL

#define MBA_IS_LINEAR			0x4
#define MAX_MBA_BW_AMD			0x800

extern unsigned int rdt_mon_features;

/**
 * struct rdt_hw_domain - group of cpus sharing an RDT resource
 * @resctrl:    Properties exposed to the resctrl file system
 * @ctrl_val:	array of cache or mem ctrl values (indexed by CLOSID)
 */
struct rdt_hw_domain {
	struct rdt_domain		resctrl;
	u32				*ctrl_val;
};

static inline struct rdt_hw_domain *resctrl_to_arch_dom(struct rdt_domain *r)
{
	return container_of(r, struct rdt_hw_domain, resctrl);
}

/**
 * struct msr_param - set a range of MSRs from a domain
 * @res:       The resource to use
 * @low:       Beginning index from base MSR
 * @high:      End index
 */
struct msr_param {
	struct rdt_resource	*res;
	u32			low;
	u32			high;
};

/**
 * struct rdt_hw_resource - hw attributes of an RDT resource
 * @hw_num_closid:     The actual number of closids, regardless of CDP
 * @msr_base:		Base MSR address for CBMs
 * @msr_update:		Function pointer to update QOS MSRs
 */
struct rdt_hw_resource {
	struct rdt_resource     resctrl;
	u32			hw_num_closid;
	unsigned int		msr_base;
	void (*msr_update)	(struct rdt_domain *d, struct msr_param *m,
				 struct rdt_resource *r);
};

static inline struct rdt_hw_resource *resctrl_to_arch_res(struct rdt_resource *r)
{
	return container_of(r, struct rdt_hw_resource, resctrl);
}

extern struct rdt_hw_resource rdt_resources_all[];

static inline struct rdt_resource *resctrl_inc(struct rdt_resource *res)
{
	struct rdt_hw_resource *hw_res = resctrl_to_arch_res(res);

	hw_res++;
	return &hw_res->resctrl;
}

#define for_each_rdt_resource(r)					      \
	for (r = &rdt_resources_all[0].resctrl;				      \
	     r < &rdt_resources_all[RDT_NUM_RESOURCES].resctrl;		      \
	     r = resctrl_inc(r))

#define for_each_capable_rdt_resource(r)				      \
	for_each_rdt_resource(r)					      \
		if (r->alloc_capable || r->mon_capable)

#define for_each_alloc_capable_rdt_resource(r)				      \
	for_each_rdt_resource(r)					      \
		if (r->alloc_capable)

#define for_each_mon_capable_rdt_resource(r)				      \
	for_each_rdt_resource(r)					      \
		if (r->mon_capable)

/* CPUID.(EAX=10H, ECX=ResID=1).EAX */
union cpuid_0x10_1_eax {
	struct {
		unsigned int cbm_len:5;
	} split;
	unsigned int full;
};

/* CPUID.(EAX=10H, ECX=ResID=3).EAX */
union cpuid_0x10_3_eax {
	struct {
		unsigned int max_delay:12;
	} split;
	unsigned int full;
};

/* CPUID.(EAX=10H, ECX=ResID).EDX */
union cpuid_0x10_x_edx {
	struct {
		unsigned int cos_max:16;
	} split;
	unsigned int full;
};

void rdt_ctrl_update(void *arg);
int rdt_get_mon_l3_config(struct rdt_resource *r);

#endif /* _ASM_X86_RESCTRL_INTERNAL_H */
