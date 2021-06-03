/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
#ifndef _UAPIUUACCE_H
#define _UAPIUUACCE_H

#include <linux/types.h>
#include <linux/ioctl.h>

/*
 * UACCE_CMD_START_Q: Start queue
 */
#define UACCE_CMD_START_Q	_IO('W', 0)

/*
 * UACCE_CMD_PUT_Q:
 * User actively stop queue and free queue resource immediately
 * Optimization method since close fd may delay
 */
#define UACCE_CMD_PUT_Q		_IO('W', 1)

#define UACCE_CMD_SHARE_SVAS	_IO('W', 2)

/*
 * UACCE_CMD_GET_SS_DMA: Get dma address of ss region
 */
#define UACCE_CMD_GET_SS_DMA    _IOR('W', 3, unsigned long)

/*
 * UACCE Device flags:
 * UACCE_DEV_SVA: Shared Virtual Addresses
 *		  Support PASID
 *		  Support device page faults (PCI PRI or SMMU Stall)
 */
#define UACCE_DEV_SVA		BIT(0)
#define UACCE_DEV_NOIOMMU	BIT(1)
#define UACCE_DEV_IOMMU		BIT(7)

/* uacce mode of the driver */
#define UACCE_MODE_NOUACCE	0 /* don't use uacce */
#define UACCE_MODE_SVA		1 /* use uacce sva mode */
#define UACCE_MODE_NOIOMMU	2 /* use uacce noiommu mode */

#define UACCE_API_VER_NOIOMMU_SUBFIX	"_noiommu"

/**
 * enum uacce_qfrt: queue file region type
 * @UACCE_QFRT_MMIO: device mmio region
 * @UACCE_QFRT_DUS: device user share region
 * @UACCE_QFRT_SS: static share memory for user
 */
enum uacce_qfrt {
	UACCE_QFRT_MMIO = 0,
	UACCE_QFRT_DUS = 1,
	UACCE_QFRT_SS = 2,
};

/* Pass DMA SS region slice size by granularity 64KB */
#define UACCE_GRAN_SIZE			0x10000ull
#define UACCE_GRAN_SHIFT		16
#define UACCE_GRAN_NUM_MASK		0xfffull

#endif
