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

/*
 * UACCE_CMD_GET_SS_DMA: Get dma address of ss region
 */
#define UACCE_CMD_GET_SS_DMA    _IOR('W', 100, unsigned long)

/*
 * UACCE Device flags:
 * UACCE_DEV_SVA: Shared Virtual Addresses
 *		  Support PASID
 *		  Support device page faults (PCI PRI or SMMU Stall)
 */
#define UACCE_DEV_SVA		BIT(0)

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

/**
 * struct uacce_pin_address - Expected pin user space address and size
 * @addr: Address to pin
 * @size: Size of pin address
 */
struct uacce_pin_address {
	unsigned long addr;
	unsigned long size;
};

/* UACCE_CMD_PIN: Pin a range of memory */
#define UACCE_CMD_PIN		_IOW('W', 2, struct uacce_pin_address)

/* UACCE_CMD_UNPIN: Unpin a range of memory */
#define UACCE_CMD_UNPIN		_IOW('W', 3, struct uacce_pin_address)

#endif
