#ifndef _VIRTUAL_DEV_H
#define _VIRTUAL_DEV_H
#ifdef __cplusplus
        extern "C" {
#endif
#include <linux/cdev.h>/*struct cdev*/

/*内核公用头文件定义added by p00165084*/
/* part1: KERNEL API*/
#include <linux/module.h>
#include <linux/kernel.h>    /*printk*/
#include <linux/init.h>
#include <linux/slab.h>           /*kmalloc*/
#include <linux/spinlock.h>     /*spinlock*/
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/semaphore.h>  /*semaphore*/
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/cacheflush.h>
#include <linux/list.h> /*p00165084*/
#include <linux/poll.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <asm/uaccess.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/mtd/mtd.h>
#include <linux/err.h>

#include "hrdCommon.h"
#include "hrdOs.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define TOTAL_MPTG_MOD_NUM  256  //need to be modified in the future
#define MIN_MPTG_MOD_NUM    0
#define MAX_MPTG_MOD_NUM    256
#define DEV_NUM 5
#define NAME_SIZE      32

#ifndef BYTE_SWAP32
#define BYTE_SWAP32(x)        ((((unsigned)(x) & 0x000000ff) << 24) | \
			                             (((x) & 0x0000ff00) <<  8) | \
			                             (((x) & 0x00ff0000) >>  8) | \
			                             (((x) & 0xff000000) >> 24))
#endif

#ifndef REG_READ
#define REG_READ(addr,data)             ((data) = *(volatile unsigned int *)(addr))
#endif

#ifndef SYS_CTRL_REG_READ
#define SYS_CTRL_REG_READ(offset,data)   do \
                                         { \
                                            (REG_READ((offset), (data))); \
                                            (data) = (BYTE_SWAP32(data)); \
                                         }while(0)
#endif


/*----------------------------------------------*
 *设备命令格式定义                                       *
 *----------------------------------------------*/
#define CMD_RESERVE_SIZE    8
#define CMD_FUNC_SIZE       8
#define CMD_DEVICE_SIZE     8
#define CMD_MODULE_SIZE     8

#define CMD_FUNC_MASK       ((0x1UL << CMD_FUNC_SIZE)   - 1)
#define CMD_DEVICE_MASK     ((0x1UL << CMD_DEVICE_SIZE) - 1)
#define CMD_MODULE_MASK     ((0x1UL << CMD_MODULE_SIZE) - 1)

#define CMD_RESERVE_SHIFT   0
#define CMD_FUNC_SHIFT      (CMD_RESERVE_SHIFT + CMD_RESERVE_SIZE)
#define CMD_DEVICE_SHIFT    (CMD_FUNC_SHIFT    + CMD_FUNC_SIZE)
#define CMD_MODULE_SHIFT    (CMD_DEVICE_SHIFT  + CMD_DEVICE_SIZE)

#define GET_MODULE_ID(cmd)  ((cmd)>>24)
#define GET_MODULE_INDEX(cmd) (((cmd)>>16)&0xff)
#define GET_MODULE_FUNCTION(cmd)  (((cmd)>>8)&0xff)

typedef struct st_devtype
{
    u32 dev_major;
    u32 dev_minor;
    struct cdev cdev;    /* Char device structure */
    struct class *dev_class;
}CDEV_ST;


extern void comm_cleanup_dev(CDEV_ST* pDev);
extern int comm_init_dev(CDEV_ST *pDev, struct file_operations *pfoprs, s8 *pdevname );

/*----------------------------------------------*
 *具体设备功能定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 外部函数定义                                     *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 定义功能模块ID                                    *
 *----------------------------------------------*/


#ifdef __cplusplus
  }
#endif

#endif
