/* SPDX-License-Identifier: GPL-2.0 */
/*
 * iommu trace points
 *
 * Copyright (C) 2013 Shuah Khan <shuah.kh@samsung.com>
 *
 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM iommu

#if !defined(_TRACE_IOMMU_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_IOMMU_H

#include <linux/tracepoint.h>
#include <linux/iommu.h>
#include <uapi/linux/iommu.h>

struct device;

DECLARE_EVENT_CLASS(iommu_group_event,

	TP_PROTO(int group_id, struct device *dev),

	TP_ARGS(group_id, dev),

	TP_STRUCT__entry(
		__field(int, gid)
		__string(device, dev_name(dev))
	),

	TP_fast_assign(
		__entry->gid = group_id;
		__assign_str(device, dev_name(dev));
	),

	TP_printk("IOMMU: groupID=%d device=%s",
			__entry->gid, __get_str(device)
	)
);

DEFINE_EVENT(iommu_group_event, add_device_to_group,

	TP_PROTO(int group_id, struct device *dev),

	TP_ARGS(group_id, dev)

);

DEFINE_EVENT(iommu_group_event, remove_device_from_group,

	TP_PROTO(int group_id, struct device *dev),

	TP_ARGS(group_id, dev)
);

DECLARE_EVENT_CLASS(iommu_device_event,

	TP_PROTO(struct device *dev),

	TP_ARGS(dev),

	TP_STRUCT__entry(
		__string(device, dev_name(dev))
	),

	TP_fast_assign(
		__assign_str(device, dev_name(dev));
	),

	TP_printk("IOMMU: device=%s", __get_str(device)
	)
);

DEFINE_EVENT(iommu_device_event, attach_device_to_domain,

	TP_PROTO(struct device *dev),

	TP_ARGS(dev)
);

DEFINE_EVENT(iommu_device_event, detach_device_from_domain,

	TP_PROTO(struct device *dev),

	TP_ARGS(dev)
);

TRACE_EVENT(map,

	TP_PROTO(unsigned long iova, phys_addr_t paddr, size_t size),

	TP_ARGS(iova, paddr, size),

	TP_STRUCT__entry(
		__field(u64, iova)
		__field(u64, paddr)
		__field(size_t, size)
	),

	TP_fast_assign(
		__entry->iova = iova;
		__entry->paddr = paddr;
		__entry->size = size;
	),

	TP_printk("IOMMU: iova=0x%016llx paddr=0x%016llx size=%zu",
			__entry->iova, __entry->paddr, __entry->size
	)
);

TRACE_EVENT(unmap,

	TP_PROTO(unsigned long iova, size_t size, size_t unmapped_size),

	TP_ARGS(iova, size, unmapped_size),

	TP_STRUCT__entry(
		__field(u64, iova)
		__field(size_t, size)
		__field(size_t, unmapped_size)
	),

	TP_fast_assign(
		__entry->iova = iova;
		__entry->size = size;
		__entry->unmapped_size = unmapped_size;
	),

	TP_printk("IOMMU: iova=0x%016llx size=%zu unmapped_size=%zu",
			__entry->iova, __entry->size, __entry->unmapped_size
	)
);

DECLARE_EVENT_CLASS(iommu_error,

	TP_PROTO(struct device *dev, unsigned long iova, int flags),

	TP_ARGS(dev, iova, flags),

	TP_STRUCT__entry(
		__string(device, dev_name(dev))
		__string(driver, dev_driver_string(dev))
		__field(u64, iova)
		__field(int, flags)
	),

	TP_fast_assign(
		__assign_str(device, dev_name(dev));
		__assign_str(driver, dev_driver_string(dev));
		__entry->iova = iova;
		__entry->flags = flags;
	),

	TP_printk("IOMMU:%s %s iova=0x%016llx flags=0x%04x",
			__get_str(driver), __get_str(device),
			__entry->iova, __entry->flags
	)
);

DEFINE_EVENT(iommu_error, io_page_fault,

	TP_PROTO(struct device *dev, unsigned long iova, int flags),

	TP_ARGS(dev, iova, flags)
);

TRACE_EVENT(dev_fault,

	TP_PROTO(struct device *dev,  struct iommu_fault *evt),

	TP_ARGS(dev, evt),

	TP_STRUCT__entry(
		__string(device, dev_name(dev))
		__field(int, type)
		__field(int, reason)
		__field(u64, addr)
		__field(u64, fetch_addr)
		__field(u32, pasid)
		__field(u32, grpid)
		__field(u32, flags)
		__field(u32, prot)
	),

	TP_fast_assign(
		__assign_str(device, dev_name(dev));
		__entry->type = evt->type;
		if (evt->type == IOMMU_FAULT_DMA_UNRECOV) {
			__entry->reason		= evt->event.reason;
			__entry->flags		= evt->event.flags;
			__entry->pasid		= evt->event.pasid;
			__entry->grpid		= 0;
			__entry->prot		= evt->event.perm;
			__entry->addr		= evt->event.addr;
			__entry->fetch_addr	= evt->event.fetch_addr;
		} else {
			__entry->reason		= 0;
			__entry->flags		= evt->prm.flags;
			__entry->pasid		= evt->prm.pasid;
			__entry->grpid		= evt->prm.grpid;
			__entry->prot		= evt->prm.perm;
			__entry->addr		= evt->prm.addr;
			__entry->fetch_addr	= 0;
		}
	),

	TP_printk("IOMMU:%s type=%d reason=%d addr=0x%016llx fetch=0x%016llx pasid=%d group=%d flags=%x prot=%d",
		__get_str(device),
		__entry->type,
		__entry->reason,
		__entry->addr,
		__entry->fetch_addr,
		__entry->pasid,
		__entry->grpid,
		__entry->flags,
		__entry->prot
	)
);

TRACE_EVENT(smmu_evt,
	TP_PROTO(u64 w1, u64 w2, u64 w3, u64 w4),
	TP_ARGS(w1, w2, w3, w4),
	TP_STRUCT__entry(
		__field(u64, w1)
		__field(u64, w2)
		__field(u64, w3)
		__field(u64, w4)
	),
	TP_fast_assign(
		__entry->w1 = w1;
		__entry->w2 = w2;
		__entry->w3 = w3;
		__entry->w4 = w4;
	),
	TP_printk("0x%016llx 0x%016llx 0x%016llx 0x%016llx",
		__entry->w1,
		__entry->w2,
		__entry->w3,
		__entry->w4
	)
);

TRACE_EVENT(smmu_resume,
	TP_PROTO(u32 sid, u32 stag, u32 resp),
	TP_ARGS(sid, stag, resp),
	TP_STRUCT__entry(
		__field(u32, sid)
		__field(u32, stag)
		__field(u32, resp)
	),
	TP_fast_assign(
		__entry->sid = sid;
		__entry->stag = stag;
		__entry->resp = resp;
	),
	TP_printk("sid=0x%x stag=0x%x resp=0x%x",
		__entry->sid,
		__entry->stag,
		__entry->resp
	)
);

TRACE_EVENT(smmu_cdsync,
	    TP_PROTO(u32 sid, u32 ssid, u8 leaf),
	    TP_ARGS(sid, ssid, leaf),
	    TP_STRUCT__entry(
		__field(u32, sid)
		__field(u32, ssid)
		__field(u8, leaf)),
	    TP_fast_assign(
		__entry->sid = sid;
		__entry->ssid = ssid;
		__entry->leaf = leaf;
		),
	    TP_printk("sid=0x%x ssid=0x%x leaf=%d",
		      __entry->sid, __entry->ssid, __entry->leaf)
);

TRACE_EVENT(smmu_cdwrite,
	    TP_PROTO(u64 val),
	    TP_ARGS(val),
	    TP_STRUCT__entry(__field(u64, val)),
	    TP_fast_assign(__entry->val = val),
	    TP_printk("0x%llx", __entry->val)
);

TRACE_EVENT(smmu_l1cdwrite,
	    TP_PROTO(u64 val),
	    TP_ARGS(val),
	    TP_STRUCT__entry(__field(u64, val)),
	    TP_fast_assign(__entry->val = val),
	    TP_printk("0x%llx", __entry->val)
);

TRACE_EVENT(dev_page_response,

	TP_PROTO(struct device *dev,  struct iommu_page_response *msg),

	TP_ARGS(dev, msg),

	TP_STRUCT__entry(
		__string(device, dev_name(dev))
		__field(int, code)
		__field(u32, pasid)
		__field(u32, grpid)
	),

	TP_fast_assign(
		__assign_str(device, dev_name(dev));
		__entry->code = msg->code;
		__entry->pasid = msg->pasid;
		__entry->grpid = msg->grpid;
	),

	TP_printk("IOMMU:%s code=%d pasid=%d group=%d",
		__get_str(device),
		__entry->code,
		__entry->pasid,
		__entry->grpid
	)
);

TRACE_EVENT(iopf,
	    TP_PROTO(int errn),
	    TP_ARGS(errn),
	    TP_STRUCT__entry(__field(int, errn)),
	    TP_fast_assign(__entry->errn = errn),
	    TP_printk("err %x", __entry->errn)
);

DECLARE_EVENT_CLASS(io_mm,
	    TP_PROTO(int pasid),
	    TP_ARGS(pasid),
	    TP_STRUCT__entry(
		__field(int, pasid)
	    ),
	    TP_fast_assign(
		__entry->pasid = pasid;
	    ),
	    TP_printk("pasid=%d", __entry->pasid)
);

DEFINE_EVENT(io_mm, io_mm_alloc,
	    TP_PROTO(int pasid),
	    TP_ARGS(pasid)
);

DEFINE_EVENT(io_mm, io_mm_reuse,
	    TP_PROTO(int pasid),
	    TP_ARGS(pasid)
);

DEFINE_EVENT(io_mm, io_mm_free,
	    TP_PROTO(int pasid),
	    TP_ARGS(pasid)
);

DECLARE_EVENT_CLASS(io_mm_dev,
	    TP_PROTO(int pasid, struct device *dev),
	    TP_ARGS(pasid, dev),
	    TP_STRUCT__entry(
		__string(dev, dev_name(dev))
		__field(int, pasid)
	    ),
	    TP_fast_assign(
		__assign_str(dev, dev_name(dev));
		__entry->pasid = pasid;
	    ),
	    TP_printk("pasid=%d dev=%s", __entry->pasid, __get_str(dev))
);

DEFINE_EVENT(io_mm_dev, iopf_flush,
	    TP_PROTO(int pasid, struct device *dev),
	    TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_attach_alloc,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_attach_get,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_detach,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_exit,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_unbind_put,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

DEFINE_EVENT(io_mm_dev, io_mm_unbind_free,
	     TP_PROTO(int pasid, struct device *dev),
	     TP_ARGS(pasid, dev)
);

TRACE_EVENT(io_mm_invalidate,
	    TP_PROTO(int pasid, unsigned long start,
		     unsigned long end),
	    TP_ARGS(pasid, start, end),
	    TP_STRUCT__entry(
		__field(int, pasid)
		__field(u64, start)
		__field(u64, end)
	    ),
	    TP_fast_assign(
		__entry->pasid = pasid;
		__entry->start = start;
		__entry->end = end;
	    ),
	    TP_printk("pasid=%d start=0x%llx end=0x%llx",
		__entry->pasid, __entry->start, __entry->end
	    )
);

TRACE_EVENT(io_mm_release_done,
	    TP_PROTO(int dummy),
	    TP_ARGS(dummy),
	    TP_STRUCT__entry(__field(int, dummy)),
	    TP_fast_assign(__entry->dummy = dummy;),
	    TP_printk("%d", __entry->dummy)
	   );

#endif /* _TRACE_IOMMU_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
