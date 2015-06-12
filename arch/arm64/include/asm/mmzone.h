#ifndef _ASM_ARM64_MMZONE_H
#define _ASM_ARM64_MMZONE_H

#include <linux/numa.h>

extern pg_data_t pgdat_list[MAX_NUMNODES];
#define NODE_DATA(nid)		(&pgdat_list[nid])

#endif /* _ASM_ARM64_MMZONE_H */
