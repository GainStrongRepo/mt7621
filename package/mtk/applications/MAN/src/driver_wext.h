
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
    	driver_wext.h
*/

#ifndef __DRIVER_WEXT_H__
#define __DRIVER_WEXT_H__

#include "types.h"


/* Ralink defined OIDs */
#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define OID_GET_SET_TOGGLE					0x8000
#define OID_BNDSTRG_TEST					0x0530
#define OID_BNDSTRG_MSG						0x0950
#define OID_802_11_RRM_COMMAND   			0x094C
#define OID_802_11_RRM_EVENT				0x094D

#define OID_WH_EZ_MAN_DEAMON_EVENT	 			 0x200A
#define OID_WH_EZ_MAN_TRIBAND_EZ_DEVINFO_EVENT	 			 0x200B
#define OID_WH_EZ_MAN_TRIBAND_NONEZ_DEVINFO_EVENT	 			 0x200c
#define OID_WH_EZ_UPDATE_STA_INFO						0x2010
#define OID_WH_EZ_MAN_TRIBAND_SCAN_COMPLETE_EVENT	 			 0x200d
#define OID_WH_EZ_MAN_PLUS_NONMAN_EZ_DEVINFO_EVENT	 			 0x200E
#define OID_WH_EZ_MAN_PLUS_NONMAN_NONEZ_DEVINFO_EVENT	 			 0x200F
#define OID_WH_EZ_MAN_CONF_EVENT	 			 0x2011
#define OID_WH_EZ_GROUP_ID_UPDATE	 			 0x2014
#define OID_WH_EZ_REGROUP_DBG_LEVEL_EVENT                        0x2020
#define OID_WH_EZ_REGROUP_THRESHOLD_EVENT                        0x2021
#define OID_WH_EZ_REGROUP_SHOW_CANDIDATE_RSSI_EVENT              0x2022


enum {
    DRVEVNT_FIRST_ID = 0,
    WHC_DRVEVNT_STA_PROBE_REQ,
    WHC_DRVEVNT_AP_PROBE_RSP,
    WHC_DRVEVNT_STA_JOIN,
    WHC_DRVEVNT_STA_LEAVE,
    WHC_DRVEVNT_EXT_UPLINK_STAT,
    WHC_DRVEVNT_STA_TIMEOUT,
    WHC_DRVEVNT_STA_AUTH_REJECT,
    WHC_DRVEVNT_CHANNEL_LOAD_REPORT,
    WHC_DRVEVNT_STA_RSSI_TOO_LOW,
    WHC_DRVEVNT_STA_ACTIVITY_STATE,
    DRVEVNT_END_ID,
};

#define RT_OID_WE_VERSION_COMPILED              0x0622
#define OID_802_11_WIFI_VER                     0x0920
#define OID_802_11_HS_TEST                      0x0921
#define OID_802_11_HS_IE                        0x0922
#define OID_802_11_HS_ANQP_REQ                  0x0923
#define OID_802_11_HS_ANQP_RSP                  0x0924
#define OID_802_11_HS_ONOFF                     0x0925
#define OID_802_11_HS_PARAM_SETTING             0x0927
#define OID_802_11_WNM_BTM_REQ                  0x0928
#define OID_802_11_WNM_BTM_QUERY                0x0929
#define OID_802_11_WNM_BTM_RSP                  0x093a
#define OID_802_11_WNM_PROXY_ARP                0x093b
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST      0x093c
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST      0x093d
#define OID_802_11_SECURITY_TYPE                0x093e
#define OID_802_11_HS_RESET_RESOURCE            0x093f
#define OID_802_11_HS_AP_RELOAD                 0x0940
#define OID_802_11_HS_BSSID                     0x0941
#define OID_802_11_HS_OSU_SSID                  0x0942
//#define OID_802_11_HS_OSU_NONTX                 0x0944
#define OID_802_11_HS_SASN_ENABLE               0x0943
#define OID_802_11_WNM_NOTIFY_REQ               0x0944
#define OID_802_11_QOSMAP_CONFIGURE             0x0945
#define OID_802_11_GET_STA_HSINFO             	0x0946
#define OID_802_11_BSS_LOAD			           	0x0947

struct driver_wext_data {
	int opmode;
	char drv_mode;
	void *priv;
#if 1
	struct netlink_data *netlink;
#endif
	int ioctl_sock;
	int we_version_compiled;
};

struct anqp_req_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 anqp_req_len;
	char anqp_req[0];
};

struct anqp_rsp_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u16 status;
	u32 anqp_rsp_len;
	char anqp_rsp[0];
};

struct hs_onoff {
	u32 ifindex;
	u8 hs_onoff;
	u8 event_trigger;
	u8 event_type;
};

struct hs_param_setting {
	u32 param;
	u32 value;
};

struct proxy_arp_entry {
	u32 ifindex;
	u8 ip_type;
	u8 from_ds;
	u8 IsDAD;
	char source_mac_addr[6];
	char target_mac_addr[6];
	char ip_addr[0];
};

struct security_type {
	u32 ifindex;
	u8 auth_mode;
	u8 encryp_type;
};

struct proxy_arp_ipv4_unit {
	u8   target_mac_addr[6];
	u8   target_ip_addr[4];
};

struct proxy_arp_ipv6_unit {
	u8 target_mac_addr[6];
	u8 target_ip_type;
	u8 target_ip_addr[16];
};

struct wnm_req_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 type;
	u32 wnm_req_len;	
	char wnm_req[0];
};

struct qosmap_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 qosmap_len;
	char qosmap[0];
};
#endif /* __DRIVER_WEXT_H__ */
