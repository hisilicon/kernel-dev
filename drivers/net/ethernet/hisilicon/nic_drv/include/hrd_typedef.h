/**
 * @file hrd_typedef.h
 *
 * Copyright(C), 2008-2050, Huawei Tech. Co., Ltd. ALL RIGHTS RESERVED. \n
 *
 * 描述：定义基本数据类型和数据结构。 \n
 */

/**
 * @addtogroup hrd_sys 系统基本功能接口
 * @ingroup
 * @{
*/

#ifndef _HRD_TYPEDEF_H
#define _HRD_TYPEDEF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifndef INLINE
#define INLINE                                              static __inline
#endif

#ifndef SRE_TYPE_DEF
#define SRE_TYPE_DEF

/* type definitions */
/* SRE接口层使用的数据类型 */
typedef unsigned char                                       UINT8;
typedef unsigned short                                      UINT16;
typedef unsigned int                                        UINT32;
typedef signed short                                        INT16;
typedef signed int                                          INT32;
typedef float                                               FLOAT;
typedef double                                              DOUBLE;
typedef char                                                CHAR;
typedef unsigned long long                                  UINT64;
typedef signed long long                                    INT64;
typedef unsigned int                                        UINTPTR;
typedef signed int                                          INTPTR;
typedef char                                                INT8;
#define VOID                                                void



/* Linux接口层使用的数据类型，如果HAL层需要在Linux下提供，则需要使用下面这个类型
   定义，否则使用SRE接口层使用的类型定义 */
#ifdef HRD_OS_SRE
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef char                s8;
typedef short               s16;
typedef int                 s32;
typedef long long           s64;
#endif /* HRD_OS_SRE */

#ifdef HRD_OS_LINUX
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef unsigned long long  U64;
typedef char                S8;
typedef short               S16;
typedef int                 S32;
typedef long long           S64;

#endif /* HRD_OS_LINUX */


#endif  /*end of #ifndef SRE_TYPE_DEF*/

#ifndef NULL
#define NULL                                                ((VOID *)0)
#endif

typedef unsigned int                                        BOOL;


#ifndef HRD_STATUS
#define HRD_STATUS int
#endif


typedef void* hrd_handle_t;

#ifndef FALSE
#define FALSE                                               ((BOOL)0)
#endif

#ifndef TRUE
#define TRUE                                                ((BOOL)1)
#endif

#ifndef OK
#define OK      0
#endif

#ifndef ERROR
#define ERROR	(-1)
#endif

/* 在Linux中大部分函数可以定义成static，但是在SRE下，这些函数会被其他文件直接调用，
   无法定义成static，所以定义下面的宏，通过不同的编译选项来选择 */
#ifdef HRD_OS_SRE
#ifndef LINUX_STATIC
#define LINUX_STATIC
#endif
#else
#ifndef LINUX_STATIC
#define LINUX_STATIC static
#endif
#endif /* HRD_OS_SRE */

#ifdef HRD_OS_SRE
#ifndef OSAL_ERR_PTR
#define OSAL_ERR_PTR(error) ((void *)error)
#endif
#else
#ifndef OSAL_ERR_PTR
#define OSAL_ERR_PTR(error) ERR_PTR(error)
#endif
#endif /* HRD_OS_SRE */

#ifdef HRD_OS_SRE
#ifndef OSAL_PTR_ERR
#define OSAL_PTR_ERR(ptr) ((long)ptr)
#endif
#else
#ifndef OSAL_PTR_ERR
#define OSAL_PTR_ERR(ptr) PTR_ERR(ptr)
#endif
#endif /* HRD_OS_SRE */


#define ENDNESS_LITTLE 123
#define ENDNESS_BIG    321


#define ENDNESS ENDNESS_LITTLE




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _HRD_TYPEDEF_H */

/**
 * @}
*/

/**
 * History:
 *
 * vi: set expandtab ts=4 sw=4 tw=80:
*/

