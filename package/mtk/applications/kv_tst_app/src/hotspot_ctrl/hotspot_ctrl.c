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
	hotspot_ctrl.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot_ctrl.h"

struct hotspot_ctrl *hotsot_ctrl_open(const char *ctrl_path)
{
	struct hotspot_ctrl *ctrl;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ctrl = os_zalloc(sizeof(*ctrl));

	if (!ctrl) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return NULL;
	}

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);

	if (ctrl->s < 0) {
		os_free(ctrl);
		DBGPRINT(RT_DEBUG_ERROR, "create socket for ctrl interface fail\n");
		return NULL;
	}

	ctrl->local.sun_family = AF_UNIX;
	os_snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path), "/tmp/kvctrl_%d",
					getpid());

	if (bind(ctrl->s, (struct sockaddr *)&ctrl->local, sizeof(ctrl->local)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Bind local domain address fail\n");
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
	}

	ctrl->dest.sun_family = AF_UNIX;
	os_strncpy(ctrl->dest.sun_path, ctrl_path, sizeof(ctrl->dest.sun_path));
	DBGPRINT(RT_DEBUG_ERROR, "\n\nctrl_path: %s\n",ctrl_path);

	if (connect(ctrl->s, (struct sockaddr *)&ctrl->dest, sizeof(ctrl->dest)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Connect to server address fail\n");	
		unlink(ctrl->local.sun_path);
		close(ctrl->s);
		os_free(ctrl);
		return NULL;
	}
	
	return ctrl;
}

void hotspot_ctrl_close(struct hotspot_ctrl *ctrl)
{
	unlink(ctrl->local.sun_path);
	close(ctrl->s);
	os_free(ctrl);
}

int hotspot_ctrl_command(struct hotspot_ctrl *ctrl, const char *cmd, size_t cmd_len,
							char *reply, size_t *reply_len)
{
	struct timeval tv;
	int ret;
	fd_set rfds;

	if (send(ctrl->s, cmd, cmd_len, 0) < 0) {
		printf("send command to ctrol socket fail\n");
		return -1;
	}

	for(;;) {
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(ctrl->s, &rfds);
		ret = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);

		if (FD_ISSET(ctrl->s, &rfds)) {
			ret = recv(ctrl->s, reply, *reply_len, 0);
				if (ret < 0)
					return ret;
		
			*reply_len = ret;
			break;

		} else
			return -1;
	}

	return 0;
}

int hotspot_ctrl_event_regiser(struct hotspot_ctrl *ctrl)
{
	char buf[10];
	int ret;
	size_t len = 10;

	os_memset(buf, 0, 10);

	ret = hotspot_ctrl_command(ctrl, "EVENT_REGISTER", 14, buf, &len);

	if (ret < 0)
		return ret;

	if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
		return 0;

	return -1;
}

int hotspot_ctrl_event_unregister(struct hotspot_ctrl *ctrl)
{
	char buf[10];
	int ret;
	size_t len = 10;

	os_memset(buf, 0, 10);

	ret = hotspot_ctrl_command(ctrl, "EVENT_UNREGISTER", 16, buf, &len);

	if (ret < 0)
		return ret;

	if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
		return 0;

	return -1; 
}
