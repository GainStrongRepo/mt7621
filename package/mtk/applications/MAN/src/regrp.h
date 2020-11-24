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
    	regroup.h
*/


#ifndef __REGRP_H__
#define __REGRP_H__
#include "main.h"

/*Types*/

typedef  unsigned char UINT8;
typedef  signed char INT8;
typedef  unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned char UCHAR;

/**************************************************************************************************/
extern char interface_5g[16];
extern char interface_2g[16];


/*Definitions*/
#define PACKED __attribute__((packed))
#ifndef GNU_PACKED
#define GNU_PACKED	__attribute__ ((packed))
#endif // GNU_PACKED //

#define REGROUP_SUPPORT					1
#define EZ_MAX_DEVICE_SUPPORT   		7
#define MAX_VIRTUAL_REPT_NUM 			(EZ_MAX_DEVICE_SUPPORT - 1)
#define MAX_MAC_NUM						2
#define TAG_LEN							4
#define MAC_ADDR_LEN 					6
#define NETWORK_WEIGHT_LEN  			(MAC_ADDR_LEN + 2)
#define MAX_LEN_OF_SSID 				32 
#define LEN_PMK    						32
#define LEN_PSK							64
#define FALSE 							0
#define TRUE 							1
#define PORT1 							5001
#define PORT2 							5002
#define PORT3 							5003
#define PORT_VR							5005

#define REPT_INFO_TAG   		"REPT_INFO"	
#define REPT_INFO_TAG_LEN		strlen(REPT_INFO_TAG) 

#define SNIFF_REQ_TAG   		"SNIFF_REQ"	
#define SNIFF_REQ_TAG_LEN		strlen(SNIFF_REQ_TAG) 


#define BUFLEN 							512  //Max length of buffer
#define Band_2G 						2
#define Band_5G 						5
#define MAX_STA_SUPPORT   		        64
#define MOBILE_APP_SUPPORT     			1
#define DEDICATED_MAN_AP       			1
#define PN_LEN							6
#define MAX_VR_STA_NUM					32
#define MAC_ADDR_EQUAL(pAddr1,pAddr2)   !memcmp((void *)(pAddr1), (void *)(pAddr2), MAC_ADDR_LEN)

#define OID_VIRTUAL_ROAM_COMMAND   					0x0985
#define OID_VIRTUAL_ROAM_EVENT 						0x0986

#define OID_VR_CONFIG_VIRTUAL_INTERFACE       			0x1
#define OID_VR_QUERY_INTERFACE_DETAILS	      			0x2
#define OID_VR_QUERY_NODE_NUMBER_WT	      			0x3


#define NO_BETTER_REPEATER_FOUND 				1
#define RESULTS_INCOMPLETE					    2


#define FIRST_VR_WDEV_ID					  6

#define SCAN_GENERAL	1
#define SCAN_CONNECTED_SSID	2
#define SCAN_SPECIFIED_SSID	3

#define OID_WH_EZ_REGROUP_COMMAND					0x2012
#define OID_WH_EZ_REGROUP_EVENT					0x2013

typedef struct _regrp_threshold_event_t{
	signed char default_rssi_threshold;
	signed char custom_rssi_th;
} regrp_threshold_event_t;

#define	REGROUP_TIMEOUT_SEC					300
#define	SCAN_TIMEOUT_SEC					10
#define RESTART_REGROUP_TIME				30

#define IP_CHANGED							(1 << 0)
#define NODE_NUMBER_CHANGED					(1 << 1)
#define WEIGHT_CHANGED						(1 << 2)


#define DEFAULT_RSSI_THRESHOLD g_default_rssi_threshold //(-73)
#define DEFAULT_MAX_RSSI_THRESHOLD  g_default_max_rssi_threshold//(-86)
#define CUSTOM_RSSI_TH  g_custom_rssi_th//(-68)


#define PACKED __attribute__((packed))

#define PRINT_MAC(addr) \
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
	printf(__fmt);

#define memzero(a,b)		memset(a, 0, b)
/**************************************************************************************************/

/*Enums*/
#if 0
enum {
	SINGLE_BAND = 0x1,
	DBDC_NON_MULTI = 0x2,
	DBDC_MULTI = 0x3,
	DUAL_BAND = 0x4,
	TRIBAND_NON_MULTI = 0x5,
	TRIBAND_MULTI = 0x6
};
#endif
enum REGRP_MODE{
	NON_REGRP_MODE = 0x0,		// ensure to set this back
	REGRP_MODE_BLOCKED = 0x1,
};


enum DEVICE_MASTER_REGROUP_STATE {
	REGRP_STATE_INIT,
	REGRP_STATE_IDLE,		
	REGRP_STATE_TRIGGERED,
	REGRP_STATE_SCAN,
	REGRP_STATE_DONE,
	REGRP_STATE_RETRY,
	REGRP_STATE_ABORT,		// check if required
	REGRP_STATE_MAX
};
enum DEVICE_SLAVE_REGROUP_STATE {
	REGRP_SLAVE_STATE_INIT,
	REGRP_SLAVE_STATE_IDLE,
	REGRP_SLAVE_STATE_SCAN = REGRP_STATE_MAX,
	REGRP_SLAVE_STATE_WAIT_MSG_3,
	REGRP_SLAVE_STATE_DONE,
	REGRP_SLAVE_STATE_ABORT,		// check if required
	REGRP_SLAVE_STATE_MAX
};


enum PEER_REGROUP_STATE {
	REGRP_STATE_PEER_IDLE,
	REGRP_STATE_PEER_WAIT_MSG_2 = REGRP_SLAVE_STATE_MAX,
	REGRP_STATE_PEER_SCAN_DONE,
	//REGRP_STATE_PEER_WAIT_MSG_4,
	REGRP_STATE_PEER_DONE
};


enum regrp_status_code {
	REGRP_CODE_SUCCESS = 0,
	REGRP_CODE_FAILURE = 1,
	REGRP_CODE_COMPLETED = 2,
	REGRP_CODE_REJECTED = 3,
	REGRP_CODE_ABORTED = 4,
	REGRP_CODE_INVALID = 5,
	REGRP_CODE_NOT_READY = 6,
	REGRP_CODE_ROAMING = 7,
	REGRP_CODE_TIMEOUT = 8,
	REGRP_CODE_INVALID_ARG = 9
}regrp_status;
enum REGROUP_COMMAND {
	OID_REGROUP_CMD_CLI_INFO				= 0x01,
	OID_REGROUP_CMD_CONNECTED_PEER_LIST		= 0x02,
	//OID_REGROUP_CMD_EZ_CAP_INFO			= 0x03,
	OID_REGROUP_CMD_SCAN					= 0x04,
	OID_REGROUP_CMD_CAND_LIST				= 0x05,
	OID_REGROUP_CMD_CLEAR_CAND_LIST 		= 0x06,
	OID_REGROUP_CMD_TRIGGER_REGRP			= 0x07,
	OID_REGROUP_CMD_TERMINATE_REGRP 		= 0x08,
	OID_REGROUP_CMD_REGRP_SUPP				= 0x09,
	OID_REGROUP_CMD_EN_REGRP_MODE			= 0x0A, // rename to avoid confusion with regrp supp
	OID_REGROUP_QUERY_INTERFACE_DETAILS	      	= 0x0B,
	OID_REGROUP_QUERY_NODE_NUMBER_WT	      		= 0x0C
};

enum REGROUP_EVENT {
	//REGROUP_EVT_CUSTOM_DATA		= 0x01,
	//REGROUP_EVT_INTF_REGRP_SUPP	<- to inform regrp supp for just enabled intf
	REGROUP_EVT_USER_REGROUP_REQ 	= 0x02,
	REGROUP_EVT_SCAN_COMPLETE	= 0x03,
	REGROUP_EVT_INTF_REGRP_DONE = 0x04,
	// OTHER EVENTS RELATED TO EVENT NOTIFIER
	REGROUP_EVT_INTF_CONNECTED = 0x5,
	REGROUP_EVT_INTF_DISCONNECTED = 0x6,
};

enum CAND_ACTION
{
	NON_MAN_ABOVE_TH,
	INTERNET_ABOVE_TH,
	BEST_RSSI_ABOVE_TH,
	NON_MAN_BELOW_TH,
	INTERNET_BELOW_TH,
	BEST_RSSI_BELOW_TH
};

enum msg_action
{
	MSG_REGRP_INIT,
	MSG_1,
	MSG_2,
	MSG_3,
	MSG_REGRP_END
};

/**************************************************************************************************/

/*Structures*/

/*RepeaterInfo*/

#if 0
typedef struct GNU_PACKED _ez_node_number {
	UCHAR path_len; //path len is the length of the entire node number including the root_mac
	UCHAR root_mac[MAC_ADDR_LEN];
	UCHAR path[EZ_MAX_DEVICE_SUPPORT];
}EZ_NODE_NUMBER;
#endif
#if 0
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
	char update_parameters;
	char third_party_present;
	char new_updated_received;
	unsigned char is_push;
	unsigned char sta_cnt;
	unsigned char sta_mac[10][MAC_ADDR_LEN];
} device_config_to_app_t;
#endif

#if 0
typedef struct device_info_to_app_s
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
	unsigned char no_of_blank_scan_5g;
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer2p4mac[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	unsigned char sta_mac[10][MAC_ADDR_LEN];
} device_info_to_app_t;
#endif


typedef struct PACKED _ntw_info {
	UINT8 Non_MAN;
	UINT8 ssid[32];
	char rssi; // unsigned?
	UINT8 bssid[MAC_ADDR_LEN];
	unsigned char internet_status;
	UINT8 network_wt[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	//UINT8 processed;
	//UINT8 channel;
}ntw_info,*pntw_info;

typedef struct ap_info {
	//UINT8 mode;  // singleband, dualBand, triband etc.
	UINT8 ssid_len;
	UINT8 ssid[33];
	UINT8 intf_prefix[8];
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT8 wdev_id;//! a non zero value means a virtual repeater is created on wdev_index 
	//UINT32 ip_addr;
	//INT8	rssi;
	/*Add More*/
} ap_info_struct, *p_ap_info_struct;

typedef struct ap_shared_info {
	UINT8 ssid_len;
	UINT8 ssid[33];
	UINT8 mac_addr[MAC_ADDR_LEN];
} ap_shared_info_struct, *p_ap_shared_info_struct;



typedef struct device_shared_info {
	UINT8 br_mac[MAC_ADDR_LEN];
	ap_shared_info_struct ap_shared_info_5g;
	ap_shared_info_struct ap_shared_info_2g;
	UINT32 ip_addr;
	EZ_NODE_NUMBER node_number;
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
} device_shared_info_struct, *p_device_shared_info_struct;



typedef struct PACKED repeater_list {	
/*General Info*/
	UINT8 valid;
	UINT8 regrp_init_done;
	UINT8 tcp_link_done;
	UINT8 dialog_token;
	UINT8 entry_idx;
	UINT8 udp_count;
	UINT8 ap_info_num;
	UINT8 br_mac[MAC_ADDR_LEN];
	UINT8 master_mac[MAC_ADDR_LEN];
	UINT8 cli_2g_bssid[MAC_ADDR_LEN];
	UINT8 cli_5g_bssid[MAC_ADDR_LEN];
	ap_info_struct ap_info_5g;
	ap_info_struct ap_info_2g;
	EZ_NODE_NUMBER node_number;
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	UINT8 non_ez_connection;
	UINT32 ip_addr;
	int tcp_rx_sock;
	int tcp_tx_sock;
	struct os_time last_frame_time;
	
/*Regrp state info*/
	UINT8 is_master;
	UINT8 regrp_state;
	UINT8 scan_done_2g;
	UINT8 scan_done_5g;
	UINT8 cand_list_2g_count;
	ntw_info cand_list_2g[MAX_VIRTUAL_REPT_NUM];
	UINT8 cand_list_5g_count;
	ntw_info cand_list_5g[MAX_VIRTUAL_REPT_NUM];
	UINT8 is_processed;
	char target_rssi_5g;
	char target_rssi_2g;
	UINT8 is_processed_tmp;
	UINT8 non_man_selected;
	UINT8 cli_target_mac_2g[MAC_ADDR_LEN];
	UINT8 cli_target_mac_5g[MAC_ADDR_LEN];
	UINT8 need_retrigger_regrp;
	/*Add More*/
} repeater_list_struct, *p_repeater_list_struct;


typedef struct regrp_trigger_handoff_s
{
	UCHAR rssi;
	UCHAR band;
	UINT32 ip_addr;
	UCHAR mac_addr[MAC_ADDR_LEN];
} regrp_trigger_handoff_t, *p_regrp_trigger_handoff_t;

typedef struct regrp_command_s
{
	UINT8 command_id;
	UINT8 command_body[0];
} regrp_command_t, *p_regrp_command_t;

typedef struct regrp_event_s
{
	UINT8 event_id;
	UINT8 event_body[0];
} regrp_event_t, *p_regrp_event_t;

typedef struct regrp_config_interface_s
{
	UINT8 main_idx;
	UINT8 regrp_idx;
	UINT8 regrp_init;
	UINT8 regrp_mac[MAC_ADDR_LEN];
} regrp_config_interface_t, *p_regrp_config_interface_t;

typedef struct regrp_sniff_request_struct
{
	UINT8 br_mac[MAC_ADDR_LEN];
	UINT8 client_mac[MAC_ADDR_LEN];
	UINT8 band;
}regrp_sniff_request_t, *p_regrp_sniff_req_t;


typedef struct sniff_result_s
{
	char rssi;
	UINT32 recvd_packet_count;
	p_repeater_list_struct regrp_entry;
	
} sniff_result_t, *p_sniff_result_t;

typedef struct sta_sniff_cb_s
{
	UINT8 sniff_results_count;
	sniff_result_t sniff_result[MAX_VIRTUAL_REPT_NUM];
}sta_sniff_cb_t, *p_sta_sniff_cb_t;

typedef struct sta_entry {
	UINT8 valid;
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT8 ifindex; 						// interface index to which it is connected, ra0/rai0 etc.
	UINT8 state;   						// currently with us or transfered or intermediate state
	UINT8 dialog_token;
	UINT8 PN[PN_LEN];
	UINT32 ip_addr;
	UINT8 band;
	sta_sniff_cb_t sniff_cb;
	char last_rssi_sample;
	/*Add More*/
}sta_entry_struct, *p_sta_entry_struct;


// TODO:  Raghav: can be a union for all 3 messages.
typedef struct msg_1
{
	UINT8 br_mac[MAC_ADDR_LEN];
	UINT8 action;
	UINT8 dialog_token;
}msg_1_struct,*p_msg_1_struct, msg_init_struct , *p_msg_init_struct;

typedef struct msg_2
{
	UINT8 br_mac[MAC_ADDR_LEN];
	UINT8 action;
	UINT8 dialog_token;
	UINT8 status;
	UINT8 cli_2g_bssid[MAC_ADDR_LEN];
	UINT8 cli_5g_bssid[MAC_ADDR_LEN];
	UINT8 cand_list_2g_count;
	ntw_info cand_list_2g[MAX_VIRTUAL_REPT_NUM];
	UINT8 cand_list_5g_count;
	ntw_info cand_list_5g[MAX_VIRTUAL_REPT_NUM];
}msg_2_struct, *p_msg_2_struct;

typedef struct msg_3
{
	UINT8 br_mac[MAC_ADDR_LEN];
	UINT8 action;
	UINT8 dialog_token;
	UINT32 status;
	UINT8 target_5g_mac[MAC_ADDR_LEN];
	UINT8 target_2g_mac[MAC_ADDR_LEN];
	UINT8 target_wt_1[MAC_ADDR_LEN];
	UINT8 target_wt_2[MAC_ADDR_LEN]; // in case of root AP connection, wt can be either of the 2 AP MAC
} msg_3_struct, *p_msg_3_struct;


typedef struct regrp_sta_entry
{
	UINT8 valid;
	UINT8 state;
	UINT8 regrp_rept_inf_idx;
	UINT8 PN[PN_LEN];
	UINT32 ip_addr;
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT8 dialog_token;
	UINT16 mlme_state_len;
	UINT8 sta_mlme_state[1024]; //driver specific
} regrp_sta_entry_struct, *p_regrp_sta_entry_struct;

typedef struct regrp_sta_update
{
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT16 mlme_state_len;
	UINT8 sta_mlme_state[0]; //driver specific
}regrp_sta_update_struct, *p_regrp_sta_update_struct;



typedef struct peer_sniff_result_s
{
	char rssi;
	UINT32 recvd_packet_count;
	UINT8 mr_mac[MAC_ADDR_LEN];
	UINT8 client_entry[MAC_ADDR_LEN];
	//p_repeater_list_struct regrp_entry;
	
} peer_sniff_result_t, *p_peer_sniff_result_t;


typedef struct GNU_PACKED _regrp_command {
	UINT8 command_id;
	UINT8 command_len;
	UINT8 command_body[0];
}regrp_command,*pregrp_command;

typedef struct GNU_PACKED _regrp_cmd_hdr {
	UINT8 command_id;
	UINT8 command_len;
}regrp_cmd_hdr,*pregrp_cmd_hdr;

typedef struct GNU_PACKED _regrp_cmd_scan {
	struct _regrp_cmd_hdr hdr;
	UINT8 scan_type;
	UINT8 ssid[32];
}regrp_cmd_scan,*pregrp_cmd_scan;


typedef struct GNU_PACKED _cmd_regrp_mode{
	struct _regrp_cmd_hdr hdr;
	UINT8 mode;
}cmd_regrp_mode,*pcmd_regrp_mode;

typedef struct GNU_PACKED _regrp_cmd_cand_list{
	struct _regrp_cmd_hdr hdr;
	UINT8 cand_count;
	UINT8 list[0];	// one or more entries of type struct _ntw_info
}regrp_cmd_cand_list,*pregrp_cmd_cand_list;

typedef struct _node_num_wt
{
	EZ_NODE_NUMBER node_number;
	UINT8 network_wt[NETWORK_WEIGHT_LEN];
}node_num_wt, *p_node_num_wt;


typedef struct GNU_PACKED _regrp_event {
	UINT8 event_id;
	UINT8 event_len;
	UINT8 event_body[0];
}regrp_event,*pregrp_event;

extern char interface_5g[16];
extern char interface_2g[16];
extern UINT8 regrp_pending;
extern char cli_interface_5g[16];
extern char cli_interface_2g[16];

/*************************************************************************************************/

/*Functions*/

/*regrp_drv.c*/
int regrp_drv_update_sta(p_regrp_sta_entry_struct p_entry);

void regrp_drv_update_activate_regrp_sta(p_regrp_sta_entry_struct p_entry);

void regrp_drv_disable_sta(p_sta_entry_struct p_entry);

void regrp_drv_get_pn(p_sta_entry_struct p_entry);

void regrp_drv_get_sta_mlme_state(UINT8* mac_addr, UINT8 ifindex, UINT8 * mlme_state, UINT16 *mlme_state_len);
UINT8 is_regrp_master_candidate(p_repeater_list_struct p_own_rept);


/*regrp_mlme*/
p_repeater_list_struct regrp_mlme_search_rept_by_ip(UINT32 ip_addr);

p_repeater_list_struct get_rept_by_br_mac(UINT8 *mac);
void restart_regroup(void * eloop_ctx, void * user_ctx);
void regrp_mlme_compose_send_msg_1(p_repeater_list_struct p_own_rept,p_repeater_list_struct p_rept);
void regrp_trigger_reconnection();
void regrp_connect(p_repeater_list_struct p_own_rept);
void trigger_regroup();
void regroup_timeout(void *eloop_data, void *user_ctx); 	//decide when to cancel
void set_regrp_mode(UCHAR * ifname, UINT8 mode);
UINT8 regrp_mlme_compose_send_msg_regrp_initiate(p_repeater_list_struct p_rept);
extern repeater_list_struct g_own_rept_info;
extern UINT8 trigger_regrp;
extern struct os_time last_entry_added_time;
extern void regrp_periodic_exec();
extern void regrp_terminate(int sig, void *signal_ctx);
char regrp_is_target_wt_same(UINT8 *target_wt_1,UINT8 *target_wt_2);

/*************************************************************************************************/
#endif /* __BNDSTRG_H__ */


