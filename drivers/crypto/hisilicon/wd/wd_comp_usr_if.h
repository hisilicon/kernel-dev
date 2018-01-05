/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


/* This file is shared bewteen WD user and kernel space, which is
 * including attibutions of user caring for
 */

#ifndef __WD_COMP_USR_IF_H
#define __WD_COMP_USR_IF_H


/* compressing algorithms' parameters */
struct wd_comp_param {
	__u32 window_size;
	__u32 comp_level;
	__u32 mode;
	__u32 alg;
};


/* WD defines all the algorithm names here */
#define zlib			"zlib"
#define gzip			"gzip"


#endif
