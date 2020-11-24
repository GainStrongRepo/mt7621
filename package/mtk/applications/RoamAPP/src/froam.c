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
    	froam.c
*/

#include <stdlib.h>
#include <stdio.h>
#include "froam.h"
#include "driver_wext.h"
#include <sys/ioctl.h>  
#include <pthread.h>
#include <arpa/inet.h>

extern struct froam_drv_ops froam_drv_wext_ops;

u8 ap1channel = 0;
int ap2up = DOWN, ap5up = DOWN;
BOOLEAN single_band = FALSE;
UCHAR ap_intf[SUPP_BAND][MAX_AP_INTF_LEN+1];
char br0_addr[MAC_ADDR_LEN];

void send_peers_froam_msg(char *buf, u8 buf_len);
void send_froam_msg(char *buf, u8 buf_len, struct in_addr *pPeerAddr);
int del_drvr_mntr_entry(pfroam_ctx pfroam,pmnt_staentry pentry, u8 index);

// Obtain bridge Mac
int  get_br0_mac(char *buf)
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	DBGPRINT(DEBUG_INFO, "-->\n");
	
	strcpy(s.ifr_name, "br-lan");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		int i;

		DBGPRINT(DEBUG_OFF, ": ");
		for (i = 0; i < 6; ++i)
	    	printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
	    puts("\n");
	    memcpy(buf, s.ifr_hwaddr.sa_data, 6);
		DBGPRINT(DEBUG_INFO, "<--\n");
	    return 0;
	}

	DBGPRINT(DEBUG_INFO, "<--\n");

    return -1;
}

// Check specified interface to be active
static int CheckIntf(char *ifname) {

	struct ifreq if_req;
	int socId;
	int rv;

	DBGPRINT(DEBUG_OFF, "=> %s\n", ifname);

	socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	if (socId < 0){
		DBGPRINT(DEBUG_ERROR,"Socket failed. Errno = %d\n", errno);
		return DOWN;
	}

	memset(&if_req,0,sizeof(if_req));
	
    (void) strncpy(if_req.ifr_name, ifname, sizeof(if_req.ifr_name));

	rv = ioctl(socId, SIOCGIFFLAGS, &if_req);
    close(socId);

    if ( rv == -1){ 
		DBGPRINT(DEBUG_ERROR,"Ioctl failed. Errno = %d\n", errno);
		return DOWN;
    }

	DBGPRINT(DEBUG_INFO, "<--\n");

    return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
}

// Determine active AP & bridge interfaces
static void check_active_intf()
{
	UCHAR *pOtherApInt = NULL;
	DBGPRINT(DEBUG_INFO, "->\n");

	memset(ap_intf,0,sizeof(ap_intf));

	//CARD1_AP1 would have been checked by caller of this funtion

	if(CheckIntf(CARD1_AP2_MP))
	{
		pOtherApInt = (UCHAR *)CARD1_AP2_MP;
	}	
	else if(CheckIntf(CARD1_AP2))
	{
		pOtherApInt = (UCHAR *)CARD1_AP2;
	}	
	else if(CheckIntf(CARD2_AP))
	{
		pOtherApInt = (UCHAR *)CARD2_AP;
	}	
	else
	{
		single_band = TRUE;
	}

	if(ap1channel <= 14){
		ap2up = UP;
		memcpy(&ap_intf[INTF2][0], CARD1_AP1, strlen(CARD1_AP1));

		if(pOtherApInt != NULL){
			ap5up = UP;
			memcpy(&ap_intf[INTF5][0], pOtherApInt, strlen((char *)pOtherApInt));
		}
	}
	else{
		ap5up = UP;
		memcpy(&ap_intf[INTF5][0], CARD1_AP1, strlen(CARD1_AP1));

		if(pOtherApInt != NULL){
			ap2up = UP;
			memcpy(&ap_intf[INTF2][0], pOtherApInt, strlen((char *)pOtherApInt));
		}
	}

	DBGPRINT(DEBUG_TRACE, "ap2inft : %s, ap2up : %d single_band : %d\n", &ap_intf[0][0], ap2up, single_band);
	DBGPRINT(DEBUG_TRACE, "ap5intf : %s, ap5up : %d \n", &ap_intf[1][0], ap5up);

	if(CheckIntf("br-lan")){
		get_br0_mac(br0_addr);
	}
	else{
		DBGPRINT(DEBUG_ERROR, "!!!! br-lan down !!!! \n");
		// todo handle error??
	}

}

// Socket to recv Air Monitor custom packets from driver
static int mtk_open_notify_socket( const char* if_name )
{
    DBGPRINT(DEBUG_INFO,"entering, if_name=%s\n", if_name );

    int ifindex = if_nametoindex( if_name );
    if ( ifindex == 0 ) {
        DBGPRINT(DEBUG_ERROR, "if_nametoindex(%s) failed\n", if_name );
        return -1;
    }

    DBGPRINT(DEBUG_OFF,"entering, ifindex=%d\n", ifindex );

    int sock = socket( PF_PACKET, SOCK_RAW, htons(/*ETH_P_PAE*/ETH_P_AIR_MONITOR/*ETH_P_ALL*/) );
    if ( sock < 0 ) {
        DBGPRINT(DEBUG_ERROR, "%s(%d): %s\n", __FILE__, __LINE__, strerror(errno) );
        return -1;
    }

    struct sockaddr_ll addr;
    memset( &addr, 0, sizeof(addr) );
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifindex;

    int rc = bind( sock, (struct sockaddr*)&addr, sizeof(addr) );
    if ( rc < 0 ) {
        DBGPRINT(DEBUG_ERROR, "%s(%d): %s\n", __FILE__, __LINE__, strerror(errno) );
        close( sock );
        return -1;
    }
	DBGPRINT(DEBUG_INFO, "<--\n");

    return sock;
}

// Process Air Monitor Custom Packets from driver
static void mtk_handle_l2(pfroam_ctx pfroam, UCHAR *buf, int len)
{
    AIR_RADIO_INFO *pwlan_radio_tap = NULL;
    HEADER_802_11_4_ADDR *pwlan_header = NULL;
	AIR_RAW *recv = NULL;
    static int valid_len = (LENGTH_802_3 + sizeof(AIR_RAW));
	froam_msg_sta_detect msg;
	s32 avg_rssi = 0;
	char max_rssi = 0;
	u8 idx = 0, idx2 = 0, non_zero_count = 0;

	//DBGPRINT(DEBUG_OFF, "-->\n");

	if (buf && (len >= valid_len))
	{
		recv = (AIR_RAW *)(buf + LENGTH_802_3);
        pwlan_radio_tap = &recv->wlan_radio_tap;
        pwlan_header = &recv->wlan_header;

		switch (pwlan_header->FC.Type)
		{
			case BTYPE_DATA:
				if( !(FROAM_MNTR_RULE & RULE_DATA) )
					return;
				break;
            case BTYPE_MGMT:
				if( !(FROAM_MNTR_RULE & RULE_MGT) )
					return;
				break;
            case BTYPE_CNTL:
				if( !(FROAM_MNTR_RULE & RULE_CTL) )
					return;
				break;
			default:
	       		//hex_dump("$$$$ mtk_handle_l2 drop ==>", (uint8_t*)recv, (len - LENGTH_802_3));
				return;
		}

		// own logic implemented, for more accurate logic based on rx ant, refer driver RTMPAvgRssi()
		for(idx = 0;idx < 4; idx++){
			if(pwlan_radio_tap->RSSI[idx] != 0){
				avg_rssi += pwlan_radio_tap->RSSI[idx];

				if(max_rssi != 0){
					max_rssi = ((pwlan_radio_tap->RSSI[idx] > max_rssi )? pwlan_radio_tap->RSSI[idx] : max_rssi);
				}
				else
					max_rssi = pwlan_radio_tap->RSSI[idx];
				
				non_zero_count++;
			}
		}

		avg_rssi /= non_zero_count;

		//DBGPRINT(DEBUG_OFF,"Peer(%02x-%02x-%02x-%02x-%02x-%02x),Type:0x%x Chan:%d, MaxRssi=%d,AvgRssi=%d,  RSSI(ant) = %d,%d,%d,%d\n",
		//	pwlan_header->Addr2[0],pwlan_header->Addr2[1],pwlan_header->Addr2[2],
		//	pwlan_header->Addr2[3],pwlan_header->Addr2[4],pwlan_header->Addr2[5], pwlan_header->FC.Type,pwlan_radio_tap->Channel,max_rssi, avg_rssi,
		//	pwlan_radio_tap->RSSI[0],pwlan_radio_tap->RSSI[1],
		//	pwlan_radio_tap->RSSI[2],pwlan_radio_tap->RSSI[3]);
	
		//if(avg_rssi > pfroam->sta_detect_rssi_threshold)
		{
			pthread_mutex_lock(&pfroam->mntrlock);
			
			for(idx = 0; pfroam->mntr_sta_count && (idx <MAX_SUPP_STA); idx++){
				if((pfroam->mntr_stalist[idx].valid) && (pfroam->mntr_stalist[idx].processed == DRVR_ENTRY_ADDED) &&
					!memcmp(pwlan_header->Addr2,pfroam->mntr_stalist[idx].mac,MAC_ADDR_LEN) &&
					(pfroam->mntr_stalist[idx].channel == pwlan_radio_tap->Channel))
				{

					pfroam->mntr_stalist[idx].pkt_count++;

					if(pfroam->mntr_stalist[idx].pkt_count < pfroam->mntr_min_pkt_count){	// make configurable
						pfroam->mntr_stalist[idx].rssi[(pfroam->mntr_stalist[idx].pkt_count)] = max_rssi;
						avg_rssi = 0;
					}
					else{
						avg_rssi = 0;
						for(idx2 = 0; idx2 < (pfroam->mntr_min_pkt_count-1); idx2++ ){
							pfroam->mntr_stalist[idx].rssi[idx2] = pfroam->mntr_stalist[idx].rssi[idx2+1];
							avg_rssi += pfroam->mntr_stalist[idx].rssi[idx2];
						}
						pfroam->mntr_stalist[idx].rssi[idx2] = max_rssi;
						avg_rssi += pfroam->mntr_stalist[idx].rssi[idx2];
							
						avg_rssi = avg_rssi/pfroam->mntr_min_pkt_count;	// make configurable
					}

					//DBGPRINT(DEBUG_OFF, "Found matching mntr entry %d => Count:%d AvgRssi:%d\n",
					//	idx,pfroam->mntr_stalist[idx].pkt_count,avg_rssi);

					if( (pfroam->mntr_stalist[idx].timeticks >= pfroam->mntr_min_time) && 
					    (avg_rssi != 0) && (avg_rssi > pfroam->sta_detect_rssi_threshold) )	// make configurable
					{
						pfroam->mntr_stalist[idx].processed = DRVR_ENTRY_REMOVED; // do quickly to avoid more pkt processing

						DBGPRINT(DEBUG_TRACE, "Found matching mntr entry %d => Count:%d AvgRssi:%d\n",
							idx,pfroam->mntr_stalist[idx].pkt_count,avg_rssi);

						// send oid to clear mntr entry in driver
						if(del_drvr_mntr_entry(pfroam,&pfroam->mntr_stalist[idx],idx) != FROAM_CODE_SUCCESS){
							DBGPRINT(DEBUG_ERROR, "Failed to delete Mntr entry in driver\n");
						}

						memset(&msg,0,sizeof(froam_msg_sta_detect));
						
						memcpy(msg.hdr.Identifier,FROAM_HDR_IDENTIFIER,4);
						memcpy(msg.hdr.br0_mac,br0_addr,MAC_ADDR_LEN);
						msg.hdr.cmd = FROAM_MSG_STA_DETECT;
						msg.hdr.payloadLen = sizeof(froam_msg_sta_detect) - sizeof(froam_hdr);

						DBGPRINT(DEBUG_TRACE, "Form FROAM_MSG_STA_DETECT=> PayloadLen:%d\n",msg.hdr.payloadLen);
				
						memcpy(msg.mac,pfroam->mntr_stalist[idx].mac,MAC_ADDR_LEN);
						msg.channel = pfroam->mntr_stalist[idx].channel;

						send_froam_msg((char *)&msg,sizeof(froam_msg_sta_detect),&pfroam->mntr_stalist[idx].PeerAddr);

						send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_detect));
						//send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_detect));
						//send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_detect));

					}
					break;
				}
			}

			pthread_mutex_unlock(&pfroam->mntrlock);
		}
	}

	//DBGPRINT(DEBUG_OFF, "<--\n");

}

// Air Monitor Thread
void *mtk_if_notify_thread(void *pctx)
{
	pfroam_ctx pfroam = (pfroam_ctx)pctx;
    fd_set rfds;
    UCHAR buf[1024];
    int len;
    //int ret =0;

	DBGPRINT(DEBUG_INFO, "->\n");

    if ( pfroam == NULL ) {
        DBGPRINT(DEBUG_ERROR, "invalid pfroam pointer" );
        return NULL;
    }
    
	while(!pfroam->terminated)//(1) 
	{
        if (pfroam->air_mntr_sock == 0) {
            const int sock = mtk_open_notify_socket("br-lan");
            if (sock < 0) {
                DBGPRINT(DEBUG_ERROR, "unable to open notify socket; will sleep 1s and retry!\n");
                sleep(1);
                continue;
            }
            pfroam->air_mntr_sock = sock;
        }

        FD_ZERO( &rfds );
        FD_SET( pfroam->air_mntr_sock, &rfds );

        select( pfroam->air_mntr_sock + 1, &rfds, NULL, NULL, NULL );

        if ( ! FD_ISSET( pfroam->air_mntr_sock, &rfds ) ) {
            continue;
        }

        len = recv( pfroam->air_mntr_sock, buf, sizeof(buf), 0 );
        if ( len <= 0 ) {
            DBGPRINT(DEBUG_ERROR, "didn't recv() any data: %s\n", strerror(errno) );
            close(pfroam->air_mntr_sock);
            pfroam->air_mntr_sock = 0;
            return NULL;
        }
		if(pfroam->en_force_roam_supp){
        	mtk_handle_l2(pfroam, buf, len);
		}
		else{
            DBGPRINT(DEBUG_ERROR, "en_force_roam_supp disabled\n");
		}
    }

	close(pfroam->air_mntr_sock);
	pfroam->air_mntr_sock = 0;

	DBGPRINT(DEBUG_INFO, "<-\n");

	return NULL;

}

int get_froam_supp(pfroam_ctx pfroam, UCHAR *ifname,BOOLEAN *pfroam_supp, u8 *pIntf_channel)
{
	DBGPRINT(DEBUG_INFO, "=> %s\n", ifname);

	// validation

	if(pfroam->drv_ops->drv_get_froam_supp(pfroam->drv_data,ifname, pfroam_supp, pIntf_channel) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int add_acl_entry_req(pfroam_ctx pfroam, UCHAR *ifname,UCHAR *peer_addr)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_add_acl_entry_req(pfroam->drv_data,ifname,peer_addr) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int del_acl_entry_req(pfroam_ctx pfroam, UCHAR *ifname,UCHAR *peer_addr)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_del_acl_entry_req(pfroam->drv_data,ifname,peer_addr) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int set_acl_policy(pfroam_ctx pfroam, UCHAR *ifname,u8 policy)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_set_acl_policy(pfroam->drv_data,ifname,policy) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int add_mntr_entry_req(pfroam_ctx pfroam, UCHAR *ifname,UCHAR *peer_addr, u8 index, u8 channel)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_add_mntr_entry_req(pfroam->drv_data,ifname,peer_addr,index,channel) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int del_mntr_entry_req(pfroam_ctx pfroam, UCHAR *ifname,UCHAR *peer_addr, u8 index, u8 channel)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_del_mntr_entry_req(pfroam->drv_data,ifname,peer_addr,index,channel) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int set_mntr_rule(pfroam_ctx pfroam, UCHAR *ifname,u8 rule)
{
	DBGPRINT(DEBUG_INFO, "=> \n");

	// validation

	if(pfroam->drv_ops->drv_set_mntr_rule(pfroam->drv_data,ifname, rule) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int get_thresholds(pfroam_ctx pfroam, UCHAR *ifname,struct _threshold_info *pthr_info)
{
	DBGPRINT(DEBUG_INFO, "=> %s\n", ifname);

	// validation

	if(pfroam->drv_ops->drv_get_thresholds(pfroam->drv_data,ifname, pthr_info) == 0){
		return FROAM_CODE_SUCCESS;
	}

	return FROAM_CODE_FAILURE;
}

int get_drv_froam_supp(pfroam_ctx pfroam)
{
	int ret = FROAM_CODE_SUCCESS;
	BOOLEAN en_force_roam_supp = FALSE;
	u8 intf_channel = 0;

	DBGPRINT(DEBUG_INFO, "=> \n");

	ret = get_froam_supp(pfroam,(UCHAR *)CARD1_AP1,&en_force_roam_supp, &intf_channel);

	if(ret == FROAM_CODE_SUCCESS){
		pfroam->en_force_roam_supp = en_force_roam_supp;
		ap1channel = intf_channel;

		DBGPRINT(DEBUG_TRACE, " Driver Froam Support : %d, Ap1 channel: %d\n", pfroam->en_force_roam_supp, intf_channel);
	}
	else
		DBGPRINT(DEBUG_ERROR, " FAILED \n");
	
	return ret;

}

int add_drvr_acl_entry(pfroam_ctx pfroam,pacl_staentry pentry)
{
	int ret = FROAM_CODE_SUCCESS;
	UCHAR mac_addr[MAC_ADDR_LEN];
	DBGPRINT(DEBUG_INFO, "=> \n");

	memcpy(mac_addr,pentry->mac,MAC_ADDR_LEN);

	if(single_band){
		ret = add_acl_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr);
	}
	else{
		if(ap2up && (pentry->channel <= 14)){
			ret = add_acl_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr);
		}
		else if(ap5up && (pentry->channel > 14)){
			ret = add_acl_entry_req(pfroam,&ap_intf[INTF5][0],mac_addr);		
		}
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	return ret;

}

int del_drvr_acl_entry(pfroam_ctx pfroam,pacl_staentry pentry)
{
	int ret = FROAM_CODE_SUCCESS;
	UCHAR mac_addr[MAC_ADDR_LEN];
	DBGPRINT(DEBUG_INFO, "=> \n");

	memcpy(mac_addr,pentry->mac,MAC_ADDR_LEN);

	if(single_band){
		ret = del_acl_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr);
	}
	else{	
		if(ap2up && (pentry->channel <= 14)){
			ret = del_acl_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr);
		}
		else if(ap5up && (pentry->channel > 14)){
			ret = del_acl_entry_req(pfroam,&ap_intf[INTF5][0],mac_addr);	
		}
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	return ret;
}

int set_drvr_acl_policy(pfroam_ctx pfroam,u8 policy)
{
	int ret = FROAM_CODE_SUCCESS;
	DBGPRINT(DEBUG_INFO, "=> \n");

	if(ap2up){
		ret = set_acl_policy(pfroam,&ap_intf[INTF2][0],policy);
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	if(ap5up){
		ret = set_acl_policy(pfroam,&ap_intf[INTF5][0],policy);		

		if(ret != FROAM_CODE_SUCCESS)
			DBGPRINT(DEBUG_ERROR, " FAILED \n");
	}

	return ret;

}

int add_drvr_mntr_entry(pfroam_ctx pfroam,pmnt_staentry pentry,u8 index)
{
	int ret = FROAM_CODE_SUCCESS;
	UCHAR mac_addr[MAC_ADDR_LEN];
	DBGPRINT(DEBUG_INFO, "=> \n");

	memcpy(mac_addr,pentry->mac,MAC_ADDR_LEN);

	if(single_band){
		ret = add_mntr_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr,index,pentry->channel);
	}
	else{	
		if(ap2up && (pentry->channel <= 14)){
			ret = add_mntr_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr,index,pentry->channel);
		}
		else if(ap5up && (pentry->channel > 14)){
			ret = add_mntr_entry_req(pfroam,&ap_intf[INTF5][0],mac_addr,index,pentry->channel);
		}
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	return ret;

}

int del_drvr_mntr_entry(pfroam_ctx pfroam,pmnt_staentry pentry, u8 index)
{
	int ret = FROAM_CODE_SUCCESS;
	UCHAR mac_addr[MAC_ADDR_LEN];
	DBGPRINT(DEBUG_INFO, "=> \n");

	memcpy(mac_addr,pentry->mac,MAC_ADDR_LEN);

	if(single_band){
		ret = del_mntr_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr,index,pentry->channel);
	}
	else{	
		if(ap2up && (pentry->channel <= 14)){
			ret = del_mntr_entry_req(pfroam,&ap_intf[INTF2][0],mac_addr,index,pentry->channel);
		}
		else if(ap5up && (pentry->channel > 14)){
			ret = del_mntr_entry_req(pfroam,&ap_intf[INTF5][0],mac_addr,index,pentry->channel);
		}
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	return ret;

}

int set_drvr_mntr_rule(pfroam_ctx pfroam,u8 rule)
{
	int ret = FROAM_CODE_SUCCESS;
	DBGPRINT(DEBUG_INFO, "=> \n");

	if(ap2up){
		ret = set_mntr_rule(pfroam,&ap_intf[INTF2][0],rule);
	}

	if(ret != FROAM_CODE_SUCCESS)
		DBGPRINT(DEBUG_ERROR, " FAILED \n");

	if(ap5up){	// send on both to ensure pad updated for two chips if so
		ret = set_mntr_rule(pfroam,&ap_intf[INTF5][0],rule);	

		if(ret != FROAM_CODE_SUCCESS)
			DBGPRINT(DEBUG_ERROR, " FAILED \n");
	}

	return ret;
}

int get_drv_thresholds(pfroam_ctx pfroam)
{
	int ret = FROAM_CODE_SUCCESS;
	struct _threshold_info thr_info;
	DBGPRINT(DEBUG_INFO, "=> \n");

	memset(&thr_info,0x0,sizeof(thr_info));

	if(ap2up){
		ret = get_thresholds(pfroam,&ap_intf[INTF2][0],&thr_info);
	}
	else if(ap5up){
		ret = get_thresholds(pfroam,&ap_intf[INTF5][0],&thr_info);	
	}

	if(ret == FROAM_CODE_SUCCESS){

		if(thr_info.sta_ageout_time)
			pfroam->sta_ageout_time = thr_info.sta_ageout_time;
		if(thr_info.mntr_ageout_time)
			pfroam->mntr_ageout_time = thr_info.mntr_ageout_time;
		if(thr_info.mntr_min_pkt_count)
			pfroam->mntr_min_pkt_count = thr_info.mntr_min_pkt_count;
		if(thr_info.mntr_min_time)
			pfroam->mntr_min_time = thr_info.mntr_min_time;
		if(thr_info.mntr_avg_rssi_pkt_count)
			pfroam->mntr_avg_rssi_pkt_count = thr_info.mntr_avg_rssi_pkt_count;
		if(thr_info.sta_detect_rssi_threshold)
			pfroam->sta_detect_rssi_threshold = thr_info.sta_detect_rssi_threshold;
		if(thr_info.acl_ageout_time)
			pfroam->acl_ageout_time = thr_info.acl_ageout_time;
		if(thr_info.acl_hold_time)
			pfroam->acl_hold_time = thr_info.acl_hold_time;

		DBGPRINT(DEBUG_TRACE, "StaAgeTime:%d, StaDetectRSSI:%d, \n",
			pfroam->sta_ageout_time, pfroam->sta_detect_rssi_threshold);
		DBGPRINT(DEBUG_TRACE, "MntrAgeTime:%d, MntrMinPktCount:%d, MntrMinTime:%d, AvgRSSIPktCount:%d, \n",
			pfroam->mntr_ageout_time, pfroam->mntr_min_pkt_count, pfroam->mntr_min_time,
			pfroam->mntr_avg_rssi_pkt_count);
		DBGPRINT(DEBUG_TRACE, "ACLAgeTime:%d, ACLHoldTime:%d\n",
			pfroam->acl_ageout_time, pfroam->acl_hold_time);

	}
	else
		DBGPRINT(DEBUG_ERROR, " FAILED \n");
	
	return ret;

}

void *process_sta_list(void *pctx)
{
	pfroam_ctx pfroam = (pfroam_ctx)pctx;
	froam_msg_sta_mntr msg;
	u8 idx = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};

	DBGPRINT(DEBUG_INFO, "=> \n");

	while(!pfroam->terminated)//(1) 
	{
		// take lock
		pthread_mutex_lock(&pfroam->stalock);

		for(idx = 0; (idx <MAX_SUPP_STA) && pfroam->sta_count; idx++){
			if(!pfroam->stalist[idx].valid)
				continue;
			if(pfroam->stalist[idx].processed || !pfroam->en_force_roam_supp){
				pfroam->stalist[idx].timeticks++;

				if((pfroam->stalist[idx].timeticks > pfroam->sta_ageout_time) || !pfroam->en_force_roam_supp){
					DBGPRINT(DEBUG_TRACE, "AgeOut StaEnt:%d Tick:%d\n",idx,pfroam->stalist[idx].timeticks);

					pfroam->stalist[idx].valid = FALSE;
					memset(&pfroam->stalist[idx],0x0,sizeof(sta_entry));
					pfroam->sta_count--;
				}
			}
			else
			{
				DBGPRINT(DEBUG_TRACE, "Process StaEnt:%d\n",idx);

				memset(&msg,0,sizeof(froam_msg_sta_mntr));

				memcpy(msg.hdr.Identifier,FROAM_HDR_IDENTIFIER,4);
				memcpy(msg.hdr.br0_mac,br0_addr,MAC_ADDR_LEN);
				msg.hdr.cmd = FROAM_MSG_STA_MNTR;
				msg.hdr.payloadLen = sizeof(froam_msg_sta_mntr) - sizeof(froam_hdr);

				DBGPRINT(DEBUG_TRACE, "Form FROAM_MSG_STA_MNTR=> PayloadLen:%d\n",msg.hdr.payloadLen);

				memcpy(msg.mac,pfroam->stalist[idx].mac,MAC_ADDR_LEN);
				msg.channel = pfroam->stalist[idx].channel;

				send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_mntr));
				send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_mntr));
				//send_peers_froam_msg((char *)&msg,sizeof(froam_msg_sta_mntr));

				pfroam->stalist[idx].processed = TRUE;

				//pfroam->stalist[idx].timeticks++; 	do this now or in next second?
			}
		}
		// release lock
		pthread_mutex_unlock(&pfroam->stalock);

		// take lock
		pthread_mutex_lock(&pfroam->mntrlock);
		for(idx = 0; (idx <MAX_SUPP_STA) && pfroam->mntr_sta_count; idx++){
			if(!pfroam->mntr_stalist[idx].valid)
				continue;

			if( !memcmp(zero_mac,pfroam->mntr_stalist[idx].mac,MAC_ADDR_LEN) ){
				pfroam->mntr_stalist[idx].valid = FALSE;
				continue;
			}

			if( (pfroam->mntr_stalist[idx].processed != NOT_PROCESSED) || !pfroam->en_force_roam_supp){
				pfroam->mntr_stalist[idx].timeticks++;

				if((pfroam->mntr_stalist[idx].timeticks > pfroam->mntr_ageout_time) || !pfroam->en_force_roam_supp){
					// send oid to clear mntr entry in driver
					DBGPRINT(DEBUG_TRACE, "AgeOut MntrEnt:%d Tick:%d Count:%d\n",
						idx,pfroam->mntr_stalist[idx].timeticks, pfroam->mntr_stalist[idx].pkt_count);

					if(pfroam->mntr_stalist[idx].processed == DRVR_ENTRY_ADDED){
						if(del_drvr_mntr_entry(pfroam,&pfroam->mntr_stalist[idx],idx) != FROAM_CODE_SUCCESS){
							DBGPRINT(DEBUG_TRACE, "Failed to delete drvr mntr entry\n");
						}
					}

					pfroam->mntr_stalist[idx].valid = FALSE;
					memset(&pfroam->mntr_stalist[idx],0x0,sizeof(mnt_sta_entry));
					pfroam->mntr_sta_count--;
				}
			}
			else if(pfroam->mntr_stalist[idx].processed == NOT_PROCESSED)
			{
				DBGPRINT(DEBUG_TRACE, "Process MntrEnt:%d\n",idx);
				// send oid to add mntr entry in driver
				if(add_drvr_mntr_entry(pfroam,&pfroam->mntr_stalist[idx],idx) == FROAM_CODE_SUCCESS){
					pfroam->mntr_stalist[idx].processed = DRVR_ENTRY_ADDED;//TRUE;
				}
				else{
					DBGPRINT(DEBUG_TRACE, "Drvr add mntr entry failed, delete entry\n");

					pfroam->mntr_stalist[idx].valid = FALSE;
					memset(&pfroam->mntr_stalist[idx],0x0,sizeof(sta_entry));
					pfroam->mntr_sta_count--;
				}
			}
		}
		// release lock
		pthread_mutex_unlock(&pfroam->mntrlock);

		// take lock
		pthread_mutex_lock(&pfroam->acllock);
		
		for(idx = 0; (idx <MAX_SUPP_STA) && pfroam->acl_sta_count; idx++){
			if(!pfroam->acl_stalist[idx].valid)
				continue;

			if( !memcmp(zero_mac,pfroam->acl_stalist[idx].mac,MAC_ADDR_LEN) ){
				pfroam->acl_stalist[idx].valid = FALSE;
				continue;
			}

			if(pfroam->acl_stalist[idx].processed || !pfroam->en_force_roam_supp){
				pfroam->acl_stalist[idx].timeticks++;

				if((pfroam->acl_stalist[idx].timeticks > pfroam->acl_ageout_time) || !pfroam->en_force_roam_supp){
					// send oid to clear acl entry in driver
					DBGPRINT(DEBUG_TRACE, "AgeOut ACLEnt:%d Tick:%d\n",idx,pfroam->acl_stalist[idx].timeticks);

					if(pfroam->acl_stalist[idx].processed){
						if(del_drvr_acl_entry(pfroam,&pfroam->acl_stalist[idx]) != FROAM_CODE_SUCCESS){
							DBGPRINT(DEBUG_TRACE, "Failed to delete drvr acl entry\n");
						}
					}

					pfroam->acl_stalist[idx].valid = FALSE;
					memset(&pfroam->acl_stalist[idx],0x0,sizeof(acl_sta_entry));
					pfroam->acl_sta_count--;
				}
			}
			else
			{
				DBGPRINT(DEBUG_TRACE, "ACLEnt:%d Tick: %d\n",idx, pfroam->acl_stalist[idx].timeticks);

				if(pfroam->acl_stalist[idx].timeticks < pfroam->acl_hold_time){
					pfroam->acl_stalist[idx].timeticks++;
				}
				else{
					DBGPRINT(DEBUG_TRACE, "Process ACLEnt:%d\n",idx);
					// send oid to add acl entry in driver
					if(add_drvr_acl_entry(pfroam,&pfroam->acl_stalist[idx]) == FROAM_CODE_SUCCESS){
							pfroam->acl_stalist[idx].processed = TRUE;
					}
					else{
						DBGPRINT(DEBUG_TRACE, "Drvr add acl entry failed, delete entry\n");

						pfroam->acl_stalist[idx].valid = FALSE;
						memset(&pfroam->acl_stalist[idx],0x0,sizeof(sta_entry));
						pfroam->acl_sta_count--;
					}
				}
			}
		}

		pthread_mutex_unlock(&pfroam->acllock);

		sleep(1); // use usleep for smaller duration

		// do periodic sta list processign for age out here or in separate thread?
	}

	DBGPRINT(DEBUG_INFO, "<-\n");

	return NULL;
}

static int event_sta_poor_rssi (pfroam_ctx pfroam, char *data)
{
	u8 status = FROAM_CODE_SUCCESS;
	pfroam_event_sta_low_rssi pevt_data = (pfroam_event_sta_low_rssi)data;
	u8 idx = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};

	DBGPRINT(DEBUG_INFO, "->\n");

	if(pevt_data->hdr.event_len != (sizeof(froam_event_sta_low_rssi) - sizeof(froam_event_hdr))){
		DBGPRINT(DEBUG_ERROR, ": Len error\n");
		return FROAM_CODE_INVALID_ARG;
	}

	if( !memcmp(zero_mac,pevt_data->mac,MAC_ADDR_LEN) ){
		DBGPRINT(DEBUG_ERROR, ": Zero MAC\n");
		return FROAM_CODE_INVALID_ARG;		
	}
	else
		DBGPRINT(DEBUG_TRACE, ": STA MAC %02x-%02x-%02x-%02x-%02x-%02x Channel=%d\n",
		    pevt_data->mac[0],pevt_data->mac[1],pevt_data->mac[2],pevt_data->mac[3],
		    pevt_data->mac[4],pevt_data->mac[5], pevt_data->channel);

	// take lock??		todo
	pthread_mutex_lock(&pfroam->stalock);

	if(pfroam->sta_count >= MAX_SUPP_STA){
		// release lock
		pthread_mutex_unlock(&pfroam->stalock);
		DBGPRINT(DEBUG_ERROR, ": STA List Full\n");
		return FROAM_CODE_TABLE_FULL;
	}

	for(idx = 0;(idx < MAX_SUPP_STA) && pfroam->sta_count; idx++){
		if( !memcmp(pfroam->stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN) && pfroam->stalist[idx].valid){
			//pfroam->stalist[idx].processed = FALSE; // resend pkt for this sta??
			//pfroam->stalist[idx].timeticks = 0; // consider entry as just added ??
			// release lock
			pthread_mutex_unlock(&pfroam->stalock);
			DBGPRINT(DEBUG_TRACE, ": STA entry already present and active\n");
			return FROAM_CODE_SUCCESS;	//ignore event as sta already exists  OR over-write if any change??	rethink
		}
	}

	for(idx = 0;(idx < MAX_SUPP_STA); idx++){
		if(!pfroam->stalist[idx].valid)
			break;
	}

	if(idx < MAX_SUPP_STA){
		memset(&pfroam->stalist[idx],0x0,sizeof(sta_entry));
		pfroam->stalist[idx].valid = TRUE;
		memcpy(pfroam->stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN);
		pfroam->stalist[idx].channel = pevt_data->channel;
		pfroam->stalist[idx].timeticks = 0;	// to use for time track		todo
		pfroam->stalist[idx].processed = FALSE;
		pfroam->sta_count++;
		DBGPRINT(DEBUG_TRACE, ": Assigned idx:%x, TotalCount:%d\n",idx,pfroam->sta_count);
	}
	// todo: check if msg can be sent imemdiately

	// release lock	
	pthread_mutex_unlock(&pfroam->stalock);

	DBGPRINT(DEBUG_INFO, "<-\n");

	return status;

}

static int event_sta_good_rssi (pfroam_ctx pfroam, char *data)
{
	u8 status = FROAM_CODE_SUCCESS;
	pfroam_event_sta_good_rssi pevt_data = (pfroam_event_sta_good_rssi)data;
	u8 idx = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	BOOLEAN found = FALSE;
	
	DBGPRINT(DEBUG_INFO, "->\n");

	if(pevt_data->hdr.event_len != (sizeof(froam_event_sta_good_rssi) - sizeof(froam_event_hdr))){
		DBGPRINT(DEBUG_ERROR, ": Len error, PayloadLen:%d, expected:%d\n",
			pevt_data->hdr.event_len,(int)(sizeof(froam_event_sta_good_rssi) - sizeof(froam_event_hdr)));
		return FROAM_CODE_INVALID_ARG;
	}

	if( !memcmp(zero_mac,pevt_data->mac,MAC_ADDR_LEN) ){
		DBGPRINT(DEBUG_ERROR, ": Zero MAC\n");
		return FROAM_CODE_INVALID_ARG;
	}
	else
		DBGPRINT(DEBUG_TRACE, ": STA MAC %02x-%02x-%02x-%02x-%02x-%02x\n",
		    pevt_data->mac[0],pevt_data->mac[1],pevt_data->mac[2],pevt_data->mac[3],
		    pevt_data->mac[4],pevt_data->mac[5]);

	pthread_mutex_lock(&pfroam->acllock);
	
	for(idx = 0; (idx <MAX_SUPP_STA) && pfroam->acl_sta_count; idx++){
		if( !memcmp(pfroam->acl_stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN) && pfroam->acl_stalist[idx].valid){
			found = TRUE;

			if(pfroam->acl_stalist[idx].processed){
				// send oid to clear acl entry in driver
				if(del_drvr_acl_entry(pfroam,&pfroam->acl_stalist[idx]) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Failed to delete drvr acl entry\n");
				}
			}

			pfroam->acl_stalist[idx].valid = FALSE;
			memset(&pfroam->acl_stalist[idx],0x0,sizeof(sta_entry));
			pfroam->acl_sta_count--;

			DBGPRINT(DEBUG_TRACE, ": Deleted ACL idx:%x, TotalCount:%d\n",idx,pfroam->acl_sta_count);
		}
	}
	pthread_mutex_unlock(&pfroam->acllock);


	if(!found){
		pthread_mutex_lock(&pfroam->stalock);

		for(idx = 0;(idx < MAX_SUPP_STA) && pfroam->sta_count; idx++){
			if( !memcmp(pfroam->stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN) && pfroam->stalist[idx].valid){
				found = TRUE;
				pfroam->stalist[idx].valid = FALSE;
				memset(&pfroam->stalist[idx],0x0,sizeof(sta_entry));
				pfroam->sta_count--;
				DBGPRINT(DEBUG_TRACE, ": Deleted idx:%x, TotalCount:%d\n",idx,pfroam->sta_count);
			}
		}
		pthread_mutex_unlock(&pfroam->stalock);
	}

	DBGPRINT(DEBUG_INFO, "<-\n");
	return status;

}

static int event_sta_disconnected (pfroam_ctx pfroam, char *data)
{
	u8 status = FROAM_CODE_SUCCESS;
	pfroam_event_sta_disconn pevt_data = (pfroam_event_sta_disconn)data;
	u8 idx = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	BOOLEAN found = FALSE;

	DBGPRINT(DEBUG_INFO, "->\n");

	if(pevt_data->hdr.event_len != (sizeof(froam_event_sta_disconn) - sizeof(froam_event_hdr))){
		DBGPRINT(DEBUG_ERROR, ": Len error, PayloadLen:%d, expected:%d\n",
			pevt_data->hdr.event_len,(int)(sizeof(froam_event_sta_disconn) - sizeof(froam_event_hdr)));
		return FROAM_CODE_INVALID_ARG;
	}

	if( !memcmp(zero_mac,pevt_data->mac,MAC_ADDR_LEN) ){
		DBGPRINT(DEBUG_ERROR, ": Zero MAC\n");
		return FROAM_CODE_INVALID_ARG;
	}
	else
		DBGPRINT(DEBUG_TRACE, ": STA MAC %02x-%02x-%02x-%02x-%02x-%02x\n",
		    pevt_data->mac[0],pevt_data->mac[1],pevt_data->mac[2],pevt_data->mac[3],
		    pevt_data->mac[4],pevt_data->mac[5]);

	pthread_mutex_lock(&pfroam->acllock);
	
	for(idx = 0; (idx <MAX_SUPP_STA) && pfroam->acl_sta_count; idx++){
		if( !memcmp(pfroam->acl_stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN) && pfroam->acl_stalist[idx].valid){
			found = TRUE;

			if(!pfroam->acl_stalist[idx].processed){
				pfroam->acl_stalist[idx].valid = FALSE;
				memset(&pfroam->acl_stalist[idx],0x0,sizeof(sta_entry));
				pfroam->acl_sta_count--;
				
				DBGPRINT(DEBUG_TRACE, ": Deleted ACL idx:%x, TotalCount:%d\n",idx,pfroam->acl_sta_count);
			}
		}
	}
	pthread_mutex_unlock(&pfroam->acllock);


	if(!found){
		pthread_mutex_lock(&pfroam->stalock);

		for(idx = 0;(idx < MAX_SUPP_STA) && pfroam->sta_count; idx++){
			if( !memcmp(pfroam->stalist[idx].mac,pevt_data->mac,MAC_ADDR_LEN) ){
				found = TRUE;
				pfroam->stalist[idx].valid = FALSE;
				memset(&pfroam->stalist[idx],0x0,sizeof(sta_entry));
				pfroam->sta_count--;
				DBGPRINT(DEBUG_TRACE, ": Deleted idx:%x, TotalCount:%d\n",idx,pfroam->sta_count);
			}
		}

		pthread_mutex_unlock(&pfroam->stalock);
	}

	DBGPRINT(DEBUG_INFO, "<-\n");

	return status;

}

static int event_clear_mntr_list (pfroam_ctx pfroam)
{
	u8 status = FROAM_CODE_SUCCESS;
	u8 idx = 0;

	DBGPRINT(DEBUG_INFO, "->\n");

	pthread_mutex_lock(&pfroam->mntrlock);

	for(idx = 0;(idx < MAX_SUPP_STA) && pfroam->mntr_sta_count; idx++){
		pfroam->mntr_stalist[idx].valid = FALSE;
		memset(&pfroam->mntr_stalist[idx],0x0,sizeof(mnt_sta_entry));
		pfroam->mntr_sta_count--;
	}

	pthread_mutex_unlock(&pfroam->mntrlock);

	DBGPRINT(DEBUG_INFO, ": TotalCount:%d\n",pfroam->mntr_sta_count);

	return status;

}

static int event_clear_acl_list (pfroam_ctx pfroam)
{
	u8 status = FROAM_CODE_SUCCESS;
	u8 idx = 0;

	DBGPRINT(DEBUG_INFO, "->\n");

	pthread_mutex_lock(&pfroam->acllock);

	for(idx = 0;(idx < MAX_SUPP_STA) && pfroam->acl_sta_count; idx++){
		pfroam->acl_stalist[idx].valid = FALSE;
		memset(&pfroam->acl_stalist[idx],0x0,sizeof(acl_sta_entry));
		pfroam->acl_sta_count--;
	}

	pthread_mutex_unlock(&pfroam->acllock);

	DBGPRINT(DEBUG_INFO, ": TotalCount:%d\n",pfroam->acl_sta_count);

	return status;

}

int froam_event_handle(pfroam_ctx pfroam, char *data)
{
	struct _froam_event *froam_event_data = (struct _froam_event *)data;
	DBGPRINT(DEBUG_INFO, "->\n");

	switch (froam_event_data->event_id)
	{
		case FROAM_EVT_FROAM_SUPP:
		{
			DBGPRINT(DEBUG_OFF,"FROAM_EVT_FROAM_SUPP : %u\n", (UINT32) froam_event_data->event_body[0]);
			pfroam->en_force_roam_supp = froam_event_data->event_body[0];

			if(pfroam->en_force_roam_supp){
				if(set_drvr_acl_policy(pfroam,ACL_POLICY_NEGATIVE_LIST) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Set ACL Policy failed, Exit App\n");
					exit(1);
				}
			
				if(set_drvr_mntr_rule(pfroam, FROAM_MNTR_RULE) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Set Mntr Rule failed, Exit App\n");
					exit(1);
				}
			
				if(get_drv_thresholds(pfroam) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Get Threshold failed, will use defaults\n");
				}
			}
			break;
		}
		case FROAM_EVT_STA_RSSI_LOW:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_RSSI_LOW\n");
			if(pfroam->en_force_roam_supp)
				event_sta_poor_rssi(pfroam, data);
			break;

		case FROAM_EVT_STA_RSSI_GOOD:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_RSSI_GOOD\n");
			if(pfroam->en_force_roam_supp)
				event_sta_good_rssi(pfroam, data);
			break;

		case FROAM_EVT_STA_DISCONNECT:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_DISCONNECT\n");
			if(pfroam->en_force_roam_supp)
				event_sta_disconnected(pfroam, data);
			break;

		case FROAM_EVT_CLEAR_MNTR_LIST:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_CLEAR_MNTR_LIST\n");
			if(pfroam->en_force_roam_supp)
				event_clear_mntr_list(pfroam);
			break;

		case FROAM_EVT_CLEAR_ACL_LIST:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_CLEAR_ACL_LIST\n");
			if(pfroam->en_force_roam_supp)
				event_clear_acl_list(pfroam);
			break;

		case FROAM_EVT_STA_AGEOUT_TIME:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_AGEOUT_TIME : %u\n", (UINT32) froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->sta_ageout_time = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_MNTR_AGEOUT_TIME:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_MNTR_AGEOUT_TIME : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->mntr_ageout_time = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_MNTR_MIN_PKT_COUNT:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_MNTR_MIN_PKT_COUNT : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->mntr_min_pkt_count = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_MNTR_MIN_TIME:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_MNTR_MIN_TIME : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->mntr_min_time = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_STA_AVG_RSSI_PKT_COUNT:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_AVG_RSSI_PKT_COUNT : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->mntr_avg_rssi_pkt_count = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_STA_DETECT_RSSI_THRESHOLD:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_STA_DETECT_RSSI_THRESHOLD : %d\n", (char)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->sta_detect_rssi_threshold = (char)froam_event_data->event_body[0];
			break;

		case FROAM_EVT_ACL_AGEOUT_TIME:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_ACL_AGEOUT_TIME : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->acl_ageout_time = froam_event_data->event_body[0];
			break;

		case FROAM_EVT_ACL_HOLD_TIME:
			DBGPRINT(DEBUG_TRACE,"FROAM_EVT_ACL_HOLD_TIME : %u\n", (UINT32)froam_event_data->event_body[0]);
			if(pfroam->en_force_roam_supp)
				pfroam->acl_hold_time = froam_event_data->event_body[0];
			break;

		default:
			DBGPRINT(DEBUG_ERROR,"Unkown event. (%u)\n",
						froam_event_data->event_id);
			break;
	}

	DBGPRINT(DEBUG_INFO, "<-\n");	
	return 0;
}

int handle_sta_mntr_msg(pfroam_ctx pfroam,char *data, struct in_addr *pPeerAddr, u8 *pSrc)
{
	u8 status = FROAM_CODE_SUCCESS;
	pfroam_msg_sta_mntr pmsg = (pfroam_msg_sta_mntr)data;
	u8 idx = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};

	DBGPRINT(DEBUG_INFO, "->\n");

	if(pmsg->hdr.payloadLen != (sizeof(froam_msg_sta_mntr) - sizeof(froam_hdr))){
		DBGPRINT(DEBUG_ERROR, ": Len error, PayloadLen:%d, expected:%d\n",
			pmsg->hdr.payloadLen,(int)(sizeof(froam_msg_sta_mntr) - sizeof(froam_hdr)));
		return FROAM_CODE_INVALID_ARG;	// i.e. ignore such msg
	}

	if( !memcmp(zero_mac,pmsg->mac,MAC_ADDR_LEN) ){
		DBGPRINT(DEBUG_ERROR, ": Zero MAC\n");
		return FROAM_CODE_INVALID_ARG;	// i.e. ignore such msg	
	}
	else
		DBGPRINT(DEBUG_TRACE, ": STA MAC %02x-%02x-%02x-%02x-%02x-%02x Channel:%d\n",
			pmsg->mac[0],pmsg->mac[1],pmsg->mac[2],pmsg->mac[3],
			pmsg->mac[4],pmsg->mac[5], pmsg->channel);

	pthread_mutex_lock(&pfroam->mntrlock);

	if(pfroam->mntr_sta_count >= MAX_SUPP_STA){
		pthread_mutex_unlock(&pfroam->mntrlock);
		DBGPRINT(DEBUG_OFF, ": Mntr List Full\n");
		return FROAM_CODE_TABLE_FULL;	// i.e. ignore such msg
	}

	for(idx = 0;pfroam->mntr_sta_count && idx < MAX_SUPP_STA; idx++){
		if( !memcmp(pfroam->mntr_stalist[idx].mac,pmsg->mac,MAC_ADDR_LEN) && pfroam->mntr_stalist[idx].valid){
			//pfroam->mntr_stalist[idx].processed = FALSE; // reprocess for this sta??
			//pfroam->mntr_stalist[idx].timeticks = 0; // consider entry as just added ??
			pthread_mutex_unlock(&pfroam->mntrlock);
			DBGPRINT(DEBUG_ERROR, ": Mntr entry already present and active\n");
			return FROAM_CODE_SUCCESS;	//ignore event as sta already exists  OR over-write if any change??	rethink
		}
	}

	for(idx = 0;(idx < MAX_SUPP_STA); idx++){
		if(!pfroam->mntr_stalist[idx].valid)
			break;
	}

	if(idx < MAX_SUPP_STA){
		memset(&pfroam->mntr_stalist[idx],0x0,sizeof(mnt_sta_entry));
		pfroam->mntr_stalist[idx].valid = TRUE;
		memcpy(pfroam->mntr_stalist[idx].mac,pmsg->mac,MAC_ADDR_LEN);
		pfroam->mntr_stalist[idx].channel = pmsg->channel;
		pfroam->mntr_stalist[idx].timeticks = 0;	// to use for time track		todo
		pfroam->mntr_stalist[idx].processed = NOT_PROCESSED;//FALSE;
		memcpy(&pfroam->mntr_stalist[idx].PeerAddr,pPeerAddr, sizeof(pfroam->mntr_stalist[idx].PeerAddr));
		pfroam->mntr_sta_count++;
		DBGPRINT(DEBUG_TRACE, ": Assigned Mntr idx:%x, TotalCount:%d\n",idx,pfroam->mntr_sta_count);
	}
	pthread_mutex_unlock(&pfroam->mntrlock);

	DBGPRINT(DEBUG_INFO, "<-\n");

	return status;

}

int handle_sta_detect_msg(pfroam_ctx pfroam,char *data)
{
	u8 status = FROAM_CODE_SUCCESS;
	pfroam_msg_sta_detect pmsg = (pfroam_msg_sta_detect)data;
	u8 idx = 0, idx2 = 0;
	u8 zero_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	BOOLEAN found = FALSE;

	DBGPRINT(DEBUG_INFO, "->\n");

	if(pmsg->hdr.payloadLen != (sizeof(froam_msg_sta_detect) - sizeof(froam_hdr))){
		DBGPRINT(DEBUG_ERROR, ": Len error, PayloadLen:%d, expected:%d\n",
			pmsg->hdr.payloadLen,(int)(sizeof(froam_msg_sta_detect) - sizeof(froam_hdr)));
		return FROAM_CODE_INVALID_ARG;	// i.e. ignore such msg
	}
	
	if( !memcmp(zero_mac,pmsg->mac,MAC_ADDR_LEN) ){
		DBGPRINT(DEBUG_ERROR, ": Zero MAC\n");
		return FROAM_CODE_INVALID_ARG;	// i.e. ignore such msg	
	}
	else
		DBGPRINT(DEBUG_TRACE, ": STA MAC %02x-%02x-%02x-%02x-%02x-%02x Channel:%d\n",
			pmsg->mac[0],pmsg->mac[1],pmsg->mac[2],pmsg->mac[3],
			pmsg->mac[4],pmsg->mac[5], pmsg->channel);

	pthread_mutex_lock(&pfroam->mntrlock);

	for(idx = 0;pfroam->mntr_sta_count && (idx < MAX_SUPP_STA); idx++){
		if( !memcmp(pfroam->mntr_stalist[idx].mac,pmsg->mac,MAC_ADDR_LEN) && pfroam->mntr_stalist[idx].valid){
			found = TRUE;
			if(pfroam->mntr_stalist[idx].processed != DRVR_ENTRY_REMOVED){
				DBGPRINT(DEBUG_TRACE, ": Discard Mntr Entry idx:%x\n",idx);

				// send oid to remove mntry entry from driver
				if(pfroam->mntr_stalist[idx].processed == DRVR_ENTRY_ADDED){
					if(del_drvr_mntr_entry(pfroam,&pfroam->mntr_stalist[idx],idx) != FROAM_CODE_SUCCESS){
						DBGPRINT(DEBUG_ERROR, "Failed to delete drvr mntr entry\n");
					}
				}
				pfroam->mntr_stalist[idx].processed = DRVR_ENTRY_REMOVED;
			}
		}
	}

	pthread_mutex_unlock(&pfroam->mntrlock);

	if(found){
		DBGPRINT(DEBUG_INFO, "<-\n");
		return status;
	}

	pthread_mutex_lock(&pfroam->stalock);

	for(idx = 0;pfroam->sta_count && (idx < MAX_SUPP_STA); idx++){
		if( !memcmp(pfroam->stalist[idx].mac,pmsg->mac,MAC_ADDR_LEN) && pfroam->stalist[idx].valid &&
			(pfroam->stalist[idx].channel == pmsg->channel)){
			pfroam->stalist[idx].valid = FALSE;
			memset(&pfroam->stalist[idx],0x0,sizeof(sta_entry));
			pfroam->sta_count--;
			found = TRUE;
			DBGPRINT(DEBUG_TRACE, ": Deleted STA idx:%x, TotalCount:%d\n",idx,pfroam->sta_count);
		}
	}

	pthread_mutex_unlock(&pfroam->stalock);

	if(found){

		pthread_mutex_lock(&pfroam->acllock);
		if(pfroam->acl_sta_count >= MAX_SUPP_STA){

			pthread_mutex_unlock(&pfroam->acllock);
			DBGPRINT(DEBUG_ERROR, ": ACL List Full\n");
		}
		else{
			
			for(idx2 = 0;(idx2 < MAX_SUPP_STA); idx2++){
				if(!pfroam->acl_stalist[idx2].valid)
					break;
			}
			
			if(idx2 < MAX_SUPP_STA){
				memset(&pfroam->acl_stalist[idx2],0x0,sizeof(acl_sta_entry));
				pfroam->acl_stalist[idx2].valid = TRUE;
				memcpy(pfroam->acl_stalist[idx2].mac,pmsg->mac,MAC_ADDR_LEN);
				pfroam->acl_stalist[idx2].channel = pmsg->channel;//pfroam->stalist[idx].channel
				pfroam->acl_stalist[idx2].timeticks = 0; // to use for time track		todo
				pfroam->acl_stalist[idx2].processed = FALSE;
				pfroam->acl_sta_count++;
				DBGPRINT(DEBUG_TRACE, ": Assigned ACL idx:%x, TotalCount:%d\n",idx2,pfroam->acl_sta_count);
			}

		}

		pthread_mutex_unlock(&pfroam->acllock);

	}

	DBGPRINT(DEBUG_INFO, "<-\n");
	return status;

}

int handle_froam_msg(void *pctx,char *data, struct in_addr	*pPeerAddr)
{
	pfroam_ctx pfroam = (pfroam_ctx)pctx;
	p_froam_hdr pmsg_hdr = (p_froam_hdr)data;

	switch(pmsg_hdr->cmd){

	case FROAM_MSG_STA_MNTR:
		DBGPRINT(DEBUG_TRACE,"FROAM_MSG_STA_MNTR from %02x-%02x-%02x-%02x-%02x-%02x \n",
			pmsg_hdr->br0_mac[0],pmsg_hdr->br0_mac[1],pmsg_hdr->br0_mac[2],pmsg_hdr->br0_mac[3],pmsg_hdr->br0_mac[4],pmsg_hdr->br0_mac[5]);
		if(pfroam->en_force_roam_supp)
			handle_sta_mntr_msg(pfroam,data, pPeerAddr, pmsg_hdr->br0_mac);
		break;

	case FROAM_MSG_STA_DETECT:
		DBGPRINT(DEBUG_TRACE,"FROAM_MSG_STA_DETECT from %02x-%02x-%02x-%02x-%02x-%02x \n",
			pmsg_hdr->br0_mac[0],pmsg_hdr->br0_mac[1],pmsg_hdr->br0_mac[2],pmsg_hdr->br0_mac[3],pmsg_hdr->br0_mac[4],pmsg_hdr->br0_mac[5]);
		if(pfroam->en_force_roam_supp)
			handle_sta_detect_msg(pfroam,data);
		break;

	default:
		DBGPRINT(DEBUG_ERROR,"Unkown msg. (%u)\n",
						pmsg_hdr->cmd);
		break;
	}
	
	return FROAM_CODE_SUCCESS;
}

struct _froam_event_ops froam_event_ops = {
	.event_handle = froam_event_handle,
};

void *server(void *ctx)
{
	pfroam_ctx pfroam = (pfroam_ctx) ctx;
	int server_socket;
	struct sockaddr_in server_address, client_address;
	char buf[512];
	unsigned int clientLength;
	int checkCall, message;
	int broadcast = 1;
	struct in_addr	PeerAddr;

	DBGPRINT(DEBUG_INFO, "->\n");

	/*Create socket */
	server_socket=socket(AF_INET, SOCK_DGRAM, 0);

	if(server_socket == -1)
		perror("Error: socket failed");

	setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST,	&broadcast, sizeof(broadcast));

	bzero((char*) &server_address, sizeof(server_address));

	/*Fill in server's sockaddr_in*/
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(FROAM_PORT);

	/*Bind server socket and listen for incoming clients*/
	checkCall = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	if(checkCall == -1)
		perror("Error: bind call failed");

	while(!pfroam->terminated)//(1)
	{
		clientLength = sizeof(client_address);
		memset(buf, '\0', BUFLEN);
		message = 0;
		message = recvfrom(server_socket, buf, BUFLEN, 0,
			  (struct sockaddr*) &client_address, &clientLength);

		if(message == -1)
			perror("Error: recvfrom call failed");

		if(!memcmp(&buf[0], "ROAM", 4))
		{
			if(!memcmp(&buf[4], br0_addr, MAC_ADDR_LEN)){
				//printf("Error: own device packet rcvd !!! Ignore");
				continue;
			}

			DBGPRINT(DEBUG_TRACE,"SERVER: read %d bytes from IP %s(%s)\n", message,
			  inet_ntoa(client_address.sin_addr), buf);

			memcpy(&PeerAddr,&(client_address.sin_addr),sizeof(PeerAddr));		
			handle_froam_msg(ctx, buf, &PeerAddr);
		}
	}

	checkCall = close(server_socket);

	if(checkCall == -1)
		perror("Error: bind call failed");

	DBGPRINT(DEBUG_INFO, "<-\n");

	return NULL;	
}

void send_peers_froam_msg(char *buf, u8 buf_len)
{

	int sock, sinlen; //status,
	struct sockaddr_in sock_in;
	int yes = 1;
	char netif[] = "br-lan";
	DBGPRINT(DEBUG_INFO, "->\n");

	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	
	sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	//status = 
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
//		DBGPRINT(DEBUG_OFF,"CLIENT: Setsockopt Status = %d\n", status);

	setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, netif, sizeof(netif));
	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK 
	*/ 
	
	sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
	sock_in.sin_port = htons(FROAM_PORT); /* port number */
	sock_in.sin_family = AF_INET;

	//status = 
	sendto(sock, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);

	//DBGPRINT(DEBUG_OFF,"CLIENT: sendto Status = %d\n", status);

	close(sock);
	shutdown(sock, 2);
	sleep (1);

}

void send_froam_msg(char *buf, u8 buf_len, struct in_addr *pPeerAddr)
{
	int sock, sinlen; //status,
	struct sockaddr_in sock_in;
	//int yes = 1;
	char netif[] = "br-lan";
	DBGPRINT(DEBUG_INFO, "->\n");

	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	
	sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, netif, sizeof(netif));
	/* -1 = 255.255.255.255 this is a BROADCAST address,
	 a local broadcast address could also be used.
	 you can comput the local broadcat using NIC address and its NETMASK 
	*/ 

	sock_in.sin_addr.s_addr= pPeerAddr->s_addr;
	sock_in.sin_port = htons(FROAM_PORT); /* port number */
	sock_in.sin_family = AF_INET;

	DBGPRINT(DEBUG_TRACE,"Send Pkt to IP %s\n",inet_ntoa(sock_in.sin_addr));

	//status = 
	sendto(sock, buf, buf_len, 0, (struct sockaddr *)&sock_in, sinlen);

	close(sock);
	shutdown(sock, 2);
	sleep (1);

}

int froam_init(pfroam_ctx pfroam, 
				 struct _froam_event_ops *event_ops,
				 int drv_mode)
{
	int ret = 0;

	DBGPRINT(DEBUG_OFF, "->\n");

	/* Initialze event loop */
	ret = eloop_init();
	
	if (ret)
	{	
		DBGPRINT(DEBUG_ERROR, "eloop_register_timeout failed.\n");
		return -1;
	}

	memset(pfroam, 0 , sizeof(froam_ctx));

	/* use wireless extension */
	pfroam->drv_ops = &froam_drv_wext_ops;

	pfroam->event_ops = event_ops;

	pfroam->drv_data = pfroam->drv_ops->drv_inf_init(pfroam);

	pfroam->en_force_roam_supp = FROAM_SUPP_DEF;
	pfroam->sta_ageout_time = STALIST_AGEOUT_TIME;
	pfroam->mntr_ageout_time = MNTRLIST_AGEOUT_TIME;
	pfroam->mntr_min_pkt_count = MNTR_MIN_PKT_COUNT;
	pfroam->mntr_min_time = MNTR_MIN_TIME;
	pfroam->mntr_avg_rssi_pkt_count = AVG_RSSI_PKT_COUNT;
	pfroam->sta_detect_rssi_threshold = (-1) * STA_DETECT_RSSI;
	pfroam->acl_ageout_time = ACLLIST_AGEOUT_TIME;
	pfroam->acl_hold_time = ACLLIST_HOLD_TIME;

	pthread_mutex_init(&pfroam->stalock, NULL);//0);
	pthread_mutex_init(&pfroam->mntrlock, NULL);//0);
	pthread_mutex_init(&pfroam->acllock, NULL);//0);

	pfroam->sta_count = 0;
	pfroam->mntr_sta_count = 0;
	pfroam->acl_sta_count = 0;

	memset(pfroam->stalist, 0 , (MAX_SUPP_STA * sizeof(sta_entry)));
	memset(pfroam->mntr_stalist, 0 , (MAX_SUPP_STA * sizeof(mnt_sta_entry)));
	memset(pfroam->acl_stalist, 0 , (MAX_SUPP_STA * sizeof(acl_sta_entry)));

	return 0;
}

int froam_deinit(pfroam_ctx pfroam)
{
    int ret = 0;

    DBGPRINT(DEBUG_OFF, "->\n");

    ret = pfroam->drv_ops->drv_inf_exit(pfroam);

    if (ret)
        return -1;

    return 0;
}

static void froam_terminate(int sig, void *signal_ctx)
{
	pfroam_ctx pfroam = (pfroam_ctx) signal_ctx;

	DBGPRINT(DEBUG_OFF, "->\n");

	eloop_terminate();

	pfroam->terminated = 1;

	froam_deinit(pfroam);

	pthread_mutex_destroy(&pfroam->stalock);
	pthread_mutex_destroy(&pfroam->mntrlock);
	pthread_mutex_destroy(&pfroam->acllock);
}

void froam_run(pfroam_ctx pfroam)
{
	pthread_t td1, td2, td3;

	DBGPRINT(DEBUG_OFF, "->\n");
	
	eloop_register_signal_terminate(froam_terminate, pfroam);

recheck_intf:
	
	sleep(5);

	if(!pfroam->terminated){

		if( CheckIntf(CARD1_AP1) ){
			if(get_drv_froam_supp(pfroam) != FROAM_CODE_SUCCESS){
				DBGPRINT(DEBUG_ERROR, "Get FROAM Supp failed, Exit App\n");
				exit(1);
			}

			check_active_intf();

			if(pfroam->en_force_roam_supp){
				if(set_drvr_acl_policy(pfroam,ACL_POLICY_NEGATIVE_LIST) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Set ACL Policy failed, Exit App\n");
					exit(1);
				}

				if(set_drvr_mntr_rule(pfroam, FROAM_MNTR_RULE) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Set Mntr Rule failed, Exit App\n");
					exit(1);
				}

				if(get_drv_thresholds(pfroam) != FROAM_CODE_SUCCESS){
					DBGPRINT(DEBUG_ERROR, "Get Threshold failed, will use defaults\n");
				}
			}
			else{
				DBGPRINT(DEBUG_OFF, "Froam driver Support found disabled\n");
			}

			//p_regrp->regroup_req_allowed = TRUE;

			pthread_create(&td1, NULL, server, pfroam);
			pthread_create(&td2, NULL, mtk_if_notify_thread, pfroam);
			pthread_create(&td3, NULL, process_sta_list, pfroam);

			DBGPRINT(DEBUG_OFF, "Force Roam Init done\n");

			eloop_run();
		}
		else{
			DBGPRINT(DEBUG_INFO, "ra0 interface down, Recheck post 5 sec\n");
			goto recheck_intf;
			//froam_terminate(SIGTERM, pfroam);
			//exit(1);		
		}
	}

}


