/******************************************************************************

				  版权所有 (C), 2012-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : hrd_error.h
  版 本 号   : 初稿
  作	者   : z00142220
  生成日期   : 2013年7月26日
  最近修改   :
  功能描述   : 公共错误码定义头文件
  函数列表   :
  修改历史   :
  1.日	期   : 2013年7月26日
	作	者   : z00142220
	修改内容   : 创建文件

******************************************************************************/
#ifndef _HRD_ERROR_H
#define _HRD_ERROR_H
/* #include "hrd_typedef.h" */


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#define HRD_OK										  (0)
#define HRD_ERR										 (1)

#define HRD_ERR_BASE									(1024)

/* 公共错误码 */
#define HRD_COMMON_ERR_BASE							 (HRD_ERR_BASE)
#define HRD_COMMON_ERR_NULL_POINTER					 (HRD_COMMON_ERR_BASE + 1)
#define HRD_COMMON_ERR_UNKNOW_DEVICE					(HRD_COMMON_ERR_BASE + 2)
#define HRD_COMMON_ERR_UNKNOW_FUNCTION				  (HRD_COMMON_ERR_BASE + 3)
#define HRD_COMMON_ERR_OPEN_FAIL						(HRD_COMMON_ERR_BASE + 4)
#define HRD_COMMON_ERR_READ_FAIL						(HRD_COMMON_ERR_BASE + 5)
#define HRD_COMMON_ERR_WRITE_FAIL					   (HRD_COMMON_ERR_BASE + 6)
#define HRD_COMMON_ERR_MMAP_FAIL						(HRD_COMMON_ERR_BASE + 7)
#define HRD_COMMON_ERR_GET_MEN_RES_FAIL				 (HRD_COMMON_ERR_BASE + 8)
#define HRD_COMMON_ERR_GET_IRQ_RES_FAIL				 (HRD_COMMON_ERR_BASE + 9)
#define HRD_COMMON_ERR_INPUT_INVALID					(HRD_COMMON_ERR_BASE + 10)
#define HRD_COMMON_ERR_UNKNOW_MODE					  (HRD_COMMON_ERR_BASE + 11)
#define HRD_COMMON_ERR_NOT_ENOUGH_RES				   (HRD_COMMON_ERR_BASE + 12)
#define HRD_COMMON_ERR_RES_NOT_EXIST					(HRD_COMMON_ERR_BASE + 13)



/**
 * @brief 定义模块错误码
 *
 * @description
 * 宏定义，定义模块错误码
 *
 * @attention 无
 * @param[in]   MID 模块ID编号。
 * @param[in]   ERRNO 错误码编号。
 *
 * @retval 无
 * @depend
 * iwr_errno.h: 该宏定义所在的头文件。
 * @since HRD ARM PV660 V100R001
 * @see
 */
#define HRD_ERRNO(MID,ERRNO)  \
			(int)(HRD_ERR_BASE - ((UINT32)(MID) << 8) - (UINT32)(ERRNO))
#if 0			/* 在SRE下采用下面的定义 */
 /**
 * @brief 定义模块错误码
 *
 * @description
 * 宏定义，定义模块错误码
 *
 * @attention 无
 * @param[in]   MID 模块ID编号。
 * @param[in]   ERRNO 错误码编号。
 *
 * @retval 无
 * @depend
 * SRE_errno.h: 该宏定义所在的头文件。
 * @since SRE V100R001
 * @see SRE_ERRNO_OS_FATAL | SRE_ERRNO_OS_WARN | SRE_ERRNO_OS_NORMAL
 */
#define HRD_ERRNO(MID,ERRNO)  \
			(SRE_ERRNO_DRV_ID | ((UINT32)(MID) << 8) | SRE_ERRTYPE_ERROR | (ERRNO))
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#endif /* _HRD_ERROR_H */

