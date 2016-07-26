/*
 * Copyright (c) 2016 Hisilicon Limited.
 * Copyright (c) 2007, 2008 Mellanox Technologies. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/of_platform.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_smi.h>
#include <rdma/ib_user_verbs.h>
#include "hns_roce_device.h"

static int hns_roce_get_cfg(struct hns_roce_dev *hr_dev)
{
	int i;
	u8 phy_port;
	int port_cnt = 0;
	struct device *dev = &hr_dev->pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *net_node;
	struct net_device *netdev = NULL;
	struct platform_device *pdev = NULL;
	struct resource *res;

	if (of_device_is_compatible(np, "hisilicon,hns-roce-v1")) {
		hr_dev->hw = &hns_roce_hw_v1;
	} else {
		dev_err(dev, "device no compatible!\n");
		return -EINVAL;
	}

	res = platform_get_resource(hr_dev->pdev, IORESOURCE_MEM, 0);
	hr_dev->reg_base = devm_ioremap_resource(dev, res);
	if (!hr_dev->reg_base)
		return -ENOMEM;

	for (i = 0; i < HNS_ROCE_MAX_PORTS; i++) {
		net_node = of_parse_phandle(np, "eth-handle", i);
		if (net_node) {
			pdev = of_find_device_by_node(net_node);
			netdev = platform_get_drvdata(pdev);
			phy_port = (u8)i;
			if (netdev) {
				hr_dev->iboe.netdevs[port_cnt] = netdev;
				hr_dev->iboe.phy_port[port_cnt] = phy_port;
			} else {
				return -ENODEV;
			}
			port_cnt++;
		}
	}

	if (port_cnt == 0) {
		dev_err(dev, "Unable to get available port by eth-handle!\n");
		return -EINVAL;
	}

	hr_dev->caps.num_ports = port_cnt;

	/* Cmd issue mode: 0 is poll, 1 is event */
	hr_dev->cmd_mod = 1;
	hr_dev->loop_idc = 0;

	for (i = 0; i < HNS_ROCE_MAX_IRQ_NUM; i++) {
		hr_dev->irq[i] = platform_get_irq(hr_dev->pdev, i);
		if (hr_dev->irq[i] <= 0) {
			dev_err(dev, "Get No.%d irq resource failed!\n", i);
			return -EINVAL;
		}

		if (of_property_read_string_index(np, "interrupt-names", i,
						  &hr_dev->irq_names))
			return -EINVAL;
	}

	return 0;
}

/**
* hns_roce_probe - RoCE driver entrance
* @pdev: pointer to platform device
* Return : int
*
*/
static int hns_roce_probe(struct platform_device *pdev)
{
	int ret;
	struct hns_roce_dev *hr_dev;
	struct device *dev = &pdev->dev;

	hr_dev = (struct hns_roce_dev *)ib_alloc_device(sizeof(*hr_dev));
	if (!hr_dev)
		return -ENOMEM;

	memset((u8 *)hr_dev + sizeof(struct ib_device), 0,
		sizeof(struct hns_roce_dev) - sizeof(struct ib_device));

	hr_dev->pdev = pdev;
	platform_set_drvdata(pdev, hr_dev);

	if (dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64ULL)) &&
	    dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32ULL))) {
		dev_err(dev, "No usable DMA addressing mode\n");
		ret = -EIO;
		goto error_failed_get_cfg;
	}

	ret = hns_roce_get_cfg(hr_dev);
	if (ret) {
		dev_err(dev, "Get Configuration failed!\n");
		goto error_failed_get_cfg;
	}

	ret = hr_dev->hw->reset(hr_dev, true);
	if (ret) {
		dev_err(dev, "Reset RoCE engine failed!\n");
		goto error_failed_get_cfg;
	}

	hr_dev->hw->hw_profile(hr_dev);

error_failed_get_cfg:
	ib_dealloc_device(&hr_dev->ib_dev);

	return ret;
}

/**
* hns_roce_remove - remove RoCE device
* @pdev: pointer to platform device
*/
static int hns_roce_remove(struct platform_device *pdev)
{
	struct hns_roce_dev *hr_dev = platform_get_drvdata(pdev);

	hr_dev->hw->reset(hr_dev, false);

	ib_dealloc_device(&hr_dev->ib_dev);

	return 0;
}

static const struct of_device_id hns_roce_of_match[] = {
	{ .compatible = "hisilicon,hns-roce-v1",},
	{},
};
MODULE_DEVICE_TABLE(of, hns_roce_of_match);

static struct platform_driver hns_roce_driver = {
	.probe = hns_roce_probe,
	.remove = hns_roce_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = hns_roce_of_match,
	},
};

module_platform_driver(hns_roce_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wei Hu <xavier.huwei@huawei.com>");
MODULE_AUTHOR("Nenglong Zhao <zhaonenglong@hisilicon.com>");
MODULE_AUTHOR("Lijun Ou <oulijun@huawei.com>");
MODULE_DESCRIPTION("HNS RoCE Driver");
