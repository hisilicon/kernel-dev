#ifndef _OSAL_API_H_
#define _OSAL_API_H_

#include "hrd_typedef.h"

#define OS_SUCCESS					 (0)
#define OS_FAIL						(1)
#define OS_ERROR					   (-1)
#define OS_INVALID_POINTER			 (-2)
#define OS_ERROR_ADDRESS_MISALIGNED	(-3)
#define OS_ERROR_TIMEOUT			   (-4)
#define OS_INVALID_INT_NUM			 (-5)
#define OS_SEM_FAILURE				 (-6)
#define OS_SEM_TIMEOUT				 (-7)
#define OS_QUEUE_EMPTY				 (-8)
#define OS_QUEUE_FULL				  (-9)
#define OS_QUEUE_TIMEOUT			   (-10)
#define OS_QUEUE_INVALID_SIZE		  (-11)
#define OS_QUEUE_ID_ERROR			  (-12)
#define OS_ERR_NAME_TOO_LONG		   (-13)
#define OS_ERR_NO_FREE_IDS			 (-14)
#define OS_ERR_NAME_TAKEN			  (-15)
#define OS_ERR_INVALID_ID			  (-16)
#define OS_ERR_NAME_NOT_FOUND		  (-17)
#define OS_ERR_SEM_NOT_FULL			(-18)
#define OS_ERR_INVALID_PRIORITY		(-19)
#define OS_INVALID_SEM_VALUE		   (-20)
#define OS_ERR_FILE					(-27)
#define OS_ERR_NOT_IMPLEMENTED		 (-28)
#define OS_TIMER_ERR_INVALID_ARGS	  (-29)
#define OS_TIMER_ERR_TIMER_ID		  (-30)
#define OS_TIMER_ERR_UNAVAILABLE	   (-31)
#define OS_TIMER_ERR_INTERNAL		  (-32)



/*
** Platform Configuration Parameters for the OS API
*/

#define OS_MAX_TASKS				64
#define OS_MAX_QUEUES			   64
#define OS_MAX_COUNT_SEMAPHORES	 20
#define OS_MAX_BIN_SEMAPHORES	   20
#define OS_MAX_MUTEXES			  20
#define OS_MAX_SPINLOCKS			60

/*
** Maximum length for an absolute path name
*/
#define OS_MAX_PATH_LEN	 64

/*
** Maximum length for a local or host path/filename.
** This parameter is used for the filename on the host OS.
** Because the local or host path can add on to the OSAL virtual path,
** This needs to be longer than OS_MAX_PATH_LEN.
**  On a system such as RTEMS, where the virtual paths can be the same as
**  local paths, it does not have to be much bigger.
** On a system such as Linux, where a OSAL virtual drive might be
** mapped to something like: "/home/bob/projects/osal/drive1", the
**  OS_MAX_LOCAL_PATH_LEN might need to be 32 more than OS_MAX_PATH_LEN.
*/
#define OS_MAX_LOCAL_PATH_LEN (OS_MAX_PATH_LEN + 16)


/*
** The maxium length allowed for a object (task,queue....) name
*/
#define OS_MAX_API_NAME	 20

/*
** The maximum length for a file name
*/
#define OS_MAX_FILE_NAME	20

/*
** These defines are for OS_printf
*/
#define OS_BUFFER_SIZE 172
#define OS_BUFFER_MSG_DEPTH 100


HRD_STATUS OS_MutexCreate(UINT32 *mutex_id);

HRD_STATUS OS_MutexLock(UINT32 mutex_id);

HRD_STATUS OS_MutexUnLock(UINT32 mutex_id);

HRD_STATUS OS_MutexDelete(UINT32 mutex_id);

HRD_STATUS OS_SpinLockCreate(UINT32 *lock_Id);

HRD_STATUS OS_SpinLockIrqSave(UINT32 lock_id, unsigned long flags);

HRD_STATUS OS_SpinUnLockIrqRestore(UINT32 lock_id, unsigned long flags);

HRD_STATUS OS_SpinLockDelete(UINT32 lock_id);

extern VOID SRE_DelayMs(UINT32 uwDelay);

/******************************************************************************
Linux
*******************************************************************************/
#ifdef HRD_OS_LINUX
#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/delay.h>
/* linux下不通过这个宏指定数据段 */
#define OS_SEC_DATA_SHARED

#define osal_mdelay(ms) mdelay(ms)


#define osal_printf(format, args...) \
	do  \
	{   \
		printk(format, ## args);  \
	}   \
	while (0)
#define osal_pr(format, args...) \
	do  \
	{   \
		printk(format, ## args);  \
	}   \
	while (0)

#else

#define osal_printf(format, args...) \
	do  \
	{   \
		printf(format, ## args);  \
	}   \
	while (0)
#define osal_pr(format, args...) \
	do  \
	{   \
		printf(format, ## args);  \
	}   \
	while (0)

#endif /* __KERNEL__ */

/* 内存管理 */
#define osal_kmalloc(size, flags) kmalloc(size, flags)
#define osal_kfree(ptr) kfree(ptr)
#define osal_kzalloc(size, flags) kzalloc(size, flags)
#define osal_vmalloc(size) vmalloc(size)
#define osal_vfree(ptr) vfree(ptr)
#define osal_kmalloc_node(size, flags, node) kmalloc_node(size, flags, node)
#define osal_vmalloc_node(size, node) vmalloc_node(size, node)


#elif defined (HRD_OS_SRE) /* HRD_OS_LINUX */
/******************************************************************************
SRE
*******************************************************************************/
#include "SRE_mem.h"
#include "SRE_shell.h"

struct mac_device;
typedef struct netdrv_private
{
	struct mac_device *mac_dev;
}netdrv_private_s;

struct net_device {
	netdrv_private_s *priv;

};
static inline void *netdev_priv(const struct net_device *dev)
{
	return dev->priv;
}


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif


#define osal_printf(format, args...) \
	do  \
	{   \
		(void)SRE_printf(format, ## args);  \
	}   \
	while (0)

#if !defined _LIBC && (!defined __GNUC__ || __GNUC__<3)
#define __builtin_expect(expr, val) (expr)
#endif

#if __GNUC__ >= 3
	#define likely(x)	 __builtin_expect(!!(x), 1)
	#define unlikely(x)   __builtin_expect(!!(x), 0)
#else
	#define likely(x)	 (x)
	#define unlikely(x)   (x)
#endif

#define osal_mdelay(ms) SRE_DelayMs(ms)

#define OSAL_MAX_ERRNO	0x7fffffff
#define OSAL_IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-OSAL_MAX_ERRNO)
static inline long OSAL_IS_ERR(const void *ptr)
{
	return OSAL_IS_ERR_VALUE((unsigned long)ptr);
}


/* 内存管理 */
#define osal_kmalloc(size, flags) SRE_MemAlloc(0,OS_MEM_DEFAULT_SHARED_PT,size)
#define osal_kfree(ptr) \
	{										  \
		UINT32 ulRetTmp = SRE_MemFree(0, ptr); \
		if (ulRetTmp)						  \
		{									  \
			hrd_log_err(HRD_DEF_LOG_ID, "SRE_MemFree(0,%p) fail, ret = 0x%x\r\n", ptr, ulRetTmp, 0,0);  \
		}									  \
	}

static inline void * osal_kzalloc_f(size_t size)
{
	void * p = NULL;
	p = osal_kmalloc(size, 0);
	if(NULL != p)
	{
		memset(p,0,size);
	}
	return p;
}
/* 在SRE下为了不定义Linux相关的枚举，用宏将第二个参数屏蔽掉 */
#define osal_kzalloc(size, flags) osal_kzalloc_f(size)


#define osal_vmalloc(size) SRE_MemAlloc(0,OS_MEM_DEFAULT_SHARED_PT,size)
#define osal_vfree(ptr) \
	{										  \
		UINT32 ulRetTmp = SRE_MemFree(0, ptr); \
		if (ulRetTmp)						  \
		{									  \
			hrd_log_err(HRD_DEF_LOG_ID, "SRE_MemFree(0,%p) fail, ret = 0x%x\r\n", ptr, ulRetTmp, 0,0);  \
		}									  \
	}

#define osal_kmalloc_node(size, flags, node) \
	SRE_MemAlloc(0, OS_MEM_DEFAULT_SHARED_PT + node * OS_MEM_PT_NUM_PER_SKT, size)
#define osal_vmalloc_node(size, node) \
	SRE_MemAlloc(0, OS_MEM_DEFAULT_SHARED_PT + node * OS_MEM_PT_NUM_PER_SKT, size)

#else /* HRD_OS_SRE */

#error HRD_OS was not defined!

#endif



#endif
