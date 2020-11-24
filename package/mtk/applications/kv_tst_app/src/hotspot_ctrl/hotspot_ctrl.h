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
	hotspot_ctrl.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __HOTSPOT_CTRL_H__
#define __HOTSPOT_CTRL_H__

#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../debug.h"
#include "../os.h"
#include "../driver_wext.h"

struct hotspot_ctrl {
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

struct hotspot_ctrl *hotsot_ctrl_open(const char *path);

void hotspot_ctrl_close(struct hotspot_ctrl *ctrl);

int hotspot_ctrl_command(struct hotspot_ctrl *ctrl, const char *cmd, size_t cmd_len,
							char *reply, size_t *reply_len);

int hotspot_ctrl_event_regiser(struct hotspot_ctrl *ctrl);

int hotspot_ctrl_event_unregister(struct hotspot_ctrl *ctrl);


#endif /* __HOTSPOT_CTRL_H__ */
