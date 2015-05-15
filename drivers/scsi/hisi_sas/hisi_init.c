#include "hisi_sas.h"

static __init int hisi_sas_init(void)
{
	pr_info("hisi_sas_init");
	return 0;
}

static __exit void hisi_sas_exit(void)
{

}

module_init(hisi_sas_init);
module_exit(hisi_sas_exit);
