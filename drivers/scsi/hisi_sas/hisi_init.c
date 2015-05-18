#include "hisi_sas.h"

static int hisi_sas_probe(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);

	return 0;
}

static int hisi_sas_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id sas_of_match[] = {
	{ .compatible = "hisilicon,p660-sas",},
	{},
};

static struct platform_driver sas_driver = {
	.probe = hisi_sas_probe,
	.remove = hisi_sas_remove,
	.driver = {
		.name = "hisi_sas_controller",
		.owner = THIS_MODULE,
		.of_match_table = sas_of_match,
	},
};



static __init int hisi_sas_init(void)
{
	int rc;

	pr_info("%s\n", __func__);

	rc = platform_driver_register(&sas_driver);
	if (rc)
		return rc;

	return 0;
}

static __exit void hisi_sas_exit(void)
{

}

module_init(hisi_sas_init);
module_exit(hisi_sas_exit);
