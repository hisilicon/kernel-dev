/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfWS9Irl5dOlu4MZmf/BOy7NdrK/dZ1wdkRQMXXJEBZkw4AWKHOaVIwXDACsQKkFzOIR6
VNaS3b1UnhjWtgOHhJHerIwvYnKeCOEwOkx16Ur7aAmTqkJ92xhIAuLs5/SYsQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_sub_peri.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : P660 PERI 复位及去复位
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/


#include "sre_sys_ctrl.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
UINT64 sub_sysctrl_base_addr = 0;
UINT64 sub_sysctrl_pa_addr = 0;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

 /* 定义Dsaf SUB的寄存器读写接口 */

#ifdef HRD_OS_LINUX
 inline UINT32 SYSCTRL_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;

     temp = ioread32((void __iomem *)(sub_sysctrl_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2)));

     return temp;
 }

#define SYSCTRL_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_sysctrl_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2))))

#else
#define SYSCTRL_SUB_READ_REG(pRegBase, ulRegIndex) \
                   OS_READ_REG(pRegBase, ulRegIndex)

#define SYSCTRL_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
                   OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)


#endif


/*****************************************************************************
 函 数 名  : HRD_GetBaseAddrByPllId
 功能描述  : 根据传入的PLL编号确定sysctrl基地址
 输入参数  : SRE_PLL_SEL_E ulPllSel:PLL编号
 输出参数  : 无
 返 回 值  : UINT64
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT64 HRD_GetSysCtrlAddrById(UINT32 ulSktId,SRE_PLL_SEL_E ulPllSel)
{
    UINT64 ulSubCtrlAddr;
    /* TOTEMC */
    if(ulPllSel < SRE_TOTEMA_PLL0)
    {
        /*TOTEMC PLL 2/3的配置信息在sysctrl中*/
        if((ulPllSel == SRE_TOTEMC_PLL2) || (ulPllSel == SRE_TOTEMC_PLL3))
            ulSubCtrlAddr = TOTEMC_SYSCTRL_BASE + ulSktId*HRD_VA_OFFSET;
        /*TOTEMC 其它PLL的配置信息在POE_sub中*/
        else
            ulSubCtrlAddr = TOTEMC_PEOSUB_BASE + ulSktId*HRD_VA_OFFSET;
    }
    /* TOTEMA */
    else if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        /*TOTEMA PLL 2/3的配置信息在sysctrl中*/
        if((ulPllSel == SRE_TOTEMA_PLL2) || (ulPllSel == SRE_TOTEMA_PLL3))
            ulSubCtrlAddr = TOTEMA_SYSCTRL_BASE + ulSktId*HRD_VA_OFFSET;
        /*TOTEMA 其它PLL的配置信息在POE_sub中*/
        else
            ulSubCtrlAddr = TOTEMA_PEOSUB_BASE + ulSktId*HRD_VA_OFFSET;
    }
    /* NIMBUS */
    else
        ulSubCtrlAddr = NIMBUS_SYSCTRL_BASE + ulSktId*HRD_VA_OFFSET;
    return ulSubCtrlAddr;
}

/*****************************************************************************
 函 数 名  : HRD_GetPllType
 功能描述  : 获取多片PLL选型信息
 输入参数  : UINT64  ulSubCtrlAddr :所要读取的subctrl基地址
             SRE_PLL_SEL_E ulPllSel:要获取片内哪个PLL的信息
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_GetPllType(UINT64 ulSubCtrlAddr,SRE_PLL_SEL_E ulPllSel)
{
    UINT32 ulType;
    /*TOTEM的PLL会有两种类型，而Nimbus则只有一种类型*/
    if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        if((ulPllSel%5) == SRE_TOTEMC_PLL0)
            ulType = SLICION_PLL;
        else if((ulPllSel%5) == SRE_TOTEMC_PLL1)
            ulType = TSMC_PLL;
        else
        {
            ulType = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+SYSCTRL_SC_PLL_SEL_DDR_REG-SYSCTRL_BASE,0x0);
            if((ulPllSel%5) == SRE_TOTEMC_PLL2)
                ulType = BIT_I(ulType,0);
            if((ulPllSel%5) == SRE_TOTEMC_PLL3)
                ulType = BIT_I(ulType,1);
            if((ulPllSel%5) == SRE_TOTEMC_PLL4)
                ulType = BIT_I(ulType,2);
        }
    }
    else
        ulType = SLICION_PLL;
    return ulType;
}


/*****************************************************************************
 函 数 名  : HRD_GetPllRegOffset
 功能描述  : 获取不同类型的同一PLL的寄存器配置偏移地址。
 输入参数  : UINT32 ulPllType      :PLL类型
             SRE_PLL_SEL_E ulPllSel:PLL 标号
             同一PLL不同类型的配置地址不一样。
 输出参数  : 无
 返 回 值  : 当前时钟频率，以M为单位。
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_GetPllRegOffset(UINT32 ulPllType,SRE_PLL_SEL_E ulPllSel)
{
    UINT32 ulRegOffset = 0;
    /*TOTEM中各个PLL的偏移*/
    if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        if(ulPllSel% 5 == SRE_TOTEMC_PLL0)
            ulRegOffset = POE_SUB_SC_PLLFCTRL0_REG - POE_SUB_BASE;
        if(ulPllSel% 5 == SRE_TOTEMC_PLL1)
            ulRegOffset = POE_SUB_SC_TSMC_PLLCTRL1_REG - POE_SUB_BASE;
        if(ulPllSel% 5 == SRE_TOTEMC_PLL2)
        {
            if(ulPllType == SLICION_PLL)
                ulRegOffset = SYSCTRL_SC_PLLFCTRL4_REG - SYSCTRL_BASE;
            else
                ulRegOffset = SYSCTRL_SC_TSMC_PLLCTRL2_REG - SYSCTRL_BASE;
        }
        if(ulPllSel% 5 == SRE_TOTEMC_PLL3)
        {
            if(ulPllType == SLICION_PLL)
                ulRegOffset = SYSCTRL_SC_PLLFCTRL6_REG - SYSCTRL_BASE;
            else
                ulRegOffset = SYSCTRL_SC_TSMC_PLLCTRL3_REG - SYSCTRL_BASE;
        }
        if(ulPllSel% 5 == SRE_TOTEMC_PLL4)
        {
            if(ulPllType == SLICION_PLL)
                ulRegOffset = POE_SUB_SC_PLLFCTRL8_REG - POE_SUB_BASE;
            else
                ulRegOffset = POE_SUB_SC_TSMC_PLLCTRL4_REG - POE_SUB_BASE;
        }
    }
    else
    {
        if(ulPllSel == SRE_NIMBUS_PLL0)
            ulRegOffset = ALG_SUB_SC_PLLFCTRL0_REG - ALG_SUB_BASE;
        if(ulPllSel == SRE_NIMBUS_PLL1)
            ulRegOffset = ALG_SUB_SC_PLLFCTRL2_REG - ALG_SUB_BASE;
        if(ulPllSel == SRE_NIMBUS_PLL2)
            ulRegOffset = ALG_SUB_SC_PLLFCTRL4_REG - ALG_SUB_BASE;
        if(ulPllSel == SRE_NIMBUS_PLL3)
            ulRegOffset = ALG_SUB_SC_PLLFCTRL6_REG - ALG_SUB_BASE;
        if(ulPllSel == SRE_NIMBUS_PLL4)
            ulRegOffset = ALG_SUB_SC_PLLFCTRL8_REG - ALG_SUB_BASE;
    }
    return ulRegOffset;
}


/*****************************************************************************
 函 数 名  : HRD_GetPllFrez
 功能描述  : 获取PLL的时钟频率
 输入参数  : UINT64  ulSubCtrlAddr :所要读取的subctrl基地址
             UINT32 ulPllType      :PLL类型
             SRE_PLL_SEL_E ulPllSel:PLL 标号
 输出参数  : 无
 返 回 值  : 当前时钟频率，以M为单位。
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_GetPllFrez(UINT64 ulSubCtrlAddr,UINT32 ulPllType,SRE_PLL_SEL_E ulPllSel)
{
    UINT32 i;
    UINT32 NF = 2;
    UINT32 NR = 1;
    UINT32 No = 1;
    UINT32 ulFrez;
    UINT32 ulRefDiv;
    UINT32 ulFbDiv;
    UINT32 ulPostDiv1;
    UINT32 ulRegOffset;
    SC_SlicionCrePLLTRL ulSliconCtrl;
    SC_TSMC_PLLCTRL     ulTsmcCtrl;

    ulRegOffset = HRD_GetPllRegOffset(ulPllType,ulPllSel);

    /*Silicon Creation PLL*/
    if(ulPllType == SLICION_PLL)
    {
        ulSliconCtrl.u32 = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+ulRegOffset,0x0);
        ulRefDiv     = ulSliconCtrl.bits.pll_refdiv;
        ulFbDiv      = ulSliconCtrl.bits.pll_fbdiv;
        ulPostDiv1   = ulSliconCtrl.bits.pll_postdiv1;
        ulFrez = SYS_FREF/ulRefDiv*ulFbDiv/ulPostDiv1;
    }
    else
    {
        ulTsmcCtrl.u32 = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+ulRegOffset,0x0);
        ulRefDiv    = ulTsmcCtrl.bits.tsmc_pll_refdiv;
        ulFbDiv    = ulTsmcCtrl.bits.tsmc_pll_fbdiv;
        ulPostDiv1  = ulTsmcCtrl.bits.tsmc_pll_postdiv;
        for(i=0;i<8;i++)
        {
            NF += 2*(1<<i)*BIT_I(ulFbDiv,i);
        }
        for(i=0;i<5;i++)
        {
            NR += (1<<i)*BIT_I(ulRefDiv,i);
        }
        No = 1<<ulPostDiv1;
        if(ulPostDiv1 >=4)
            No = 0x10;
        ulFrez = SYS_FREF/NR*NF/No;
    }
    return ulFrez;
}


/*****************************************************************************
 函 数 名  : HRD_GetPllLockStatus
 功能描述  : 获取PLL Lock信息
 输入参数  : UINT64  ulSubCtrlAddr :所要读取的subctrl基地址
             SRE_PLL_SEL_E ulPllSel:要获取片内哪个PLL的信息
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_GetPllLockStatus(UINT64 ulSubCtrlAddr,SRE_PLL_SEL_E ulPllSel)
{
    UINT32 ulLockStatus;
    /*TOTEM的PLL会有两种类型，而Nimbus则只有一种类型*/
    if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        ulLockStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+SYSCTRL_SC_PLL_LOCK_STATUS_REG-SYSCTRL_BASE,0x0);
        ulLockStatus = BIT_I(ulSubCtrlAddr,ulPllSel%5);
    }
    else
    {
        ulLockStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+ALG_SUB_SC_PLL_LOCK_ST_REG-ALG_SUB_BASE,0x0);
        ulLockStatus = BIT_I(ulSubCtrlAddr,ulPllSel%10);
    }
    return ulLockStatus;
}

/*****************************************************************************
 函 数 名  : HRD_GetBypassStatus
 功能描述  : 获取时钟bypass状态
 输入参数  : UINT64  ulSubCtrlAddr :所要读取的subctrl基地址
             SRE_PLL_SEL_E ulPllSel:要获取片内哪个PLL的信息
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
void HRD_GetBypassStatus(UINT64 ulSubCtrlAddr,SRE_PLL_SEL_E ulPllSel)
{
    UINT32 ulBypasStatus;
    /*TOTEM上各个时钟状态*/
    if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        if(((ulPllSel%5) == SRE_TOTEMC_PLL2) || ((ulPllSel%5) == SRE_TOTEMC_PLL3))
        {
            ulBypasStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+SYSCTRL_SC_PLL_CLK_BYPASS_DDR_REG-SYSCTRL_BASE,0x0);
            if((ulPllSel%5) == SRE_TOTEMC_PLL2)
            {
                if(BIT_I(ulBypasStatus,2) == 0x1)
                    OS_Printf("Extern Bypass:Yes.\n");
                else
                    OS_Printf("Extern Bypass:No.\n");
                if(BIT_I(ulBypasStatus,17) == 0x1)
                    OS_Printf("Peri Mode:Yes.\n");
                else
                    OS_Printf("Peri Mode:No.\n");
            }
            else
            {
                if(BIT_I(ulBypasStatus,3) == 0x1)
                    OS_Printf("Extern Bypass:Yes.\n");
                else
                    OS_Printf("Extern Bypass:No.\n");
                if(BIT_I(ulBypasStatus,18) == 0x1)
                    OS_Printf("Peri Mode:Yes.\n");
                else
                    OS_Printf("Peri Mode:No.\n");
            }
        }
        else
        {
            ulBypasStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+POE_SUB_SC_PLL_CLK_BYPASS1_REG - POE_SUB_BASE,0x0);
            if((ulPllSel%5) == SRE_TOTEMC_PLL0)
            {
                if(BIT_I(ulBypasStatus,0x0) == 0x1)
                    OS_Printf("Extern Bypass:Yes.\n");
                else
                    OS_Printf("Extern Bypass:No.\n");
            }
            if((ulPllSel%5) == SRE_TOTEMC_PLL1)
            {
                if(BIT_I(ulBypasStatus,0x1) == 0x1)
                    OS_Printf("Extern Bypass:Yes.\n");
                else
                    OS_Printf("Extern Bypass:No.\n");
            }
            if((ulPllSel%5) == SRE_TOTEMC_PLL4)
            {
                if(BIT_I(ulBypasStatus,0x4) == 0x1)
                    OS_Printf("Extern Bypass:Yes.\n");
                else
                    OS_Printf("Extern Bypass:No.\n");
            }
            ulBypasStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+POE_SUB_SC_PLL_CLK_BYPASS0_REG - POE_SUB_BASE,0x0);
            if((ulPllSel%5) == SRE_TOTEMC_PLL4)
            {
                if(ulBypasStatus == 0x1)
                    OS_Printf("Peri Mode:Yes.\n");
                else
                    OS_Printf("Peri Mode:No.\n");
            }
        }
    }
    /* Nimbus上各个时钟状态 */
    else
    {
        ulBypasStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+ALG_SUB_SC_PLL_CLK_BYPASS1_REG - ALG_SUB_BASE,0x0);
        if(BIT_I(ulBypasStatus,ulPllSel%10) == 0x1)
            OS_Printf("Extern Bypass:Yes.\n");
        else
            OS_Printf("Extern Bypass:No.\n");
        ulBypasStatus = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+ALG_SUB_SC_PLL_CLK_BYPASS0_REG - ALG_SUB_BASE,0x0);
        ulBypasStatus = ulBypasStatus >> 16;
        if(BIT_I(ulBypasStatus,ulPllSel%10) == 0x1)
            OS_Printf("Peri Mode:Yes.\n");
        else
            OS_Printf("Peri Mode:No.\n");
    }
}

/*****************************************************************************
 函 数 名  : HRD_GetPllDfxInfo
 功能描述  : 获取多片PLL信息
 输入参数  : UINT32        ulSktId :配置获取哪个片上的PLL信息.0:片0，1:片1。
             SRE_PLL_SEL_E ulPllSel:要获取片内哪个PLL的信息
             每个TOTEM有5个PLL，每个NIMBUS有5个PLL。
             传入编号0-4代表TOTEMC的PLL0-4;5-9代表TOTEMA PLL0-4;
             10-14代表NIMBUS的PLL0-4。
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
void HRD_GetPllDfxInfo(UINT32 ulSktId, SRE_PLL_SEL_E ulPllSel)
{
    UINT32 ulFrez;
    UINT32 ulPllType;
    UINT32 ulSysStatus;
    UINT32 ulLockStatus;
    UINT64 ulSubCtrlAddr;
    if(ulPllSel < SRE_NIMBUS_PLL0)
        OS_Printf("Socket%d Totem%d Pll%d:\n",ulSktId,ulPllSel/5,ulPllSel%5);
    else
        OS_Printf("Socket%d Nimbus Pll%d:\n",ulSktId,ulPllSel%10);

    ulSubCtrlAddr = HRD_GetSysCtrlAddrById(ulSktId,ulPllSel);

    /*获取各个PLL类型*/
    ulPllType = HRD_GetPllType(ulSubCtrlAddr,ulPllSel);
    if(ulPllType == SLICION_PLL)
        OS_Printf("pllType:Silicon Creation PLL.\n");
    else
        OS_Printf("pllType:TSMC PLL.\n");

    /*获取各个PLL频率*/
    ulFrez = HRD_GetPllFrez(ulSubCtrlAddr,ulPllType,ulPllSel);
    OS_Printf("Frequency:%dM.\n",ulFrez);

    /*获取各个PLL时钟锁定状态*/
    ulLockStatus = HRD_GetPllLockStatus(ulSubCtrlAddr,ulPllSel);
    if(ulLockStatus == 0x1)
        OS_Printf("LockStatus:Locked.\n");
    else
        OS_Printf("LockStatus:Locked.\n");

    /*获取各个PLL时钟bypass状态*/
    HRD_GetBypassStatus(ulSubCtrlAddr,ulPllSel);

    /*获取当前CPU用的PLL*/
    if(((ulPllSel%5) <= SRE_TOTEMC_PLL1)&&(ulPllSel < SRE_NIMBUS_PLL0))
    {
        if(((SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+0x3048,0x0))&0x1)== 0x0)
            SRE_printf("Current CPU PLL:Pll%d.\n",0);
        else
            SRE_printf("Current CPU PLL:Pll%d.\n",1);
    }

    /*获取系统运行状态*/
    if(ulPllSel < SRE_NIMBUS_PLL0)
    {
        if(ulPllSel < SRE_TOTEMA_PLL0 )/*Totemc的sysctrl*/
            ulSubCtrlAddr = TOTEMC_SYSCTRL_BASE + ulSktId*HRD_VA_OFFSET;
        else/*Totema的sysctrl*/
            ulSubCtrlAddr = TOTEMA_SYSCTRL_BASE + ulSktId*HRD_VA_OFFSET;

        ulSysStatus  = SYSCTRL_SUB_READ_REG(ulSubCtrlAddr+SYSCTRL_SC_SYSMODE_CTRL_REG - SYSCTRL_BASE,0x0);
        if(BIT_I(ulSysStatus,0x2) == 0x1)
            OS_Printf("SysMode:Normal.\n");
        else
            OS_Printf("SysMode:Slow.\n");

    }

}

/*****************************************************************************
 函 数 名  : HRD_GetCurCpuFreq
 功能描述  : 获取当前CPU的频率，单位为Hz
 输入参数  : ulSktId: socket ID, ulDieID: 0 - CDIE A, 1- CDIE C                  
 输出参数  : 无
 返 回 值  : UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014/9/10
    作    者   : n00283962
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_GetCurCpuFreq(UINT32 ulSktId, UINT32 ulDieID)
{
    UINT32 ulFreq;
    UINT32 ulPllType;
    UINT64 ulSubCtrlAddr = 0;
    SRE_PLL_SEL_E ulPllSel = SRE_TOTEMC_PLL0;

    if (ulDieID == 0) 
    {
        ulSubCtrlAddr = HRD_GetSysCtrlAddrById(ulSktId, SRE_TOTEMA_PLL0);        
    }
    else if (ulDieID == 1)
    {
        ulSubCtrlAddr = HRD_GetSysCtrlAddrById(ulSktId, SRE_TOTEMC_PLL0);
    }
    
    /*获取当前CPU用的PLL*/
    if (((SYSCTRL_SUB_READ_REG(ulSubCtrlAddr + 0x3048, 0x0)) & 0x1) == 0x0)
    {
        if (ulDieID == 0)
        {
            ulPllSel = SRE_TOTEMA_PLL0;            
        }
        else if (ulDieID == 1)
        {
            ulPllSel = SRE_TOTEMC_PLL0; 
        }
    }
    else
    { 
        if (ulDieID == 0)
        {
            ulPllSel = SRE_TOTEMA_PLL1;            
        }
        else if (ulDieID == 1)
        {
            ulPllSel = SRE_TOTEMC_PLL1; 
        }
    }

    ulSubCtrlAddr = HRD_GetSysCtrlAddrById(ulSktId, ulPllSel);
    /*获取各个PLL类型*/
    ulPllType = HRD_GetPllType(ulSubCtrlAddr, ulPllSel);

    /*获取各个PLL频率*/
    ulFreq = HRD_GetPllFrez(ulSubCtrlAddr, ulPllType, ulPllSel);
    ulFreq *= 1000000;
    return ulFreq;

}

/*****************************************************************************
 函 数 名  : HRD_SysCtrl_Link
 功能描述  : 用于app_link中调用，避免被优化
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_SysCtrl_Link()
{
    return 0;
}
#ifdef HRD_OS_LINUX
EXPORT_SYMBOL(HRD_GetPllDfxInfo);
#endif

