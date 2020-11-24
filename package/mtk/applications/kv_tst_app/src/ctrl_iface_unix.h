/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ctrl_iface_unix.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __CTRL_IFACE_UNIX_H__
#define __CTRL_IFACE_UNIX_H__

#include <sys/un.h>
#include "list.h"

struct hotspot_ctrl_set_param {
	const char *param;
	void (*set_param)(char *value);
};

struct hotspot_ctrl_dst {
	struct dl_list list;
	struct sockaddr_un addr;
	socklen_t addrlen;
};

struct hotspot_ctrl_iface {
	int sock;
	struct dl_list hs_ctrl_dst_list;
};

struct hotspot_ctrl_iface *hotspot_ctrl_iface_init(void *data);
void hotspot_ctrl_iface_deinit(void *data);



#endif /* __CTRL_IFACE_UNIX_H__ */
