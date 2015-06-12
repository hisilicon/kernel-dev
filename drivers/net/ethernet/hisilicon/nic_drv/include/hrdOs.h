#ifndef _MV_HRD_OS_H_
#define _MV_HRD_OS_H_

#include "hrdCommon.h"
#include "osal_api.h"

static inline u32 __hrd_raw_readl(const volatile void *addr)
{
    u32 val = 0;
    asm volatile("ldr %w0, [%1]" : "=r" (val) : "r" (addr));
    return val;
}


static inline void __hrd_raw_writel(u32 val, volatile void *addr)
{
    asm volatile("str %w0, [%1]" : : "r" (val), "r" (addr));
}

/* for kernel space */
#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/mtd/nand.h>

#include <asm/system_misc.h>
#include <asm/switch_to.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/hardirq.h>
#include <asm/dma.h>
#include <asm/io.h>

#include <linux/random.h>

#define HRD_ASM              __asm__ __volatile__
#define _INIT               __init
#define OS_Printf          printk
#define OS_EarlyPrintf   hrd_early_printk
#define OS_Output          printk
#define OS_SPrintf         sprintf
#define OS_Malloc(_size_)  kmalloc(_size_, GFP_KERNEL)
#define OS_Free            kfree
#define OS_Kzalloc(_size_) kzalloc(_size_, GFP_KERNEL)
#define OS_Memcpy          memcpy
#define OS_Memset          memset
#define OS_Sleep(_mils_)   mdelay(_mils_)
#define OS_TaskLock()
#define OS_TaskUnlock()
#define strtol             simple_strtoul
#define OS_Delay(x)        mdelay(x)
#define OS_UDelay(x)       udelay(x)
#define OS_CopyFromOs        copy_from_user
#define OS_CopyToOs          copy_to_user
#define OS_Warning()       WARN_ON(1)
#define OS_GetTicks()      jiffies
#define OS_GetTicksFreq()  HZ
#define OS_READL            readl
#define OS_WRITEL           writel
#define OS_SYNC


#define OS_IoRemap    ioremap
#define OS_IoUnRemap    iounmap

#include "hrdTypes.h"
#include "hrdCommon.h"
#include "hrd_bsp_mod.h"
#include "iware_comm_kernel_api.h"
#include "hrd_base_api.h"

static inline UINT64 OS_VirtToPhys (void *pVirtAddr)
{
    return virt_to_phys(pVirtAddr);
}

static inline void* OS_PhysToVirt(UINT64 ullPhyAddr)
{
    return phys_to_virt(ullPhyAddr);
}



#define HRD_REG_WRITE(addr,data)        __hrd_raw_writel(data,addr)  //(*(volatile unsigned int *)(addr) = (data))
#define HRD_REG_READ(addr,data)     ({ unsigned int __v = __hrd_raw_readl(addr); data = __v; })      //((data) = *(volatile unsigned int *)(addr))
#define HRD_READ_WORD(addr)	({ unsigned int __v = __hrd_raw_readl(addr); __v; })
#define HRD_WRITE_WORD(value,addr)  __hrd_raw_writel(value,addr)     //(*(volatile unsigned int *)(addr) = (value))


/* CPU cache information */
#define CPU_I_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */
#define CPU_D_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */

#endif
