/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : hrd_bsp_mod.h
  版 本 号   : 初稿
  作    者   : w00146306
  生成日期   : 2013年4月25日
  最近修改   :
  功能描述   : uniBSP内核态头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2013年4月25日
    作    者   : w00146306
    修改内容   : 创建文件

******************************************************************************/

#ifndef _UNIBSP_MOD_H
#define _UNIBSP_MOD_H

#include <linux/cdev.h>
#include <linux/device.h>


#include "hrd_base_api.h"
#include "hrd_mod_api.h"


/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#define MCSS_DEVS_EACH_MAX_MAC_NUM  3

typedef struct tagMcssDev2
{
    struct cdev      stChrDev;
    struct device   *pstDev;
    struct semaphore stSem;
}MCSS_S;

#pragma pack(4)
typedef struct tagMcssDev
{
    UINT32            uwIndex;   /* 设备编号 */
    UINT32            uwPid;     /* 进程ID */
    UINT32            uwRingID;  /* 设备对应的RingID */
    UINT32            uwChnFlg;  /* 通道是否打开 */
    UINT32            uwPktFlg;
    UINT32            uwSemFlg;  /* 设备占有信号量标记 */
    atomic_t          stOpenFlg; /* 设备打开标记 */
    wait_queue_head_t stQueue;
    UINT32            uwTxPort;  /* 发送端口号 */
    struct cdev       stChrDev;
    struct device    *pstDev;
}MCSS_DEV_S;

typedef struct tagMcssDevMacInfo
{
    UINT32              uwNum;  /* 每个PID已配置的MAC个数 */
    UINT32              uwPid;  /* 进程ID */
    MCSS_TYPE_MAC_S     stMac[MCSS_DEVS_EACH_MAX_MAC_NUM]; /* 每个pid对应多个MAC地址 */
}MCSS_DEV_MAC_INFO_S;

typedef struct tagMcssDevInfo
{
    UINT32              uwFixedMac2Pid;
    UINT32              uwInstanseFlg;
    spinlock_t          stLock;
    struct mutex        stMutex;
    struct timer_list   stTimer;
    UINT32              auwRingDeadId[32];
    MCSS_DEV_MAC_INFO_S stMacInfo[MCSS_MAX_INSTANCE_NUM];
}MCSS_DEV_INFO_S;


typedef struct tagBspCmd
{
    char   acCmd[0x100];
    UINT64 ulArg1;
    UINT64 ulArg2;
    UINT64 ulArg3;
    UINT64 ulArg4;
    UINT64 ulArg5;
    UINT64 ulArg6;
    UINT64 ulArg7;
    UINT64 ulArg8;
    UINT64 ulArg9;
    UINT64 ulArg10;
    UINT64 ulArg11;
    UINT64 ulArg12;
    UINT64 ulArg13;
    UINT64 ulArg14;
    UINT64 ulArg15;
} BSP_CMD_S;
#pragma pack()

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
INT32  MC_Init(void);
INT32  MC_CommInitVm(void);
UINT32 MC_MmuAssign(UINT64 *puwVirtAddr, UINT64 udwPhyAddr, UINT32 uwSize,
                    UINT32 uwPriority, char *acName);
UINT32 MC_MmuVA2PA(UINT64 ulVirtAddr, UINT64 *pudlPhyAddr);
UINT32 MC_MmuPA2VA(UINT64 udlPhyAddr, UINT64 *pulVirtAddr);
UINT32 MC_IrqSetTarget(UINT32 ulIrq, UINT32 ulCore);
void   MC_DumpMem(UINT8 *pucAddr, UINT32 uwSize);
UINT32 MC_MmuIsMapped(UINT64 uwVirtAddr, UINT32 uwLen);
UINT32 MC_MmuShow(void);
UINT32 SRE_GetCoreID(void);

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

#endif /*_UNIBSP_MOD_H*/


