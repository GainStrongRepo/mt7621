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
    	event.h
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include "rrm_wnm.h"

struct rrm_wnm_event_ops {
	int (*rrm_event_handle)(struct rrm_wnm *rrm_wnm, char *data);
	int (*wnm_event_handle)(struct rrm_wnm *rrm_wnm, char *data);
};

#endif /* __EVENT_H__ */
