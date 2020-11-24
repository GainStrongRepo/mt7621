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
#ifndef __MAIN_H__
#define __MAIN_H__

#include "eloop.h"
/*Definitions*/
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif
#ifndef GNU_PACKED
#define GNU_PACKED	__attribute__ ((packed))
#endif // GNU_PACKED //

/*------------------------------------#-defined--------------------------------------------*/
#define MAX_STA_SUPPORT            64
#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           !memcmp((void *)(pAddr1), (void *)(pAddr2), MAC_ADDR_LEN)
#define MOBILE_APP_SUPPORT     1
#define DEDICATED_MAN_AP       1
#define CONFIG_PUSH_VER_SUPPORT 1
#define BND_STEERING 		0x1
#define WNM_802_11_V		0x2
#define RRM_802_11_K		0x3
#define INTERNET_CHK		0x4
#define TAG_LEN			4
#define MAC_ADDR_LEN 				6
#ifdef CONFIG_PUSH_VER_SUPPORT
#define NETWORK_WEIGHT_LEN                              (MAC_ADDR_LEN + 2)
#else
#define NETWORK_WEIGHT_LEN  				(MAC_ADDR_LEN + 1)
#endif
#define MAX_LEN_OF_SSID 			32 
#define LEN_PMK    				32
#define LEN_PSK					64
#define FT_MDID_LEN				2
#define EZ_MAX_DEVICE_SUPPORT   7
#define FALSE 					0
#define TRUE 					1
#define PORT1 5001
#define PORT2 5002
#define PORT3 5003
#define BUFLEN 512  //Max length of buffer
#define Band_2G 				2
#define Band_5G 				5
#define PACKED __attribute__((packed))
#define PRINT_MAC(addr) \
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
		printf(__fmt);	
#define PRINT_MAC(addr) \
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
		printf(__fmt);
#define ROAMING_STATE_INIT							0
#define ROAMING_STATE_TRIGGER_5G_SCAN				1
#define ROAMING_STATE_5G_SCAN_TRIGGERED  			2
#define ROAMING_STATE_5G_SCAN_RESULTS_RECEIVED  	3
#define ROAMING_STATE_TRIGGER_2G_SCAN				4

#define ROAMING_STATE_2G_SCAN_TRIGGERED             5
#define ROAMING_STATE_2G_SCAN_RESULTS_RECEIVED  	6
#define ROAMING_STATE_HALT							7

/*------------------------------------typedef structures--------------------------------------------*/

typedef struct PACKED device_connection_info_s{
	char tag[4];
	char device_mac[6];
	unsigned char device_ssid_len;
	char device_ssid[MAX_LEN_OF_SSID];
	char peer_mac[6];
	unsigned char peer_ssid_len;
	char peer_ssid[MAX_LEN_OF_SSID];
} device_connection_info_t, *p_device_connection_info_t;


typedef struct PACKED device_internet_info_s{
	char tag[4];
	char device_mac[6];
} device_internet_info_t, *p_device_internet_info_t;

typedef struct web_conf_info_s{
	unsigned char data_len;
	char data[250];	
} web_conf_info_t;

typedef  unsigned char UINT8;
#if 0
typedef struct   __attribute__((__packed__)) _ez_node_number {
	unsigned char path_len; //path len is the length of the entire node number including the root_mac
	unsigned char root_mac[MAC_ADDR_LEN];
	unsigned char path[EZ_MAX_DEVICE_SUPPORT];
}EZ_NODE_NUMBER;
#else
typedef struct GNU_PACKED _ez_node_number {
	unsigned char path_len; //path len is the length of the entire node number including the root_mac
	unsigned char root_mac[MAC_ADDR_LEN];
	unsigned char path[EZ_MAX_DEVICE_SUPPORT];
}EZ_NODE_NUMBER;
#endif

typedef  unsigned char UINT8;

typedef struct device_info_to_app_s
{
	
	unsigned char dual_chip_dbdc;
	unsigned char ssid_len;
	unsigned char internet_access;
	char ssid[MAX_LEN_OF_SSID];
	unsigned char pmk[LEN_PMK];
	unsigned char device_connected[2];
	unsigned char no_of_blank_scan_5g;
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer2p4mac[MAC_ADDR_LEN];	
} device_info_to_app_t;
typedef struct station_list_s
{
	unsigned char mac_addr[MAC_ADDR_LEN];
}station_list_t;
typedef struct triband_ez_device_info_to_app_s
{
	unsigned char ssid_len;
	unsigned char internet_access;
	char is_non_ez_connection;
	char ssid[MAX_LEN_OF_SSID];
	char non_ez_ssid1[MAX_LEN_OF_SSID];
	char non_ez_ssid2[MAX_LEN_OF_SSID];
	unsigned char non_ez_ssid1_len;
	unsigned char non_ez_ssid2_len;
	unsigned char need_non_ez_update_ssid[2];
	unsigned char pmk[LEN_PMK];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer_mac[MAC_ADDR_LEN];
	unsigned char is_forced;
	char update_parameters;
	char third_party_present;
	char new_updated_received;
} triband_ez_device_info_to_app_t;

typedef struct triband_non_ez_device_info_to_app_s
{
	unsigned char non_ez_psk1[LEN_PSK];
	unsigned char non_ez_psk2[LEN_PSK];
	unsigned char non_ez_auth_mode1[20];
	unsigned char non_ez_auth_mode2[20];	
	unsigned char non_ez_encryptype1[20];
	unsigned char non_ez_encryptype2[20];		
	unsigned char need_non_ez_update_psk[2];
	unsigned char need_non_ez_update_secconfig[2];
} triband_nonez_device_info_to_app_t;

typedef struct man_plus_nonman_ez_device_info_to_app_s
{
	unsigned char ssid_len;
	unsigned char internet_access;
	char is_non_ez_connection;
	char ssid[MAX_LEN_OF_SSID];
	unsigned char pmk[LEN_PMK];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer_mac[MAC_ADDR_LEN];	
	char update_parameters;
	char third_party_present;
	char new_updated_received;
} man_plus_nonman_ez_device_info_to_app_t;

typedef struct man_plus_nonman_non_ez_device_info_to_app_s
{
	unsigned char non_ez_ssid[MAX_LEN_OF_SSID];
	unsigned char non_ez_ssid_len;
	unsigned char non_ez_psk[LEN_PSK];
	unsigned char non_ez_encryptype[32];
	unsigned char non_ez_auth_mode[32];
	UINT8 ftmdid[FT_MDID_LEN];	
	unsigned char need_non_ez_update_ssid;
	unsigned char need_non_ez_update_psk;
	unsigned char need_non_ez_update_secconfig;
} man_plus_nonman_nonez_device_info_to_app_t;

typedef struct roam_candidate_s
{
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char ez_device;
	char 		  rssi;
	char 		  channel; 
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
}roam_candidate_t;

typedef struct roam_candidate_to_app_s
{
	unsigned int no_of_candidates;
	unsigned char ssid[32];
	unsigned char ssid_length;
	roam_candidate_t roam_candidate[5];	
} roam_candidate_to_app_t;

typedef struct scan_done_results_s
{
	device_info_to_app_t device_info;
	roam_candidate_to_app_t roam_candidates;
} scan_done_results_t;

typedef struct ez_config_data_cmd_s {
	UINT8 ssid_len;
	UINT8 ssid[32];
	UINT8 psk_len;
	UINT8 psk[64];
}ez_config_data_cmd_t, *p_ez_config_data_cmd_t;

typedef struct device_config_to_app_s
{
	unsigned char dual_chip_dbdc;
	unsigned char ssid_len1;
	unsigned char ssid_len2;
	unsigned char internet_access;
	char ssid1[MAX_LEN_OF_SSID];
	char ssid2[MAX_LEN_OF_SSID];
	unsigned char pmk1[LEN_PMK];
	unsigned char pmk2[LEN_PMK];
	unsigned char device_connected[2];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer2p4mac[MAC_ADDR_LEN];
	unsigned char non_ez_connection;
	unsigned char update_parameters;
	unsigned char is_forced;
	unsigned char third_party_present;
	unsigned char new_updated_received;
	unsigned char is_push;	
	unsigned char sta_cnt;
	unsigned char stamac[10][MAC_ADDR_LEN];	
} device_config_to_app_t;

#define BAND_WITH_EZSTATUS_LEN 10
#define INTERFACE_LEN   20
#define MAX_BAND       20

typedef struct device_info_from_user_s                //structure to store inputs from user
{
	char band;
	char Ez_status;
	unsigned char ap_interface[INTERFACE_LEN];
	unsigned char cli_interface[INTERFACE_LEN];
} device_info_from_user_t;
	
/*------------------------------------functions--------------------------------------------*/

int eloop_register_timeout(unsigned int secs, unsigned int usecs,
			   eloop_timeout_handler handler,
			   void *eloop_data, void *user_data);

/*------------------------------------enumeration-----------------------------------------*/
enum {
	SINGLE_BAND = 0x1,
	DBDC_NON_MULTI = 0x2,
	DBDC_MULTI = 0x3,
	DUAL_BAND = 0x4,
	TRIBAND_NON_MULTI = 0x5,
	TRIBAND_MULTI = 0x6,
	DUAL_BAND_MAN_NONMAN = 0x7
};

#define OPEN_GROUP_MAX_LEN		20
#define GROUPID_LEN_BUF		128

typedef struct GNU_PACKED _ez_group_id {
unsigned char ucFlags;
unsigned int open_group_id_len;
unsigned char open_group_id[OPEN_GROUP_MAX_LEN];
unsigned int ez_group_id_len;	//for localy maintain EzGroupID
unsigned char ez_group_id[GROUPID_LEN_BUF + 1]; 	//for localy maintain EzGroupID
}ez_group_id_t;

#endif

