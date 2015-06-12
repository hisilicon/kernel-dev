/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfUuJbCM3d7V3bPDo3IXLHvr351qh2RbEllDtVh7rYzY006B3P7D2OTEUv70yPVe510ON
gyP7XcmNe3n+26NetHKBZPzfQxABzNVCjtsqNqQrkIdoaXeJ6yvu6eE7hayzVg==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#ifdef HRD_OS_LINUX
#include "iware_comm_kernel_api.h"
#endif


#ifdef HRD_OS_LINUX
extern UINT32 HRD_Crg_LinuxInit(void);
extern UINT32 HRD_Crg_LinuxExit(void);

int comm_init_dev(CDEV_ST *pdev, struct file_operations *pfoprs, s8 *pdevname )
{
    dev_t ndev;
    int result;
    struct device *pdevTmp;

    if ( pdev == NULL || pfoprs == NULL || pdevname == NULL )
    {
        printk("error : input parmas error,pdev %p,pfoprs %p,pdevname %s\n",pdev,pfoprs,pdevname);
        return OS_ERROR;
    }

    if (pdev->dev_major)
    {
        ndev = MKDEV(pdev->dev_major, pdev->dev_minor);
        result = register_chrdev_region(ndev, 1, pdevname);
    }
    else
    {
        result = alloc_chrdev_region(&ndev,pdev->dev_minor,1,pdevname);
        pdev->dev_major = MAJOR(ndev);
    }

    if ( result < 0 )
    {
        printk(KERN_WARNING "can't get major %d\n", pdev->dev_major);
        return result;
    }

    printk("char dev %s init,major = %d, minor = %d\n",pdevname,MAJOR(ndev),MINOR(ndev));

    /* 初始化字符设备 */
    cdev_init(&pdev->cdev,pfoprs);
    pdev->cdev.owner = THIS_MODULE;
    result = cdev_add(&pdev->cdev,ndev,1);
    if ( OK != result )
    {
        pr_err("%s %d: cdev_add fail\n", __FILE__,__LINE__);
        goto cdev_add_fail;  /* Make this more graceful */
    }

    /* 创建虚设备类 */
    pdev->dev_class = class_create(THIS_MODULE, pdevname);
    if(IS_ERR(pdev->dev_class))
    {
        result = OS_ERROR; /*(int)(pdev->dev_class);  */
        printk(KERN_ALERT "Err:failed in creating class.\n");
        goto class_create_fail;
    }

    pdevTmp = device_create(pdev->dev_class , NULL, MKDEV(pdev->dev_major, pdev->dev_minor), NULL, pdevname);
    if(IS_ERR(pdevTmp))
    {
        result = OS_ERROR; /*(int)pdevTmp;  */
        printk(KERN_ALERT "Err:failed in creating device.\n");
            goto device_create_fail;
    }

    printk("char dev %s init success\n",pdevname);

    return OK;

device_create_fail:
     class_destroy(pdev->dev_class);
class_create_fail:
     cdev_del(&pdev->cdev);
cdev_add_fail:
    unregister_chrdev_region(MKDEV(pdev->dev_major, pdev->dev_minor), 1);
    return result;

}

EXPORT_SYMBOL(comm_init_dev);


void comm_cleanup_dev(CDEV_ST* pdev)
{
    dev_t dev_no = MKDEV(pdev->dev_major,pdev->dev_minor);

    if ( pdev->dev_class != NULL)
    {
        device_destroy(pdev->dev_class, dev_no);
        class_destroy(pdev->dev_class);
    }

    if ( &pdev->cdev != NULL)
    {
        cdev_del(&pdev->cdev);
    }

    if ( pdev != NULL )
    {
        kfree(pdev);
    }

    unregister_chrdev_region(dev_no, 1);

    return;
}

EXPORT_SYMBOL(comm_cleanup_dev);


INT32 __init hrd_common_init(void)
{
    (void)HRD_Crg_LinuxInit();

    return 0;
}

void __exit hrd_common_exit(void)
{
    (void)HRD_Crg_LinuxExit();

    return;
}

module_init(hrd_common_init);
module_exit(hrd_common_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("x68638");
MODULE_DESCRIPTION("osal common module");
MODULE_VERSION("V0.1");


#endif
