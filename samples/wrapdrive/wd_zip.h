/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __WD_HZIP_H
#define __WD_HZIP_H

#include <stdlib.h>
#include <errno.h>

struct wd_zip_msg {
	char *alg;
	__u16 sq_num;

	/* address type */
	__u32 aflags;
	__u32 size;
	__u64 src;
	__u64 dst;
};

#endif
