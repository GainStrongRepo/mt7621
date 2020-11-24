
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
#include "wnm.h"
#include "rrm.h"
#include "driver_wext.h"
extern int DebugLevel ;
#define RRM_802_11_K			0x3

//extern struct bndstrg_drv_ops bndstrg_drv_ranl_ops;

extern struct wnm_drv_ops wnm_drv_wext_ops;
u8 peer_mac_addr[6] = {0xD0, 0xA6, 0x37,0x2B, 0x6D, 0x0c};//{0x9c, 0x35, 0xeb,0xf0, 0x34, 0x0c};// {0x60,0x03,0x08,0x57,0xb2,0xf4};//60:03:08:57:b2:f4
extern int app;


#if 0
void get_current_system_tick(
	struct timespec *now)
{
	clock_gettime(CLOCK_REALTIME, now);
}
#endif


int wnm_send_btm_req(struct wnm *wnm, u8 * peer_mac_addr,u8 ifindex, const u8 * btm_req, u8 btm_req_len)
{


		wnm->drv_ops->drv_send_btm_req(wnm->drv_data,IFNAME_2G,(const char *)peer_mac_addr,(const char *)btm_req,btm_req_len);
		return 0;
#if 0
	int len;
	struct wnm_command *cmd_data;
	struct btm_req_data *btm_req_data;

	len = sizeof(struct wnm_command)+ sizeof(struct btm_req_data) +btm_req_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);
	cmd_data->command_id = OID_802_11_WNM_CMD_SEND_BTM_REQ;
	cmd_data->command_len = sizeof(struct btm_req_data)+ btm_req_len;


	btm_req_data = (struct btm_req_data *)cmd_data->command_body;
	btm_req_data->ifindex = ifindex;
	os_memcpy(btm_req_data->peer_mac_addr, peer_mac_addr,6);
	btm_req_data->btm_req_len = btm_req_len;
	os_memcpy(btm_req_data->btm_req,btm_req,btm_req_len );

	hex_dump("btm_req",cmd_data->command_body, cmd_data->command_len)
	//send_btm_req();
	driver_wext_set_oid(wnm->drv_data,IFNAME_2G, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
#endif
}



int event_btm_query (struct wnm *wnm, struct wnm_event *wnm_event_data)
{
	u8 btm_req[255] = {0,0,0,0,0,0,0,0};
	int btm_req_len =sizeof(btm_req);
	struct btm_query_data *query_data = (struct btm_query_data *)wnm_event_data->event_body;


	struct btm_req_frame *frame = (struct btm_req_frame *)btm_req;
	frame->request_mode = (frame->request_mode & ~0x04) | (1<<2);
	frame->disassociation_timer = 600;
	frame->validity_interval = 200;
	btm_req_len = 4;

	hex_dump("btm_query", wnm_event_data->event_body, wnm_event_data->event_len);
	
	DBGPRINT(DEBUG_TRACE, "%s sending btm req\n", __FUNCTION__);
	wnm_send_btm_req(wnm, query_data->peer_mac_addr,query_data->ifindex,(const u8 *)btm_req,btm_req_len);
	return 0;
}



int event_btm_rsp (struct wnm *wnm, struct wnm_event *wnm_event_data)
{
	hex_dump("BTMRsp", wnm_event_data->event_body, wnm_event_data->event_len);
	return 0;

}

int wnm_event_handle(struct wnm *wnm, char *data)
{
	struct wnm_event *wnm_event_data = (struct wnm_event *)data;

	//memcpy(&msg, data, sizeof(struct bndstrg_msg));

	switch (wnm_event_data->event_id)
	{
		case OID_802_11_WNM_EVT_BTM_QUERY:
			printf("EVT_ BTM_QUERY\n");
			event_btm_query(wnm, wnm_event_data);			
			break;
			
		case OID_802_11_WNM_EVT_BTM_RSP:
			
			printf("EVT_ BTM_RSP\n");
			event_btm_rsp(wnm, wnm_event_data);			
			break;
			
		default:
			BND_STRG_DBGPRINT(DEBUG_WARN,
						"Unkown event. (%u)\n",
						wnm_event_data->event_id);
			break;
	}
	
	return 0;
}


struct wnm_event_ops wnm_event_ops = {
	.event_handle = wnm_event_handle,
};

inline int wnm_onoff(
				struct wnm *wnm,
				const char *iface,
				u8 onoff)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	ret = wnm->drv_ops->drv_wnm_onoff(wnm->drv_data, iface, onoff);

	return ret;
}


inline int wnm_btm_onoff(
				struct wnm *wnm,
				const char *iface,
				u8 onoff)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	ret = wnm->drv_ops->drv_btm_onoff(wnm->drv_data, iface, onoff);

	return ret;
}

char *session_url = "http://we.mediatek.inc/Home/Index";
void wnm_periodic_exec(void *eloop_data, void *user_ctx)
{
	struct wnm * wnm = (struct wnm *) user_ctx;
	u8 btm_req[255] = {0,0,0,0,0,0,0,0};
	int btm_req_len=0;
	char capabilities[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	//int btm_req_len =sizeof(btm_req);
	static int flag =0;
	static int rrm_init_done = 0;
	struct btm_req_frame *frame = (struct btm_req_frame *)btm_req;

	frame->request_mode = (frame->request_mode & ~0x04) | (1<<2);
	frame->request_mode = (frame->request_mode & ~0x10) | (1<<4);
	frame->disassociation_timer = 600;
	frame->validity_interval = 200;
	frame->variable[0] = strlen(session_url);
	os_memcpy(&frame->variable[1],session_url,frame->variable[0] );
	btm_req_len = 4 +1 + frame->variable[0] ;

	if (app == RRM_802_11_K) {
	if (!rrm_init_done)
		{
			driver_rrm_enable_disable(wnm->drv_data,"ra0", 1);
			driver_rrm_set_capabilities(wnm->drv_data,"ra0",capabilities);
		}
		driver_rrm_send_beacon_request(wnm->drv_data,"ra0");
		rrm_init_done = 1;
	} else {
		if (flag == 0) {
				flag =1;
				wnm_onoff(wnm,IFNAME_2G,1);
				wnm_btm_onoff(wnm,IFNAME_2G,1);
			}
		DBGPRINT(DEBUG_ERROR, "%s sending btm req\n", __FUNCTION__);
	}
	wnm_send_btm_req(wnm, peer_mac_addr,0,btm_req,btm_req_len );

	eloop_register_timeout(30, 0, wnm_periodic_exec, NULL, wnm);
}

int wnm_init(struct wnm *wnm, 
				 struct wnm_event_ops *event_ops,
				 int drv_mode,
				 int opmode,
				 int version)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* Initialze event loop */
	ret = eloop_init();
	
	if (ret)
	{	
		DBGPRINT(DEBUG_OFF, "eloop_register_timeout failed.\n");
		return -1;
	}

	/* use wireless extension */
	wnm->drv_ops = &wnm_drv_wext_ops;

	wnm->event_ops = event_ops;

	wnm->version = version;

	wnm->drv_data = wnm->drv_ops->drv_inf_init(wnm, opmode, drv_mode);


	if (ret == BND_STRG_SUCCESS)
		ret = eloop_register_timeout(30, 0, wnm_periodic_exec, NULL, wnm);

	return 0;
}

int wnm_deinit(struct wnm *wnm)
{
    int ret = 0;

    DBGPRINT(DEBUG_TRACE, "\n");

    ret = wnm->drv_ops->drv_inf_exit(wnm);

    if (ret)
        return -1;

    return 0;
}

static void wnm_terminate(int sig, void *signal_ctx)
{
	DBGPRINT(DEBUG_TRACE, "\n");
	
	eloop_terminate();
}

void wnm_run(struct wnm *wnm)
{

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	eloop_register_signal_terminate(wnm_terminate, wnm);

	eloop_run();
}
