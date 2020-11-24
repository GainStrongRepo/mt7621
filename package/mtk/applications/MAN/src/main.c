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
#include "main.h"
#include "regrp.h"
unsigned char is_triband = 0;
unsigned char is_man_nonman = 0;
int cmd = 0;
int DebugLevel = DEBUG_ERROR;
char NonEzBands = 0;
char EzBands = 0;
char is_multipro = 0;
char dedicated_regrp_app = 0;
unsigned char send_sta_update_count = 2;
char is_dual_chip_dbdc = 0;
#ifdef SINGLE_BACKHAUL_LINK
unsigned char roam_triggered_on_2_g = 0;
unsigned char roam_triggered_on_5_g = 0;
#endif
#ifdef MOBILE_APP_SUPPORT
char No_of_Band[1];	
#endif
int app;
int s;
int NetStatus;
char system_ssid1[33];
char system_ssid2[33];
char system_ssid3[33];
char system_wpapsk[65];
char system_authmode[32];
char system_encryptype[32];
unsigned char system_ftmdid[3];
char rai0_mac[6];
char apclii0_peer_mac[6];
char is_non_ez_connection = 0;
char ra0_mac[6];
char sys_ssid[65];
char sys_pmk[65];
char sys_pmk_string[129];
char sys_ssid1[33];
char sys_ssid2[33];
char sys_ssid3[33];
char sys_psk1[33];
char sys_psk2[33];
char sys_pmk1[32];
char sys_pmk2[32];
char sys_pmk3[32];
char sys_pmk_string1[65];
char sys_pmk_string2[65];
unsigned char device_connected_0;
unsigned char device_connected_1;	
unsigned char non_ez_connection;
char Peer2p4mac[MAC_ADDR_LEN];
unsigned int try_roaming_count = 5;
unsigned int try_5G_roam_count;
unsigned int try_roam_for;
unsigned char extra_attempt_to_5g = 1;
unsigned char is_5G_connected;
unsigned char is_2G_connected;
unsigned char channel_2860, channel_rtdev, channel_wifi3;
char zone_2G[5], zone_5G[5];
unsigned char is_third_party_present;
unsigned char is_roam_ongoing;
device_info_to_app_t device_info_glbl;
station_list_t station_list[MAX_STA_SUPPORT];
unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};
unsigned char roaming_scheduled;
#ifdef IPHONE_SUPPORT
char differ_dhcp_handling = 0;
char last_stable_weight[NETWORK_WEIGHT_LEN];
EZ_NODE_NUMBER last_stable_node_number;
#ifdef REGROUP_SUPPORT
char regrp_disconnect_sta = TRUE;
char regrp_dhcp_timer_running;
char regrp_dhcp_defer;
char regrp_current_wt[NETWORK_WEIGHT_LEN];
char regrp_expected_wt_1[MAC_ADDR_LEN];
char regrp_expected_wt_2[MAC_ADDR_LEN];

signed char g_default_rssi_threshold = -73;
signed char g_default_max_rssi_threshold = -86;
signed char g_custom_rssi_th =-68;

#endif
char expected_weight[NETWORK_WEIGHT_LEN];
unsigned char server_killed_by_differ_dhcp = 0;
unsigned char is_weight_not_change = 0;
#endif
UINT32 regrp_periodic_time;

device_info_from_user_t input_device_info[MAX_BAND] = {0};
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

#define MAX_IP_STR_LEN      16


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


/*
    ========================================================================
    Routine Description:
        To set ManConf default value to corresponding profile

    Arguments:
        NULL

    Return Value:
       NULL

    ========================================================================
*/
void set_ManConf_default2dat()
{
	char cmd[1024];

        /*
        Single Chip with DBDC 
	        nvram_set 2860 ManConf "2 2Ez rax0 apclix0 5Ez ra0 apcli0" 
        Dual Chip 
                nvram_set 2860 ManConf "2 2Ez ra0 apcli0 5Ez rai0 apclii0"
        Triband 
                nvram_set 2860 ManConf "3 5Ez rai0 apclii0 2NEz rax0 apclix0 5NEz ra0 apcli0"
        */

        switch_profile("2860");
        
	if ((chipbit & CHIP_7615D) && (chipbit & CHIP_SECOND_7615N)) {
        	sprintf(cmd, "wificonf set ManConf \"3 5Ez rai0 apclii0 2NEz rax0 apclix0 5NEz ra0 apcli0\"");
        	system(cmd);		
	}
	else if (chipbit & CHIP_7615D) {
        	sprintf(cmd, "wificonf set ManConf \"2 2Ez rax0 apclix0 5Ez ra0 apcli0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_FIRST_7615N) && (chipbit & CHIP_SECOND_7615N)) {
        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_SECOND_7615N)) {
        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_7612)) {
        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7628) && (chipbit & CHIP_7612)) {
        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	printf("%s %d cmd: %s\n", __FUNCTION__,__LINE__,cmd);
	return;
}



/*
input band-
	for tri-band
		2860:5g LB
		wifi3:2G
		rtdev:5G-HB
	for 7615D
		2860:5G
		rtdev:2G
	for 7603 + 7615
		2860:2G
		rtdev:5G
	for 7603 + 7612
		2860:2G
		rtdev:5G
*/
void switch_profile(char *nvram_type)
{
	char cmd[1024];

	memset(cmd, 0, sizeof(cmd));
	if ((chipbit & CHIP_7615D) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
			system(cmd);
		}
		else if (!memcmp(nvram_type, "wifi3", min(strlen("wifi3"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_2G_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
	}
	else if (chipbit & CHIP_7615D) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615D_2G_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_FIRST_7615N) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615N_FIRST_CARD_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_7612)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7612_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_7628) && (chipbit & CHIP_7612)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7628_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7612_PROFILE);
			system(cmd);
		}
	}

        
        DBGPRINT(DEBUG_OFF, "\n %s %d system call is \n%s\n",__FUNCTION__,__LINE__, cmd);
	return;
}


/*
    ========================================================================
    Routine Description:
        To get zone information corresponding to the interface

    Arguments:
        buf         - contains interface string.

    Return Value:
       const char *

    ========================================================================
*/

const char *get_nvram_zone(char *buf)
{
	if(!strcmp(buf,"ra0") || !strcmp(buf,"ra1") || !strcmp(buf,"apcli0") || !strcmp(buf,"apcli1"))
	{
		return "2860";
	}
	else if(!strcmp(buf,"rai0") || !strcmp(buf,"apclii0"))
	{
		return "rtdev";
	}
	else if(!strcmp(buf,"rax0") || !strcmp(buf,"apclix0"))
	{
		return "wifi3";
	}
	else
		return NULL;
}
/*
    ========================================================================
    Routine Description:
        To get the first Ez index in case of MAN NONMAN and Triband

    Arguments:
        void

    Return Value:
       int

    ========================================================================
*/

int get_ez_idx()
{
	int i;
	for(i = 0; i <(No_of_Band[0]-'0'); i++)

	{
		if(input_device_info[i].Ez_status == '1')
			return i;
	}
}


/*
    ========================================================================
    Routine Description:
        To get bridge interface ip address

    Arguments:
        buf for saving IP address
        
    Return Value:
        void

    ========================================================================
*/
void get_br_ip (char *buf)
{
	FILE *fp;
	int i =0;
	char command [100] = {0};
	sprintf(command, "uci -q get network.lan.ipaddr > temp_file_br_ip");
	system(command);
	fp = fopen("temp_file_br_ip", "r");
	fgets(buf, MAX_IP_STR_LEN, fp);
	fclose(fp);

    DBGPRINT(DEBUG_OFF, "the ip of br interface: %s \n",buf);
    return ;
}



/*
    ========================================================================
    Routine Description:
        Helps user to know about the format

    Arguments:
        void

    Return Value:
       int

    ========================================================================
*/


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
	To parse the user input and store it 

    Arguments:
      Command Line Argument count, Arguments, Filename, mode, driver mode, debug level, version, app

    Return Value:
    	int

    ========================================================================
*/

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

/*
    ========================================================================
    Routine Description:
	To update the nvram with zones and ssid

    Arguments:
        systems ssid

    Return Value:
       void

    ========================================================================
*/


void update_nvram(char *ssid1, char *ssid2, char *ssid3)
{
	char command[100];

	DBGPRINT(DEBUG_OFF, "\n cmd = %d, ssid1 %s, ssid2 %s, ssid3 %s", cmd, ssid1, ssid2, ssid3);
	switch(cmd)
	{
		case SINGLE_BAND :
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf set SSID1 \"%s\"",ssid1);
			system(command);
			memset(command, 0, 100);
			sprintf(command, "wificonf set WPAPSK1 \"\"");
			system(command);
			break;
			
		case DUAL_BAND_MAN_NONMAN :
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[get_ez_idx()].ap_interface));
			sprintf(command, "wificonf set SSID1 \"%s\"", ssid1);
			system(command);
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[get_ez_idx()].ap_interface));
			sprintf(command, "wificonf set WPAPSK1 \"\"");
			system(command);
			break;
		case DUAL_BAND:
                        printf("%s %d input_device_info[0].ap_interface %s,input_device_info[0].ap_interface %s\n",
                                __FUNCTION__,__LINE__,input_device_info[0].ap_interface,input_device_info[1].ap_interface);
			if(!strcmp(get_nvram_zone(input_device_info[0].ap_interface), get_nvram_zone(input_device_info[1].ap_interface))) { // Arvind use MACRO for strcmp 
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set SSID1 \"%s\"", ssid1);
				system(command);
				memset(command, 0, 100);
				sprintf(command, "wificonf set SSID2 \"%s\"", ssid2);
				system(command);
				memset(command,0,100);
				sprintf(command, "wificonf set WPAPSK1 \"%s\"");
				system(command);
			} 
			else {
        			memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
                                
                                printf("%s %d input_device_info[0].ap_interface %s, get_nvram_zone(input_device_info[0].ap_interface) %s\n",__FUNCTION__,__LINE__, input_device_info[0].ap_interface, get_nvram_zone(input_device_info[0].ap_interface));
                                printf("%s %d wificonf set SSID1 %s\n",sys_ssid1);
				sprintf(command, "wificonf set SSID1 \"%s\"", sys_ssid1);
        			system(command);
        			memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
                                printf("%s %d input_device_info[0].ap_interface %s, get_nvram_zone(input_device_info[0].ap_interface) %s\n",__FUNCTION__,__LINE__, input_device_info[1].ap_interface, get_nvram_zone(input_device_info[1].ap_interface));
                                printf("%s %d wificonf set SSID1 %s\n",sys_ssid2);
				sprintf(command, "wificonf set SSID1 \"%s\"", sys_ssid2);
        			system(command);
        			memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set WPAPSK1 \"\"");
        			system(command);
        			memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
				sprintf(command, "wificonf set WPAPSK1 \"\"");
        			system(command);
			}

                        
                        DBGPRINT(DEBUG_OFF, "\n %s %d command = %s",__FUNCTION__,__LINE__, command);
			break;

		case TRIBAND_NON_MULTI :
			//	todo :
			break;
		case TRIBAND_MULTI :        //normal triband
			// todo :
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf set SSID1 \"%s\"", ssid1);
			system(command);
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
			sprintf(command, "wificonf set SSID1 \"%s\"", ssid2);
			system(command);
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[2].ap_interface));
			sprintf(command, "wificonf set SSID1 \"%s\"", ssid3);
			system(command);			
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf set WPAPSK1 \"\"");
			system(command);
			break;	

		default :
			DBGPRINT(DEBUG_OFF,("wrong case \n"));
			break;
	}

	return;
}

/*
    ========================================================================
    Routine Description:
        To kill DHCP server

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/


void kill_dhcp_server()
{
	char command[150];
	memset (command, 0, sizeof(command));
	sprintf(command, "uci set dhcp.lan.ignore=1; uci commit; /etc/init.d/dnsmasq reload");
        
	system(command);

	DBGPRINT(DEBUG_OFF, "kill_dhcp_server: %s \n",command);
        
}

/*
    ========================================================================
    Routine Description:
        To kill DHCP client

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

void kill_dhcp_client()
{
	char command[150];
	memset (command, 0, sizeof(command));
	sprintf(command, ("killall -9 udhcpc"));
        
	system(command);

	DBGPRINT(DEBUG_OFF, "kill_dhcp_client: %s \n",command);
        
}

/*
    ========================================================================
    Routine Description:
        To check that the node is root node

    Arguments:
        Node number

    Return Value:
        int

    ========================================================================
*/


int is_root_node(EZ_NODE_NUMBER *node_number)
{
	if ((node_number->path_len == 6))
	{
		return TRUE;
	} else {
		return FALSE;
	}
	
}

/*
    ========================================================================
    Routine Description:
        To run DHCP server

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/


void run_dhcp_server()
{
	char command[150];
	char br_ip_str[MAX_IP_STR_LEN] = {0};

	memset(br_ip_str, 0, MAX_IP_STR_LEN);
	memset (command, 0, sizeof(command));
	kill_dhcp_client();
	/* need reset br-lan ip to 192.168.1.1,and then reload dhcp server */
	get_br_ip(br_ip_str);
	sprintf(command, "ifconfig br-lan %s up; uci set dhcp.lan.ignore=\"\"; uci commit; /etc/init.d/dnsmasq reload", br_ip_str);
	system(command);
	DBGPRINT(DEBUG_OFF, "run_dhcp_server: %s \n",command);
}

/*
    ========================================================================
    Routine Description:
        To run DHCP client

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/


void run_dhcp_client()
{
	char command[150];
	memset (command, 0, sizeof(command));
	sprintf(command, "udhcpc -i br-lan &");
        
	system(command);

	DBGPRINT(DEBUG_OFF, "run_dhcp_client: %s \n",command);
}

/*
    ========================================================================
    Routine Description:
        To handle differed DHCP

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

#ifdef IPHONE_SUPPORT
#ifdef REGROUP_SUPPORT
void regrp_clear_dhcp_defer();
#endif

void Handle_Differed_DHCP()
{
	printf("differed DHCP handle called\n");
	differ_dhcp_handling = 0;
	
	if (memcmp(last_stable_weight, expected_weight, NETWORK_WEIGHT_LEN)
#ifdef REGROUP_SUPPORT
		|| regrp_disconnect_sta == TRUE
#endif
		)
	{
		server_killed_by_differ_dhcp = 0;
		is_weight_not_change = 0;
		printf("latest weight not as per expectations\n");
		Handle_DHCP(&last_stable_node_number);
	} else if(server_killed_by_differ_dhcp == 1) {
		is_weight_not_change = 1;
		Handle_DHCP(&last_stable_node_number);		
	}
#ifdef REGROUP_SUPPORT
	regrp_clear_dhcp_defer();
#endif
}
#endif // IPHONE_SUPPORT

/*
    ========================================================================
    Routine Description:
        To handle DHCP

    Arguments:
        node number

    Return Value:
        void

    ========================================================================
*/

void Handle_DHCP(EZ_NODE_NUMBER *node_number)
{
	static char dhcp_client_running;
	static char dhcp_server_running = 1;
	char command[100]={0};
	
	DBGPRINT(DEBUG_OFF, "Handle_DHCP started\n");

#ifdef IPHONE_SUPPORT	
	if (differ_dhcp_handling)
	{
		printf("will process DHCP in differed context\n");
		if (dhcp_server_running)
		{
			printf("kill existing dhcp server\n");
			kill_dhcp_server();
			dhcp_server_running = 0;
			server_killed_by_differ_dhcp = 1;
		}
		return;
	}

	if(server_killed_by_differ_dhcp && is_weight_not_change){
		server_killed_by_differ_dhcp = 0;
		is_weight_not_change = 0;
		printf("server_killed_by_differ_dhcp %d, is_weight_not_change %d\n", server_killed_by_differ_dhcp, is_weight_not_change);
		printf("Run DHCP Server which is killed by differ_dhcp_handling \n");
		DBGPRINT(DEBUG_OFF, ("Root node now again. Run DHCP server!!!\n"));
		
		run_dhcp_server();
		dhcp_server_running = 1;
		return ;
	}
#endif // IPHONE_SUPPORT

	{	
		if (is_root_node(node_number))
		{
			if(!dhcp_server_running
#ifdef SINGLE_BACKHAUL_LINK
				&& !roam_triggered_on_2_g && !roam_triggered_on_5_g
#endif
			)		
			{
				DBGPRINT(DEBUG_OFF, ("Root node now. Run DHCP server!!!\n"));
			
				run_dhcp_server();
				dhcp_server_running = 1;
			}
			if(dhcp_client_running)
			{
		
				kill_dhcp_client();
				dhcp_client_running = 0;
			}			
		} 
		else 
		{
				kill_dhcp_server();
				kill_dhcp_client();
				sleep(1);
				run_dhcp_client();
				dhcp_client_running = 1;
				dhcp_server_running = 0;
				DBGPRINT(DEBUG_OFF, ("DHCP Server changed. Run DHCP Client again\n"));
			}
		}
	printf("\n----------------------\nRaghav: Deauth All STA\n----------------------\n");
	/* when DHCP server change, disconnect all non easy station for renew IP */
							memset(command, 0, 100);
	sprintf(command,"iwpriv %s set ez_send_broadcast_deauth=1",input_device_info[get_ez_idx()].cli_interface);
							system(command);
						
						} 
						
/*
    ========================================================================
    Routine Description:
        To convert PMK to string
						
    Arguments:
        pmk, to string

    Return Value:
        void

    ========================================================================
*/

void convert_hex_pmk_to_string(unsigned char *pmk, unsigned char *sys_pmk_string)
{
	int ret = 0;
	for (ret = 0; ret < 64;  ret++)
	{
		sys_pmk_string[ret] = (ret % 2) ? (pmk[ret / 2]  & 0xf) : ((pmk[ret / 2] & 0xf0 ) >> 4);
		if(sys_pmk_string[ret] < 10)	
			sys_pmk_string[ret] += '0';
		else 
			sys_pmk_string[ret] += 'a' - 10;
	}
	sys_pmk_string[64] = '\0';
}
/*
    ========================================================================
    Routine Description:
        Triggered scan for roaming

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

void try_roaming()
{
	unsigned int status = 0;
	char temp_command[100]={0};
	DBGPRINT(DEBUG_TRACE, "%s try_roaming_count %d \n", __FUNCTION__, try_roaming_count);

	if (dedicated_regrp_app == 1)
	{
		DBGPRINT(DEBUG_ERROR, "%s---> Dedicated Regroup app is present, no need to attempt roam here.\n", __FUNCTION__);
		return;
	}
	if (is_non_ez_connection == 1)
	{
		return;
	}
#ifdef STAR_ALWAYS
	if (is_third_party_present)
	
#endif
	{
		if (try_roaming_count)
		{	
			try_roaming_count--;
			if (try_roaming_count == 0)
			{
				roaming_scheduled = 0;
				is_roam_ongoing = 0;
				DBGPRINT(DEBUG_OFF, "%s Roaming cycle completed \n", __FUNCTION__);			
				return;
			}
		} else {
			return;
		}
	}

	if(is_triband || is_man_nonman || (cmd == SINGLE_BAND)) {		

		DBGPRINT(DEBUG_TRACE, " \n try_roaming triband or man_nonman or SINGLE_BAND \n");
		memset(temp_command, 0, 100);
		sprintf(temp_command, "iwpriv %s set SiteSurvey=",input_device_info[get_ez_idx()].ap_interface);			
		status = system(temp_command);
	} else if(cmd == DUAL_BAND) {

		DBGPRINT(DEBUG_TRACE, " \n try_roaming DUAL_BAND \n");
		if(try_roam_for == Band_5G) {
			memset(temp_command, 0, 100);
			sprintf(temp_command, "iwpriv %s set SiteSurvey=",input_device_info[1].ap_interface);			
			status = system(temp_command);	
		} else if(try_roam_for == Band_2G) {
			memset(temp_command, 0, 100);
			sprintf(temp_command, "iwpriv %s set SiteSurvey=",input_device_info[0].ap_interface);			
			status = system(temp_command);	
		}
	}
	
	if (status == 0)
		is_roam_ongoing = 1;
}

/*
    ========================================================================
    Routine Description:
        If any candidate found then roam, time out mechanism for roaming

    Arguments:
        buff --- event

    Return Value:
        void

    ========================================================================
*/

void man_triband_scan_complete_event(char * buff)
{
	char command[100] = {0};
	char temp_command[100]={0};
	unsigned char is_found;
	unsigned char *buf = (unsigned char *)buff;

	DBGPRINT(DEBUG_TRACE, " man_triband_scan_complete_event cmd %d\n", cmd);	
	if (is_third_party_present || (is_roam_ongoing == 0))
	{
		if(is_roam_ongoing ==0)
			DBGPRINT(DEBUG_ERROR, "unknown scan event.\n");

		return;
	}
	is_roam_ongoing = 0;
	if(is_triband || is_man_nonman || (cmd == SINGLE_BAND))
		{
		DBGPRINT(DEBUG_TRACE, "triband or man_nonman or single band\n");
		if (MAC_ADDR_EQUAL(buf,ZERO_MAC_ADDR)) {
			DBGPRINT(DEBUG_OFF, "No Candidate found, trigger scan again in 10 secs\n");
			eloop_register_timeout(10, 0, try_roaming, NULL, NULL);		
		} else {
			sprintf(command, "iwpriv %s set ez_apcli_roam_bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",input_device_info[get_ez_idx()].cli_interface, PRINT_MAC(buf));			
			system(command);
		}
	}
	else
	{
		is_found = !MAC_ADDR_EQUAL(buf,ZERO_MAC_ADDR);
		DBGPRINT(DEBUG_TRACE, "isfound %d\n", is_found);
			if(is_found && (try_roam_for == Band_5G)) {
				DBGPRINT(DEBUG_OFF, "Found !!! Roam for 5G\n");
#ifdef SINGLE_BACKHAUL_LINK
				roam_triggered_on_5_g = 1;
#endif
	        if(cmd == DUAL_BAND) 
			{
				sprintf(command, "iwpriv %s set ez_apcli_roam_bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",input_device_info[1].cli_interface,PRINT_MAC(buf)); 
#ifdef SINGLE_BACKHAUL_LINK					
				memset(temp_command, 0, 100);
				sprintf(temp_command,"iwpriv %s set ApCliEnable=1",input_device_info[1].cli_interface);
				system(temp_command);
#endif
				}
				if (system(command))
				{
					printf("Error in roam command\n");
#ifdef SINGLE_BACKHAUL_LINK
					roam_triggered_on_5_g = 0;
#endif
				}
				printf("\n %s \n", command);
			} else if(is_found && (try_roam_for == Band_2G) && extra_attempt_to_5g && is_5G_connected) {
				printf("Extra Attempt For 5G \n");
				extra_attempt_to_5g = 0;
				try_roam_for = Band_5G;
				try_roaming_count++;
				eloop_register_timeout(2, 0, try_roaming, NULL, NULL);					
			} else if(is_found && (try_roam_for == Band_2G)) {
				printf("Found !!! Roam for 2G\n");		
					if (cmd == DUAL_BAND)
					{
#ifdef SINGLE_BACKHAUL_LINK
				memset(temp_command, 0, 100);
				sprintf(temp_command,"iwpriv %s set ApCliEnable=0",input_device_info[1].cli_interface);
				system(temp_command);
#endif
				sprintf(command, "iwpriv %s set ez_apcli_roam_bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",input_device_info[0].cli_interface, PRINT_MAC(buf));

#ifdef SINGLE_BACKHAUL_LINK
				memset(temp_command, 0, 100);
				sprintf(temp_command,"iwpriv %s set ApCliEnable=1",input_device_info[0].cli_interface);
				system(temp_command);
#endif
				}

#ifdef SINGLE_BACKHAUL_LINK
				roam_triggered_on_2_g = 1;
#endif
				if (system(command))
				{
					printf("Error in roam command\n");
#ifdef SINGLE_BACKHAUL_LINK
					roam_triggered_on_2_g = 0;
#endif
				}
				printf("\n %s \n", command);
				
			} else {
				if (try_roam_for == Band_5G)
				{
					DBGPRINT(DEBUG_TRACE, "Try Roam For 2G\n");		
					try_roam_for = Band_2G;
					try_roaming();
				} else {
					printf("Try Roam For 5G\n");		
					try_roam_for = Band_5G;
					eloop_register_timeout(10, 0, try_roaming, NULL, NULL); 				
				}
				
			}
		}	
	}

/*
    ========================================================================
    Routine Description:
        To send station update

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

void send_sta_update()
{

	int sock1, sinlen, buf_len;
	struct sockaddr_in sock_in;
	int yes = 1;
	char netif[] = "br-lan";
	char buf[256];
	int i;
	int sta_cnt = 0;
	{
		{
			sinlen = sizeof(struct sockaddr_in);
			memset(&sock_in, 0, sinlen);
			//sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);			
			sock1 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			//setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
			setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
			//setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, netif, sizeof(netif));
			setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE, netif, sizeof(netif));
			/* -1 = 255.255.255.255 this is a BROADCAST address,
			 a local broadcast address could also be used.
			 you can comput the local broadcat using NIC address and its NETMASK 
			*/ 
			sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
			sock_in.sin_port = htons(PORT3); /* port number */
			sock_in.sin_family = AF_INET;
			memcpy(&buf[0], "5004", TAG_LEN);
			if(is_triband || is_man_nonman)
				memcpy(&buf[TAG_LEN], rai0_mac, MAC_ADDR_LEN);			
			else
				memcpy(&buf[TAG_LEN], ra0_mac, MAC_ADDR_LEN);			
	
			for(i=0; i < MAX_STA_SUPPORT; i++)
			{
				if (MAC_ADDR_EQUAL(station_list[i].mac_addr, ZERO_MAC_ADDR))
				{
					continue;
				}
				memcpy(&buf[TAG_LEN + MAC_ADDR_LEN +  (i * MAC_ADDR_LEN)], station_list[i].mac_addr, MAC_ADDR_LEN);					
				sta_cnt++;
			}	
			buf_len = TAG_LEN + ((sta_cnt + 1) * MAC_ADDR_LEN);
			sendto(sock1, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);
			close(sock1);
			shutdown(sock1, 2);			
		}
	}
}
/*
    ========================================================================
    Routine Description:
        To try update 3 times

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

void send_sta_update_3_times(){
	send_sta_update();
	while(send_sta_update_count)
	{
		--send_sta_update_count;
		eloop_register_timeout(3, 0, send_sta_update_3_times, NULL, NULL);
	}
}
/*
    ========================================================================
    Routine Description:
        To find whether device is connected or not

    Arguments:
        interface string

    Return Value:
        unsigned char 0 or 1
	
    ========================================================================
*/

unsigned char is_get_connected (char *buf)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig %s | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac", buf);
	DBGPRINT(DEBUG_TRACE, "\n command %s \n", command);
	system(command);
	fp = fopen("temp_file_mac", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	DBGPRINT(DEBUG_TRACE, "%s\n", peer_mac);
	if (peer_mac[0] != 'N')
	{
		return 1;
	} 
	else 
	{
		return 0;
	}
#endif
}

/*
    ========================================================================
    Routine Description:
	To print given value in hex of a particular length

    Arguments:
        string, string to change, length of string

    Return Value:
        void

    ========================================================================
		*/ 

void ez_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
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
}

/*
    ========================================================================
    Routine Description:
        To get interface MAC address

    Arguments:
        buf for saving MAC, interface name

    Return Value:
        int

    ========================================================================
*/

int get_rai0_mac (char *buf, char *ap_interface)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig %s | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac_rai",ap_interface);
	system(command);
	fp = fopen("temp_file_mac_rai", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	
	if (peer_mac[0] != '\0') 
	{
		for (i = 0; i < 17 ; i++)
		{
			if (i % 3 == 0)
			{
				buf[i/3] = (peer_mac[i] <= '9') ? ((peer_mac[i] - '0') << 4) : ((peer_mac[i] - 'A' + 10) << 4) ;
			}
			else if (i % 3 == 1)
			{
				buf[i/3] |= (peer_mac[i] <= '9') ? ((peer_mac[i] - '0')) : ((peer_mac[i] - 'A' + 10));
			} else {
				continue;
			}
		}
	} else {
		return 0;
	}
#endif
	return 1;
}

/*
    ========================================================================
    Routine Description:
        To get net status by ping

    Arguments:
        void
	
    Return Value:
        void

    ========================================================================
*/

void Net()
{
	if ( system("ping -c1 8.8.8.8 -w 2 > ping_8.txt") == 0)
	{
		NetStatus = 1;
	}
	else if(system("ping -c1 208.67.222.222 -w 2 > ping_22.txt") == 0)
	{   
		NetStatus = 1;
	}
	else
	{
		NetStatus = 0;
	}

        /* if ping failed,we need clear so many "Network is unreachable" */
        system("sed -i '/Network is unreachable/d' /tmp/ManDaemon.log");

	eloop_register_timeout(5, 0, Net, NULL, NULL);

}
/*
    ========================================================================
    Routine Description:
        Band support

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

#ifdef MOBILE_APP_SUPPORT
void Band_Support()
{
	char buf[10];
	int sock1, sinlen, buf_len, status;
	struct sockaddr_in sock_in;
	int yes = 1;
	char netif1[] = "br-lan";
	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	sock1 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
	setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));

	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK 
	*/ 
	sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
	sock_in.sin_port = htons(PORT2); /* port number */
	sock_in.sin_family = AF_INET;

	/* For Mobile APP to detect no of band support. e.g Single/dual/tri band */
	memset(buf, '\0', 10);
	memcpy(&buf[0], "5005", TAG_LEN);
	memcpy(&buf[TAG_LEN], No_of_Band, 1);
	buf_len = strlen(buf);
	DBGPRINT(DEBUG_TRACE, "buf_len %d\n", buf_len);
	status = sendto(sock1, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);
	DBGPRINT(DEBUG_TRACE, "status = %d\n", status);
	close(sock1);
	shutdown(sock1, 2);
	eloop_register_timeout(2, 0, Band_Support, NULL, NULL);
}
#endif //MOBILE_APP_SUPPORT

/*
    ========================================================================
    Routine Description:
        To find a third party connection

    Arguments:
	 void

    Return Value:
        int

    ========================================================================
*/

int is_connected_to_3party ()
{
	if (non_ez_connection)
	{
		return 1;
	} else {
		return 0;
	}
}

/*
    ========================================================================
    Routine Description:
        To get cli interface MAC address

    Arguments:
        buf for saving MAC, cli interface name
        
    Return Value:
        int

    ========================================================================
*/

int get_connected_ap_mac (char *buf, char *cli_interface)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig %s | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac",cli_interface);
	system(command);
	fp = fopen("temp_file_mac", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	if (peer_mac[0] != 'N')
	{
		for (i = 0; i < 17 ; i++)
		{
			if (i % 3 == 0)
			{
				buf[i/3] = (peer_mac[i] <= '9') ? ((peer_mac[i] - '0') << 4) : ((peer_mac[i] - 'A' + 10) << 4) ;
			}
			else if (i % 3 == 1)
			{
				buf[i/3] |= (peer_mac[i] <= '9') ? ((peer_mac[i] - '0')) : ((peer_mac[i] - 'A' + 10));
			} else {
				continue;
			}
		}
	} else {
		memset(buf, 0, MAC_ADDR_LEN);
	}
#endif
}

/*
    ========================================================================
    Routine Description:
        Sends a broadcast packet to mobile app
        It sends 2 packets periodically
        1. for information of MAC address and  ssid
        2. for internet connectivity status

    Arguments:
        void

    Return Value:
        void
    ========================================================================
*/

void client()
{
	int sock1, sinlen, config_len;
	char broadcast_mac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	char config[256];
	char ssid[100];
	char for_3party[20];	
	unsigned char ssid_len;
	struct sockaddr_in sock_in;
	char internet[10];
	int yes = 1;
	char netif1[] = "br-lan";
	char sta_ssid[]="STA";       
	unsigned char sta_ssid_len=0;	
	sta_ssid_len = strlen(sta_ssid);
	char dummy[18] = {0x35, 0x30, 0x30, 0x32, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0};
	int i;
	{
		sinlen = sizeof(struct sockaddr_in);
		memset(&sock_in, 0, sinlen);
		sock1 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
		setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));

		/* -1 = 255.255.255.255 this is a BROADCAST address,
		 a local broadcast address could also be used.
		 you can comput the local broadcat using NIC address and its NETMASK 
		*/ 
		sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
		sock_in.sin_port = htons(PORT2); /* port number */
		sock_in.sin_family = AF_INET;

		/* for on the behalf of Third party AP or if not connected */
		memcpy(&for_3party[0], "5002", TAG_LEN);
		memcpy(&for_3party[TAG_LEN], broadcast_mac, MAC_ADDR_LEN);
		for_3party[TAG_LEN + MAC_ADDR_LEN] = 0x00;
		memcpy(&for_3party[TAG_LEN + MAC_ADDR_LEN +1], broadcast_mac, MAC_ADDR_LEN);			
		for_3party[TAG_LEN + 2*MAC_ADDR_LEN +1] = 0x00;
		memset(config, 0, 256);
		memset(ssid, 0, 100);
		if(is_triband) {
			memcpy(ssid, system_ssid1, strlen(system_ssid1));
			ssid_len = strlen(ssid);
			memcpy(ssid + ssid_len, ":", 1);
			ssid_len = strlen(ssid);
				memcpy(ssid + ssid_len, system_ssid2, strlen(system_ssid2));
				ssid_len = strlen(ssid);
				memcpy(ssid + ssid_len, ":", 1);
				ssid_len = strlen(ssid);			
				memcpy(ssid + ssid_len, system_ssid3, strlen(system_ssid3)); 	
				ssid_len = strlen(ssid);
		}else if(is_man_nonman) {
			memcpy(ssid, system_ssid1, strlen(system_ssid1));
			ssid_len = strlen(ssid);
			memcpy(ssid + ssid_len, ":", 1);
			memcpy(ssid + ssid_len + 1, system_ssid2, strlen(system_ssid2));
			ssid_len = strlen(ssid);
		}
		else
		{
			memcpy(ssid, sys_ssid, strlen(sys_ssid));
			ssid_len = strlen(ssid);
		}

		memcpy(&config[0], "5002", TAG_LEN);
		if(is_triband || is_man_nonman)
		{
			if (MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR))
			{
				memcpy(&config[TAG_LEN], broadcast_mac, MAC_ADDR_LEN);
			} else {
				memcpy(&config[TAG_LEN], rai0_mac, MAC_ADDR_LEN);
			}
		}
		else
		{			
			memcpy(&config[TAG_LEN], ra0_mac, MAC_ADDR_LEN);
		}
		DBGPRINT(DEBUG_TRACE, "ssid : %s\n", ssid);
		DBGPRINT(DEBUG_TRACE, "ssid len : %d\n", ssid_len);		
		memcpy(&config[TAG_LEN + MAC_ADDR_LEN], &ssid_len, 1);
		memcpy(&config[11], ssid, ssid_len);
		memcpy(&config[17 + ssid_len], &ssid_len, 1);
		memcpy(&config[18 + ssid_len], ssid, ssid_len);
		if(is_triband || is_man_nonman)
		{
			if (MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR) || is_non_ez_connection)
			{
				memcpy(&config[11 + ssid_len], broadcast_mac, MAC_ADDR_LEN);
			}
			else 
			{
				memcpy(&config[11 + ssid_len], apclii0_peer_mac, MAC_ADDR_LEN);
			}
		}
		else
		{
			if(device_connected_0 || device_connected_1)
			{
				if(non_ez_connection)
				{
					memcpy(&config[11 + ssid_len], broadcast_mac, MAC_ADDR_LEN);
				}	
				else
				{
					memcpy(&config[11 + ssid_len], Peer2p4mac, MAC_ADDR_LEN);
				}
			}
			
		}
		config_len = TAG_LEN + (2 * MAC_ADDR_LEN) + 2 + (2 * ssid_len);
		if(((is_triband || is_man_nonman)&& !MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR))
			|| device_connected_0 || device_connected_1)
		{
			sendto(sock1, config, config_len, 0, (struct sockaddr *)&sock_in, sinlen);
			if (is_non_ez_connection)
			{
				sendto(sock1, dummy, sizeof(dummy), 0, (struct sockaddr *)&sock_in, sinlen);
			}
		}
		else
		{
			sendto(sock1, for_3party, 18, 0, (struct sockaddr *)&sock_in, sinlen);
		}


#if 1
        for(i=0; i < MAX_STA_SUPPORT; i++)
        {
        	if (MAC_ADDR_EQUAL(station_list[i].mac_addr, ZERO_MAC_ADDR))
        	{
        		continue;
        	}
			memcpy(&config[0], "5002", TAG_LEN);
            memcpy(&config[TAG_LEN], station_list[i].mac_addr, MAC_ADDR_LEN);
			ssid_len = strlen(ssid);
            memcpy(&config[TAG_LEN + MAC_ADDR_LEN], &sta_ssid_len, 1);
            memcpy(&config[11], sta_ssid, sta_ssid_len);
            memcpy(&config[11 + sta_ssid_len + MAC_ADDR_LEN], &ssid_len, 1);
            memcpy(&config[11 + sta_ssid_len + MAC_ADDR_LEN + 1], ssid, ssid_len);
			if(is_triband || is_man_nonman)
				memcpy(&config[11 + sta_ssid_len], rai0_mac, MAC_ADDR_LEN);
			else
				memcpy(&config[11 + sta_ssid_len], ra0_mac, MAC_ADDR_LEN);
            config_len = TAG_LEN + (2 * MAC_ADDR_LEN) + 2 + (ssid_len + sta_ssid_len);
			sendto(sock1, config, config_len, 0, (struct sockaddr *)&sock_in, sinlen);
		}
	}
#if 1		
		memcpy(&internet[0], "5003", TAG_LEN);
		if(is_triband || is_man_nonman)
			memcpy(&internet[TAG_LEN], rai0_mac, MAC_ADDR_LEN);
		else
			memcpy(&internet[TAG_LEN], ra0_mac, MAC_ADDR_LEN);
		if(NetStatus)
		{	
			sendto(sock1, internet, (TAG_LEN + MAC_ADDR_LEN), 0, (struct sockaddr *)&sock_in, sinlen);
		}

		//:: on the behalf of 3 party AP
		
		if (is_connected_to_3party())
		{		
			sendto(sock1, for_3party, 18, 0, (struct sockaddr *)&sock_in, sinlen);
			memcpy(&internet[0], "5003", TAG_LEN);
			memcpy(&internet[TAG_LEN], broadcast_mac, MAC_ADDR_LEN);
			if(NetStatus)
			{
				sendto(sock1, internet, (TAG_LEN + MAC_ADDR_LEN), 0, (struct sockaddr *)&sock_in, sinlen);
				
			}
		}
#endif
#endif
		close(sock1);
		shutdown(sock1, 2);
}

/*
    ========================================================================
    Routine Description:
        MAN periodic execution

    Arguments:
        void

    Return Value:
        void

    ========================================================================
*/

void man_periodic_exec()
{
	client();
	eloop_register_timeout(1, 0, man_periodic_exec, NULL, NULL);
}

/*
    ========================================================================
    Routine Description:
        convert PMK to hex

    Arguments:
        pmk string, pmk hex

    Return Value:
        int

    ========================================================================
*/


void convert_pmk_string_to_hex(char *sys_pmk_string, char *sys_pmk)
{
	int ret;
	char nibble;
	for (ret = 0; ret < 64; ret++)
	{
		nibble = sys_pmk_string[ret];
		if ((nibble <= '9'))
		{
			nibble = nibble - '0';
		} 
		else if (nibble < 'a') 
		{
			nibble = nibble - 'A' + 10;
		} else {
			nibble = nibble - 'a' + 10;			
		}
		if (ret % 2)
		{
			sys_pmk[ret/2] |= nibble; 
		}
		else 
		{
			sys_pmk[ret/2] = nibble << 4;
		}
	}
	
}
/*
    ========================================================================
    Routine Description:
        To get interface MAC address

    Arguments:
        buf for saving MAC, interface name

    Return Value:
        int

    ========================================================================
*/
int  get_ra0_mac(char *buf, char* ap_interface)
{
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, ap_interface);
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
	printf("\n---------  mandeamon ra0----------------\n");
    for (i = 0; i < 6; ++i)
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    puts("\n");
	
	printf("\n---------  mandeamon  ap_interface----------------\n");
      printf(" %s", ap_interface);
  }

  memcpy(buf, s.ifr_hwaddr.sa_data, 6);
  close(fd);
  return 0;

}
#ifdef REGROUP_SUPPORT
void regrp_init()
{
	
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	regrp_register_server();
	regrp_init_globals();



	ret = eloop_register_timeout(5, 0, regrp_periodic_exec, NULL, NULL);
	eloop_register_signal_terminate(regrp_terminate,NULL);

	return;
}
#endif

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void
	
    ========================================================================
*/
char * get_dat_file_name(char *buf)
{
        if ((chipbit & CHIP_7615D) && (chipbit & CHIP_SECOND_7615N)) {
                if(!strcmp(buf,"rax0") || !strcmp(buf,"apclix0")) {
                        return "cat /etc/wireless/mt7615e/mt7615e.1.2G.dat";
                        
                } else if(!strcmp(buf,"ra0") || !strcmp(buf,"apcli0")) {
                        return "cat /etc/wireless/mt7615e/mt7615e.1.5G.dat";
                
                } else {
                        return "cat /etc/wireless/mt7615e/mt7615e.2.dat";            
                }
        }
        else if (chipbit & CHIP_7615D) {
                if(!strcmp(buf,"rax0") || !strcmp(buf,"apclix0")) {
                        return "cat /etc/wireless/mt7615e/mt7615e.1.2G.dat";
                } else {
                        return "cat /etc/wireless/mt7615e/mt7615e.1.5G.dat";
                }
        }
        else if ((chipbit & CHIP_FIRST_7615N) && (chipbit & CHIP_SECOND_7615N)) {
                if(!strcmp(buf,"ra0") || !strcmp(buf,"apcli0")) {
                        return "cat /etc/wireless/mt7615e/mt7615e.1.dat";
                } else {
                        return "cat /etc/wireless/mt7615e/mt7615e.2.dat";
                }
        }
        else if ((chipbit & CHIP_7603) && (chipbit & CHIP_SECOND_7615N)) {
                if(!strcmp(buf,"ra0") || !strcmp(buf,"apcli0")) {
                        return "cat /etc/wireless/mt7603e/mt7603e.dat";
                } else {
                        return "cat /etc/wireless/mt7615e/mt7615e.2.dat";
                }
        }
        else if ((chipbit & CHIP_7603) && (chipbit & CHIP_7612)) {
                if(!strcmp(buf,"ra0") || !strcmp(buf,"apcli0")) {
                        return "cat /etc/wireless/mt7603e/mt7603e.dat";
                } else {
                        return "cat /etc/wireless/mt76x2e/mt7612e.dat";
                }
        }
        else if ((chipbit & CHIP_7628) && (chipbit & CHIP_7612)) {
                if(!strcmp(buf,"ra0") || !strcmp(buf,"apcli0")) {
                        return "cat /etc/wireless/mt7628/mt7628.dat";
                } else {
                        return "cat /etc/wireless/mt76x2e/mt7612e.dat";
                }
        }
}

void get_value_from_dat_file(char *inf, char *value, char *buf)
{
	char command[150];	
	FILE *fp;

	sprintf(command, " %s | grep %s | awk -F \"=\" \'\{print $2}\'> temp_file", get_dat_file_name(inf), value);
	system(command);
	printf("\n get_value_from_dat_file command : %s\n", command);
	fp = fopen("temp_file", "r");
	fgets(buf, 2,fp);
	fclose(fp);

	return;
}
#ifdef REGROUP_SUPPORT
void regrp_get_rssi_thold()
{
	char command[100];
	FILE *fp;
	char tmp[10];

	g_default_rssi_threshold = -73;
	g_default_max_rssi_threshold = -86;
	g_custom_rssi_th =-68;

	memset(command, 0, 100);
	switch_profile("2860");
	sprintf(command, "wificonf get RegrpDefaultRssiTh > temp_file");
	system(command);

	fp = fopen("temp_file", "r");
	fgets(tmp, 4, fp);
	fclose(fp);
	tmp[strlen(tmp)] = '\0';
	if(atoi(tmp))
		g_default_rssi_threshold = atoi(tmp);
	printf("g_default_rssi_threshold : %d\n", g_default_rssi_threshold);

	memset(command, 0, 100);
	sprintf(command, "wificonf get RegrpDefaultMaxRssiTh > temp_file");
	system(command);
	fp = fopen("temp_file", "r");
	fgets(tmp, 4, fp);
	fclose(fp);
	tmp[strlen(tmp)] = '\0';
	
	if(atoi(tmp))
		g_default_max_rssi_threshold = atoi(tmp);
	printf("g_custom_rssi_th : %d\n", g_default_max_rssi_threshold);

	memset(command, 0, 100);
	sprintf(command, "wificonf get RegrpCustomRssiTh > temp_file");
	system(command);
	fp = fopen("temp_file", "r");
	fgets(tmp, 4, fp);
	fclose(fp);
	tmp[strlen(tmp)] = '\0';
	if(atoi(tmp))
		g_custom_rssi_th = atoi(tmp);
	printf("g_custom_rssi_th : %d\n", g_custom_rssi_th);

}
#endif

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_init()
{
	int ret = 0, i = 0, Ez_index, nEz_index;
	char command[150], EzDefaultSetting[1];
#ifdef DEDICATED_MAN_AP	
	char ApCliEnable[2];
#endif	
	char ssid[65];
	FILE *fp;
	const char s[2] = ";";
   	char *token;
	EZ_NODE_NUMBER node_number;
	unsigned char Ez_ap_int_zone[INTERFACE_LEN];
	unsigned char nEz_ap_int_zone[INTERFACE_LEN];

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* Initialze event loop */
	ret = eloop_init();

	memset(system_ssid1, 0, 32);
	memset(system_ssid2, 0, 32);
	memset(system_ssid3, 0, 32);
        memset(sys_ssid, 0, sizeof(sys_ssid));
        memset(sys_ssid1, 0, sizeof(sys_ssid1));
        memset(sys_ssid2, 0, sizeof(sys_ssid2));	
	memset(system_wpapsk, 0, 65);
	memset(system_authmode, 0, 14);		
	memset(system_encryptype, 0, 8);
	memset(system_ftmdid, 0, 3);
	memset(command, 0, sizeof(command));
	memset(ssid, 0, sizeof(sys_ssid));	

	switch(cmd)
	{
		case SINGLE_BAND :
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf get EzDefaultSsid > temp_file");
			system(command);
			fp = fopen("temp_file", "r");
			fgets(ssid, 65, fp);
			fclose(fp);
			ssid[strlen(ssid)-1] = '\0';
			get_ra0_mac(ra0_mac,input_device_info[0].ap_interface);
			memcpy(sys_ssid1, ssid, strlen(ssid));		
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
		
		break;
		case DUAL_BAND_MAN_NONMAN :
			if (input_device_info[0].Ez_status == '1') {
				Ez_index = 0;
				nEz_index = 1;
			} else {
				Ez_index = 1;
				nEz_index = 0;
			}
			memcpy(&nEz_ap_int_zone, get_nvram_zone(input_device_info[nEz_index].ap_interface), strlen(get_nvram_zone(input_device_info[nEz_index].ap_interface)));
			memcpy(&Ez_ap_int_zone, get_nvram_zone(input_device_info[Ez_index].ap_interface), strlen(get_nvram_zone(input_device_info[Ez_index].ap_interface)));
			memset(command, 0, sizeof(command));
                        switch_profile(Ez_ap_int_zone);
			sprintf(command,"wificonf get EzDefaultSsid | awk -F \";\" \'\{print $1}\' > temp_file1");
			system (command);			
        		fp = fopen("temp_file1", "r");
        		fgets(system_ssid1, 32, fp);
			fclose(fp);
        		system_ssid1[strlen(system_ssid1) - 1] = '\0';
			memset(command, 0, sizeof(command));
                        switch_profile(nEz_ap_int_zone);
			sprintf(command,"wificonf get SSID1 | awk -F \";\" \'\{print $1}\' > temp_file2");
			system (command);

        		fp = fopen("temp_file2", "r");
        		fgets(system_ssid2, 32, fp);
			fclose(fp);
        		system_ssid2[strlen(system_ssid2) - 1] = '\0';

			memset(command, 0, sizeof(command));
			sprintf(command,"wificonf get WPAPSK1 | awk -F \";\" \'\{print $1}\' > temp_file3");
			system (command);
			
        		fp = fopen("temp_file3", "r");
        		fgets(system_wpapsk, 64, fp);
			fclose(fp);
        		system_wpapsk[strlen(system_wpapsk) - 1] = '\0';
				
			memset(command, 0, sizeof(command));
			sprintf(command,"wificonf get AuthMode | awk -F \";\" \'\{print $1}\' > temp_file4");
			system (command);
			
        		fp = fopen("temp_file4", "r");
        		fgets(system_authmode, 13, fp);
			fclose(fp);
        		system_authmode[strlen(system_authmode) - 1] = '\0';

			memset(command, 0, sizeof(command));
			sprintf(command,"wificonf get EncrypType | awk -F \";\" \'\{print $1}\' > temp_file5");
			system (command);
			
        		fp = fopen("temp_file5", "r");
        		fgets(system_encryptype, 7, fp);
			fclose(fp);
        		system_encryptype[strlen(system_encryptype) - 1] = '\0';

			memset(command, 0, sizeof(command));
			sprintf(command,"wificonf get FtMdId1 | awk -F \";\" \'\{print $1}\' > temp_file6");
			system (command);
        			
        		fp = fopen("temp_file6", "r");
        		fgets(system_ftmdid, 3, fp);
			fclose(fp);
        		system_ftmdid[FT_MDID_LEN] = '\0';

			/* pass non man info on man */
        		memset(command, 0, 150);
			sprintf(command, "iwpriv %s set ez_nonman_info=\"%s;%s;%s;%s;%s\"",input_device_info[Ez_index].ap_interface,
			system_ssid2, system_wpapsk, system_encryptype, system_authmode, system_ftmdid);
        		system(command);
			get_connected_ap_mac(apclii0_peer_mac,input_device_info[Ez_index].cli_interface); 
			get_rai0_mac(rai0_mac,input_device_info[Ez_index].ap_interface);
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
			memcpy(sys_ssid + strlen(sys_ssid1), ":", 1);			
			memcpy(sys_ssid + strlen(sys_ssid1) + 1, sys_ssid2, strlen(sys_ssid2));

		break;
		case DUAL_BAND:
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf get EzDefaultSsid > temp_file");
        		system(command);
        		fp = fopen("temp_file", "r");
        		fgets(ssid, 65, fp);
			fclose(fp);
        		ssid[strlen(ssid)-1] = '\0';

			if(!strcmp(get_nvram_zone(input_device_info[0].ap_interface), get_nvram_zone(input_device_info[1].ap_interface))) { // Arvind use MACRO for strcmp 	
				memset(sys_ssid1, 0, sizeof(sys_ssid1));
                		token = strtok(ssid, s);
                		while( token != NULL ) 
                		{
                			if(i==0) {
                				memcpy(sys_ssid1, token, strlen(token));
                				i++;
                			} else if(i==1) {
                				memcpy(sys_ssid2, token, strlen(token));
                				i++;
                			}
                			token = strtok(NULL, s);
                		}
			} else {			
				memcpy(sys_ssid1, ssid, strlen(ssid));
                                printf("%s %d sys_ssid1 %s\n", __FUNCTION__,__LINE__,sys_ssid1);
                		memset(command, 0, 150);
                		memset(ssid, 0, 65);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
				sprintf(command, "wificonf get EzDefaultSsid > temp_file");
				system(command);

                		fp = fopen("temp_file", "r");
                		fgets(ssid, 65, fp);
                		fclose(fp);
                		ssid[strlen(ssid)-1] = '\0';
		
				memcpy(sys_ssid2, ssid, strlen(ssid));
                                printf("%s %d sys_ssid2 %s\n", __FUNCTION__,__LINE__,sys_ssid2);
		        }
			get_ra0_mac(ra0_mac,input_device_info[0].ap_interface);
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
			memcpy(sys_ssid + strlen(sys_ssid1), ":", 1);			
			memcpy(sys_ssid + strlen(sys_ssid1) + 1, sys_ssid2, strlen(sys_ssid2));
			
		break;
		case TRIBAND_MULTI :
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command,"wificonf get EzDefaultSsid | awk -F \";\" \'\{print $1}\' > temp_file1");
			system (command);
			
			fp = fopen("temp_file1", "r");
			fgets(system_ssid1, 32, fp);
			fclose(fp);
			memset(command, 0, sizeof(command));
                        switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
			sprintf(command,"wificonf get SSID1 | awk -F \";\" \'\{print $1}\' > temp_file2");
			system (command);
			fp = fopen("temp_file2", "r");
			fgets(system_ssid2, 32, fp);
			fclose(fp);

			memset(command, 0, sizeof(command));
                        switch_profile(get_nvram_zone(input_device_info[2].ap_interface));
			sprintf(command,"wificonf get SSID1 | awk -F \";\" \'\{print $1}\' > temp_file3");
			system (command);
			fp = fopen("temp_file3", "r");
			fgets(system_ssid3, 32, fp);
			fclose(fp);

			system_ssid1[strlen(system_ssid1) - 1] = 0;
			system_ssid2[strlen(system_ssid2) - 1] = 0;
			system_ssid3[strlen(system_ssid3) - 1] = 0;
			get_connected_ap_mac(apclii0_peer_mac,input_device_info[0].cli_interface);
			get_rai0_mac(rai0_mac,input_device_info[0].ap_interface);
		break;
		default :
			DBGPRINT(DEBUG_OFF, "\n man_init wrong case .\n");			
			break;
	}
	
	driver_wext_init();
	if (ret)
	{	
		DBGPRINT(DEBUG_OFF, "eloop_register_timeout failed.\n");
		return;
	}
	if (ret == 0){
#ifdef MOBILE_APP_SUPPORT
		ret = eloop_register_timeout(1, 0, man_periodic_exec, NULL, NULL);
		ret = eloop_register_timeout(12, 0, Net, NULL, NULL);
#ifdef MOBILE_APP_SUPPORT
		ret = eloop_register_timeout(2, 0, Band_Support, NULL, NULL);
#endif	//MOBILE_APP_SUPPORT
#endif		
	}
	memset(command, 0, sizeof(command));
	sprintf(command,"iwpriv %s set ez_scan_same_channel_time=20",input_device_info[0].ap_interface);
	system (command);
	memset(command, 0, sizeof(command));
	sprintf(command,"iwpriv %s set ez_scan_same_channel_time=20",input_device_info[1].ap_interface);
	system (command);
	sleep(10);

	memset(command, 0, 100);
	sprintf(command, "iwpriv %s set ez_connection_allow_all=",input_device_info[get_ez_idx()].cli_interface);
	system(command);
#ifdef MOBILE_APP_SUPPORT
		memset(command, 0, 150);
		memset(EzDefaultSetting, 0, 1);	
		
	switch(cmd)
	{
		case SINGLE_BAND :
#if 0
			sprintf(command, "nvram_get %s EzDefaultSettings > temp_file",get_nvram_zone(input_device_info[0].ap_interface));
		system(command);
		fp = fopen("temp_file", "r");
		fgets(EzDefaultSetting, 2,fp);
#endif
			get_value_from_dat_file(input_device_info[0].ap_interface, "EzDefaultSettings", EzDefaultSetting);

			printf("\n EzDefaultSetting : %x \n", EzDefaultSetting[0]);
			
			if(EzDefaultSetting[0] != '1') {

				DBGPRINT(DEBUG_OFF, "\n Need default setting\n");

				memset(command, 0, 150);
				sprintf(command,"iwpriv %s set ez_set_ssid_psk=\"AP;12345678\"",input_device_info[0].ap_interface);
				system(command);

				memset(command, 0, 150);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultSettings \"1\"");
				system(command);
				sleep(1);
			}else {

				DBGPRINT(DEBUG_TRACE, "\n Dont need default etting\n");
			}

		break;
		case DUAL_BAND_MAN_NONMAN:
#if 0
			sprintf(command, "nvram_get %s EzDefaultSettings > temp_file",get_nvram_zone(input_device_info[Ez_index].ap_interface));
			system(command);
			fp = fopen("temp_file", "r");
			fgets(EzDefaultSetting, 2,fp);
#endif
			get_value_from_dat_file(input_device_info[Ez_index].ap_interface, "EzDefaultSettings", EzDefaultSetting);

			printf("\n EzDefaultSetting : %x \n", EzDefaultSetting[0]);
			
			if(EzDefaultSetting[0] != '1') {

				DBGPRINT(DEBUG_OFF, "\n Need default setting\n");

				memset(command, 0, 150);
				sprintf(command,"iwpriv %s set ez_set_ssid_psk=\"AP_2G:AP_5G;12345678:12345678\"",input_device_info[Ez_index].ap_interface);
				system(command);

				memset(command, 0, 150);
                                switch_profile(get_nvram_zone(input_device_info[Ez_index].ap_interface));
				sprintf(command, "wificonf set EzDefaultSettings \"1\"");
				system(command);
				sleep(1);
			} else {
			DBGPRINT(DEBUG_TRACE, "\n Dont need default etting\n");
		}

		break;
		case DUAL_BAND :
#if 0			
			sprintf(command, "nvram_get %s EzDefaultSettings > temp_file",get_nvram_zone(input_device_info[0].ap_interface));
			system(command);
			fp = fopen("temp_file", "r");
			fgets(EzDefaultSetting, 2,fp);
#endif
			get_value_from_dat_file(input_device_info[0].ap_interface, "EzDefaultSettings", EzDefaultSetting);

			printf("\n EzDefaultSetting : %x \n", EzDefaultSetting[0]);
			
			if(EzDefaultSetting[0] != '1') {

				DBGPRINT(DEBUG_OFF, "\n Need default setting\n");

                                memset(command, 0, 150);
				sprintf(command,"iwpriv %s set ez_set_ssid_psk=\"AP_2G:AP_5G;12345678:12345678\"",input_device_info[0].ap_interface);
				system(command);
                                DBGPRINT(DEBUG_OFF, "\n %s %d %s\n",__FUNCTION__,__LINE__, command);
				memset(command, 0, 150);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultSettings \"1\"");
				system(command);
                                DBGPRINT(DEBUG_OFF, "\n %s %d %s\n",__FUNCTION__,__LINE__, command);
				sleep(1);
			} else	{
				DBGPRINT(DEBUG_TRACE, "\n Dont need default setting\n");
			}
		
		break;
		case TRIBAND_MULTI :
#if 0			
			sprintf(command, "nvram_get %s EzDefaultSettings > temp_file",get_nvram_zone(input_device_info[0].ap_interface));
		system(command);
		fp = fopen("temp_file", "r");
		fgets(EzDefaultSetting, 2,fp);
#endif

			get_value_from_dat_file(input_device_info[0].ap_interface, "EzDefaultSettings", EzDefaultSetting);

			printf("\n EzDefaultSetting : %x \n", EzDefaultSetting[0]);

			if(EzDefaultSetting[0] != '1') {

				DBGPRINT(DEBUG_OFF, "\n Need default setting\n");

				memset(command, 0, 150);
				sprintf(command, "iwpriv %s set ez_set_ssid_psk=\"AP_BH:AP_2G:AP_5G;12345678:12345678:12345678;AES:AES;WPA2PSK:WPA2PSK\"",input_device_info[0].ap_interface);
				system(command);

			memset(command, 0, 150);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultSettings \"1\"");
			system(command);
				sleep(1);
			} else {
			DBGPRINT(DEBUG_TRACE, "\n Dont need default etting\n");
		}
		default :
				DBGPRINT(DEBUG_TRACE, "\n Wrong configuration \n");

			break;
	}

#endif

	printf("%s %d \n",__FUNCTION__,__LINE__);

	if(is_triband || is_man_nonman)
	{
		DBGPRINT(DEBUG_OFF, " This is triband or 1 man 1 nonman \n");
#ifdef DEDICATED_MAN_AP		
		memset(command, 0, 100);
                switch_profile(get_nvram_zone(input_device_info[get_ez_idx()].ap_interface));
		sprintf(command, "wificonf get ApCliEnable > temp_file");
		system(command);
		fp = fopen("temp_file", "r");
		fgets(ApCliEnable, 2,fp);
		fclose(fp);

 		if(ApCliEnable[0] == '1')
#endif		
		{
			memset(command,0,100);
			sprintf(command,"iwpriv %s set ApCliEnable=1",input_device_info[get_ez_idx()].cli_interface);
			system(command);
		}
	}
	else
	{
		if(cmd == SINGLE_BAND)
		{
			DBGPRINT(DEBUG_OFF, " This is SINGLE_BAND\n");
#ifdef DEDICATED_MAN_AP		
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf get ApCliEnable > temp_file");
			system(command);
			fp = fopen("temp_file", "r");
			fgets(ApCliEnable, 2,fp);
			fclose(fp);
			
			if(ApCliEnable[0] == '1')
#endif			
			{	
				memset(command,0,100);
				sprintf(command,"iwpriv %s set ApCliEnable=1",input_device_info[0].cli_interface);
				system(command);
			}
		}
		else
		{
			DBGPRINT(DEBUG_OFF, " This is DUAL Band \n");
#ifdef DEDICATED_MAN_AP		
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf get ApCliEnable > temp_file");
			system(command);
			fp = fopen("temp_file", "r");
			fgets(ApCliEnable, 2,fp);
			fclose(fp);
#endif

#ifdef DEDICATED_MAN_AP		
			if (ApCliEnable[0] == '1')
#endif				
			{
				memset(command, 0, 100);
				sprintf(command, "iwpriv %s set ApCliEnable=1",input_device_info[0].cli_interface);
				system(command);

                                DBGPRINT(DEBUG_OFF, "\n %s %d system call is \n%s\n",__FUNCTION__,__LINE__, command);
				memset(command, 0, 100);
				sprintf(command, "iwpriv %s set ApCliEnable=1",input_device_info[1].cli_interface);
				system(command);

                                DBGPRINT(DEBUG_OFF, "\n %s %d system call is \n%s\n",__FUNCTION__,__LINE__, command);
			}
		
		}	
				}

//	memset(command, 0, 100);
//	sprintf(command, "iwpriv %s set ez_connection_allow_all=",input_device_info[get_ez_idx()].cli_interface);
//	system(command);
	printf("%s %d \n",__FUNCTION__,__LINE__);
#ifdef REGROUP_SUPPORT
		UCHAR RegroupSupport[2];
		memset(command, 0, 100);
                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
		sprintf(command, "wificonf get RegroupSupport > temp_file");
		system(command);
		fp = fopen("temp_file", "r");
		fgets(RegroupSupport, 2,fp);
		fclose(fp);
                
		if(RegroupSupport[0] == '1')
		{
			DBGPRINT(DEBUG_OFF, "===============> REgroup Support Enabled <================= \n");
			dedicated_regrp_app = 1;
			}
			else
			{
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
			sprintf(command, "wificonf get RegroupSupport > temp_file");
			system(command);
			fp = fopen("temp_file", "r");
			fgets(RegroupSupport, 2,fp);
			fclose(fp);
			if(RegroupSupport[0] == '1')
				{
				DBGPRINT(DEBUG_OFF, "===============> REgroup Support not Enable on interface %s \n",input_device_info[0].ap_interface);
				dedicated_regrp_app = 1;
			}
			else
			{
				DBGPRINT(DEBUG_OFF, "===============> REgroup Support not Enabled <================= \n");
				dedicated_regrp_app = 0;
		}	
			
	}	
		if(dedicated_regrp_app)
		{
			
			char tmp[4];

			memset(tmp, 0, 4);

			if (input_device_info[0].Ez_status == '1')
			{
				if(input_device_info[0].band == '5')
				{
					memcpy(interface_5g, input_device_info[0].ap_interface, 16);
					memcpy(cli_interface_5g, input_device_info[0].cli_interface, 16);					
				} else {
			memcpy(interface_2g, input_device_info[0].ap_interface, 16);
			memcpy(cli_interface_2g, input_device_info[0].cli_interface, 16);
				}
			}

			
			if (input_device_info[1].Ez_status == '1')
			{
				if(input_device_info[1].band == '5')
				{
					memcpy(interface_5g, input_device_info[1].ap_interface, 16);
					memcpy(cli_interface_5g, input_device_info[1].cli_interface, 16);					
				} else {
					memcpy(interface_2g, input_device_info[1].ap_interface, 16);
					memcpy(cli_interface_2g, input_device_info[1].cli_interface, 16);
				}
			}
#if 0			
			memcpy(interface_2g, input_device_info[0].ap_interface, 16);
			memcpy(cli_interface_2g, input_device_info[0].cli_interface, 16);
			memcpy(interface_5g, input_device_info[1].ap_interface, 16);
			memcpy(cli_interface_5g, input_device_info[1].cli_interface, 16);
#endif
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf get RegroupPeriodicity > temp_file");
			system(command);
			fp = fopen("temp_file", "r");
            fgets(tmp, 4, fp);
			fclose(fp);
			tmp[strlen(tmp)] = '\0';
			regrp_periodic_time =atoi(tmp);
			if(!regrp_periodic_time)
			regrp_periodic_time = 30;
			printf("Regroup Period : %d\n", regrp_periodic_time);
			regrp_get_rssi_thold();
			regrp_init(); 
	}
#endif
	
	eloop_run();
	return;
}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void check_cli_status()
{
	char command[200];

	device_connected_0 = is_get_connected(input_device_info[0].cli_interface);
	device_connected_1 = is_get_connected(input_device_info[1].cli_interface);

	printf("2.4G connected? %d, 5G connected? %d\n", device_connected_0, device_connected_1);

	if((device_connected_1 == 1) && (device_connected_0 == 0)) {
			printf("5G connected, disable 2.4G\n");
		memset(command, 0, sizeof(command));
		sprintf(command,"iwpriv %s set ApCliEnable=0", input_device_info[0].cli_interface);
		system(command);
		} else if( (device_connected_1 == 0) && (device_connected_0 == 0)){

			printf("Both interface open now, Enable both CLIs\n");
		memset(command, 0, sizeof(command));
		sprintf(command,"iwpriv %s set ApCliEnable=1", input_device_info[0].cli_interface);
		system(command);

		memset(command, 0, sizeof(command));
		sprintf(command,"iwpriv %s set ApCliEnable=1", input_device_info[1].cli_interface);
		system(command);
		
		} else if ((device_connected_1 == 0) && (device_connected_0 == 1)){

			printf("2.4G Connect, 5G not connected, disable 5G now\n");
		memset(command, 0, sizeof(command));
		sprintf(command,"iwpriv %s set ApCliEnable=0",input_device_info[1].cli_interface);
		system(command);
	}
}


/////////////////////////////////////  EVENT HANDLER CODE  ////////////////////////////////////////////////////////////////////////////////

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_update_station_event(char * buf, int num_sta)
{
	int i = 0;
	int j = 0;
	unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};
	for (j = 0; j < num_sta; j ++)
	{
		for (i = 0; i < MAX_STA_SUPPORT; i++)
		{
			if (MAC_ADDR_EQUAL(station_list[i].mac_addr, &buf[j * MAC_ADDR_LEN]))
			{
				DBGPRINT(DEBUG_OFF, "delete from station list\n");
				
				memcpy(station_list[i].mac_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
				break;
			}
		}
	}
}	

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_delete_station_event(char * buf)
{
	int i = 0;
	unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};
	for (i = 0; i < MAX_STA_SUPPORT; i++)
	{
		if (MAC_ADDR_EQUAL(station_list[i].mac_addr, &buf[2]))
		{
			DBGPRINT(DEBUG_OFF, "delete from station list\n");
			
			memcpy(station_list[i].mac_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
			break;
		}
	}
}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_add_station_event(char * buf)
{
	int i = 0;
	for (i = 0; i < MAX_STA_SUPPORT; i++)
	{
		if (MAC_ADDR_EQUAL(station_list[i].mac_addr,&buf[2]))
		{
			break;
		}
		if (MAC_ADDR_EQUAL(station_list[i].mac_addr,ZERO_MAC_ADDR))
		{
			DBGPRINT(DEBUG_OFF, "Add to station list\n");
			memcpy(station_list[i].mac_addr, &buf[2], MAC_ADDR_LEN);
			break;
		}
	}
	send_sta_update_count = 2;
	send_sta_update_3_times();
}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_triband_ez_driver_event_handle(char *buf)
{
	triband_ez_device_info_to_app_t *ez_device_info = (triband_ez_device_info_to_app_t *)buf;
	unsigned char sys_pmk_string[65];
	char command[200];
	int flag = 0;
#ifdef IPHONE_SUPPORT	
	int ret;
#endif // IPHONE_SUPPORT
#if 0
	if (ez_device_info->forced_update)
	{
			system("iwpriv apclii0 set ApCliEnable=0");	
		system("iwpriv apclii0 set ApCliEnable=1");
		}
#endif
#ifdef REGROUP_SUPPORT
		if(dedicated_regrp_app)
		{

		UINT8 is_prev_master_cand=0;
		p_repeater_list_struct p_own_rept = &g_own_rept_info;
		is_prev_master_cand = is_regrp_master_candidate(p_own_rept);
	
		
		memcpy(p_own_rept->network_weight, ez_device_info->network_weight, NETWORK_WEIGHT_LEN);
		memcpy(&p_own_rept->node_number,&ez_device_info->node_number, sizeof(EZ_NODE_NUMBER));
	
		hex_dump("Network Weight",p_own_rept->network_weight,NETWORK_WEIGHT_LEN);
	
		hex_dump("Node Number",(UINT8 *)&p_own_rept->node_number, sizeof(EZ_NODE_NUMBER));
		p_own_rept->non_ez_connection = ez_device_info->is_non_ez_connection;
		printf("is master = %d\n", p_own_rept->is_master);
		printf("%d: %d %d:\n",is_regrp_master_candidate(p_own_rept),p_own_rept->regrp_state,is_prev_master_cand);
			DBGPRINT(DEBUG_OFF, "%s %d ===> \n",__FUNCTION__,__LINE__);
		regrp_clean_all_states();
			DBGPRINT(DEBUG_OFF, "%s %d ===> \n",__FUNCTION__,__LINE__);
		if(is_regrp_master_candidate(p_own_rept) && (p_own_rept->regrp_state == REGRP_STATE_DONE))
		{
			printf("%s:Restart Regroup\n",__func__);
			eloop_register_timeout(RESTART_REGROUP_TIME, 0, restart_regroup,NULL, NULL);
			g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
			trigger_regrp = 0;
		}
				DBGPRINT(DEBUG_OFF, "%s %d ===> \n",__FUNCTION__,__LINE__);
		if(is_prev_master_cand == 0 && is_regrp_master_candidate(p_own_rept))
		{
			// this is created master from slave. Trigger regroup from periodic exec.
			printf("device became a master\n");
			trigger_regrp = 1;
			os_get_time(&last_entry_added_time);
		}
				DBGPRINT(DEBUG_OFF, "%s %d ===> \n",__FUNCTION__,__LINE__);
		if(!is_regrp_master_candidate(p_own_rept))
		{
			p_own_rept->is_master = 0;
			p_own_rept->regrp_state = REGRP_SLAVE_STATE_IDLE;
		}
				DBGPRINT(DEBUG_OFF, "%s %d ===> \n",__FUNCTION__,__LINE__);
		}
#endif

#ifdef IPHONE_SUPPORT
#ifdef REGROUP_SUPPORT
	if(regrp_dhcp_defer)
	{
		if(regrp_is_target_wt_same(regrp_expected_wt_1, regrp_expected_wt_2))
		{
			printf("Same wt acheived. Handle dhcp now %d.\n",regrp_disconnect_sta);
			memcpy(expected_weight, g_own_rept_info.network_weight, NETWORK_WEIGHT_LEN);
			memcpy(last_stable_weight, ez_device_info->network_weight, NETWORK_WEIGHT_LEN);
			memcpy(&last_stable_node_number, &ez_device_info->node_number, sizeof(EZ_NODE_NUMBER));
			if(eloop_is_timeout_registered(Handle_Differed_DHCP ,NULL,NULL))
				eloop_cancel_timeout(Handle_Differed_DHCP ,NULL,NULL);
			if(regrp_disconnect_sta == TRUE)
				Handle_Differed_DHCP();
			else
				printf("Raghav:Don't handle dhcp\n");
			regrp_clear_dhcp_defer();
			differ_dhcp_handling = 0;
			flag = 1;
		}
	}
#endif
	if (flag == 0 && ez_device_info->is_forced && !differ_dhcp_handling)
	{
		if(last_stable_weight[0] == 0x0F && ez_device_info->network_weight[0]== 0x0F)
		{
			printf("Wt changed from 0xf to 0xf. Don't start DHCP\n");
			flag =1;
		}
		else
	{
		printf("Start differed DHCP handling\n");
		differ_dhcp_handling = 1;
		memcpy(expected_weight, last_stable_weight, NETWORK_WEIGHT_LEN);
		ret = eloop_register_timeout(20, 0, Handle_Differed_DHCP, NULL, NULL);
	}
	}
	if(flag == 0 && (!memcmp(expected_weight,ez_device_info->network_weight,NETWORK_WEIGHT_LEN )
			|| ez_device_info->network_weight[0]== 0x0F) && differ_dhcp_handling)
	{
		printf("Raghav: expected wt acheived\n");
		
		memcpy(last_stable_weight, ez_device_info->network_weight, NETWORK_WEIGHT_LEN);
		memcpy(&last_stable_node_number, &ez_device_info->node_number, sizeof(EZ_NODE_NUMBER));
		if(eloop_is_timeout_registered(Handle_Differed_DHCP ,NULL,NULL))
			eloop_cancel_timeout(Handle_Differed_DHCP ,NULL,NULL);
			Handle_Differed_DHCP();
                flag = 1;
	}
#endif
	if(flag == 0) // don't need restart DHCP for configuraion push by this device to peer during connecton 
	Handle_DHCP(&ez_device_info->node_number);
	flag = 0;
#ifdef IPHONE_SUPPORT
	memcpy(last_stable_weight, ez_device_info->network_weight, NETWORK_WEIGHT_LEN);
	memcpy(&last_stable_node_number, &ez_device_info->node_number, sizeof(EZ_NODE_NUMBER));
#endif	
	is_non_ez_connection = ez_device_info->is_non_ez_connection;
	DBGPRINT(DEBUG_OFF, "NON EZ CONNECTION???????????????? %d\n", is_non_ez_connection);
	if (ez_device_info->update_parameters)
	{
		memset(system_ssid1, 0, sizeof(system_ssid1));
		memcpy(system_ssid1, ez_device_info->ssid, ez_device_info->ssid_len);

		memset(command, 0, sizeof(command));
                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
		sprintf(command, "wificonf set EzDefaultSsid \"%s\"", system_ssid1);
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);
		
		convert_hex_pmk_to_string(ez_device_info->pmk,sys_pmk_string);

		memset(command, 0, sizeof(command));
		sprintf(command, "wificonf set EzDefaultPmk \"%s\"", sys_pmk_string);
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);

		memset(command, 0, sizeof(command));
		sprintf(command,"wificonf set EzDefaultPmkValid 1");
		system(command);
		
	} else {
		DBGPRINT(DEBUG_OFF, "No need to update EZ parameters\n");
	}
	
	if (ez_device_info->need_non_ez_update_ssid[0])
	{
		memset(system_ssid2, 0, sizeof(system_ssid2));
		memcpy(system_ssid2, ez_device_info->non_ez_ssid1, ez_device_info->non_ez_ssid1_len);

		memset(command, 0, sizeof(command));
                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
		sprintf(command, "wificonf set SSID1 \"%s\"",system_ssid2); 
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);

	}

	if (ez_device_info->need_non_ez_update_ssid[1])
	{

		memset(system_ssid3, 0, sizeof(system_ssid3));
		memcpy(system_ssid3, ez_device_info->non_ez_ssid2, ez_device_info->non_ez_ssid2_len);	
		memset(command, 0, sizeof(command));	
                switch_profile(get_nvram_zone(input_device_info[2].ap_interface));
		sprintf(command, "wificonf set SSID1 \"%s\"", system_ssid3);
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);
	}
	
	
	get_connected_ap_mac(apclii0_peer_mac,input_device_info[0].cli_interface); // Arvind revisit //Vineet Done

#ifdef MOBILE_APP_SUPPORT

	if (ez_device_info->third_party_present)
	{
#ifndef STAR_ALWAYS
		DBGPRINT(DEBUG_OFF, "Third party Available in network, avoid any further roam\n");
		try_roaming_count = 0;
#else
		is_third_party_present = 1;
#endif
	}	
	else 
	{
#ifdef STAR_ALWAYS
		is_third_party_present =0;
		try_roaming_count = 3;
#endif
	}
		
	
	if (ez_device_info->new_updated_received)
		{
		DBGPRINT(DEBUG_OFF, "new_updates_received, try roaming again\n");
		try_roaming_count = 3;
	}
#ifdef STAR_ALWAYS
if(!MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR) && try_roaming_count != 0)
#else

	if(!MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR) 
		&& (ez_device_info->network_weight[0] != 0xF)
		&& !ez_device_info->third_party_present
		&& !roaming_scheduled
		&& try_roaming_count != 0)
#endif
			{
		eloop_register_timeout(3, 0, try_roaming, NULL, NULL);
	}
#endif
	if (ez_device_info->update_parameters)
	{
		update_nvram(system_ssid1, system_ssid2, system_ssid3);
	}
}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_triband_nonez_driver_event_handle(char *buf)
{
	triband_nonez_device_info_to_app_t *ez_device_info = (triband_nonez_device_info_to_app_t *)buf;
	char command[200];
	char psk_local[LEN_PSK + 1];

	DBGPRINT(DEBUG_OFF, "%s\n", __FUNCTION__);

	if (ez_device_info->need_non_ez_update_psk[0])
	{
		memset(psk_local, 0, sizeof(psk_local));
		memset(command, 0, sizeof(command));
		memcpy(psk_local, ez_device_info->non_ez_psk1, LEN_PSK);
                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
		sprintf(command, "wificonf set WPAPSK1 \"%s\"",psk_local);
		system(command);
		DBGPRINT(DEBUG_TRACE, "%s\n", command);
	}

	if (ez_device_info->need_non_ez_update_psk[1])
	{
		memset(psk_local, 0, sizeof(psk_local));
		memset(command, 0, sizeof(command));	
		memcpy(psk_local, ez_device_info->non_ez_psk2, LEN_PSK);
                switch_profile(get_nvram_zone(input_device_info[2].ap_interface));
		sprintf(command, "wificonf set WPAPSK1 \"%s\"", psk_local);
		system(command);
		DBGPRINT(DEBUG_TRACE, "%s\n", command);
	}

	
	if (ez_device_info->need_non_ez_update_secconfig[0])
	{
		memset(command, 0, sizeof(command));
                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
                sprintf(command, "wificonf set EzDefaultSettings \"1\"");
		sprintf(command, "wificonf set AuthMode \"%s\"",ez_device_info->non_ez_auth_mode1);
		system(command);
		DBGPRINT(DEBUG_TRACE, "%s\n", command);

		memset(command, 0, sizeof(command));	
		sprintf(command, "wificonf set EncrypType \"%s\"", ez_device_info->non_ez_encryptype1);
		system(command);
		DBGPRINT(DEBUG_TRACE, "%s\n", command);

	}

	if (ez_device_info->need_non_ez_update_secconfig[1])
	{
		memset(command, 0, sizeof(command));	
                switch_profile(get_nvram_zone(input_device_info[2].ap_interface));
		sprintf(command, "wificonf set AuthMode \"%s\"", ez_device_info->non_ez_auth_mode2);
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);

		memset(command, 0, sizeof(command));	
		sprintf(command, "wificonf set EncrypType \"%s\"", ez_device_info->non_ez_encryptype2);
		system(command);
		DBGPRINT(DEBUG_OFF, "%s\n", command);

	}

}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_plus_nonman_ez_driver_event_handle(char *buf)
{
	man_plus_nonman_ez_device_info_to_app_t *ez_device_info = (man_plus_nonman_ez_device_info_to_app_t *)buf;
	unsigned char sys_pmk_string[65];
	char command[200];
	int Ez_index;
	unsigned char Ez_ap_int_zone[INTERFACE_LEN];

	Handle_DHCP(&ez_device_info->node_number);

	is_non_ez_connection = ez_device_info->is_non_ez_connection;
	DBGPRINT(DEBUG_TRACE, "NON EZ CONNECTION???????????????? %d\n", is_non_ez_connection);

	memset(system_ssid1, 0, sizeof(system_ssid1));
	memcpy(system_ssid1, ez_device_info->ssid, ez_device_info->ssid_len);

	if(input_device_info[0].Ez_status == '1')
		Ez_index = 0;
	else
		Ez_index = 1;

	memcpy(&Ez_ap_int_zone, get_nvram_zone(input_device_info[Ez_index].ap_interface), strlen(get_nvram_zone(input_device_info[Ez_index].ap_interface)));

	if (ez_device_info->update_parameters)
	{
		memset(command, 0, 200);
                switch_profile(Ez_ap_int_zone);
		sprintf(command, "wificonf set EzDefaultSsid \"%s\"",system_ssid1);				
		system(command);
	
		convert_hex_pmk_to_string(ez_device_info->pmk,sys_pmk_string);

		memset(command, 0, 200);
		sprintf(command, "wificonf set EzDefaultPmk \"%s\"", sys_pmk_string);
		system(command);	

		memset(command, 0, sizeof(command));
		sprintf(command,"wificonf set EzDefaultPmkValid 1");
		system(command);
		
	}	

	get_connected_ap_mac(apclii0_peer_mac,input_device_info[0].cli_interface); // Arvind revisit //Vineet Done

#ifdef MOBILE_APP_SUPPORT
	if (ez_device_info->third_party_present)
	{
		DBGPRINT(DEBUG_OFF, "Third party Available in network, avoid any further roam\n");
		try_roaming_count = 0;
	}
	
	if (ez_device_info->new_updated_received)
	{
		DBGPRINT(DEBUG_OFF, "new_updates_received, try roaming again\n");
		try_roaming_count = 3;
	}

	if(!MAC_ADDR_EQUAL(apclii0_peer_mac, ZERO_MAC_ADDR) 
		&& !ez_device_info->third_party_present
		&& !roaming_scheduled
		&& try_roaming_count != 0)
	{
		eloop_register_timeout(3, 0, try_roaming, NULL, NULL);
	}
#endif
	if (ez_device_info->update_parameters)
	{
		update_nvram(system_ssid1, NULL, NULL);
			}
		}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID, PSK,AuthMode and EncrypType for non easy band .
        This is for one EASY and one NON EASY band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_plus_nonman_nonez_driver_event_handle(char *buf)
{
	man_plus_nonman_nonez_device_info_to_app_t *ez_device_info = (man_plus_nonman_nonez_device_info_to_app_t *)buf;
	char command[200];
	char psk_local[LEN_PSK + 1];
	int nEz_index;
	unsigned char nEz_ap_int_zone[INTERFACE_LEN];

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	memset(system_ssid2, 0, sizeof(system_ssid2));
	memcpy(system_ssid2, ez_device_info->non_ez_ssid, ez_device_info->non_ez_ssid_len);

	memset(psk_local, 0, sizeof(psk_local));
	memcpy(psk_local, ez_device_info->non_ez_psk, LEN_PSK);

	if(input_device_info[0].Ez_status == '0')
		nEz_index = 0;
		else
		nEz_index = 1;
	
	
	memcpy(&nEz_ap_int_zone, get_nvram_zone(input_device_info[nEz_index].ap_interface), strlen(get_nvram_zone(input_device_info[nEz_index].ap_interface)));	
	
	/*	update SSID, PSK, AuthMode and EncrypType on nvram */
	

	if (ez_device_info->need_non_ez_update_ssid) {
		memset(command, 0, 200);
                switch_profile(nEz_ap_int_zone);
		sprintf(command, "wificonf set SSID1 \"%s\"", system_ssid2);
		system(command);
	}
	if (strlen( ez_device_info->non_ez_psk) < 8) {
		memcpy(ez_device_info->non_ez_psk, "12345678", strlen("12345678"));
	}

	if (ez_device_info->need_non_ez_update_psk) {
		memset(command, 0, 200);	
                switch_profile(nEz_ap_int_zone);
		sprintf(command, "wificonf set WPAPSK1 \"%s\"", psk_local);
		system(command);
	}
	if (ez_device_info->need_non_ez_update_secconfig) {
		if ((strlen(ez_device_info->non_ez_auth_mode) > 1) && (strlen(ez_device_info->non_ez_encryptype) > 1)) {
			memset(command, 0, 200);	
                        switch_profile(nEz_ap_int_zone);
			sprintf(command, "wificonf set AuthMode \"%s\"", ez_device_info->non_ez_auth_mode);
			system(command);
			
			memset(command, 0, 200);	
			sprintf(command, "wificonf set EncrypType \"%s\"", ez_device_info->non_ez_encryptype);
			system(command);
		}
	}

	/*	update SSID, PSK, AuthMode and EncrypType in driver*/
	memset(command, 0, 200);
	sprintf(command, "iwpriv %s set ez_set_ssid_psk=\"%s;%s;%s;%s\"", input_device_info[nEz_index].ap_interface, system_ssid2,
			psk_local, ez_device_info->non_ez_encryptype, ez_device_info->non_ez_auth_mode);	
	system(command);
	DBGPRINT(DEBUG_OFF, "%s\n", command);

	/*	update ftmdid in driver*/
	memset(command, 0, 200);
	sprintf(command, "iwpriv %s set ez_set_ftmdid=\"%s\"", input_device_info[nEz_index].ap_interface, ez_device_info->ftmdid);
	system(command);
	DBGPRINT(DEBUG_OFF, "%s\n", command);

}

/*
    ========================================================================
    Routine Description:
        handle driver event  with new configuration for update SSID & PMK .
        This is for single band and dual band.

    Arguments:
        buf         - contains device information with update data.

    Return Value:
       void

    ========================================================================
*/
void man_driver_event_handle(char *buf)
{
	device_config_to_app_t *device_info = (device_config_to_app_t *)buf;
	char command[200];
	int ret;
	static char flag =0;

#ifdef  REGROUP_SUPPORT
		if(dedicated_regrp_app)
		{
		UINT8 is_prev_master_cand=0;
		p_repeater_list_struct p_own_rept = &g_own_rept_info;
		is_prev_master_cand = is_regrp_master_candidate(p_own_rept);
	
		
		memcpy(p_own_rept->network_weight, 
			device_info->network_weight, NETWORK_WEIGHT_LEN);
		memcpy(&p_own_rept->node_number,&device_info->node_number, sizeof(EZ_NODE_NUMBER));
	
		hex_dump("Network Weight",p_own_rept->network_weight,NETWORK_WEIGHT_LEN);
	
		hex_dump("Node Number",(UINT8 *)&p_own_rept->node_number, sizeof(EZ_NODE_NUMBER));
		p_own_rept->non_ez_connection = device_info->non_ez_connection;
		printf("is master = %d\n", p_own_rept->is_master);
		printf("%d: %d %d:\n",is_regrp_master_candidate(p_own_rept),p_own_rept->regrp_state,is_prev_master_cand);
		regrp_clean_all_states();
		if(is_regrp_master_candidate(p_own_rept) && (p_own_rept->regrp_state == REGRP_STATE_DONE))
			{
			printf("%s:Restart Regroup\n",__func__);
			eloop_register_timeout(RESTART_REGROUP_TIME, 0, restart_regroup,NULL, NULL);
			g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
			trigger_regrp = 0;
			}
		if(is_prev_master_cand == 0 && is_regrp_master_candidate(p_own_rept))
		{
			// this is created master from slave. Trigger regroup from periodic exec.
			printf("device became a master\n");
			trigger_regrp = 1;
			os_get_time(&last_entry_added_time);
		}	
		if(!is_regrp_master_candidate(p_own_rept))
		{
			p_own_rept->is_master = 0;
			p_own_rept->regrp_state = REGRP_SLAVE_STATE_IDLE;
	}
}
#endif

#ifdef IPHONE_SUPPORT
#ifdef REGROUP_SUPPORT
	if(regrp_dhcp_defer)
	{
		if(regrp_is_target_wt_same(regrp_expected_wt_1, regrp_expected_wt_2))
		{
			printf("Same wt acheived. Handle dhcp now %d.\n",regrp_disconnect_sta);
			memcpy(expected_weight, g_own_rept_info.network_weight, NETWORK_WEIGHT_LEN);
			memcpy(last_stable_weight, device_info->network_weight, NETWORK_WEIGHT_LEN);
			memcpy(&last_stable_node_number, &device_info->node_number, sizeof(EZ_NODE_NUMBER));
			if(eloop_is_timeout_registered(Handle_Differed_DHCP ,NULL,NULL))
				eloop_cancel_timeout(Handle_Differed_DHCP ,NULL,NULL);
			if(regrp_disconnect_sta == TRUE)
				Handle_Differed_DHCP();
			else
				printf("Raghav:Don't handle dhcp\n");
			regrp_clear_dhcp_defer();
			differ_dhcp_handling = 0;
			flag = 1;
		}
	}
#endif
	if (flag == 0 && device_info->is_forced && !differ_dhcp_handling)
	{
		if(last_stable_weight[0] == 0x0F && device_info->network_weight[0]== 0x0F)
		{
			printf("Wt changed from 0xf to 0xf. Don't start DHCP\n");
			flag =1;
		}
		else
	{
		printf("Start differed DHCP handling\n");
		differ_dhcp_handling = 1;
		memcpy(expected_weight, last_stable_weight, NETWORK_WEIGHT_LEN);
		ret = eloop_register_timeout(20, 0, Handle_Differed_DHCP, NULL, NULL);
	}
	}
	if(flag == 0 && (!memcmp(expected_weight,device_info->network_weight,NETWORK_WEIGHT_LEN )
		|| device_info->network_weight[0]== 0x0F) && differ_dhcp_handling)
{
		printf("Raghav: expected wt acheived\n");
		
		memcpy(last_stable_weight, device_info->network_weight, NETWORK_WEIGHT_LEN);
		memcpy(&last_stable_node_number, &device_info->node_number, sizeof(EZ_NODE_NUMBER));
		if(eloop_is_timeout_registered(Handle_Differed_DHCP ,NULL,NULL))
			eloop_cancel_timeout(Handle_Differed_DHCP ,NULL,NULL);
			Handle_Differed_DHCP();
		flag = 1;
	}
#endif

	if(flag == 0 && !device_info->is_push) // don't need restart DHCP for configuraion push by this device to peer during connecton 
		Handle_DHCP(&device_info->node_number);
	flag = 0;

#ifdef IPHONE_SUPPORT
	memcpy(last_stable_weight, device_info->network_weight, NETWORK_WEIGHT_LEN);
	memcpy(&last_stable_node_number, &device_info->node_number, sizeof(EZ_NODE_NUMBER));
#endif	

	if(device_info->non_ez_connection)
		non_ez_connection = 1;
	else 
		non_ez_connection = 0;

	device_connected_0 = is_get_connected(input_device_info[0].cli_interface);
	device_connected_1 = is_get_connected(input_device_info[1].cli_interface);
	DBGPRINT(DEBUG_OFF, "\n device_connected_0 : %d, device_connected_0 : %d\n\n", device_connected_0, device_connected_1);	

	memcpy(Peer2p4mac, device_info->peer2p4mac, MAC_ADDR_LEN);

	/* save latest SSID on nvram zone for default use on device next bootup*/
	if(memcmp(device_info->ssid1, sys_ssid1, device_info->ssid_len1)|| memcmp(device_info->ssid2, sys_ssid2, device_info->ssid_len2)
		|| (device_info->ssid_len1 != strlen(sys_ssid1)) || (device_info->ssid_len2 != strlen(sys_ssid2)))
	{
		memset(sys_ssid1, 0, sizeof(sys_ssid1));
		memcpy(sys_ssid1, device_info->ssid1, device_info->ssid_len1);
		memset(sys_ssid2, 0, sizeof(sys_ssid2));
		memcpy(sys_ssid2, device_info->ssid2, device_info->ssid_len2);
		memset(command, 0, 100);

		if(cmd == SINGLE_BAND) {
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf set EzDefaultSsid \"%s\"", sys_ssid1);
			system(command);
		} else {
			if(!strcmp(get_nvram_zone(input_device_info[0].ap_interface), get_nvram_zone(input_device_info[1].ap_interface))) {
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
                                printf("%s %d wificonf set EzDefaultSsid %s;%s\n", __FUNCTION__,__LINE__, sys_ssid1, sys_ssid2);
				sprintf(command, "wificonf set EzDefaultSsid \"%s;%s\"", sys_ssid1, sys_ssid2);
				system(command);
			} else {
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultSsid \"%s\"", sys_ssid1);
                                printf("%s %d wificonf set EzDefaultSsid %s\n", __FUNCTION__,__LINE__,  sys_ssid1);
				system(command);
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
                                printf("%s %d wificonf set EzDefaultSsid %s\n", __FUNCTION__,__LINE__,  sys_ssid2);
				sprintf(command, "wificonf set EzDefaultSsid \"%s\"", sys_ssid2);
				system(command);
			}
	        }
                DBGPRINT(DEBUG_OFF, "\n %s %d system call is \n%s\n",__FUNCTION__,__LINE__, command);
	
		/*  update sys_ssid for sending updated ssid to mobile android/iphone app */
		memset(sys_ssid, 0, 65);
		
		if(cmd == SINGLE_BAND) {
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
		} 
		else {
			memcpy(sys_ssid, sys_ssid1, strlen(sys_ssid1));
			memcpy(sys_ssid + strlen(sys_ssid1), ":", 1);
			memcpy(sys_ssid + strlen(sys_ssid1) + 1, sys_ssid2, strlen(sys_ssid2)); 
		}
		
		update_nvram(sys_ssid1, sys_ssid2, NULL);
	}

	/* save latest PMK on nvram zone for default use on device next bootup*/
	if(memcmp(device_info->pmk1, sys_pmk1, LEN_PMK) || memcmp(device_info->pmk2, sys_pmk2, LEN_PMK))
	{
		memcpy(sys_pmk1, device_info->pmk1, LEN_PMK);
		memcpy(sys_pmk2, device_info->pmk2, LEN_PMK);	
		for (ret = 0; ret < 64;  ret++)
		{
			sys_pmk_string1[ret] = (ret % 2) ? (sys_pmk1[ret / 2]  & 0xf) : ((sys_pmk1[ret / 2] & 0xf0 ) >> 4);
			if(sys_pmk_string1[ret] < 10)	
				sys_pmk_string1[ret] += '0';
			else 
				sys_pmk_string1[ret] += 'a' - 10;
		}
		sys_pmk_string1[64] = '\0';
		DBGPRINT(DEBUG_ERROR, "device PMK = %s\n", sys_pmk_string1);

		for (ret = 0; ret < 64;  ret++)
	{
			sys_pmk_string2[ret] = (ret % 2) ? (sys_pmk2[ret / 2]  & 0xf) : ((sys_pmk2[ret / 2] & 0xf0 ) >> 4);
			if(sys_pmk_string2[ret] < 10)	
				sys_pmk_string2[ret] += '0';
			else 
				sys_pmk_string2[ret] += 'a' - 10;
		}
		sys_pmk_string2[64] = '\0';
		DBGPRINT(DEBUG_TRACE, "device PMK = %s\n", sys_pmk_string2);
		

		if(cmd == SINGLE_BAND) {
			memset(command, 0, 100);
                        switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
			sprintf(command, "wificonf set EzDefaultPmk \"%s\"", sys_pmk_string1);
			system(command);

			memset(command, 0, 100);
			sprintf(command, "wificonf set EzDefaultPmkValid \"1\"");
			system(command);
		}
		else { // for dual band
			if(!strcmp(get_nvram_zone(input_device_info[0].ap_interface), get_nvram_zone(input_device_info[1].ap_interface))) { // Arvind use MACRO for strcmp
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultPmk \"%s;%s\"", sys_pmk_string1, sys_pmk_string2);
				system(command);

				memset(command, 0, 100);
				sprintf(command, "wificonf set EzDefaultPmkValid \"1;1\"");
				system(command);
	} else {

				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
				sprintf(command, "wificonf set EzDefaultPmk \"%s\"", sys_pmk_string1);
				system(command);

				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
 				sprintf(command, "wificonf set EzDefaultPmk \"%s\"", sys_pmk_string2);
				system(command);

				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[0].ap_interface));
 				sprintf(command, "wificonf set EzDefaultPmkValid \"1\"");
				system(command);
				memset(command, 0, 100);
                                switch_profile(get_nvram_zone(input_device_info[1].ap_interface));
 				sprintf(command, "wificonf set EzDefaultPmkValid \"1\"");
				system(command);
				memset(command, 0, 100);
			}  
		}
	}

#ifdef MOBILE_APP_SUPPORT

	if(device_info->is_push) {
		DBGPRINT(DEBUG_OFF, " This is form push, dont required roam \n");
#ifndef STAR_ALWAYS
		try_roaming_count = 0;
#endif		
		return;
	}
	
	if(cmd == DUAL_BAND ) {
		is_2G_connected = is_get_connected(input_device_info[0].cli_interface);
		is_5G_connected = is_get_connected(input_device_info[1].cli_interface);
	}
	if (device_info->third_party_present)
	{
#ifndef STAR_ALWAYS
		DBGPRINT(DEBUG_OFF, "Third party Available in network, avoid any further roam\n");
#ifdef SINGLE_BACKHAUL_LINK
		roam_triggered_on_5_g = 0;
		roam_triggered_on_2_g = 0;
#endif		
		try_roaming_count = 0;
#endif
		is_third_party_present =1;
	}
	else
	{
		is_third_party_present=0;
#ifdef STAR_ALWAYS
		try_roaming_count = 5;
#endif
	}
	if (device_info->new_updated_received)
	{
		DBGPRINT(DEBUG_OFF, "new_updates_received, try roaming again\n");
		try_roaming_count = 5;
//		try_5G_roam_count = 2;
		extra_attempt_to_5g = 1;
	}	

	if(is_5G_connected)
	{
		try_roam_for = Band_5G;
//		--try_5G_roam_count;
	}	
	else if(is_2G_connected)
	{
		try_roam_for = Band_2G;
	}
	
	if(!is_2G_connected && !is_5G_connected)
	{
		DBGPRINT(DEBUG_OFF, "Both apcli not connected, Dont need roam...\n");
		return;
	}

#ifdef STAR_ALWAYS
	if((is_2G_connected || is_5G_connected)
		&& (non_ez_connection == 0)
		&& try_roaming_count != 0)
#else
	if((is_2G_connected || is_5G_connected)
		&& !device_info->third_party_present
		&& !(device_info->network_weight[0] == 0xf)
		&& !roaming_scheduled
		&& try_roaming_count != 0)
#endif
	{
		eloop_register_timeout(3, 0, try_roaming, NULL, NULL);
	}
#endif
}

/*
    ========================================================================
    Routine Description:
        broacast data on port 5001.

    Arguments:
        buffer         - data for broadcast 

    Return Value:
       void

    ========================================================================
*/
void man_conf_event_handle(char *buffer)
{
	int sock0, status, sinlen;
	struct sockaddr_in sock_in;
	char buf[250];
	int yes = 1, buf_len;
	web_conf_info_t *conf_info = (web_conf_info_t *)buffer;		
	char netif0[] = "br-lan";
			
	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);

	sock0 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);				
	setsockopt(sock0, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
	setsockopt(sock0, SOL_SOCKET, SO_BINDTODEVICE, netif0, sizeof(netif0));

	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK 
	*/ 
	sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
	sock_in.sin_port = htons(PORT1); /* port number */
	sock_in.sin_family = AF_INET;

	memset(buf, 0, 250);
	memcpy(&buf[0],"5001", TAG_LEN);
	memcpy(&buf[TAG_LEN], conf_info->data, conf_info->data_len);
	buf_len = TAG_LEN + conf_info->data_len;

	status = sendto(sock0, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);				

	close(sock0);
	shutdown(sock0, 2);
}

unsigned char *NewGroupId;
unsigned char *NewGroupIdstring;
unsigned char lastcount = 0;
unsigned int lastindex = 0;

int update_group_id(char *data)
{
		ez_group_id_t *group_id = (ez_group_id_t *)data;
		char *command;
		int i;
		unsigned int size = 60;
		/* size variable is maximum extra byte in command (exept ez_group_id_len) 
			e.g. nvram_set <interface> EzGenGroupID <value>
			nvram_set =9 byte
			ra0/apcli0 = 3/6 byte
			EzGenGroupID = 12 byte
		*/
		if((group_id->ucFlags & 0x7f) == 1)
		{
			if (NewGroupId == NULL)
			{
				NewGroupId = os_malloc(group_id->ez_group_id_len+1);
				if (NewGroupId)
					memset(NewGroupId, 0,group_id->ez_group_id_len+1);
				else
				{
					lastindex = 0;
					lastcount = 0;
					DBGPRINT(DEBUG_OFF, " Memory Allocation failure Group ID Update Fail \n");
					return;
				}
			}
			memcpy(&NewGroupId[lastindex], group_id->ez_group_id, group_id->ez_group_id_len);
			NewGroupId[lastindex + group_id->ez_group_id_len] = '\0';

			command = os_malloc(lastindex+group_id->ez_group_id_len + size);
			if(group_id->ucFlags & 0x80)
			{

				DBGPRINT(DEBUG_OFF, " New Gen Seed %s\n",NewGroupId);

				for(i = 0; i <(No_of_Band[0]-'0'); i++)
				{
					if(input_device_info[i].Ez_status == '1')
					{
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						switch_profile(get_nvram_zone(input_device_info[i].ap_interface));
						sprintf(command, "wificonf set EzOpenGroupID \"%s\"",group_id->open_group_id);
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzOpenGroupID \"%s\"",group_id->open_group_id);
						system(command);
						
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set EzGenGroupID \"%s\"",NewGroupId);
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzGenGroupID \"%s\"",NewGroupId);
						system(command);
						
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set EzGroupID \"\"");
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzGroupID \"\"");
						system(command);
					}
				}
			}
			else
			{
				DBGPRINT(DEBUG_OFF, " New Group Id %s\n",NewGroupId);

				for(i = 0; i <(No_of_Band[0]-'0'); i++)
				{
					if(input_device_info[i].Ez_status == '1')
					{
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						switch_profile(get_nvram_zone(input_device_info[i].ap_interface));
						sprintf(command, "wificonf set EzOpenGroupID \"%s\"",group_id->open_group_id);
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzOpenGroupID \"%s\"",group_id->open_group_id);
						system(command);											

						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set EzGroupID \"%s\"",NewGroupId);
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzGroupID \"%s\"",NewGroupId);
						system(command);
						
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set EzGenGroupID \"\"");
						system(command);
						memset(command, 0, lastindex+group_id->ez_group_id_len + size);
						sprintf(command, "wificonf set ApCliEzGenGroupID \"\"");
						system(command);
					}
				}
			}
			os_free(command);

			lastindex = 0;
			lastcount = 0;

			if(NewGroupId)
			{
				os_free(NewGroupId);
				NewGroupId = NULL;
			}
		}
		else
		{
			if(NewGroupId && (lastcount != ((group_id->ucFlags & 0x7f) + 1)))
			{
				os_free(NewGroupId);
				NewGroupId = NULL;
				NewGroupId = os_malloc(GROUPID_LEN_BUF * (group_id->ucFlags & 0x7f));
				if (NewGroupId)
					memset(NewGroupId, 0,GROUPID_LEN_BUF * (group_id->ucFlags & 0x7f));
				else
				{
					lastindex = 0;
					lastcount = 0;
					DBGPRINT(DEBUG_OFF, " Memory Allocation failure Group ID Update Fail \n");
					return;
				}
			}
			else if (NewGroupId == NULL)
			{
				NewGroupId = os_malloc(GROUPID_LEN_BUF * (group_id->ucFlags & 0x7f));
				if (NewGroupId)
					memset(NewGroupId, 0,GROUPID_LEN_BUF * (group_id->ucFlags & 0x7f));
				else
				{
					lastindex = 0;
					lastcount = 0;
					DBGPRINT(DEBUG_OFF, " Memory Allocation failure Group ID Update Fail \n");
					return;
				}
			}
			lastcount = (group_id->ucFlags & 0x7f);

			memcpy(&NewGroupId[lastindex], group_id->ez_group_id, group_id->ez_group_id_len);
			lastindex += group_id->ez_group_id_len;

		}
}
/*
    ========================================================================
    Routine Description:
        Periodic evaluate antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
       int

    ========================================================================
*/
int main(int argc, char *argv[])
{
	char no_of_bands_input, band_with_EzStatus[BAND_WITH_EZSTATUS_LEN];
	int ret=0, i=0, argument_no, index_no;
	char c;
	char nvram_input_args[10][10];
	int loop_var=0;
	pid_t child_pid;
	FILE *fp;
	char command[150];
	int nvram_input_index=0;

        get_chip_combo();

        set_ManConf_default2dat();
        
	memset(command,0,150);
        switch_profile("2860");
	sprintf(command, "wificonf get ManConf > ManDeamon_conf");
	system(command);		
	fp = fopen("ManDeamon_conf", "r");
	memset(nvram_input_args, 0, sizeof(nvram_input_args));
	while(! feof(fp))
	{
		c = fgetc(fp);
		int temp=0;
		while((c != ' ') && (!feof(fp))&& (c != '\n'))
		{
			nvram_input_args[nvram_input_index][temp] = c;
			temp++;
			c = fgetc(fp);
		}
		nvram_input_index++;
		if(c == '\n')
			break;
	}
	fclose(fp);
	no_of_bands_input=nvram_input_args[0][0];

	DBGPRINT(DEBUG_OFF, "arguments = %d, no_of_bands = %c\n", nvram_input_index, no_of_bands_input);

	if ((nvram_input_index < 4) || (nvram_input_index != (((no_of_bands_input - '0') * 3) + 1)))
	{
		DBGPRINT(DEBUG_OFF, "INSUFFICIENT ARGUMENTS arguments = %d, no_of_bands = %c\n", (nvram_input_index), no_of_bands_input);
		return 0;
	}


	is_triband = FALSE;
	is_man_nonman = FALSE;
	
	switch(no_of_bands_input)
	{
		case '1':
			
			input_device_info[0].band = nvram_input_args[1][0];
			if((nvram_input_args[1][1] == 'E') || (nvram_input_args[1][1] == 'e'))
			{
				input_device_info[0].Ez_status = '1';
			}
                        else
			{
				DBGPRINT(DEBUG_ERROR, "Wrong Input... Enter Easy setup band info !!\n");				
				return 0;
			}		
			memcpy(&input_device_info[0].ap_interface[0], &nvram_input_args[2], strlen(nvram_input_args[2]));
			memcpy(&input_device_info[0].cli_interface[0], &nvram_input_args[3], strlen(nvram_input_args[3]));
			cmd = SINGLE_BAND;	
			DBGPRINT(DEBUG_OFF, "INT0 %s %s \n", &input_device_info[0].ap_interface[0], 
													&input_device_info[0].cli_interface[0]);
			
		break;				
		case '2':

			for(i=0; i<2; i++)
			{
				/* argv[2][0] will be band for first interface and argv[5][0] for second second as per user input
				 to select which index of input_device_info will have to be updated*/

				argument_no = (3*i) + 1; 

				/* 2.4G band info will be store on index 0 and 5G band on index 1*/				
				if(nvram_input_args[argument_no][0] == '2')    
					index_no = 0;
				else if(nvram_input_args[argument_no][0] == '5')
					index_no = 1;
				else {
					DBGPRINT(DEBUG_ERROR, " Worng band %c entered !! Band support 2G or 5G only ....\n",nvram_input_args[argument_no][0]);					
					return 0;
				}

				input_device_info[index_no].band = nvram_input_args[argument_no][0];
				
				
				if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
				{
					input_device_info[index_no].Ez_status = '1';
				}
	                        else
				{
					input_device_info[index_no].Ez_status = '0';
				}	
				DBGPRINT(DEBUG_OFF, "%s %s \n", nvram_input_args[argument_no + 1], nvram_input_args[argument_no + 2]);
				memcpy(&input_device_info[index_no].ap_interface[0], nvram_input_args[argument_no + 1], strlen(nvram_input_args[argument_no + 1]));
				memcpy(&input_device_info[index_no].cli_interface[0], nvram_input_args[argument_no + 2], strlen(nvram_input_args[argument_no + 2]));
				
			}

			DBGPRINT(DEBUG_OFF, "INT0 %s %s \n INT1 %s %s \n", 
						&input_device_info[0].ap_interface[0], &input_device_info[0].cli_interface[0], 
						&input_device_info[1].ap_interface[0], &input_device_info[1].cli_interface[0]);
			if(((input_device_info[0].Ez_status == '0') && (input_device_info[1].Ez_status == '1'))
				|| ((input_device_info[0].Ez_status == '1') && (input_device_info[1].Ez_status == '0'))) {

		                cmd = DUAL_BAND_MAN_NONMAN;
				is_man_nonman = TRUE;
			}	
			else if((input_device_info[0].Ez_status == '1') && (input_device_info[1].Ez_status == '1')) {
				cmd = DUAL_BAND;
			}
			else {
				DBGPRINT(DEBUG_ERROR, "Entered configuration is not supported !!\n");				
				return 0;
			}	

		break;			
		case '3':

			for(i=0; i<3; i++)
			{			
				/* argv[2][0] will be band for first interface and argv[5][0] for second second as per user input
				 to select which index of input_device_info will have to be updated*/

				argument_no = (3*i) + 1;  

				/* 5G EZ band will be store on index 0 , 2.4G NEZ band on index 1 and 5G NEZ band on index 1*/								
				if(nvram_input_args[argument_no][0] == '5') {
					if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
						index_no = 0;
					else
						index_no = 2;
				}
				else if(nvram_input_args[argument_no][0] == '2') {
					index_no = 1;
				}				
				else {
					DBGPRINT(DEBUG_ERROR, " Worng band %c entered !! Band support 2G or 5G only ....\n", nvram_input_args[argument_no][0]); 				
					return 0;
				}
				input_device_info[index_no].band = nvram_input_args[argument_no][0];

				if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
				{
					input_device_info[index_no].Ez_status = '1';
				}
	                        else
				{
					input_device_info[index_no].Ez_status = '0';
				}
				memcpy(&input_device_info[index_no].ap_interface[0], nvram_input_args[argument_no + 1], strlen(nvram_input_args[argument_no + 1]));
				memcpy(&input_device_info[index_no].cli_interface[0],nvram_input_args[argument_no + 2], strlen(nvram_input_args[argument_no + 2]));
			}
	
			DBGPRINT(DEBUG_OFF, "INT0 %s %s \n INT1 %s %s \n INT2 %s %s \n", 
									&input_device_info[0].ap_interface[0], &input_device_info[0].cli_interface[0],
									&input_device_info[1].ap_interface[0], &input_device_info[1].cli_interface[0],
									&input_device_info[2].ap_interface[0], &input_device_info[2].cli_interface[0]);
			cmd = TRIBAND_MULTI;
			is_triband = TRUE;
		break;

		default:
			DBGPRINT(DEBUG_OFF, " the no of bands should be 1 or 2 or 3 \n"); 					
				return 0;
	}

#ifdef MOBILE_APP_SUPPORT      //no of band argument is already known	

	memcpy(&No_of_Band, &no_of_bands_input, 1);	
	DBGPRINT(DEBUG_OFF, "No_of_Band : %c\n", No_of_Band[0]);

#endif //MOBILE_APP_SUPPORT
	child_pid = fork();
	if (child_pid == 0) {	
		DBGPRINT(DEBUG_TRACE,"%s %d child process \n",__FUNCTION__, __LINE__);
		 man_init();	
	} 
	else
	{
		return 0;	
	}
	
	return ret;

}



