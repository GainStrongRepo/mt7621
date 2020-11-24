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
    	rrm.h
*/

#ifndef __RRM_H__
#define __RRM_H__
typedef  unsigned char UINT8;
typedef unsigned int UINT32;
#define OID_802_11_RRM_CMD_ENABLE	   			0x01
#define	OID_802_11_RRM_CMD_CAP					0x02
#define OID_802_11_RRM_CMD_SEND_BEACON_REQ  	0x03
#define OID_802_11_RRM_EVT_BEACON_REPORT  	 	0x01

#define PACKED __attribute__((packed))
typedef struct PACKED rrm_command_s
{
	UINT8 command_id;
	UINT8 command_len;
	UINT8 command_body[0];
} rrm_command_t, *p_rrm_command_t;

#define PACKED __attribute__((packed))
typedef struct PACKED rrm_event_s
{
	UINT8 event_id;
	UINT8 event_len;
	UINT8 event_body[0];
} rrm_event_t, *p_rrm_event_t;

typedef struct PACKED rrm_priv_s
{
	void *drv_data;
}rrm_priv_t, *p_rrm_priv_t;

typedef struct PACKED bcn_req_data_s
{
	u32 ifindex;
	UINT8 dialog_token;
	UINT8 peer_address[6];
	u32 bcn_req_len;
	UINT8 bcn_req[0];
} bcn_req_data_t, *p_bcn_req_data_t;

typedef struct GNU_PACKED bcn_rsp_data_s
{
	
	UINT8 	dialog_token;
	u32 	ifindex;
	UINT8 	peer_address[6];
	u32 	bcn_rsp_len;
	UINT8 	bcn_rsp[0];
} bcn_rsp_data_t, *p_bcn_rsp_data_t;


int driver_rrm_set_capabilities(
				void *drv_data,
				const char *ifname,
				char *capabilities);
int driver_rrm_send_beacon_request(
				void *drv_data,
				const char *ifname);

int driver_rrm_enable_disable(
				void *drv_data,
				const char *ifname,
				u8 enable);

#endif /* __BNDSTRG_H__ */


