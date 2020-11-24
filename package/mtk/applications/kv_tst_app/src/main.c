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
    	main.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "rrm_wnm.h"

extern struct rrm_wnm_event_ops rrm_wnm_event_ops;

int usage()
{

	DBGPRINT(DEBUG_OFF, "kv  [-d <debug level>]\n");
	DBGPRINT(DEBUG_OFF, "-d <sampleApp debug level>\n");
	DBGPRINT(DEBUG_OFF, "-h help\n");
	return 0;
}


int process_options(int argc, char *argv[], char *filename,
					u8 *opmode, u8 *drv_mode, u8 *debug_level, u32 *version)
{
	int c;
	int ret = 0;
	unsigned char *cvalue = NULL;
	
	opterr = 0;

	while ((c = getopt(argc, argv, "d:v:a:p:b:c:u:i:s:m:t:D")) != -1) {
		switch (c) {
		case 'd':
			cvalue = optarg;
			if (os_strcmp(cvalue, "0") == 0)
				*debug_level = DEBUG_OFF;
			else if (os_strcmp(cvalue, "1") == 0)
				*debug_level = DEBUG_ERROR;
			else if (os_strcmp(cvalue, "2") == 0)
				*debug_level = DEBUG_WARN;
			else if (os_strcmp(cvalue, "3") == 0)
				*debug_level = DEBUG_TRACE;
			else if (os_strcmp(cvalue, "4") == 0)
				*debug_level = DEBUG_INFO;
			else {
				DBGPRINT(DEBUG_ERROR, "-d option does not have this debug_level %s\n", cvalue);
				ret = -1;
				goto out;
			}
			break;
		case 'f':
			cvalue = optarg;
			os_strcpy(filename, cvalue);
			break;
		case 'v':
			cvalue = optarg;
			*version = atoi(cvalue);
 			break;
		case 'h':
			cvalue = optarg;
			usage();
			break;
		case '?':
			if (optopt == 'f') {
				DBGPRINT(DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'd') {
				DBGPRINT(DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'd') {
				DBGPRINT(DEBUG_OFF, "Option -%c requires an argument\n1: BandSteering \n2:11V\n3:11K", optopt);
			} else if (isprint(optopt)) {
				DBGPRINT(DEBUG_OFF, "Unknow options -%c\n", optopt);
			} else {

			}
			ret = -1;
			goto out;
			break;
		}
	}
out:
	return ret;
}

int main(int argc, char *argv[])
{

	int ret = 0;
	u8 opmode;
	u8 drv_mode;
	u8 debug_level;
	u8 version = 2;
	char filename[256] = {0}; 
	struct rrm_wnm rrm_wnm;
	pid_t child_pid;

	/* default setting */

	/* options processing */
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version);

	if (ret) {
		usage();
		return -1;
	}

	RTDebugLevel = debug_level;
	child_pid = fork();
	if (child_pid == 0) {	
		DBGPRINT(DEBUG_OFF, "running 802_11_KV_SampleApp\n");
		ret = rrm_wnm_init(&rrm_wnm, &rrm_wnm_event_ops, drv_mode, opmode, version);
		if (ret)
			goto error;
		rrm_wnm_run(&rrm_wnm);
	} else {
		DBGPRINT(DEBUG_ERROR, "child_pid != 0\n");
		return 0;
	}


error0:
	rrm_wnm_deinit(&rrm_wnm);
error:
	return ret;

}
