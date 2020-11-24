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

#include "bndstrg.h"
#include "wnm.h"
#include "internetchk.h"
#include "froam.h"

extern struct bndstrg_event_ops bndstrg_event_ops;
extern struct wnm_event_ops wnm_event_ops;
extern struct _froam_event_ops froam_event_ops;

// App Start Index
#define BND_STEERING 		0x1
#define WNM_802_11_V		0x2
#define RRM_802_11_K		0x3
#define INTERNET_CHK		0x4
#define REGROUP				0x5
#define FORCEROAM			0x6
#define MAX_APP_IDX			FORCEROAM

int usage()
{

	DBGPRINT(DEBUG_OFF, "sampleApp  [-d <debug level>]\n");
	DBGPRINT(DEBUG_OFF, "-d <sampleApp debug level>\n");
	DBGPRINT(DEBUG_OFF, "-a <SampleApplication>\n");
	DBGPRINT(DEBUG_OFF, "-h help\n");
	return 0;
}
int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level, int *version, char *app)
{
	int c;
	char *cvalue = NULL;
	char actual_value;
	
	opterr = 0;

	while ((c = getopt(argc, argv, "d:v:a:")) != -1) {
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
				return - 1;
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
			return -1;
			break;
		case 'a':
			cvalue = optarg;
			actual_value = atoi(cvalue);
			if ((actual_value > 0) && (actual_value <= MAX_APP_IDX))
			{
				*app = actual_value;
			} else {
				DBGPRINT(DEBUG_OFF, "Expecting App index in range 1-4\n");
			}
			
		}
	}
	return 0;

}

int app;
	

int main(int argc, char *argv[])
{

	int ret;
	int opmode;
	int drv_mode;
	int debug_level;
	int version = 2;
	char filename[256] = {0}; 
	struct bndstrg bndstrg;
	struct wnm wnm;
	froam_ctx froam;
	pid_t child_pid;

	/* default setting */

	/* options processing */
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level, &version, (char *)&app);

	if (ret) {
		usage();
		return -1;
	}

	DebugLevel = debug_level;
	
	switch (app)
	{
		case BND_STEERING:
		{
			child_pid = fork();
			if (child_pid == 0) {	
				DBGPRINT(DEBUG_OFF, "Initialize bndstrg\n");
				ret = bndstrg_init(&bndstrg, &bndstrg_event_ops, drv_mode, opmode, version);
			
				if (ret)
					goto error;

				bndstrg_run(&bndstrg);

			} else
				return 0;
			break;
		}
		case WNM_802_11_V:
		case RRM_802_11_K:
			if (app == WNM_802_11_V) {
				DBGPRINT(DEBUG_OFF, "running 802_11_V_SampleApp\n");
			} else {
				DBGPRINT(DEBUG_OFF, "running 802_11_K_SampleApp\n");
			}
			{
				ret = wnm_init(&wnm, &wnm_event_ops, drv_mode, opmode, version);
			
				if (ret)
					goto error;

				wnm_run(&wnm);


			}
			break;
			
		case INTERNET_CHK:
		{
			child_pid = fork();
			if (child_pid == 0) {	
				DBGPRINT(DEBUG_OFF, "running INTERNET CHK SampleApp\n");
				internet_status_process();
			}			
			else
				return 0;
		}
			break;

		case FORCEROAM:
		{
			child_pid = fork();	// run in background
			if (child_pid == 0) {	

				DBGPRINT(DEBUG_OFF, "Triggered Force Roam SampleApp\n");

				ret = froam_init(&froam, &froam_event_ops, drv_mode);

				DBGPRINT(DEBUG_OFF, "Force Roam Init done\n");

				if (ret)
					goto error;

				froam_run(&froam);
			}			
			else
				return 0;
		}
			break;			
	}
#if 0
error0:
	bndstrg_deinit(&hs);
#endif
error:
	
	return ret;

}
