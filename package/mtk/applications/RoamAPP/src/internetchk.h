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
    	internetchk.h
*/
#include "debug.h"

#ifndef __INTERNET_CHK__
#define __INTERNET_CHK__

#define UP                      		1
#define Down                    		0
#define CONNECTED						1
#define DISCONNECTED					0
#define SUCCESS							1
#define FAILED							0
#define OID_GET_SET_TOGGLE      		0x8000
#define SIOCIWFIRSTPRIV         		0x8BE0
#define RT_PRIV_IOCTL           		(SIOCIWFIRSTPRIV + 0x01)
#define OID_802_11_INTERNET_COMMAND		0x2005
#define IFNAME_2G               		"ra0"
#define IFNAME_5G               		"rai0"
#define SERVER1							"8.8.8.8"
#define SERVER2							"208.67.222.222"	
//#define ERROR(fmt, ...) 				do { printf(fmt, __VA_ARGS__); return -1; } while(0)

typedef unsigned char BOOLEAN;

#define PACKED __attribute__((packed))
typedef struct PACKED internet_command_s
{
	BOOLEAN Net_status;
	
} internet_command_t, *p_internet_command_t;

void internet_status_process();

#endif /* __INTERNET_CHK__ */


