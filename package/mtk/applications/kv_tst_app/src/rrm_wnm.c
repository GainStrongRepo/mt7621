
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
#include "rrm_wnm.h"
#include "driver_wext.h"

int RTDebugLevel = RT_DEBUG_ERROR;
u8 btmtoken = 0;
u64 TSF = 0;
u16 duration = 0;
u8 abridged = 0;
u8 valint = 0;
u8 url_len= 0;
u8 url[URL_LEN] = {0};
u8 num_rpt = 0;
u16 random_ivl = 0;
u16 m_duration= 0;
u8 mode = 0;
u8 rep_conditon = 0;
u8 ref_value = 0;
u8 detail = 0;
u8 op_class_list[OP_LEN] = {0};
u8 op_class_len = 0;
u8 ch_list[CH_LEN] = {0};
u8 ch_len = 0;
u8 request[REQ_LEN] = {0};
u8 req_len = 0;


extern struct rrm_wnm_drv_ops rrm_wnm_drv_wext_ops;

int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
							  unsigned short oid, char *data, size_t len);
int driver_wext_get_oid(struct driver_wext_data *drv_data, const char *ifname,
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


int wnm_send_btm_req(struct rrm_wnm *rrm_wnm, u8 *peer_mac_addr,u8 ifindex,u8 *ifname, const u8 *btm_req, u8 btm_req_len)
{
	rrm_wnm->drv_ops->drv_send_btm_req(rrm_wnm->drv_data,ifname,
		(const char *)peer_mac_addr,(const char *)btm_req,btm_req_len);
	return 0;
}

int wnm_send_btm_req_param(struct rrm_wnm *rrm_wnm,u8 *ifname, u8 *btm_req_param, u32 param_len)
{
	rrm_wnm->drv_ops->drv_send_btm_req_param(rrm_wnm->drv_data,ifname,
		btm_req_param, param_len);
	return 0;
}

int wnm_send_btm_req_raw(struct rrm_wnm *rrm_wnm,u8 *ifname, u8 *mac, u8 ch, u8 *bssid)
{
	char btm_req[] = 
	{
		0x01, 0x00,	0x00, 0x00, 0x34, 0x0d, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x03, 0x18, 0x00, 0x00,
		0x01, 0x00, 0x09
	};
	p_btm_req_ie_data_t p_btm_req_data = NULL;
	unsigned char *data = NULL;
	unsigned int len = 0;

	len = sizeof(btm_req) + sizeof(*p_btm_req_data);
	p_btm_req_data = (p_btm_req_ie_data_t)os_zalloc(len);
	if(!p_btm_req_data) {
		DBGPRINT(DEBUG_ERROR,"btm_req mem alloc fail\n");
		return -1;
	}

	memcpy(p_btm_req_data->peer_mac_addr, mac, MAC_ADDR_LEN);
	memcpy(p_btm_req_data->btm_req, btm_req, sizeof(btm_req));
	memcpy(&(p_btm_req_data->btm_req[6]), bssid, MAC_ADDR_LEN);
	p_btm_req_data->btm_req[17] = ch;
	p_btm_req_data->btm_req_len = sizeof(btm_req);
	
	rrm_wnm->drv_ops->drv_send_btm_req_raw(rrm_wnm->drv_data,ifname,
		(u8*)p_btm_req_data, len);
	
	os_free(p_btm_req_data);	
	return 0;
}


int rrm_send_nr_rsp_param(struct rrm_wnm *rrm_wnm,u8 *ifname,
	u8 *nr_rsp_param, u32 param_len)
{
	rrm_wnm->drv_ops->drv_send_nr_rsp_param(rrm_wnm->drv_data,ifname,
		nr_rsp_param, param_len);
	return 0;
}

int rrm_send_nr_rsp_raw(struct rrm_wnm *rrm_wnm,u8 *ifname,
	u8 *mac, u8 *bssid, u8 ch, u8 token)
{
	char nr_rsp[] = 
	{
		0x34, 0x0d,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x18, 0x00, 0x00, 0x01, 0x00, 0x09
	};
	p_nr_rsp_data_t p_nr_rsp_data = NULL;
	unsigned char *data = NULL;
	unsigned int len = 0;
	
	len = sizeof(*p_nr_rsp_data)+ sizeof(nr_rsp);
	p_nr_rsp_data = (p_nr_rsp_data_t)os_zalloc(len);
	if(!p_nr_rsp_data) {
		DBGPRINT(DEBUG_ERROR,"nr_rsp mem alloc fail\n");
		return -1;
	}

	memcpy(p_nr_rsp_data->peer_address, mac, MAC_ADDR_LEN);
	p_nr_rsp_data->dialog_token = token;
	memcpy(p_nr_rsp_data->nr_rsp, nr_rsp, sizeof(nr_rsp));
	memcpy(&(p_nr_rsp_data->nr_rsp[2]), bssid, MAC_ADDR_LEN);
	p_nr_rsp_data->nr_rsp[13] = ch;
	p_nr_rsp_data->nr_rsp_len = sizeof(nr_rsp);

	rrm_wnm->drv_ops->drv_send_nr_rsp_raw(rrm_wnm->drv_data,ifname,
			(u8*)p_nr_rsp_data, len);
	
	os_free(p_nr_rsp_data);	
	
	return 0;
}

int rrm_send_nr_req_handle_way(struct rrm_wnm *rrm_wnm,u8 *ifname, u8 way)
{
	rrm_wnm->drv_ops->drv_send_nr_req_handle_way(rrm_wnm->drv_data,ifname,
		way);
	return 0;
}

int rrm_send_bcn_req_param(struct rrm_wnm *rrm_wnm,u8 *ifname, u8 *bcn_req_param, u32 param_len)
{
	rrm_wnm->drv_ops->drv_send_bcn_req_param(rrm_wnm->drv_data,ifname,
		bcn_req_param, param_len);
	return 0;
}

int rrm_send_bcn_req_raw
	(struct rrm_wnm *rrm_wnm,u8 *ifname, u8 *mac, u8 reg, u8 ch, u8 *ssid, u8 ssid_len)
{
	char beacon_req[] = 
	{
		0x00, 0x00,	0x26, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
		0x00, 0x00, 0x14, 0x00, 0x01, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF
	};
	char reporting_detail[] = {0x02, 0x01, 0x01};
	char Beacon_report_Subelement[]={0x01, 0x02, 0x00,0x00};
	char Request_Subelement[]={0x0a, 0x04, 0x00, 0x30, 0x36, 0x46};

	p_bcn_req_data_t p_bcn_req_data = NULL;
	int bcn_req_len = 0, len = 0;
	static char token = 0;
	
	DBGPRINT(DEBUG_TRACE, "\n");

	token++;
	if(token == 0)
		token = 1;
	
	p_bcn_req_data = (p_bcn_req_data_t)os_zalloc(200);
	p_bcn_req_data->dialog_token = token;
	memcpy(p_bcn_req_data->peer_address, mac, MAC_ADDR_LEN);
	memcpy(p_bcn_req_data->bcn_req, beacon_req, sizeof(beacon_req));

	/*Add Ssid Subelement*/
	bcn_req_len = sizeof(beacon_req);
	p_bcn_req_data->bcn_req[bcn_req_len++] = 0;
	p_bcn_req_data->bcn_req[bcn_req_len++] = ssid_len;
	memcpy(&p_bcn_req_data->bcn_req[bcn_req_len], ssid, ssid_len);
	bcn_req_len += ssid_len;

	/*Add Reporting information Sub Element*/
	memcpy(&p_bcn_req_data->bcn_req[bcn_req_len], Beacon_report_Subelement, sizeof(Beacon_report_Subelement));
	bcn_req_len += sizeof(Beacon_report_Subelement);

	/*Add Reporting detail Sub Element*/
	memcpy(&p_bcn_req_data->bcn_req[bcn_req_len], reporting_detail, sizeof(reporting_detail));
	bcn_req_len += sizeof(reporting_detail);

	/*Add Request Sub Element*/
	memcpy(&p_bcn_req_data->bcn_req[bcn_req_len], Request_Subelement, sizeof(Request_Subelement));
	bcn_req_len += sizeof(Request_Subelement);

	p_bcn_req_data->bcn_req_len = bcn_req_len;

	p_bcn_req_data->bcn_req[3] = bcn_req_len -4;
	p_bcn_req_data->bcn_req[4] = token;
	p_bcn_req_data->bcn_req[7] = reg;
	p_bcn_req_data->bcn_req[8] = ch;

	len = sizeof(*p_bcn_req_data) + bcn_req_len;
	
	rrm_wnm->drv_ops->drv_send_bcn_req_raw(rrm_wnm->drv_data,ifname,
		(u8*)p_bcn_req_data, len);

	os_free(p_bcn_req_data);
	
	return 0;
}

int rrm_onoff(struct rrm_wnm *rrm_wnm,const char *iface,u8 onoff)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");
	ret = rrm_wnm->drv_ops->drv_rrm_onoff(rrm_wnm->drv_data, iface, onoff);

	return ret;
}

int rrm_query_cap(struct rrm_wnm *rrm_wnm, const char *iface,
				u8 *stamac, u8 *cap)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");
	ret = rrm_wnm->drv_ops->drv_rrm_query_cap(rrm_wnm->drv_data, iface, stamac, cap);

	return ret;
}


int event_btm_query (struct rrm_wnm *rrm_wnm, struct wnm_event *wnm_event_data)
{
#if 0
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
	wnm_send_btm_req(rrm_wnm, query_data->peer_mac_addr,query_data->ifindex,Ifname,(const u8 *)btm_req,btm_req_len);
#endif
	return 0;
}



int event_btm_rsp (struct rrm_wnm *rrm_wnm, struct wnm_event *wnm_event_data)
{
	struct btm_rsp_data *data = NULL;

	data = (struct btm_rsp_data *)wnm_event_data->event_body;
	DBGPRINT(DEBUG_TRACE, "%s STA_MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__,PRINT_MAC(data->peer_mac_addr));
	DBGPRINT(DEBUG_TRACE, "%s STA_MAC ifindex=%d\n",
		__FUNCTION__,data->ifindex);
	DBGPRINT(DEBUG_TRACE, "%s token=%d\n",
		__FUNCTION__,data->btm_rsp[0]);
	if(data->btm_rsp_len == 1) {
		DBGPRINT(DEBUG_TRACE, "%s timeout! didn't receive btm rsp!\n",
			__FUNCTION__);
	} else {
		hex_dump("BTMRsp:", (data->btm_rsp+1), data->btm_rsp_len-1);
	}
		
	return 0;

}

int event_nr_req (struct rrm_wnm *rrm_wnm, struct rrm_event_s *rrm_event_data)
{
	struct nr_req_data *data = NULL;
	u8 token = 0;
	u8 *ssid = NULL;
	u8 ssid_len = 0;
	p_eid eid_ptr = NULL;
	u32 len = 0;

	data = (struct nr_req_data *)rrm_event_data->event_body;
	DBGPRINT(DEBUG_TRACE, "STA_MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(data->peer_mac_addr));
	DBGPRINT(DEBUG_TRACE, "ifindex=%d\n", data->ifindex);
		
	token = data->nr_req[0];
	DBGPRINT(DEBUG_TRACE, "token=%d\n",token);
	
	len = data->nr_req_len - 1;
	
	/*get subie*/
	if(len>0) {
		while(len) {
			eid_ptr = &(data->nr_req[1]);
			switch(eid_ptr->eid) {
				/*ssid*/
				case 0:
					ssid = eid_ptr->octet;
					ssid_len = eid_ptr->len;
					DBGPRINT(DEBUG_TRACE, "%s ssid=%s\n",
						__FUNCTION__,ssid);
				break;
				/*vendor ie*/
				case 221:
					hex_dump("event_nr_req vendor ie:",
						eid_ptr->octet, eid_ptr->len);
				break;
				default:
					DBGPRINT(DEBUG_INFO, 
						"%s unknown Eid: %d\n",
						__FUNCTION__,eid_ptr->eid);
				break;
			}			
			len -=  (2+eid_ptr->len);
			eid_ptr = (p_eid)((u8 *)eid_ptr + 2 + eid_ptr->len);
		}
	}

	return 0;

}

int event_bcn_rep (struct rrm_wnm *rrm_wnm, struct rrm_event_s *rrm_event_data)
{
	p_bcn_rsp_data_t data = NULL;
	
	data = (p_bcn_rsp_data_t)rrm_event_data->event_body;

	DBGPRINT(DEBUG_TRACE, "STA_MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(data->peer_address));
	DBGPRINT(DEBUG_TRACE, "ifindex=%d\n", data->ifindex);
	DBGPRINT(DEBUG_TRACE, "token=%d\n", data->dialog_token);

	if(data->bcn_rsp_len > 3) {
		DBGPRINT(DEBUG_TRACE, "receive valid bcn rep info\n");
	} else {
		DBGPRINT(DEBUG_TRACE, "receive invalid bcn rep info\n");
	}
	hex_dump("bcn rep:", data->bcn_rsp, data->bcn_rsp_len);
}

int rrm_event_handle(struct rrm_wnm *rrm_wnm, char *data)
{
	rrm_event_t *rrm_event_data = (p_rrm_event_t)data;

	/*rrm event*/
	switch (rrm_event_data->event_id)
	{
		case OID_802_11_RRM_EVT_BEACON_REPORT:
			printf("EVT_RRM_BEACON_REPORT\n");
			event_bcn_rep(rrm_wnm, rrm_event_data);
			break;
			
		case OID_802_11_RRM_EVT_NEIGHBOR_REQUEST:	
			printf("EVT_RRM_NEIGHBOR_REQUEST\n");
			event_nr_req(rrm_wnm, rrm_event_data);			
			break;
			
		default:
			printf("unknown rrm event\n");
			break;
	}
	
	return 0;
}


int wnm_event_handle(struct rrm_wnm *rrm_wnm, char *data)
{
	struct wnm_event *wnm_event_data = (struct wnm_event *)data;

	/*wnm event*/
	switch (wnm_event_data->event_id)
	{
		case OID_802_11_WNM_EVT_BTM_QUERY:
			printf("EVT_BTM_QUERY\n");
			event_btm_query(rrm_wnm, wnm_event_data);			
			break;
			
		case OID_802_11_WNM_EVT_BTM_RSP:
			printf("EVT_BTM_RSP\n");
			event_btm_rsp(rrm_wnm, wnm_event_data);			
			break;
			
		default:
			printf("unknown wnm event\n");
			break;
	}
	
	return 0;
}


struct rrm_wnm_event_ops rrm_wnm_event_ops = {
	.rrm_event_handle = rrm_event_handle,
	.wnm_event_handle = wnm_event_handle,
};

inline int wnm_onoff(
				struct rrm_wnm *rrm_wnm,
				const char *iface,
				u8 onoff)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");
	ret = rrm_wnm->drv_ops->drv_wnm_onoff(rrm_wnm->drv_data, iface, onoff);

	return ret;
}


inline int wnm_btm_onoff(
				struct rrm_wnm *rrm_wnm,
				const char *iface,
				u8 onoff)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");
	ret = rrm_wnm->drv_ops->drv_btm_onoff(rrm_wnm->drv_data, iface, onoff);

	return ret;
}

inline int wnm_btm_query_cap(
				struct rrm_wnm *rrm_wnm,
				const char *iface,
				u8 *stamac,
				u8 *cap)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");
	ret = rrm_wnm->drv_ops->drv_btm_query_cap(rrm_wnm->drv_data, iface, stamac, cap);

	return ret;
}

int rrm_wnm_init(struct rrm_wnm *rrm_wnm, 
				 struct rrm_wnm_event_ops *event_ops,
				 int drv_mode,
				 int opmode,
				 int version)
{
	int ret = 0;

	DBGPRINT(DEBUG_TRACE, "\n");

	/* Initialze event loop */
	ret = eloop_init();
	
	if (ret)
	{	
		DBGPRINT(DEBUG_OFF, "eloop_register_timeout failed.\n");
		return -1;
	}

	/* use wireless extension */
	rrm_wnm->drv_ops = &rrm_wnm_drv_wext_ops;

	rrm_wnm->event_ops = event_ops;

	rrm_wnm->version = version;
	rrm_wnm->hs_ctrl_iface = hotspot_ctrl_iface_init(rrm_wnm); 
	rrm_wnm->drv_data = rrm_wnm->drv_ops->drv_inf_init(rrm_wnm, opmode, drv_mode);

	if (!rrm_wnm->drv_data) {
		/* deinit control interface */
		hotspot_ctrl_iface_deinit(rrm_wnm);
		return -1;

	}
	
	return 0;
}

int rrm_wnm_deinit(struct rrm_wnm *rrm_wnm)
{
    int ret = 0;

    DBGPRINT(DEBUG_TRACE, "\n");

	/* deinit control interface */
	hotspot_ctrl_iface_deinit(rrm_wnm);

    ret = rrm_wnm->drv_ops->drv_inf_exit(rrm_wnm);

    if (ret)
        return -1;

    return 0;
}

static void rrm_wnm_terminate(int sig, void *signal_ctx)
{
	DBGPRINT(DEBUG_TRACE, "\n");
	
	eloop_terminate();
}

void rrm_wnm_run(struct rrm_wnm *rrm_wnm)
{

	DBGPRINT(DEBUG_TRACE, "\n");
	
	eloop_register_signal_terminate(rrm_wnm_terminate, rrm_wnm);

	eloop_run();
}

int hex2num(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/**
 * hwaddr_aton - Convert ASCII string to MAC address (colon-delimited format)
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_aton(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num(*txt++);
		if (a < 0)
			return -1;
		b = hex2num(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}


