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

#ifndef __HISI_SEC_USR_IF_H
#define __HISI_SEC_USR_IF_H

#include <linux/types.h>
#include "../wd/wd_usr_if.h"
#include "../wd/wd_cipher_usr_if.h"

#define WD_PKT_CPY_THRESHOLD 1984


#define SECALG_AT_CY_SYM	"at_cy_sym"
#define SECALG_AT_CY_AUTH	"at_cy_auth"
#define	SECALG_AT_CY_SYM_AUTH	"at_cy_sym_auth"
#define SECALG_AT_CY_AUTH_SYM	"at_cy_auth_sym"

#define HISI_SEC_V1_API		"hisi_sec_v1"


#endif
