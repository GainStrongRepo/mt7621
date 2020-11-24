/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ctrl_iface_unix.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include <sys/socket.h>
#include <unistd.h>
#include "rrm_wnm.h"
#include "ctrl_iface_unix.h"

extern u8 btmtoken;
extern u64 TSF;
extern u16 duration;
extern u8 abridged;
extern u8 valint;
extern u8 url_len;
extern u8 url[];
extern u8 num_rpt;
extern u16 random_ivl;
extern u16 m_duration;
extern u8 mode;
extern u8 rep_conditon;
extern u8 ref_value;
extern u8 detail;
extern u8 op_class_list[];
extern u8 op_class_len;
extern u8 ch_list[];
extern u8 ch_len;
extern u8 request[];
extern u8 req_len;

static int hotspot_ctrl_iface_event_register(struct hotspot_ctrl_iface *ctrl_iface,
											 struct sockaddr_un *from,
											 socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst;

	ctrl_dst = os_zalloc(sizeof(*ctrl_dst));

	if (!ctrl_dst) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	os_memcpy(&ctrl_dst->addr, from, sizeof(struct sockaddr_un));
	ctrl_dst->addrlen = fromlen;
	dl_list_add(&ctrl_iface->hs_ctrl_dst_list, &ctrl_dst->list);
		
	return 0;
}

static int hotspot_ctrl_iface_event_unregister(struct hotspot_ctrl_iface *ctrl_iface,
											   struct sockaddr_un *from,
											   socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst, *ctrl_dst_tmp;

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
								struct hotspot_ctrl_dst, list) {
		if (fromlen == ctrl_dst->addrlen && os_memcpy(from->sun_path, ctrl_dst->addr.sun_path,
												fromlen - offsetof(struct sockaddr_un, sun_path))
												== 0) {
			dl_list_del(&ctrl_dst->list);
			os_free(ctrl_dst);
			return 0;
		}
	}

	return -1;
}

static void hotspot_ctrl_set_token_param(char *value)
{
	btmtoken = atoi(value);
}

static void hotspot_ctrl_set_TSF_param(char *value)
{
	TSF = atoi(value);
}
static void hotspot_ctrl_set_duration_param(char *value)
{
	duration = atoi(value);
}

static void hotspot_ctrl_set_abridged_param(char *value)
{
	abridged = atoi(value);
}

static void hotspot_ctrl_set_valid_interval_param(char *value)
{
	valint = atoi(value);
}

static void hotspot_ctrl_set_url_param(char *value)
{
	u8 i = 0;
	u8 len = (URL_LEN>strlen(value))? strlen(value):(URL_LEN-1);
	memcpy(url,value,len);
	url[len] = '\0';
	url_len = len;
}

static void hotspot_ctrl_set_repeat_param(char *value)
{
	num_rpt = atoi(value);
}

static void hotspot_ctrl_set_interval_param(char *value)
{
	random_ivl = atoi(value);
}

static void hotspot_ctrl_set_mduration_param(char *value)
{
	m_duration = atoi(value);
}

static void hotspot_ctrl_set_mode_param(char *value)
{
	mode = atoi(value);
}

static void hotspot_ctrl_set_rep_conditon_param(char *value)
{
	rep_conditon = atoi(value);
}

static void hotspot_ctrl_set_ref_value_param(char *value)
{
	ref_value = atoi(value);
}

static void hotspot_ctrl_set_detail_param(char *value)
{
	detail = atoi(value);
}

static void hotspot_ctrl_set_op_class_id_param(char *value)
{
	char index = 0;
	char *this_char = NULL;	

	DBGPRINT(DEBUG_OFF, "op_class: \n");
	while ((this_char = strsep((char **)&value, ",")) != NULL) {
		op_class_list[index] = atoi(this_char);
		if(op_class_list[index] == 0) {
			DBGPRINT(DEBUG_ERROR,"invalid operation class %s",this_char);
			return;
		}
		DBGPRINT(DEBUG_OFF, "%d\n",op_class_list[index]);
		index++;
	}
	op_class_len = index;
}

static void hotspot_ctrl_set_ch_id_param(char *value)
{
	char index = 0;
	char *this_char = NULL;	

	DBGPRINT(DEBUG_OFF, "channel: \n");
	while ((this_char = strsep((char **)&value, ",")) != NULL) {
		ch_list[index] = atoi(this_char);
		if(ch_list[index] == 0) {
			DBGPRINT(DEBUG_ERROR,"invalid channel %s",this_char);
			return;
		}
		DBGPRINT(DEBUG_OFF, "%d\n",ch_list[index]);
		index++;
	}
	ch_len = index;
}

static void hotspot_ctrl_set_requset_id_param(char *value)
{
	char index = 0;
	char *this_char = NULL;	

	DBGPRINT(DEBUG_OFF, "request: \n");
	while ((this_char = strsep((char **)&value, ",")) != NULL) {
		request[index] = atoi(this_char);
		DBGPRINT(DEBUG_OFF, "%d\n",request[index]);
		index++;
	}
	req_len = index;
}	


static int hotspot_ctrl_iface_cmd_btmreq(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char peer_addr[6] = {0};
	char bssid[5][6] = {0};
	char channel[5] = {0};
	char preference[5] = {0};
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL, *disassoc_timer_str = NULL;
	char *timeout_str = NULL, *token = NULL;
	char *this_char = NULL, *prefer_str = NULL;
	char index = 0, bssid_num = 0, ch_num = 0, prefer_num = 0;
	unsigned short disassoc_timer = 0;
	unsigned int timeout = 0;
	p_btm_reqinfo_t p_btm_req_data = NULL;
	unsigned char *data = NULL;
	unsigned int len = 0;
	struct nr_info *info = NULL;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				bssid_str = token;
				index = 0;
				DBGPRINT(DEBUG_OFF, "bssid: \n");
				while ((this_char = strsep((char **)&bssid_str, ",")) != NULL) {
					ret = hwaddr_aton(this_char,bssid[index]);
					if(ret) {
						DBGPRINT(DEBUG_ERROR, 
							"incorrect current bssid address %s\n", this_char);
						goto out;
					}
					index++;
					DBGPRINT(DEBUG_OFF, "%d: %s\n", index, this_char);
				}			
				bssid_num = index;
				break;
			case 2:
				channel_str = token;
				index = 0;
				DBGPRINT(DEBUG_OFF, "channel: \n");
				while ((this_char = strsep((char **)&channel_str, ",")) != NULL) {
					channel[index] = atoi(this_char);
					if(channel[index] == 0) {
						DBGPRINT(DEBUG_ERROR,"invalid channel %s",this_char);
						ret = -1;
						goto out;
					}
					DBGPRINT(DEBUG_OFF, "%d\n",channel[index]);
					index++;
				}
				ch_num = index;
				break;
			case 3:
				prefer_str = token;
				index = 0;
				DBGPRINT(DEBUG_OFF, "preference: \n");
				while ((this_char = strsep((char **)&prefer_str, ",")) != NULL) {
					preference[index] = atoi(this_char);
					DBGPRINT(DEBUG_OFF, "%d\n",preference[index]);
					index++;
				}
				prefer_num= index;
			case 4:
				disassoc_timer_str = token;
				disassoc_timer = atoi(disassoc_timer_str);
				DBGPRINT(DEBUG_OFF, "disassoc_timer=%d\n",disassoc_timer);
				break;
			case 5:
				timeout_str = token;
				timeout = atoi(timeout_str);
				DBGPRINT(DEBUG_OFF, "timeout=%d\n",timeout);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}

	if(ch_num != bssid_num ||
		bssid_num != prefer_num ||
		ch_num != prefer_num) {
		DBGPRINT(DEBUG_ERROR, "error parameters triplet bssid&channel&preference\n");
		ret = -1;
		goto out;
	}
	
	len = sizeof(*p_btm_req_data)+ bssid_num * sizeof(struct nr_info);
	data = (unsigned char *)os_zalloc(len);
	if(!data) {
		DBGPRINT(DEBUG_ERROR,"btm_req mem alloc fail\n");
		ret = -1;
		goto out;
	}
	p_btm_req_data = (p_btm_reqinfo_t)data;

	index = 0;	
	memcpy(p_btm_req_data->sta_mac, peer_addr, MAC_ADDR_LEN);
	p_btm_req_data->disassoc_timer = disassoc_timer;
	p_btm_req_data->timeout = timeout;
	if(btmtoken) {
		p_btm_req_data->dialogtoken = btmtoken;
		DBGPRINT(DEBUG_OFF, "dialogtoken=%d\n",btmtoken);
		btmtoken = 0;
	}
	if(abridged) {
		p_btm_req_data->reqmode |= (1<<1);
		DBGPRINT(DEBUG_OFF, "abridged=%d\n",abridged);
		abridged = 0;
	}
	if(TSF) {
		p_btm_req_data->TSF = TSF;
		DBGPRINT(DEBUG_OFF, "TSF=%ld\n",TSF);
		TSF = 0;
	}
	if(duration) {
		p_btm_req_data->duration = duration;
		DBGPRINT(DEBUG_OFF, "duration=%d\n",duration);
		duration = 0;
	}
	if(valint) {
		p_btm_req_data->valint = valint;
		DBGPRINT(DEBUG_OFF, "valint=%ld\n",valint);
		valint = 0;
	}
	if(url_len) {
		memcpy(p_btm_req_data->url,url,url_len);
		p_btm_req_data->url[url_len] = '\0';
		p_btm_req_data->url_len = url_len;
		DBGPRINT(DEBUG_OFF, "url=%s\n",url);
		memset(url,0,URL_LEN);
		url_len = 0;
	}
	p_btm_req_data->num_candidates = bssid_num;
	info = p_btm_req_data->candidates;
	for(index=0; index<bssid_num; index++) {
		memcpy(info->bssid, bssid[index], MAC_ADDR_LEN);
		info->channum = channel[index];
		info->preference = preference[index];
		info++;
	}

	wnm_send_btm_req_param(rrm_wnm,iface,(u8*)p_btm_req_data,len);
	os_free(data);	
	DBGPRINT(DEBUG_OFF, "sending btm req param\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_btmreq_raw(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char peer_addr[6] = {0};
	char bssid[6] = {0};
	char channel = 0;
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL;
	char *token = NULL;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				bssid_str = token;
				ret = hwaddr_aton(bssid_str,bssid);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect bssid %s\n", bssid_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(bssid));					
				break;
			case 2:
				channel_str = token;
				channel = atoi(channel_str);
				if(channel == 0) {
					DBGPRINT(DEBUG_ERROR,"invalid channel %s",channel_str);
					ret = -1;
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "%d\n",channel);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}

	wnm_send_btm_req_raw(rrm_wnm,iface,peer_addr,channel,bssid);
	
	DBGPRINT(DEBUG_OFF, "sending btm req raw data\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_btm(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	char *on_off_str = NULL;
	char on_off = 0;

	on_off_str = strtok(param_value_pair, " ");	
	on_off = atoi(on_off_str);
	
	wnm_btm_onoff(rrm_wnm,iface,on_off);

	if(on_off) {
		DBGPRINT(DEBUG_OFF, "sending btm on\n");
	} else {
		DBGPRINT(DEBUG_OFF, "sending btm off\n");
	}
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_qbtm(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	char peer_addr[6] = {0};
	char *sta_str = NULL;
	unsigned char cap = 0;
	
	sta_str = strtok(param_value_pair, " ");	
	ret = hwaddr_aton(sta_str,peer_addr);
	if(ret) {
		DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
		goto out;
	}
	DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(peer_addr));	
	
	wnm_btm_query_cap(rrm_wnm,iface,peer_addr,&cap);
			
	if(cap == 1) {
		DBGPRINT(DEBUG_OFF, 
			"sta(%02x:%02x:%02x:%02x:%02x:%02x) support btm\n",
			PRINT_MAC(peer_addr));
	} else {
		DBGPRINT(DEBUG_OFF, 
			"sta(%02x:%02x:%02x:%02x:%02x:%02x) not support btm\n",
			PRINT_MAC(peer_addr));
	}
	
out:	
	return ret;
}

static struct hotspot_ctrl_set_param hs_ctrl_set_params[] = {
	{"btmtoken", hotspot_ctrl_set_token_param},
	{"TSF", hotspot_ctrl_set_TSF_param},	
	{"bss_termination_duration", hotspot_ctrl_set_duration_param},
	{"abridged", hotspot_ctrl_set_abridged_param},
	{"validity_interval", hotspot_ctrl_set_valid_interval_param},
	{"session_information_url", hotspot_ctrl_set_url_param},
	{"repeat", hotspot_ctrl_set_repeat_param},
	{"interval", hotspot_ctrl_set_interval_param},	
	{"mduration", hotspot_ctrl_set_mduration_param},
	{"mode", hotspot_ctrl_set_mode_param},
	{"rep_conditon", hotspot_ctrl_set_rep_conditon_param},
	{"ref_value", hotspot_ctrl_set_ref_value_param},
	{"detail", hotspot_ctrl_set_detail_param},
	{"operating_class_id", hotspot_ctrl_set_op_class_id_param},
	{"channel_id", hotspot_ctrl_set_ch_id_param},
	{"request_id", hotspot_ctrl_set_requset_id_param},
}; 


static int hotspot_ctrl_iface_cmd_set(struct rrm_wnm *rrm_wnm, const char *iface,
									  char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0;
	char *token;
	struct hotspot_ctrl_set_param *match = NULL;
	struct hotspot_ctrl_set_param *param = hs_ctrl_set_params;

	token = strtok(param_value_pair, " ");
	while (param->param) {
		if (os_strcmp(param->param, token) == 0) {
			match = param;
			break;
		}
		param++;
	}
	
	if (match) {
		token = strtok(NULL, "");
		match->set_param(token);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "Unkown parameters\n");
		return -1;
	}

	return ret;
}

static int hotspot_ctrl_iface_cmd_nrrsp(struct rrm_wnm *rrm_wnm, const char *iface,
									  char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char dialog_token = 0;
	char peer_addr[6] = {0};
	char bssid[5][6] = {0};
	char channel[5] = {0};
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL, *dialog_token_str = NULL;
	char *token = NULL;
	char *this_char = NULL;
	char index = 0, bssid_num = 0, ch_num = 0;
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data = NULL;
	unsigned char *data = NULL;
	unsigned int len = 0;
	struct nr_info *info = NULL;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
					DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				bssid_str = token;
				index = 0;
				DBGPRINT(DEBUG_OFF, "bssid: \n");
				while ((this_char = strsep((char **)&bssid_str, ",")) != NULL) {
					ret = hwaddr_aton(this_char,bssid[index]);
					if(ret) {
						DBGPRINT(DEBUG_ERROR, 
							"incorrect current bssid address %s\n", this_char);
						goto out;
					}
					index++;
					DBGPRINT(DEBUG_OFF, "%d: %s\n", index, this_char);
				}			
				bssid_num = index;
				break;
			case 2:
				channel_str = token;
				index = 0;
				DBGPRINT(DEBUG_OFF, "channel: \n");
				while ((this_char = strsep((char **)&channel_str, ",")) != NULL) {
					channel[index] = atoi(this_char);
					if(channel[index] == 0) {
						DBGPRINT(DEBUG_ERROR,"invalid channel %s",this_char);
						ret = -1;
						goto out;
					}
					DBGPRINT(DEBUG_OFF, "%d\n",channel[index]);
					index++;
				}
				ch_num = index;
				break;
			case 3:
				dialog_token_str = token;
				dialog_token = atoi(dialog_token_str);
				DBGPRINT(DEBUG_OFF, "dialog_token=%d\n",dialog_token);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}

	if(ch_num != bssid_num) {
		DBGPRINT(DEBUG_ERROR, "error parameters pair bssid&channel\n");
		ret = -1;
		goto out;
	}
	
	len = sizeof(*p_nr_rsp_data)+ bssid_num * sizeof(struct nr_info);
	data = (unsigned char *)os_zalloc(len);
	if(!data) {
		DBGPRINT(DEBUG_ERROR,"nr_rsp mem alloc fail\n");
		ret = -1;
		goto out;
	}
	p_nr_rsp_data = (p_rrm_nrrsp_info_custom_t)data;

	index = 0;	
	memcpy(p_nr_rsp_data->peer_address, peer_addr, MAC_ADDR_LEN);
	p_nr_rsp_data->dialogtoken = dialog_token;
	p_nr_rsp_data->nrresp_info_count = bssid_num;
	info = p_nr_rsp_data->nrresp_info;
	for(index=0; index<bssid_num; index++) {
		memcpy(info->bssid, bssid[index], MAC_ADDR_LEN);
		info->channum = channel[index];
		info++;
	}

	rrm_send_nr_rsp_param(rrm_wnm,iface,(u8*)p_nr_rsp_data,len);
	os_free(data);	
	DBGPRINT(DEBUG_OFF, "sending nr rsp success\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_nrrsp_raw(struct rrm_wnm *rrm_wnm, const char *iface,
									  char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char dialog_token = 0;
	char peer_addr[6] = {0};
	char bssid[6] = {0};
	char channel = 0;
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL, *dialog_token_str = NULL;
	char *token = NULL;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
					DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				bssid_str = token;
				ret = hwaddr_aton(bssid_str,bssid);
				if(ret) {
					DBGPRINT(DEBUG_ERROR, "incorrect bssid %s\n", bssid_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(bssid));	
				break;
			case 2:
				channel_str = token;
				channel = atoi(channel_str);
				if(channel == 0) {
					DBGPRINT(DEBUG_ERROR,"invalid channel %s",channel_str);
					ret = -1;
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "%d\n",channel);
				break;
			case 3:
				dialog_token_str = token;
				dialog_token = atoi(dialog_token_str);
				DBGPRINT(DEBUG_OFF, "dialog_token=%d\n",dialog_token);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}
	
	rrm_send_nr_rsp_raw(rrm_wnm,iface,peer_addr,bssid,channel,dialog_token);
	DBGPRINT(DEBUG_OFF, "sending nr rsp raw data success\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_rrm(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	char *on_off_str = NULL;
	char on_off = 0;

	on_off_str = strtok(param_value_pair, " "); 
	on_off = atoi(on_off_str);
	
	rrm_onoff(rrm_wnm,iface,on_off);

	if(on_off) {
		DBGPRINT(DEBUG_OFF, "sending rrm on\n");
	} else {
		DBGPRINT(DEBUG_OFF, "sending rrm off\n");
	}
		
	return ret;
}

static int hotspot_ctrl_iface_cmd_qrrm(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	char peer_addr[6] = {0};
	char *sta_str = NULL;
	unsigned char cap[8] = {0};
	unsigned char zero_cap[8] = {0};
	
	sta_str = strtok(param_value_pair, " ");	
	ret = hwaddr_aton(sta_str,peer_addr);
	if(ret) {
		DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
		goto out;
	}
	DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(peer_addr));	
	
	rrm_query_cap(rrm_wnm,iface,peer_addr,cap);
			
	if(memcmp(cap,zero_cap,8)) {
		DBGPRINT(DEBUG_OFF, 
			"sta(%02x:%02x:%02x:%02x:%02x:%02x) support rrm\n",
			PRINT_MAC(peer_addr));
	} else {
		DBGPRINT(DEBUG_OFF, 
			"sta(%02x:%02x:%02x:%02x:%02x:%02x) not support rrm\n",
			PRINT_MAC(peer_addr));
	}
	
out:	
	return ret;
}


static int hotspot_ctrl_iface_cmd_nrbyd(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	char *str = NULL;
	unsigned char by_daemon = 0;
	
	str = strtok(param_value_pair, " ");	
	by_daemon = atoi(str);

	if(by_daemon) {
		DBGPRINT(DEBUG_OFF, "handle nr req by daemon\n");	
	} else {
		DBGPRINT(DEBUG_OFF, "handle nr req by driver\n");
	}
	
	rrm_send_nr_req_handle_way(rrm_wnm,iface,by_daemon);
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_bcnreq(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char peer_addr[6] = {0};
	char bssid[6] = {0};
	char channel = 0;
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL, *regclass_str = NULL;
	char *timeout_str = NULL, *token = NULL;
	char *ssid_str = NULL;
	char regclass = 0;
	unsigned int timeout = 0;
	p_bcn_req_info p_bcn_req_data = NULL;
	unsigned char *data = NULL;
	unsigned int len = 0;
	char ssid_len = 0;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				/*only mandatory when channel is set to 0;*/
				regclass_str = token;
				regclass = atoi(regclass_str);
				DBGPRINT(DEBUG_OFF, "regclass=%d\n",regclass);
				break;
			case 2:
				channel_str = token;
				channel = atoi(channel_str);
				DBGPRINT(DEBUG_OFF, "channel=%d\n",channel);
				break;
			case 3:
				bssid_str = token;
				ret = hwaddr_aton(bssid_str,bssid);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect bssid %s\n", bssid_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(bssid));
				break;
			case 4:
				timeout_str = token;
				timeout = atoi(timeout_str);
				DBGPRINT(DEBUG_OFF, "timeout=%ds\n",timeout);
				break;
			case 5:
				ssid_str = token;
				ssid_len = strlen(ssid_str);
				DBGPRINT(DEBUG_OFF, "ssid(len=%d) %s\n",ssid_len, ssid_str);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}
	
	len = sizeof(*p_bcn_req_data);
	data = (unsigned char *)os_zalloc(len);
	if(!data) {
		DBGPRINT(DEBUG_ERROR,"bcn_req mem alloc fail\n");
		ret = -1;
		goto out;
	}
	p_bcn_req_data = (p_bcn_req_info)data;

	memcpy(p_bcn_req_data->peer_address, peer_addr, MAC_ADDR_LEN);	
	p_bcn_req_data->regclass = regclass;
	p_bcn_req_data->channum = channel;
	memcpy(p_bcn_req_data->bssid, bssid, MAC_ADDR_LEN);
	if(ssid_len) {
		memcpy(p_bcn_req_data->req_ssid, ssid_str, ssid_len);
		p_bcn_req_data->req_ssid_len = ssid_len;
	}
	p_bcn_req_data->timeout = timeout;
	
	if(num_rpt) {
		p_bcn_req_data->num_rpt = num_rpt;
		DBGPRINT(DEBUG_OFF, "repetition=%d\n",num_rpt);
		num_rpt = 0;
	}
	if(random_ivl) {
		p_bcn_req_data->random_ivl = random_ivl;
		DBGPRINT(DEBUG_OFF, "random interval=%d\n",random_ivl);
		random_ivl = 0;
	}
	if(m_duration) {
		p_bcn_req_data->duration = m_duration;
		DBGPRINT(DEBUG_OFF, "measure duration=%d\n",m_duration);
		m_duration = 0;
	}
	if(mode) {
		p_bcn_req_data->mode = mode;
		DBGPRINT(DEBUG_OFF, "measure mode=%d\n",mode);
		mode = 0;
	}
	if(rep_conditon) {
		p_bcn_req_data->rep_conditon = rep_conditon;
		DBGPRINT(DEBUG_OFF, "report condition=%d\n",rep_conditon);
		rep_conditon = 0;
	}
	if(ref_value) {
		p_bcn_req_data->ref_value = ref_value;
		DBGPRINT(DEBUG_OFF, "reference value=%d\n",ref_value);
		ref_value = 0;
	}
	if(detail) {
		p_bcn_req_data->detail = detail;
		DBGPRINT(DEBUG_OFF, "detail=%d\n",detail);
		detail = 0;
	}
	if(op_class_len) {
		memcpy(p_bcn_req_data->op_class_list,
			op_class_list, op_class_len);
		p_bcn_req_data->op_class_len = op_class_len;
		memset(op_class_list,0,OP_LEN);
		op_class_len = 0;
	}
	if(ch_len) {
		memcpy(p_bcn_req_data->ch_list,
			ch_list, ch_len);
		p_bcn_req_data->ch_list_len = ch_len;
		memset(ch_list,0,CH_LEN);
		ch_len = 0;
	}
	if(req_len) {
		memcpy(p_bcn_req_data->request,
			request, req_len);
		p_bcn_req_data->request_len = req_len;
		memset(request,0,REQ_LEN);
		req_len = 0;
	}

	rrm_send_bcn_req_param(rrm_wnm,iface,(u8*)p_bcn_req_data,len);
	os_free(data);	
	DBGPRINT(DEBUG_OFF, "sending bcn req param\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_bcnreq_raw(struct rrm_wnm *rrm_wnm, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char peer_addr[6] = {0};
	char channel = 0;
	char *sta_str = NULL, *bssid_str = NULL;
	char *channel_str = NULL, *regclass_str = NULL;
	char *token = NULL;
	char *ssid_str = NULL;
	char regclass = 0;
	char ssid_len = 0;
	
	token = strtok(param_value_pair, " ");
	while (token != NULL) {
		switch(i)
		{
			case 0:
				sta_str = token;
				ret = hwaddr_aton(sta_str,peer_addr);
				if(ret) {
		 			DBGPRINT(DEBUG_ERROR, "incorrect peer mac address %s\n", sta_str);
					goto out;
				}
				DBGPRINT(DEBUG_OFF, "peer mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(peer_addr));	
				break;
			case 1:
				regclass_str = token;
				regclass = atoi(regclass_str);
				DBGPRINT(DEBUG_OFF, "regclass=%d\n",regclass);
				break;
			case 2:
				channel_str = token;
				channel = atoi(channel_str);
				DBGPRINT(DEBUG_OFF, "channel=%d\n",channel);
				break;
			case 3:
				ssid_str = token;
				ssid_len = strlen(ssid_str);
				DBGPRINT(DEBUG_OFF, "ssid(len=%d) %s\n",ssid_len, ssid_str);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				ret = -1;
				goto out;
				break;
		}
		i++;	
		token = strtok(NULL, " ");
	}
		
	rrm_send_bcn_req_raw(rrm_wnm,iface,peer_addr,regclass,channel,ssid_str,ssid_len);
	
	DBGPRINT(DEBUG_OFF, "sending bcn req raw data\n");
	
out:	
	return ret;
}

static int hotspot_ctrl_iface_cmd_process(struct rrm_wnm *rrm_wnm, char *buf, 
											char *reply, size_t *reply_len)
{
	int ret = 0;
	char *token, *token1;
//JERRY
	char tmp[2048];
	char iface[256], cmd[2048];
	int linelen = 0;


	token = strtok(buf, "\n");

	os_memset(tmp, 0, 2048);
	os_memset(iface, 0, 256);
	os_memset(cmd, 0, 2048);

//	printf("==>token=%s\n", *token);
	while (token != NULL) {
		linelen = os_strlen(token);
		printf("len=%d\n", linelen);
		os_strncpy(tmp, token, 2048);
		tmp[2047] = '\0';

		token1 = strtok(tmp, "=");

		if (os_strcmp(token1, "interface") == 0) {
			token1 = strtok(NULL, "");
			os_strncpy(iface, token1, 256);
			iface[255] = '\0';
		} else if (os_strcmp(token1, "cmd") == 0) {
			printf("!!!token1=%s\n", token1);
			token1 = strtok(NULL, "");
//			os_strncpy(cmd, token1, 256);
//			cmd[255] = '\0';
			printf("!!token2=%s\n", token1);
			os_strncpy(cmd, token1, 2048);
            cmd[2047] = '\0';
		}

		token = strtok(token + linelen + 1, "\n");
	}

	os_sleep(0, 5000);
	
	DBGPRINT(RT_DEBUG_ERROR, "interface = %s, cmd = %s\n", iface, cmd);

	if (os_strncmp(cmd, "btmreq_raw", 10) == 0) {
		ret = hotspot_ctrl_iface_cmd_btmreq_raw(rrm_wnm, iface, cmd + 11, reply, reply_len);
	} else if (os_strncmp(cmd, "btmreq", 6) == 0) {
		ret = hotspot_ctrl_iface_cmd_btmreq(rrm_wnm, iface, cmd + 7, reply, reply_len);
	} else if (os_strncmp(cmd, "btm", 3) == 0) {
		ret = hotspot_ctrl_iface_cmd_btm(rrm_wnm, iface, cmd + 4, reply, reply_len);
	} else if (os_strncmp(cmd, "qbtm", 4) == 0) {
		ret = hotspot_ctrl_iface_cmd_qbtm(rrm_wnm, iface, cmd + 5, reply, reply_len);
	} else if (os_strncmp(cmd, "set", 3) == 0) {
		ret = hotspot_ctrl_iface_cmd_set(rrm_wnm, iface ,cmd + 4, reply, reply_len); 
	} else if (os_strncmp(cmd, "nrrsp_raw", 9) == 0) {
		ret = hotspot_ctrl_iface_cmd_nrrsp_raw(rrm_wnm, iface ,cmd + 10, reply, reply_len); 
	} else if (os_strncmp(cmd, "nrrsp", 5) == 0) {
		ret = hotspot_ctrl_iface_cmd_nrrsp(rrm_wnm, iface ,cmd + 6, reply, reply_len); 
	} else if (os_strncmp(cmd, "rrm", 3) == 0) {
		ret = hotspot_ctrl_iface_cmd_rrm(rrm_wnm, iface, cmd + 4, reply, reply_len);
	} else if (os_strncmp(cmd, "qrrm", 4) == 0) {
		ret = hotspot_ctrl_iface_cmd_qrrm(rrm_wnm, iface, cmd + 5, reply, reply_len);
	} else if (os_strncmp(cmd, "nrbyd", 5) == 0) {
		ret = hotspot_ctrl_iface_cmd_nrbyd(rrm_wnm, iface, cmd + 6, reply, reply_len);
	} else if (os_strncmp(cmd, "bcnreq_raw", 10) == 0) {
		ret = hotspot_ctrl_iface_cmd_bcnreq_raw(rrm_wnm, iface, cmd + 11, reply, reply_len);
	} else if (os_strncmp(cmd, "bcnreq", 6) == 0) {
		ret = hotspot_ctrl_iface_cmd_bcnreq(rrm_wnm, iface, cmd + 7, reply, reply_len);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "no such command\n");
		ret = -1;
	}

	if (ret == 0 && reply_len > 0)
		ret = 1;

	return ret;
}

static void hotspot_ctrl_iface_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct rrm_wnm *rrm_wnm = eloop_ctx;
	struct hotspot_ctrl_iface *ctrl_iface = sock_ctx;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	int receive_len;
	char buf[4096];
	size_t replylen = 2047;
	char reply[2048];
	int ret;

	//os_memset(buf, 0, 256);
	os_memset(buf, 0, 4096);
	os_memset(reply, 0, 2048);

	receive_len = recvfrom(sock, buf, sizeof(buf) - 1, 0, 
							(struct sockaddr *)&from, &fromlen);

	if (receive_len < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "receive from control interface fail\n");
		return;
	}

	buf[receive_len] = '\0';

	if (os_strcmp(buf, "EVENT_REGISTER") == 0) {
		if (hotspot_ctrl_iface_event_register(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);

	} else if(os_strcmp(buf , "EVENT_UNREGISTER") == 0) {
		if (hotspot_ctrl_iface_event_unregister(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
	} else {
			ret = hotspot_ctrl_iface_cmd_process(rrm_wnm, buf, reply, &replylen);
		if (ret == -1)
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else if (ret == 0)
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, reply, replylen, 0, (struct sockaddr *)&from, fromlen);
	}

	return;
}

struct hotspot_ctrl_iface *hotspot_ctrl_iface_init(void *data)
{
	struct hotspot_ctrl_iface *ctrl_iface;
	struct sockaddr_un addr;
	struct rrm_wnm *rrm_wnm;

	rrm_wnm = (struct rrm_wnm *)data;
	ctrl_iface = os_zalloc(sizeof(*ctrl_iface));

	if (!ctrl_iface) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		goto error0;
	}

	dl_list_init(&ctrl_iface->hs_ctrl_dst_list);
	ctrl_iface->sock = -1;

	ctrl_iface->sock = socket(PF_UNIX, SOCK_DGRAM, 0);

	if (ctrl_iface->sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "create socket for ctrl interface fail\n");
		goto error1;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	//os_snprintf(addr.sun_path,sizeof(addr.sun_path),"/tmp/hotspot%s",hs->iface);	
	os_strncpy(addr.sun_path, "/tmp/hotspot", sizeof(addr.sun_path));

	if (bind(ctrl_iface->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "bind addr to ctrl interface fail\n");
		goto error2;
	}

	eloop_register_read_sock(ctrl_iface->sock, hotspot_ctrl_iface_receive, rrm_wnm, 
										ctrl_iface);

	return ctrl_iface;

error2:
	close(ctrl_iface->sock);
error1:
	os_free(ctrl_iface);
error0:
	return NULL;
}

void hotspot_ctrl_iface_deinit(void *data)
{
	struct hotspot_ctrl_iface *ctrl_iface;
	struct hotspot_ctrl_dst	*ctrl_dst, *ctrl_dst_tmp;
	char socket_path[64]={0};
	struct rrm_wnm *rrm_wnm;

	rrm_wnm = (struct rrm_wnm *)data;
	ctrl_iface = rrm_wnm->hs_ctrl_iface;

	eloop_unregister_read_sock(ctrl_iface->sock);
		
	close(ctrl_iface->sock);
	ctrl_iface->sock = -1;

	//os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot%s",hs->iface);
	os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot");
	unlink(socket_path);

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
									struct hotspot_ctrl_dst, list) {
		os_free(ctrl_dst);
	}

	os_free(ctrl_iface);
}
