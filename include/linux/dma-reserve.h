#ifndef __LINUX_RESERVE_H
#define __LINUX_RESERVE_H

#ifdef CONFIG_DMA_RESERVE

#include <linux/device.h>

void __init dma_memory_reserve(void);
int dma_memory_declear(struct device *dev);
void dma_memory_release(struct device *dev, int id);

#else

static inline void dma_memory_reserve(void) { };
static inline int dma_memory_declear(struct device *dev) { };
static inline void dma_memory_release(struct device *dev, int id) { };

#endif

#endif
