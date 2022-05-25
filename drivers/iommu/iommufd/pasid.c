// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) Intel Corporation.
 */
#include <linux/iommufd.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/file.h>
#include <linux/pci.h>
#include <linux/irqdomain.h>
#include <linux/dma-iommu.h>

#include "iommufd_private.h"

#define IOASID_BITS 20

struct iommufd_pasid_data {
	ioasid_t pasid;
	u32 vpasid;
};

int iommufd_alloc_pasid(struct iommufd_ucmd *ucmd)
{
	struct iommu_alloc_pasid *cmd = ucmd->cmd;
	struct iommufd_pasid_data *pdata;
	int rc;

	if (cmd->flags & ~IOMMU_ALLOC_PASID_IDENTICAL)
		return -EOPNOTSUPP;

	if (cmd->range.min > cmd->range.max ||
	    cmd->range.min >= (1 << IOASID_BITS) ||
	    cmd->range.max >= (1 << IOASID_BITS))
		return -EINVAL;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	if (!cmd->flags & IOMMU_ALLOC_PASID_IDENTICAL)
		pdata->vpasid = cmd->pasid;
	pdata->pasid = pdata->vpasid =
				ioasid_alloc(&ucmd->ictx->pasid_set,
				cmd->range.min, cmd->range.max,
				pdata);
	if (!pasid_valid(pdata->pasid)) {
		rc = -ENOMEM;
		goto out_free_mem;
	}

	cmd->pasid = pdata->pasid;
	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_free_pasid;

	return 0;
out_free_pasid:
	ioasid_free(pdata->pasid);
out_free_mem:
	kfree(pdata);
	return rc;
}

int iommufd_free_pasid(struct iommufd_ucmd *ucmd)
{
	struct iommu_free_pasid *cmd = ucmd->cmd;

	if (cmd->flags)
		return -EOPNOTSUPP;

	if (!pasid_valid(cmd->pasid))
		return -EINVAL;

	ioasid_free(cmd->pasid); // TODO: needs to use ioasid_put()
	return 0;
}
