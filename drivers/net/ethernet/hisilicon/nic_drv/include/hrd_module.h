/**
 * @file hrd_module.h
 *
 * Copyright(C), 2008-2050, Huawei Tech. Co., Ltd. ALL RIGHTS RESERVED. \n
 *
 * 描述：定义模块ID。 \n
 */

/**
 * @addtogroup hrd_sys 系统基本功能接口
 * @ingroup
 * @{
*/

#ifndef _HRD_MODULE_H
#define _HRD_MODULE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

typedef enum
{
    HRD_MID_SYS = 0x0,
    HRD_MID_DSAF = 0x1,
    HRD_MID_PCIE = 0x2,
    HRD_MID_ROCE = 0x3,
    HRD_MID_NAND = 0x4,
    HRD_MID_NIC = 0x5,
    HRD_MID_GIC = 0x6,
    HRD_MID_DMAC = 0x7,
    HRD_MID_RING = 0x8,
    HRD_MID_HCCS = 0x9,
    HRD_MID_LLC = 0xA,
    HRD_MID_SMMU = 0xB,
    HRD_MID_SEC = 0xC,
    HRD_MID_RSA = 0xD,
    HRD_MID_RDE = 0xE,
    HRD_MID_SAS = 0xF,
    HRD_MID_TRNG = 0x10,
    HRD_MID_UART = 0x11,
    HRD_MID_SPI = 0x12,
    HRD_MID_GPIO = 0x13,
    HRD_MID_I2C = 0x14,
    HRD_MID_TMR = 0x15,
    HRD_MID_SYSCTRL = 0x16,
    HRD_MID_PMU = 0x17

}HRD_MID_EN;



#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _HRD_MODULE_H */

/**
 * @}
*/

/**
 * History:
 *
 * vi: set expandtab ts=4 sw=4 tw=80:
*/

