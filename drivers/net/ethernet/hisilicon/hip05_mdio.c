#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_mdio.h>
#include <linux/delay.h>

#define MDIO_SINGLE_CMD			0x00
#define MDIO_READ_DATA			0x0c
#define MDIO_WRITE_DATA			0x08
#define MDIO_CTRL			0x03cc
#define MDIO_RDATA_STATUS		0x10

#define MDIO_START			BIT(14)
#define MDIO_R_VALID			BIT(0)
#define MDIO_READ			(BIT(11) | MDIO_START)
#define MDIO_WRITE			(BIT(10) | MDIO_START)
#define MDIO_ST_CLAUSE_22		BIT(12)

struct hip05_mdio_priv {
	void __iomem *base;
};

static int hip05_mdio_wait_ready(struct mii_bus *bus)
{
	struct hip05_mdio_priv *priv = bus->priv;
	void __iomem *base = priv->base;
	int i, timeout = 10000;

	for (i = 0; readl_relaxed(base + MDIO_SINGLE_CMD) & MDIO_START; i++) {
		if (i == timeout)
			return -ETIMEDOUT;
		usleep_range(10, 20);
	}

	return 0;
}

static int hip05_mdio_read(struct mii_bus *bus, int phy, int reg)
{
	struct hip05_mdio_priv *priv = bus->priv;
	void __iomem *base = priv->base;
	u32 val, command = 0;
	int ret;

	ret = hip05_mdio_wait_ready(bus);
	if (ret < 0)
		goto out;

	command = MDIO_READ | phy << 5 | reg | MDIO_ST_CLAUSE_22;
	writel_relaxed(command, base + MDIO_SINGLE_CMD);
	ret = hip05_mdio_wait_ready(bus);
	if (ret < 0)
		goto out;

	val = readl_relaxed(base + MDIO_RDATA_STATUS);
	if (val & MDIO_R_VALID) {
		dev_err(bus->parent, "SMI bus read not valid\n");
		ret = -ENODEV;
		goto out;
	}

	val = readl_relaxed(base + MDIO_READ_DATA);
	ret = val & 0xFFFF;
out:
	return ret;
}

static int hip05_mdio_write(struct mii_bus *bus, int phy, int reg, u16 val)
{
	struct hip05_mdio_priv *priv = bus->priv;
	void __iomem *base = priv->base;
	u32 ret, command;

	ret = hip05_mdio_wait_ready(bus);
	if (ret < 0)
		goto out;

	ret = readl_relaxed(base + MDIO_WRITE_DATA);
	ret = val;
	writel_relaxed(ret, base + MDIO_WRITE_DATA);
	command = MDIO_WRITE | phy << 5 | reg | MDIO_ST_CLAUSE_22;
	writel_relaxed(command, base + MDIO_SINGLE_CMD);
out:
	return ret;
}

static int hip05_mdio_reset(struct mii_bus *bus)
{
	int temp, err, i;

	for (i = 0; i < PHY_MAX_ADDR; i++) {
		hip05_mdio_write(bus, i, 22, 0);
		temp = hip05_mdio_read(bus, i, MII_BMCR);
		temp |= BMCR_RESET;
		err = hip05_mdio_write(bus, i, MII_BMCR, temp);
		if (err < 0)
			return err;
	}
	mdelay(500);
	return 0;
}

static int hip05_mdio_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mii_bus *bus;
	struct hip05_mdio_priv *priv;
	int ret;

	bus = mdiobus_alloc_size(sizeof(*priv));
	if (bus == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	bus->name = "hip05_mii_bus";
	bus->read = hip05_mdio_read;
	bus->write = hip05_mdio_write;
	bus->reset = hip05_mdio_reset;
	bus->parent = &pdev->dev;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s-mii", dev_name(&pdev->dev));
	priv = bus->priv;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->base)) {
		ret = PTR_ERR(priv->base);
		goto out_mdio;
	}

	ret = of_mdiobus_register(bus, pdev->dev.of_node);
	if (ret) {
		dev_err(&pdev->dev, "Cannot register MDIO bus (%d)\n", ret);
		goto out_mdio;
	}

	platform_set_drvdata(pdev, bus);

	return 0;

out_mdio:
	mdiobus_free(bus);
	return ret;
}

static int hip05_mdio_remove(struct platform_device *pdev)
{
	struct mii_bus *bus = platform_get_drvdata(pdev);

	mdiobus_unregister(bus);
	mdiobus_free(bus);

	return 0;
}

static const struct of_device_id hip05_mdio_match[] = {
	{ .compatible = "hisilicon, hip05-mdio" },
	{ }
};
MODULE_DEVICE_TABLE(of, hip05_mdio_match);

static struct platform_driver hip05_mdio_driver = {
	.probe = hip05_mdio_probe,
	.remove = hip05_mdio_remove,
	.driver = {
		.name = "hip05-mdio",
		.owner = THIS_MODULE,
		.of_match_table = hip05_mdio_match,
	},
};

module_platform_driver(hip05_mdio_driver);

MODULE_DESCRIPTION("HISILICON P05 MDIO interface driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:hip05-mdio");
