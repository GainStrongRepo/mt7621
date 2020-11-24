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
    	debug.h
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define NIC_DBG_STRING      ("")

#define DEBUG_OFF		0
#define DEBUG_ERROR	1
#define DEBUG_WARN		2
#define DEBUG_TRACE	3
#define DEBUG_INFO		4
#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define RT_DEBUG_INFO       4


extern int RTDebugLevel;

#define DBGPRINT(Level, fmt, args...)   \
{                                       \
	if (Level <= RTDebugLevel)          \
	{                                   \
		printf("[%s]", __FUNCTION__);	\
		printf( fmt, ## args);          \
	}                                   \
}

#define DBGPRINT_RAW(Level, fmt, args...)   \
{       	                                \
    if (Level <= RTDebugLevel)          	\
    {                                   	\
        printf( fmt, ## args);          	\
    }                                   	\
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

#endif /* __DEBUG_H__ */
