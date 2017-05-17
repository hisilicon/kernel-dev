/*
 * Copyright (c) 2014-2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _HNS_XGMAC_H
#define _HNS_XGMAC_H

#define HNS_XGMAC_DUMP_NUM		214
#define HNS_XGMAC_NO_LF_RF_INSERT	0x0
#define HNS_XGMAC_LF_INSERT		0x2

#define XGE_INT_EN_MASK ((1 << XGE_MIB_ECCERR_MUL_INT) |\
			 (1 << XGE_FEC_ECCERR_MUL_INT))

#define HNS_DSAF_XGMAC_MAX_ERR_SRC	2

enum err_src_hns_dsaf_xgmac {
	ERR_SRC_XGMAC_UNKNOWN = (0x0 << 0),
	ERR_SRC_XGMAC_XGE_MIB_ECCERR_MUL_INT = (0x1 << 0),
	ERR_SRC_XGMAC_XGE_FEC_ECCERR_MUL_INT = (0x1 << 1),
};

#endif
