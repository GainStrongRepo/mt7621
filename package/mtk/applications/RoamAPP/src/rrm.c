
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
    	bndstrg.c
*/
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "rrm.h"
#include "driver_wext.h"
#include "os.h"

int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
							  unsigned short oid, char *data, size_t len);

int driver_rrm_set_capabilities(
				void *drv_data,
				const char *ifname,
				char *capabilities)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	
	p_rrm_command_t p_rrm_command;
	p_rrm_command = os_zalloc(sizeof(rrm_command_t) + 8);
	p_rrm_command->command_id = OID_802_11_RRM_CMD_CAP;
	p_rrm_command->command_len = 8;
	memcpy(p_rrm_command->command_body, capabilities, 8);
	printf("Set Capabilities\n");
	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_802_11_RRM_COMMAND,
				(char *) p_rrm_command,
				sizeof(rrm_command_t) + 8);
	os_free(p_rrm_command);
	return ret;
}

char beacon_req[] = 
{
	0x01, 0x00,	0x26, 0x28, 0xaf, 0x00, 0x05, 33, 11, 
	0x00, 0x03, 0xe8, 0x00, 0x02, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x00, 0x08, 0x48, 0x61, 0x73, 
	0x61, 0x6e, 0x41, 0x6c, 0x69, 0x01, 0x02, 0x00, 
	0x00, 0x02, 0x01, 0x01, 0x0a, 0x05, 0x00, 0x30, 
	0x46, 0x36, 0xdd, 
};
int driver_rrm_send_beacon_request(
				void *drv_data,
				const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	
	p_rrm_command_t p_rrm_command;
	p_bcn_req_data_t p_bcn_req_data;
	//char peer_addr[] = {0x6c, 0x72, 0xE7, 0x87, 0x7D, 0xF0};
	char peer_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	//char peer_addr[] = {0x9c, 0x35, 0xeb, 0xf0, 0x34, 0x0c};
	//char peer_addr[] = {0x80, 0x6a, 0xb0, 0xe1, 0x8c, 0x18};
	//char peer_addr[] = {0x90, 0x60, 0xf1, 0x9a, 0x4b, 0xc4};
	printf("Send Beacon Request\n");
	
	p_rrm_command = os_zalloc(sizeof(rrm_command_t) + 1024);
	p_rrm_command->command_id = OID_802_11_RRM_CMD_SEND_BEACON_REQ;
	p_rrm_command->command_len = sizeof(bcn_req_data_t) + sizeof(beacon_req);
	p_bcn_req_data = (p_bcn_req_data_t)p_rrm_command->command_body;
	p_bcn_req_data->ifindex = 1;
	p_bcn_req_data->dialog_token = 1;
	memcpy(p_bcn_req_data->peer_address, peer_addr, 6);
	p_bcn_req_data->bcn_req_len = sizeof(beacon_req);
	memcpy(p_bcn_req_data->bcn_req, beacon_req, sizeof(beacon_req));
	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_802_11_RRM_COMMAND,
				(char *) p_rrm_command,
				sizeof(rrm_command_t) + sizeof(bcn_req_data_t) +sizeof(beacon_req));
	if (ret)
	{	
		printf("Error sending beacon request\n");
	}
	os_free(p_rrm_command);
	return ret;
}

int driver_rrm_enable_disable(
				void *drv_data,
				const char *ifname,
				u8 enable)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	
	printf("Enable RRM\n");
	p_rrm_command_t p_rrm_command;
	p_rrm_command = os_zalloc(3);
	p_rrm_command->command_id = OID_802_11_RRM_CMD_ENABLE;
	p_rrm_command->command_len = 1;
	*((char *)p_rrm_command + 2) = enable;
	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_802_11_RRM_COMMAND,
				(char *) p_rrm_command,
				sizeof(rrm_command_t) + 1);
	os_free(p_rrm_command);
	return ret;
}

