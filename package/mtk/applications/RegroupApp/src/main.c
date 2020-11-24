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
#include "debug.h"
#include "os.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>          
#include <sys/ioctl.h>    
#include <errno.h>        
#include <fcntl.h>
#include <stdbool.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include "eloop.h"

#include "man.h"

/*Global Definitions*/

int DebugLevel = DEBUG_ERROR;
unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};

char interface_5g[16] = {'\0'};
char interface_2g[16] = {'\0'};
char cli_interface_5g[16] = {'\0'};
char cli_interface_2g[16] = {'\0'};

unsigned char chipbit = 0;
#define MT7615D_2G_PROFILE "/etc/wireless/mt7615e/mt7615e.1.2G.dat"
#define MT7615D_5G_PROFILE "/etc/wireless/mt7615e/mt7615e.1.5G.dat"
#define MT7615N_FIRST_CARD_PROFILE "/etc/wireless/mt7615e/mt7615e.1.dat"
#define MT7615N_SECOND_CARD_PROFILE "/etc/wireless/mt7615e/mt7615e.2.dat"
#define MT7603_PROFILE "/etc/wireless/mt7603e/mt7603e.dat"
#define MT7612_PROFILE "/etc/wireless/mt76x2e/mt7612e.dat"
#define MT7628_PROFILE "/etc/wireless/mt7628/mt7628.dat"

#define CHIP_7615D		1
#define CHIP_FIRST_7615N	2
#define CHIP_SECOND_7615N	4
#define CHIP_7603		8
#define CHIP_7612		16
#define CHIP_7628		32


#define BAND1			1
#define BAND2			2
#define BAND3			3
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif


/**********************************************************/

/*Extern*/

/**********************************************************/

int eloop_register_timeout(unsigned int secs, unsigned int usecs,
			   eloop_timeout_handler handler,
			   void *eloop_data, void *user_data);
void vr_periodic_exec();

int usage()
{

	DBGPRINT(DEBUG_OFF, "sampleApp  [-d <debug level>]\n");
	DBGPRINT(DEBUG_OFF, "-d <sampleApp debug level>\n");
	DBGPRINT(DEBUG_OFF, "-a <SampleApplication>\n");
	DBGPRINT(DEBUG_OFF, "-h help\n");
	return 0;
}



/*
    ========================================================================
    Routine Description:
        To get chip combo

    Arguments:
        NULL

    Return Value:
       NULL

    ========================================================================
*/

void get_chip_combo()
{
	if (!access(MT7615N_SECOND_CARD_PROFILE, 0)) {
		chipbit |= CHIP_SECOND_7615N;
		printf("CHIP_SECOND_7615N\n");
	}
	if (!access(MT7615N_FIRST_CARD_PROFILE, 0)) {
		chipbit |= CHIP_FIRST_7615N;
		printf("CHIP_FIRST_7615N\n");
	}
	if (!access(MT7615D_2G_PROFILE, 0)) {
		chipbit |= CHIP_7615D;
		printf("CHIP_7615D\n");
	}
	if (!access(MT7615D_5G_PROFILE, 0)) {
		chipbit |= CHIP_7615D;
		printf("CHIP_7615D\n");
	}
	if (!access(MT7603_PROFILE, 0)) {
		chipbit |= CHIP_7603;
		printf("CHIP_7603\n");
	}
	if (!access(MT7612_PROFILE, 0)) {
		chipbit |= CHIP_7612;
		printf("CHIP_7612\n");
	}
	if (!access(MT7628_PROFILE, 0)) {
		chipbit |= CHIP_7628;
		printf("CHIP_7628\n");
	}
	return;
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
			if ((actual_value > 0) && (actual_value < 5))
			{
				*app = actual_value;
			} else {
				DBGPRINT(DEBUG_OFF, "Expecting App index in range 1-4\n");
			}
			
		}
	}
	return 0;

}

void ez_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
//#ifdef DBG
		unsigned char *pt;
		int x;
		pt = pSrcBufVA;
		printf("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
		for (x = 0; x < SrcBufLen; x++) {
			if (x % 16 == 0)
				printf("0x%04x : ", x);
			printf("%02x ", (unsigned char)pt[x]);
			if (x % 16 == 15)
				printf("\n");
		}
		printf("\n");
//#endif /* DBG */
}



int get_rai0_mac (char *buf)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig rai0 | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac_rai");
	system(command);
	fp = fopen("temp_file_mac_rai", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	//printf("%s\n", peer_mac);
	
	if (peer_mac[0] != 'N') 
	{
		for (i = 0; i < 17 ; i++)
		{
			if (i % 3 == 0)
			{
				buf[i/3] = (peer_mac[i] < '9') ? ((peer_mac[i] - '0') << 4) : ((peer_mac[i] - 'A' + 10) << 4) ;
			}
			else if (i % 3 == 1)
			{
				buf[i/3] |= (peer_mac[i] < '9') ? ((peer_mac[i] - '0')) : ((peer_mac[i] - 'A' + 10));
			} else {
				continue;
			}
			//printf("%d:", buf[i / 3]);	
		}
	} else {
		return 0;
	}
	//printf("\n");
#endif
	return 1;
}

int  get_mac(char *buf, char *intf)
{
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, intf);
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
    for (i = 0; i < 6; ++i)
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    puts("\n");
  }

  memcpy(buf, s.ifr_hwaddr.sa_data, 6);
  return 0;

}


int get_ifip(char *ifname, char *if_addr)
{		
	struct ifreq ifr;
	int skfd = 0;
	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{	printf("%s: open socket error\n", __FUNCTION__);
		return -1;
	}		
	strncpy(ifr.ifr_name, ifname, 32);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) 
	{
		close(skfd);
		return -1;
	}		
	memcpy(if_addr, &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 4);
	*(int *)if_addr = ntohl(*(int *)if_addr);
	//printf("if_addr = %d.%d.%d.%d", if_addr[0], if_addr[1], if_addr[2], if_addr[3]);
	close(skfd);		
	return 0;
}

#if 0
static void vr_fill_ssid_info()
{
	FILE *fp;
	char ssid[65];
	const char s[2] = ";";
   	char *token;
	char command[100];
	int i=0;

	if(is_triband)	
	{
		memset(system_ssid1, 0, 32);
		memset(system_ssid2, 0, 32);
		memset(system_ssid3, 0, 32);

		system("nvram_get rtdev EzDefaultSsid | awk -F \";\" \'\{print $1}\' > temp_file1");
		fp = fopen("temp_file1", "r");
		fgets(system_ssid1, 32, fp);

		if (strlen(system_ssid1) == 1)
		{
			system("nvram_get rtdev SSID1 | awk -F \";\" \'\{print $1}\' > temp_file1");
			fp = fopen("temp_file1", "r");
			fgets(system_ssid1, 32, fp);			
		}
		
		system("nvram_get 2860 SSID1 | awk -F \";\" \'\{print $1}\' > temp_file2");
		fp = fopen("temp_file2", "r");
		fgets(system_ssid2, 32, fp);


		system("nvram_get wifi3 SSID1 | awk -F \";\" \'\{print $1}\' > temp_file3");
		fp = fopen("temp_file3", "r");
		fgets(system_ssid3, 32, fp);

		system_ssid1[strlen(system_ssid1) - 1] = 0;

		get_connected_ap_mac(apclii0_peer_mac);
		get_rai0_mac(rai0_mac);
	}	
	else
	{

		memset(command, 0, 100);
		memset(sys_ssid, 0, 65);
		memset(sys_ssid1, 0, 32);
		memset(sys_ssid2, 0, 32);
		memset(sys_ssid3, 0, 32);
		memset(ssid, 0, 65);	

		sprintf(command, "nvram_get 2860 EzDefaultSsid > temp_file");
		system(command);
		fp = fopen("temp_file", "r");
		fgets(ssid, 65, fp);
		ssid[strlen(ssid)-1] = '\0';

		token = strtok(ssid, s);
		while( token != NULL ) 
		{
			if(i==0)
			{
				memcpy(sys_ssid1, token, strlen(token));
				i++;
			}
			else if(i==1)
			{
				memcpy(sys_ssid2, token, strlen(token));
				i++;
			}
			token = strtok(NULL, s);
		}

		memset(command, 0, 100);
		memset(ssid, 0, 65);
		
		sprintf(command, "nvram_get rtdev EzDefaultSsid > temp_file");
		system(command);
		fp = fopen("temp_file", "r");
		fgets(ssid, 65, fp);
		ssid[strlen(ssid)] = '\0';
		
		token = strtok(ssid, s);
		while( token != NULL ) 
		{
			memcpy(sys_ssid3, token, strlen(token));
			token = strtok(NULL, s);
			break;
		}

		if(strlen(sys_ssid3) - 1)
		{
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1)-1);
			memcpy(sys_ssid + strlen(sys_ssid1) -1, "_", 1);			
			memcpy(sys_ssid + strlen(sys_ssid1), sys_ssid3, strlen(sys_ssid3));
			
		}
		else
		{
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
			memcpy(sys_ssid + strlen(sys_ssid1), "_", 1);			
			memcpy(sys_ssid + strlen(sys_ssid1) + 1, sys_ssid2, strlen(sys_ssid2));
		
		}
			
		get_ra0_mac(ra0_mac);
	}


}
#endif
static void regrp_terminate(int sig, void *signal_ctx)
{
	//DBGPRINT(DEBUG_TRACE, "\n");
	printf("%s\n", __func__);
	eloop_terminate();
}

void vr_init()
{
	
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* Initialze event loop */
	ret = eloop_init();
	driver_wext_init();
	
	if (ret)
	{	
		DBGPRINT(DEBUG_OFF, "eloop_register_timeout failed.\n");
		return;
	}
	
	vr_register_server();
	vr_init_globals();
	

	if (ret == 0){

	ret = eloop_register_timeout(5, 0, vr_periodic_exec, NULL, NULL);
	}
	eloop_register_signal_terminate(regrp_terminate,NULL);

	/*infinite loop*/
	eloop_run();
	printf("Exit the program\n");
	return;
}

int main(int argc, char *argv[])
{

	int ret=0;
	int i;
	pid_t child_pid;

        get_chip_combo();

	printf("No of args:%d\n", argc);

	for (i = 0; i < argc; i++)
	{
		printf("arg %d:%s\n", i, argv[i]);
	}

	if ((argc != 4) && (argc != 7))
	{
		printf("Err: usage : vr 2 ra0 apcli0 5 rai0 apclii0\n");
		return;
	}

	if (*argv[1] == '2')
	{
		strncpy(interface_2g, argv[2], 16);		
		strncpy(cli_interface_2g, argv[3], 16);		
	} else if (*argv[1] == '5'){
		strncpy(interface_5g, argv[2], 16);		
		strncpy(cli_interface_5g, argv[3], 16);		
	}

	if (argc == 7)
	{
		if (*argv[4] == '2')
		{
			strncpy(interface_2g, argv[5], 16); 
			strncpy(cli_interface_2g, argv[6], 16); 	
		} else if (*argv[4] == '5'){
			strncpy(interface_5g, argv[5], 16); 				
			strncpy(cli_interface_5g, argv[6], 16); 	
		}

	}
	printf("5G interface name:%s   2.4 interface name:%s\n", interface_5g, interface_2g);	
	printf("5G cli interface name:%s   2.4 cli interface name:%s\n", cli_interface_5g, cli_interface_2g);
	
	child_pid = fork();
	if (child_pid == 0) {	
		 vr_init();	
	} else
		return 0;	
	
	return ret;

}
