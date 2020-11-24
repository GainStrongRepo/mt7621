
/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    	driver_wext.h
*/

#ifndef __DRIVER_WEXT_H__
#define __DRIVER_WEXT_H__

#include "types.h"

struct driver_wext_data {
	int opmode;
	char drv_mode;
	void *priv;
#if 1
	struct netlink_data *netlink;
#endif
	int ioctl_sock;
	int we_version_compiled;
};

#endif /* __DRIVER_WEXT_H__ */
