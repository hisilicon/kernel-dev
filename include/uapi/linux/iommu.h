/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * IOMMU user API definitions
 */

#ifndef _UAPI_IOMMU_H
#define _UAPI_IOMMU_H

#include <linux/types.h>

#define IOMMU_FAULT_PERM_READ	(1 << 0) /* read */
#define IOMMU_FAULT_PERM_WRITE	(1 << 1) /* write */
#define IOMMU_FAULT_PERM_EXEC	(1 << 2) /* exec */
#define IOMMU_FAULT_PERM_PRIV	(1 << 3) /* privileged */

/* Generic fault types, can be expanded IRQ remapping fault */
enum iommu_fault_type {
	IOMMU_FAULT_DMA_UNRECOV = 1,	/* unrecoverable fault */
	IOMMU_FAULT_PAGE_REQ,		/* page request fault */
};

enum iommu_fault_reason {
	IOMMU_FAULT_REASON_UNKNOWN = 0,

	/* Could not access the PASID table (fetch caused external abort) */
	IOMMU_FAULT_REASON_PASID_FETCH,

	/* PASID entry is invalid or has configuration errors */
	IOMMU_FAULT_REASON_BAD_PASID_ENTRY,

	/*
	 * PASID is out of range (e.g. exceeds the maximum PASID
	 * supported by the IOMMU) or disabled.
	 */
	IOMMU_FAULT_REASON_PASID_INVALID,

	/*
	 * An external abort occurred fetching (or updating) a translation
	 * table descriptor
	 */
	IOMMU_FAULT_REASON_WALK_EABT,

	/*
	 * Could not access the page table entry (Bad address),
	 * actual translation fault
	 */
	IOMMU_FAULT_REASON_PTE_FETCH,

	/* Protection flag check failed */
	IOMMU_FAULT_REASON_PERMISSION,

	/* access flag check failed */
	IOMMU_FAULT_REASON_ACCESS,

	/* Output address of a translation stage caused Address Size fault */
	IOMMU_FAULT_REASON_OOR_ADDRESS,
};

/**
 * struct iommu_fault_unrecoverable - Unrecoverable fault data
 * @reason: reason of the fault, from &enum iommu_fault_reason
 * @flags: parameters of this fault (IOMMU_FAULT_UNRECOV_* values)
 * @pasid: Process Address Space ID
 * @perm: requested permission access using by the incoming transaction
 *        (IOMMU_FAULT_PERM_* values)
 * @addr: offending page address
 * @fetch_addr: address that caused a fetch abort, if any
 */
struct iommu_fault_unrecoverable {
	__u32	reason;
#define IOMMU_FAULT_UNRECOV_PASID_VALID		(1 << 0)
#define IOMMU_FAULT_UNRECOV_ADDR_VALID		(1 << 1)
#define IOMMU_FAULT_UNRECOV_FETCH_ADDR_VALID	(1 << 2)
	__u32	flags;
	__u32	pasid;
	__u32	perm;
	__u64	addr;
	__u64	fetch_addr;
};

/**
 * struct iommu_fault_page_request - Page Request data
 * @flags: encodes whether the corresponding fields are valid and whether this
 *         is the last page in group (IOMMU_FAULT_PAGE_REQUEST_* values)
 * @pasid: Process Address Space ID
 * @grpid: Page Request Group Index
 * @perm: requested page permissions (IOMMU_FAULT_PERM_* values)
 * @addr: page address
 * @private_data: device-specific private information
 */
struct iommu_fault_page_request {
#define IOMMU_FAULT_PAGE_REQUEST_PASID_VALID	(1 << 0)
#define IOMMU_FAULT_PAGE_REQUEST_LAST_PAGE	(1 << 1)
#define IOMMU_FAULT_PAGE_REQUEST_PRIV_DATA	(1 << 2)
	__u32	flags;
	__u32	pasid;
	__u32	grpid;
	__u32	perm;
	__u64	addr;
	__u64	private_data[2];
};

/**
 * struct iommu_fault - Generic fault data
 * @type: fault type from &enum iommu_fault_type
 * @padding: reserved for future use (should be zero)
 * @event: fault event, when @type is %IOMMU_FAULT_DMA_UNRECOV
 * @prm: Page Request message, when @type is %IOMMU_FAULT_PAGE_REQ
 * @padding2: sets the fault size to allow for future extensions
 */
struct iommu_fault {
	__u32	type;
	__u32	padding;
	union {
		struct iommu_fault_unrecoverable event;
		struct iommu_fault_page_request prm;
		__u8 padding2[56];
	};
};

/**
 * enum iommu_page_response_code - Return status of fault handlers
 * @IOMMU_PAGE_RESP_SUCCESS: Fault has been handled and the page tables
 *	populated, retry the access. This is "Success" in PCI PRI.
 * @IOMMU_PAGE_RESP_FAILURE: General error. Drop all subsequent faults from
 *	this device if possible. This is "Response Failure" in PCI PRI.
 * @IOMMU_PAGE_RESP_INVALID: Could not handle this fault, don't retry the
 *	access. This is "Invalid Request" in PCI PRI.
 */
enum iommu_page_response_code {
	IOMMU_PAGE_RESP_SUCCESS = 0,
	IOMMU_PAGE_RESP_INVALID,
	IOMMU_PAGE_RESP_FAILURE,
};

/**
 * struct iommu_page_response - Generic page response information
 * @version: API version of this structure
 * @flags: encodes whether the corresponding fields are valid
 *         (IOMMU_FAULT_PAGE_RESPONSE_* values)
 * @pasid: Process Address Space ID
 * @grpid: Page Request Group Index
 * @code: response code from &enum iommu_page_response_code
 */
struct iommu_page_response {
#define IOMMU_PAGE_RESP_VERSION_1	1
	__u32	version;
#define IOMMU_PAGE_RESP_PASID_VALID	(1 << 0)
	__u32	flags;
	__u32	pasid;
	__u32	grpid;
	__u32	code;
};

/**
 * struct iommu_pasid_smmuv3 - ARM SMMUv3 Stream Table Entry stage 1 related
 *     information
 * @version: API version of this structure
 * @s1fmt: STE s1fmt (format of the CD table: single CD, linear table
 *         or 2-level table)
 * @s1dss: STE s1dss (specifies the behavior when @pasid_bits != 0
 *         and no PASID is passed along with the incoming transaction)
 * @padding: reserved for future use (should be zero)
 *
 * The PASID table is referred to as the Context Descriptor (CD) table on ARM
 * SMMUv3. Please refer to the ARM SMMU 3.x spec (ARM IHI 0070A) for full
 * details.
 */
struct iommu_pasid_smmuv3 {
#define PASID_TABLE_SMMUV3_CFG_VERSION_1 1
	__u32	version;
	__u8	s1fmt;
	__u8	s1dss;
	__u8	padding[2];
};

/**
 * struct iommu_pasid_table_config - PASID table data used to bind guest PASID
 *     table to the host IOMMU
 * @version: API version to prepare for future extensions
 * @format: format of the PASID table
 * @base_ptr: guest physical address of the PASID table
 * @pasid_bits: number of PASID bits used in the PASID table
 * @config: indicates whether the guest translation stage must
 *          be translated, bypassed or aborted.
 * @padding: reserved for future use (should be zero)
 * @smmuv3: table information when @format is %IOMMU_PASID_FORMAT_SMMUV3
 */
struct iommu_pasid_table_config {
#define PASID_TABLE_CFG_VERSION_1 1
	__u32	version;
#define IOMMU_PASID_FORMAT_SMMUV3	1
	__u32	format;
	__u64	base_ptr;
	__u8	pasid_bits;
#define IOMMU_PASID_CONFIG_TRANSLATE	1
#define IOMMU_PASID_CONFIG_BYPASS	2
#define IOMMU_PASID_CONFIG_ABORT	3
	__u8	config;
	__u8    padding[6];
	union {
		struct iommu_pasid_smmuv3 smmuv3;
	};
};

#endif /* _UAPI_IOMMU_H */
