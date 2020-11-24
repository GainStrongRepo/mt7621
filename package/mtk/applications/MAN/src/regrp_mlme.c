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
#include "eloop.h"

#include "regrp.h"

#ifdef REGROUP_SUPPORT
/*Global Definition*/
UINT8 g_dialog_token = 0;
regrp_sta_entry_struct g_regrp_sta_list[MAX_VR_STA_NUM];
UINT8 regrp_pending;

UINT8 cand_list_2g_count;
ntw_info cand_list_2g[MAX_VIRTUAL_REPT_NUM];
UINT8 cand_list_5g_count;
ntw_info cand_list_5g[MAX_VIRTUAL_REPT_NUM];


/*extern*/
extern repeater_list_struct g_virtual_rept_list[];
extern repeater_list_struct g_own_rept_info;
extern sta_entry_struct g_station_list[MAX_STA_SUPPORT];

extern char cli_interface_5g[16];
extern char cli_interface_2g[16];

extern unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern UINT8 trigger_regrp;
extern struct os_time last_entry_added_time;
extern UINT32 regrp_periodic_time;

extern char regrp_current_wt[NETWORK_WEIGHT_LEN];
extern char regrp_expected_wt_1[MAC_ADDR_LEN];
extern char regrp_expected_wt_2[MAC_ADDR_LEN];
extern char regrp_disconnect_sta;
extern char regrp_dhcp_timer_running;
extern char regrp_dhcp_defer;

extern signed char g_default_rssi_threshold;
extern signed char g_default_max_rssi_threshold;
extern signed char g_custom_rssi_th;

UINT8 regrp_is_ap_present();

p_repeater_list_struct regrp_get_first_rept()
{
	p_repeater_list_struct p_rept_entry = g_virtual_rept_list;
	int i = 0;
	for(i=0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (p_rept_entry[i].valid)
		{
			return &p_rept_entry[i];
		}
	}
	return NULL;

}

p_repeater_list_struct regrp_get_next_rept(p_repeater_list_struct p_rept )
{
	
	p_repeater_list_struct p_rept_entry = g_virtual_rept_list;
	int i = 0;

	for(i=p_rept->entry_idx+1; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (p_rept_entry[i].valid)
		{
			return &p_rept_entry[i];
		}
	}
	return NULL;

}
void regrp_mlme_search_best_ap(p_sta_entry_struct p_entry)
{

}

/*Handle the messages for the connected STA*/
/*each STA will have its own state machine.*/
void regrp_mlme_handle_sta_message(UINT8 *buf, UINT32 buf_len)
{
 // Handle driver messages.
}

/*update the sta mlme state*/
void regrp_mlme_get_sta_mlme_state(p_sta_entry_struct p_entry)
{


}

p_regrp_sta_entry_struct regrp_mlme_search_regrp_sta_by_mac(UINT8 * mac_addr)
{
	p_regrp_sta_entry_struct p_entry = g_regrp_sta_list;
	int i;

	for (i=0; i< MAX_VR_STA_NUM; i++)
	{
		if( p_entry[i].valid && !memcmp(mac_addr, p_entry[i].mac_addr, MAC_ADDR_LEN))
		{
			return &p_entry[i];
		}
	}
	return NULL;
}

#if 0
p_sta_entry_struct regrp_mlme_search_sta_by_mac(UINT8 * mac_addr)
{
		p_sta_entry_struct p_entry = g_station_list;
		int i;
	
		for (i=0; i< MAX_STA_SUPPORT; i++)
		{
			if(p_entry[i].valid && !memcmp(mac_addr, p_entry[i].mac_addr, MAC_ADDR_LEN))
			{
				return &p_entry[i];
			}
		}
		return NULL;
}
#endif

#if 0
p_regrp_sta_entry_struct regrp_mlme_add_regrp_sta(p_msg_1_struct p_msg_1)
{

// add vr entry and copy the structure.
	p_regrp_sta_entry_struct p_entry = g_regrp_sta_list;
	int i;

	for (i=0; i< MAX_VR_STA_NUM; i++)
	{
		if( !p_entry[i].valid)
		{
			p_entry[i].valid = TRUE;
			p_entry[i].ip_addr = p_msg_1->sta_ip_addr;
			p_entry[i].dialog_token = p_msg_1->dialog_token;
			memcpy(p_entry[i].sta_mlme_state, p_msg_1->sta_mlme_state,p_msg_1->mlme_state_len);
			p_entry[i].mlme_state_len = p_msg_1->mlme_state_len;
			memcpy(p_entry[i].mac_addr, p_msg_1->sta_mac_addr,MAC_ADDR_LEN);
		//	p_entry[i].regrp_rept_inf_idx
		// TODO: Raghav add more.
			return &p_entry[i];
		}
	}
	return NULL;
}

void  regrp_mlme_del_regrp_sta(p_regrp_sta_entry_struct p_entry)
{
	memset(p_entry,0,sizeof(regrp_sta_entry_struct));
}

#endif
p_repeater_list_struct regrp_mlme_search_rept_by_ip(UINT32 ip_addr)
{
	int i=0;
	p_repeater_list_struct p_rept = g_virtual_rept_list;
	while(i < MAX_VIRTUAL_REPT_NUM)
	{
		printf("Peer IP : %x, looking for %x\n", p_rept[i].ip_addr,ip_addr);
		if(p_rept[i].valid && p_rept[i].ip_addr == ip_addr)
		{
			return &p_rept[i];
		}
		i++;
	}
	printf("err no rept found with IP : %x\n", ip_addr);
	return NULL;
}

void regrp_msg_2_timeout (void *eloop_data, void *user_ctx)
{
	p_repeater_list_struct p_rept = (p_repeater_list_struct)user_ctx;
	p_repeater_list_struct p_next_rept ;

	printf("Msg_2 not received from rept: %x", p_rept->ip_addr);
	p_rept->regrp_state = REGRP_STATE_PEER_DONE;
	
	p_rept->is_processed_tmp = 0;
	p_rept->is_processed = 1;

	
	p_next_rept = regrp_get_next_rept(p_rept);

	if(p_next_rept == NULL)
	{
		// scan on all repeaters complete.
		regrp_trigger_reconnection();
	}
	else
	{
		regrp_mlme_compose_send_msg_1(&g_own_rept_info, p_next_rept);
	}
	g_own_rept_info.need_retrigger_regrp =1;
	return;
}

UINT8 regrp_mlme_compose_send_msg_regrp_initiate(p_repeater_list_struct p_rept)
{
	msg_init_struct msg_init;
	
	msg_init.action = MSG_REGRP_INIT;
	msg_init.dialog_token = 0;
	memcpy(msg_init.br_mac, g_own_rept_info.br_mac,MAC_ADDR_LEN);

	printf("%s: Ip: %x: %d\n", __func__, p_rept->ip_addr, p_rept->tcp_tx_sock);
	hex_dump("ToMAC", p_rept->br_mac,6);
	/*send to the socket.*/

	p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
	if(p_rept->tcp_tx_sock < 0)
	{
		regrp_handle_tx_fail();
		return FALSE;
	}

	
	if(send(p_rept->tcp_tx_sock, &msg_init, sizeof(msg_init_struct),0)<0)
	{
		//printf("Send to Rept : %x failed\n", p_rept->ip_addr);
		//perror("Send MSG_INIT");
		
		p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
		if(p_rept->tcp_link_done == 1)
		{
			if(send(p_rept->tcp_tx_sock, &msg_init, sizeof(msg_init),0)<0)
			{
				printf("Send to Rept : %x failed 2nd time\n", p_rept->ip_addr);
				perror("Send MSG_INIT");
				regrp_handle_tx_fail();
				return FALSE;
			}
		}
		else
		{
			regrp_handle_tx_fail();
			return FALSE;
		}
	}
	regrp_tcp_disconnect(p_rept);
	return TRUE;
}

#if 0
regrp_mlme_compose_send_msg_regrp_end(p_repeater_list_struct p_rept)
{
	msg_init_struct msg_init;
	
	msg_1.action = MSG_REGRP_END;
	msg_1.dialog_token = 0;
	memcpy(msg_1.br_mac, g_own_rept_info.br_mac,MAC_ADDR_LEN);

	printf("%s\n", __func__);
	/*send to the socket.*/
	if(send(p_rept->tcp_tx_sock, &msg_1, sizeof(msg_1),0)<0)
	{
		printf("Send to Rept : %x failed\n", p_rept->ip_addr);
		perror("Send MSG_REGRP_END");
		return;
	}
	return;
}
#endif

regrp_handle_tx_fail()
{
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	regrp_clear_rept_state(p_own_rept);
	
	if(is_regrp_master_candidate(p_own_rept) && (p_own_rept->regrp_state == REGRP_STATE_DONE))
	{
		printf("%s:Restart Regroup\n",__func__);
		eloop_register_timeout(10, 0, restart_regroup,NULL, NULL);
		g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
		trigger_regrp = 0;
	}


}
void regrp_mlme_compose_send_msg_1(p_repeater_list_struct p_own_rept,p_repeater_list_struct p_rept)
{
	msg_1_struct msg_1;

	msg_1.action = MSG_1;
	msg_1.dialog_token = p_rept->dialog_token = g_dialog_token++;
	memcpy(msg_1.br_mac, p_own_rept->br_mac,MAC_ADDR_LEN);

	printf("%s: %d\n", __func__,p_rept->dialog_token);
	hex_dump("ToMAC", p_rept->br_mac,6);
	/*send to the socket.*/

	p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
	if(p_rept->tcp_tx_sock < 0)
	{
		//p_own_rept->need_retrigger_regrp = 1;
		regrp_handle_tx_fail();
		return;
	}


	if(send(p_rept->tcp_tx_sock, &msg_1, sizeof(msg_1),0)<0)
	{
		p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
		if(p_rept->tcp_link_done == 1)
		{
			if(send(p_rept->tcp_tx_sock, &msg_1, sizeof(msg_1),0)<0)
			{
				printf("Send to Rept : %x failed 2nd time\n", p_rept->ip_addr);
				perror("Send MSG_INIT");
				regrp_handle_tx_fail();
				return;
			}
		}
		else
		{
			regrp_handle_tx_fail();
			return;
		}
	}

	regrp_tcp_disconnect(p_rept);
	p_rept->regrp_state = REGRP_STATE_PEER_WAIT_MSG_2;
	//start wait msg_2 timer.
	eloop_register_timeout(SCAN_TIMEOUT_SEC + 5,0, regrp_msg_2_timeout, NULL, p_rept);
	printf("%s: out\n", __func__);
	return;
}

#if 0
int get_connected_ap_mac (char *buf, char* ifname)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig %s | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac",ifname);
	system(command);
	fp = fopen("temp_file_mac", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	//printf("%s\n", peer_mac);
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
			//printf("%d:", buf[i / 3]);	
		}
	} else {
		memset(buf, 0, MAC_ADDR_LEN);
	}
	//printf("\n");
#endif
}
#endif
void regrp_msg_3_timeout(void *eloop_data, void *user_ctx)
{
	printf("Msg3 not received.\n");
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
	regrp_clear_rept_state(&g_own_rept_info);

}


void regrp_mlme_compose_send_msg_2(p_repeater_list_struct p_own_rept,p_repeater_list_struct p_master_rept)
{
	UINT8 buffer[512] = {0};
	UINT16 buffer_len;
	msg_2_struct msg_2;

	printf("%s: %d\n", __func__,p_own_rept->dialog_token);
	msg_2.action = MSG_2;
	msg_2.dialog_token = p_own_rept->dialog_token;
	msg_2.status = 0;
	msg_2.cand_list_2g_count = p_own_rept->cand_list_2g_count;
	msg_2.cand_list_5g_count = p_own_rept->cand_list_5g_count;
	memcpy(msg_2.cand_list_2g, p_own_rept->cand_list_2g,p_own_rept->cand_list_2g_count*sizeof(ntw_info));
	memcpy(msg_2.cand_list_5g, p_own_rept->cand_list_5g,p_own_rept->cand_list_5g_count*sizeof(ntw_info));
	memcpy(msg_2.br_mac, p_own_rept->br_mac,MAC_ADDR_LEN);

	if(cli_interface_2g[0] != '\0') 
	{
		get_connected_ap_mac(p_own_rept->cli_2g_bssid,cli_interface_2g);
		memcpy(msg_2.cli_2g_bssid,p_own_rept->cli_2g_bssid,MAC_ADDR_LEN);
	}
	
	if(cli_interface_5g[0] != '\0')
	{
		get_connected_ap_mac(p_own_rept->cli_5g_bssid,cli_interface_5g);
		memcpy(msg_2.cli_5g_bssid,p_own_rept->cli_5g_bssid,MAC_ADDR_LEN);

	}

	p_master_rept->tcp_tx_sock = regrp_tcp_connect(p_master_rept);
	if(p_master_rept->tcp_tx_sock < 0)
	{
		
		if(cli_interface_5g[0]!= '\0')
			set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
		if(cli_interface_2g[0]!= '\0')
			set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
		return;
	}


	/*send to the socket.*/
	if(send(p_master_rept->tcp_tx_sock, &msg_2, sizeof(msg_2),0)<0)
	{
		printf("Send to Rept : %x failed\n", p_master_rept->ip_addr);
		perror("Send MSG_2");
		
		p_master_rept->tcp_tx_sock = regrp_tcp_connect(p_master_rept);
		if(p_master_rept->tcp_link_done == 1)
		{
			if(send(p_master_rept->tcp_tx_sock, &msg_2, sizeof(msg_2),0)<0)
			{
				printf("Send to Rept : %x failed 2nd time\n", p_master_rept->ip_addr);
				perror("Send MSG_2");
				
				if(cli_interface_5g[0]!= '\0')
					set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
				if(cli_interface_2g[0]!= '\0')
					set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
				return;
			}
		}
		else
		{
			if(cli_interface_5g[0]!= '\0')
				set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
			if(cli_interface_2g[0]!= '\0')
				set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
			
			return;
		}
	}
	
	regrp_tcp_disconnect(p_master_rept);
	p_own_rept->regrp_state = REGRP_SLAVE_STATE_WAIT_MSG_3;

	eloop_register_timeout(30,0,regrp_msg_3_timeout,NULL, NULL);
	//start timer.
	return;
}



UINT8 regrp_get_ap_mac(UINT8 *ap_mac_5g,UINT8 *ap_mac_2g)
{
	p_repeater_list_struct p_rept = NULL, p_own_rept = &g_own_rept_info;
	UINT8 i,j;
	
//	first search for third party AP with good RSSI

//	printf("%s\n", __FUNCTION__);
	p_rept = p_own_rept;

	while (p_rept != NULL)
	{
		
		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
			if((p_rept->cand_list_5g[i].Non_MAN == 1))
			{
				//printf("AP present\n");
				memcpy(ap_mac_5g,p_rept->cand_list_5g[i].bssid, MAC_ADDR_LEN);
				//return TRUE;
			}
		}
		for (i=0; i< p_rept->cand_list_2g_count;i++)
		{
			if((p_rept->cand_list_2g[i].Non_MAN == 1))
			{
				//printf("AP present\n");
				
				memcpy(ap_mac_2g,p_rept->cand_list_2g[i].bssid, MAC_ADDR_LEN);
				//return TRUE;
			}
		}

		if(p_rept == &g_own_rept_info)
			p_rept = regrp_get_first_rept();
		else
			p_rept = regrp_get_next_rept(p_rept);
	}
	return FALSE;
}

regrp_fill_target_wt(UINT8 *target_wt_1, UINT8 *target_wt_2)
{
	if(!is_regrp_master_candidate(&g_own_rept_info))
	{
		printf("Err: Target Wt filled by non master\n");
		return;
	}
	if(regrp_is_ap_present())
	{
		regrp_get_ap_mac(target_wt_1,target_wt_2);
	}
	else
	{
		memcpy(target_wt_1, &g_own_rept_info.network_weight[1], MAC_ADDR_LEN);
		bzero(target_wt_2,MAC_ADDR_LEN);
	}

}

void regrp_mlme_compose_send_msg_3(p_repeater_list_struct p_rept, UINT8 status)
{
	msg_3_struct msg_3;

	bzero(&msg_3, sizeof(msg_3));
	msg_3.action = MSG_3;
	msg_3.dialog_token = p_rept->dialog_token;
	msg_3.status = status;	
	memcpy(msg_3.br_mac, g_own_rept_info.br_mac,MAC_ADDR_LEN);
	if(status != 0)
	{
		memcpy(msg_3.target_2g_mac, p_rept->cli_target_mac_2g, MAC_ADDR_LEN);
		memcpy(msg_3.target_5g_mac, p_rept->cli_target_mac_5g, MAC_ADDR_LEN);
	}
	regrp_fill_target_wt(msg_3.target_wt_1, msg_3.target_wt_2);
	hex_dump("Msg3_Rept:",(UINT8 *) p_rept->br_mac, 6);
//	hex_dump("Msg_3", (UINT8 *)&msg_3, sizeof(msg_3));
	/*send to the socket.*/
	//printf("SockNum %d\n",p_rept->tcp_tx_sock);
	p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
	if(p_rept->tcp_tx_sock ==0)
	{
		regrp_handle_tx_fail();
		return;
	}

	
	if(send(p_rept->tcp_tx_sock, &msg_3, sizeof(msg_3),0)<0)
	{
		printf("Send to Rept : %x failed\n", p_rept->ip_addr);
		perror("Send MSG_3");
		regrp_handle_tx_fail();
	}

	regrp_tcp_disconnect(p_rept);
	p_rept->regrp_state = REGRP_SLAVE_STATE_DONE;
	return;
}


int regrp_mlme_compose_send_msg_4(p_repeater_list_struct p_rept)
{
	msg_init_struct msg_end;
	
	msg_end.action = MSG_REGRP_END;
	msg_end.dialog_token = 0;
	memcpy(msg_end.br_mac, g_own_rept_info.br_mac,MAC_ADDR_LEN);

	//printf("%s: Ip: %x: %d\n", __func__, p_rept->ip_addr, p_rept->tcp_tx_sock);
	hex_dump("RegrpEndToMAC", p_rept->br_mac,6);
	/*send to the socket.*/

	p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
	if(p_rept->tcp_tx_sock < 0)
	{
		regrp_handle_tx_fail();
		return FALSE;
	}

	
	if(send(p_rept->tcp_tx_sock, &msg_end, sizeof(msg_init_struct),0)<0)
	{
		//printf("Send to Rept : %x failed\n", p_rept->ip_addr);
		//perror("Send MSG_INIT");
		
		p_rept->tcp_tx_sock = regrp_tcp_connect(p_rept);
		if(p_rept->tcp_link_done == 1)
		{
			if(send(p_rept->tcp_tx_sock, &msg_end, sizeof(msg_end),0)<0)
			{
				printf("Send to Rept : %x failed 2nd time\n", p_rept->ip_addr);
				perror("Send MSG_INIT");
				regrp_handle_tx_fail();
				return FALSE;
			}
		}
		else
		{
			regrp_handle_tx_fail();
			return FALSE;
		}
	}
	regrp_tcp_disconnect(p_rept);
	return TRUE;
}


/*Scan all the APs with the same network ID*/
static int regrp_scan_req(UCHAR *ifname)
{
	struct _regrp_cmd_scan cmd_scan;
	int ret = 0;
	int sd;

	DBGPRINT(DEBUG_OFF, "-->\n");

	memset(&cmd_scan,0x0,sizeof(regrp_cmd_scan));
	cmd_scan.hdr.command_id = OID_REGROUP_CMD_SCAN;
	cmd_scan.hdr.command_len = sizeof(regrp_cmd_scan) - sizeof(regrp_cmd_hdr);;
	cmd_scan.scan_type = SCAN_GENERAL;
//	if(pssid)
//	memcpy(cmd_scan.ssid,pssid,strlen((char *)pssid));
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_set_info(OID_WH_EZ_REGROUP_COMMAND, sd, ifname, &cmd_scan, sizeof(cmd_scan)));
	{
	}
	close(sd);
	DBGPRINT(DEBUG_OFF, "<--\n");
	return ret;

		
}


void regrp_start_scan(p_repeater_list_struct p_rept)
{
	//scan 5g iterface first
	printf("%s\n", __func__);
	if(interface_5g[0] != '\0')
	{
		regrp_scan_req(interface_5g);
	}
	else if(interface_2g[0] != '\0')
	{
		p_rept->scan_done_5g =1;
		regrp_scan_req(interface_2g);
	}
}


void regrp_scan_timeout(void *eloop_data, void *user_ctx)	//decide when to cancel
{
	p_repeater_list_struct p_own_rept = (p_repeater_list_struct) user_ctx;
	if(p_own_rept == NULL)
	{
		printf("scan timeout: entry not found!\n");
	}
	printf("scan timeout for repeater\n");

	if(p_own_rept->is_master == 0)
	{
		p_repeater_list_struct p_master_rept = get_rept_by_br_mac(p_own_rept->master_mac);
		
		//scan completed on own repeater. send msg 2 to the master.
		if(p_master_rept != NULL)
			regrp_mlme_compose_send_msg_2(p_own_rept,p_master_rept);
	}
	else
	{
		//do nothing
		regrp_clear_rept_state(p_own_rept);
		if(is_regrp_master_candidate(p_own_rept) && (p_own_rept->regrp_state == REGRP_STATE_DONE))
		{
			printf("%s:Restart Regroup\n",__func__);
			eloop_register_timeout(RESTART_REGROUP_TIME, 0, restart_regroup,NULL, NULL);
			g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
			trigger_regrp = 0;
		}
	}
}

void set_regrp_mode(UCHAR * ifname, UINT8 mode)
{
	cmd_regrp_mode cmd;
	int sd;

	DBGPRINT(DEBUG_OFF, "-->: %s %d\n",ifname,mode);

	memset(&cmd,0x0,sizeof(cmd_regrp_mode));
	cmd.hdr.command_id = OID_REGROUP_CMD_EN_REGRP_MODE;
	cmd.hdr.command_len = sizeof(cmd_regrp_mode) - sizeof(regrp_cmd_hdr);;
	cmd.mode = mode;
//	if(pssid)
//	memcpy(cmd_scan.ssid,pssid,strlen((char *)pssid));
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_set_info(OID_WH_EZ_REGROUP_COMMAND, sd, ifname, &cmd, sizeof(cmd)));
	{
	}
	close(sd);
//	DBGPRINT(DEBUG_OFF, "<--\n");
	return;

		
}

void slave_regrp_timeout(void *eloop_data, void *user_ctx)
{

	printf("%s\n", __func__);
	if(is_regrp_master_candidate(&g_own_rept_info))
		return;

	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);

	regrp_clear_rept_state(&g_own_rept_info);

}

void regrp_mlme_handle_msg_initiate(p_repeater_list_struct p_rept,p_msg_init_struct p_msg_init, UINT8 msg_len)
{
	printf("%s\n", __func__);
	if(is_regrp_master_candidate(&g_own_rept_info))
	{
		printf("regrp init to master\n");
		regrp_get_node_number_wt(&g_own_rept_info);
		if(is_regrp_master_candidate(&g_own_rept_info))
		{
			printf("regrp init to master\n drop\n");
			return;
		}
	}
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,REGRP_MODE_BLOCKED);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,REGRP_MODE_BLOCKED);

	os_sleep(2,0);
	eloop_register_timeout(REGROUP_TIMEOUT_SEC,0, slave_regrp_timeout,NULL,NULL);

}

void regrp_clear_dhcp_defer()
{
	bzero(regrp_expected_wt_1,MAC_ADDR_LEN);
	bzero(regrp_expected_wt_2,MAC_ADDR_LEN);
	bzero(regrp_current_wt,NETWORK_WEIGHT_LEN);
	regrp_disconnect_sta = TRUE;
	regrp_dhcp_timer_running = FALSE;
	regrp_dhcp_defer = 0;

}


void regrp_mlme_handle_msg_4(p_repeater_list_struct p_rept,p_msg_init_struct p_msg_end, UINT8 msg_len)
{

	printf("%s\n", __func__);
	if(is_regrp_master_candidate(&g_own_rept_info))
	{
		printf("regrp end to master\n");
		regrp_get_node_number_wt(&g_own_rept_info);
		if(is_regrp_master_candidate(&g_own_rept_info))
		{
			printf("regrp init to master\n drop\n");
			return;
		}
	}
	regrp_clear_dhcp_defer();
}

#if 0
regrp_mlme_handle_msg_end(p_repeater_list_struct p_rept,p_msg_init_struct p_msg_1, UINT8 msg_len)
{
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);

	eloop_cancel_timeout(slave_regrp_timeout,NULL,NULL)
}
#endif

void regrp_mlme_handle_msg_1(p_repeater_list_struct p_rept,p_msg_1_struct p_msg_1, UINT8 msg_len)
{
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	
	if(is_regrp_master_candidate(&g_own_rept_info))
	{
		printf("regrp msg1 to master\n");
		regrp_get_node_number_wt(&g_own_rept_info);
		if(is_regrp_master_candidate(&g_own_rept_info))
		{
			printf("regrp msg1 to master\n drop\n");
			return;
		}
	}

	if(p_own_rept->regrp_state != REGRP_SLAVE_STATE_IDLE && p_own_rept->regrp_state != REGRP_SLAVE_STATE_DONE
		&& p_own_rept->regrp_state != REGRP_STATE_DONE)
	{
		printf("HandleMsg1: wrong state: %d\n", p_own_rept->regrp_state);
		return;
	}
	printf("%s: %d\n", __FUNCTION__,p_msg_1->dialog_token);
	hex_dump("MAC",p_rept->br_mac,6);
	
	p_own_rept->dialog_token = p_msg_1->dialog_token;
	p_own_rept->is_master = 0;
	memcpy(p_own_rept->master_mac, p_msg_1->br_mac, MAC_ADDR_LEN);

	hex_dump("Master MAC", p_own_rept->master_mac, MAC_ADDR_LEN);

//	os_sleep(2,0);
	
	if(cli_interface_5g[0] != '\0' && regrp_get_cand_list(cli_interface_5g,(UCHAR *)p_own_rept->cand_list_5g,&p_own_rept->cand_list_5g_count) != REGRP_CODE_SUCCESS)
	 // triger cand list config in driver for conenct / roam
	{
		printf("5G:GetCandiateList failed");
	}
	if(cli_interface_2g[0] != '\0' && regrp_get_cand_list(cli_interface_2g,(UCHAR *)p_own_rept->cand_list_2g,&p_own_rept->cand_list_2g_count) != REGRP_CODE_SUCCESS)
	 // triger cand list config in driver for conenct / roam
	{
		printf("5G:GetCandiateList failed");
	}
	
	regrp_mlme_compose_send_msg_2(p_own_rept,p_rept);

	
}


UINT8 select_root_connection_single(UINT8 rssi_above_th, UINT8 ap_present)
{
	p_repeater_list_struct p_rept = NULL,p_own_rept = &g_own_rept_info;
	UINT8 i,j;
	UINT8 max_rssi_rept_idx =0, cand_idx_5g=0xff, cand_idx_2g=0xff;
	INT8 max_rssi = -110;
	UINT8 ret =0;
	

	if(p_own_rept->is_master == 1 && ap_present)
		p_rept = p_own_rept;
	else
		p_rept = regrp_get_first_rept();

//	first search for third party AP with good RSSI
	printf("%s\n", __FUNCTION__);
	while (p_rept != NULL)
	{
		if(p_rept->is_processed == 1)
		{
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		p_rept->non_man_selected = 0;
		hex_dump("Rept_MAC", p_rept->br_mac,6);
		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
		
			if(((ap_present &&(p_rept->cand_list_5g[i].Non_MAN == 1
				|| (p_rept->cand_list_5g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_5g[i].node_number.path_len == 6)))
				|| 
				(ap_present == 0 &&(p_rept->cand_list_5g[i].node_number.path_len == 6)))
				&& 
				((rssi_above_th == 0 && p_rept->cand_list_5g[i].rssi > DEFAULT_MAX_RSSI_THRESHOLD)|| p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD))
			{
				if(p_rept->cand_list_5g[i].rssi > max_rssi)
				{
				//	hex_dump("Cand:Sel:",p_rept->cand_list_5g[i].bssid,6);

					if(p_rept == &g_own_rept_info)
						max_rssi_rept_idx = 0xff;
					else
						max_rssi_rept_idx = p_rept->entry_idx;
					max_rssi = p_rept->cand_list_5g[i].rssi;
					cand_idx_5g = i;
				//	printf("ReptIdx = %d, Rssi: %d, canIdx = %d\n",max_rssi_rept_idx,max_rssi,cand_idx_5g);
				}
			}
		}
		if(p_rept == &g_own_rept_info)
			p_rept = regrp_get_first_rept();
		else
			p_rept = regrp_get_next_rept(p_rept);
	}
	if(cand_idx_5g == 0xff )
	{
	
		if(p_own_rept->is_master == 1 && p_own_rept->non_ez_connection == 1)
			p_rept = p_own_rept;
		else
			p_rept = regrp_get_first_rept();
	
	//	first search for third party AP with good RSSI
	//	printf("%s:2g\n", __FUNCTION__);
		while (p_rept != NULL)
		{
		
			if(p_rept->is_processed == 1)
			{
				if(p_rept == &g_own_rept_info)
					p_rept = regrp_get_first_rept();
				else
					p_rept = regrp_get_next_rept(p_rept);
				continue;
			}
			for (i=0; i< p_rept->cand_list_2g_count;i++)
			{
				if(((ap_present &&(p_rept->cand_list_2g[i].Non_MAN == 1
				|| (p_rept->cand_list_2g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_2g[i].node_number.path_len == 6)))
				|| 
				(ap_present == 0 &&(p_rept->cand_list_2g[i].node_number.path_len == 6)))
				&& 
				((rssi_above_th == 0 && p_rept->cand_list_2g[i].rssi > DEFAULT_MAX_RSSI_THRESHOLD)
				|| p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD))
				{
					//p_rept->is_processed = 1;
					if(p_rept->cand_list_2g[i].rssi > max_rssi)
					{
					
					//	hex_dump("Cand:",p_rept->cand_list_5g[i].bssid,6);
						if(p_rept == &g_own_rept_info)
							max_rssi_rept_idx = 0xff;
						else
							max_rssi_rept_idx = p_rept->entry_idx;
						max_rssi = p_rept->cand_list_2g[i].rssi;
						cand_idx_2g = i;
						
		//				printf("2G ReptIdx = %d, Rssi: %d, canIdx = %d\n",max_rssi_rept_idx,max_rssi,cand_idx_2g);
					}
				}
			}
			
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
		}
	}
	//printf("5gIdx = %d, 2gIdx = %d, ReptIdx = %d\n",cand_idx_5g,cand_idx_2g,max_rssi_rept_idx);
	if(cand_idx_5g != 0xff || cand_idx_2g != 0xff)
	{
		p_repeater_list_struct p_rept_tmp;
		if(max_rssi_rept_idx == 0xff)
			p_rept_tmp = &g_own_rept_info;
		else
			p_rept_tmp = &g_virtual_rept_list[max_rssi_rept_idx];
		hex_dump("SelRept", p_rept_tmp->br_mac,6);

		if(cand_idx_5g != 0xff)
		{
			p_rept_tmp->is_processed = 1;
			p_rept_tmp->is_processed_tmp = 0;
			if(p_rept_tmp->cand_list_5g[cand_idx_5g].Non_MAN == 1)
			{
				p_rept_tmp->non_man_selected = 1;
			}
			memcpy(p_rept_tmp->cli_target_mac_5g, p_rept_tmp->cand_list_5g[cand_idx_5g].bssid, MAC_ADDR_LEN);
			//printf("Rept= %x\n", p_rept_tmp->ip_addr);
			hex_dump("5RootMac",p_rept_tmp->cli_target_mac_5g,6 );
		}
		else if (cand_idx_2g != 0xff)
		{
			p_rept_tmp->is_processed = 1;
			p_rept_tmp->is_processed_tmp = 0;
	
			if(p_rept_tmp->cand_list_2g[cand_idx_2g].Non_MAN == 1)
			{
				p_rept_tmp->non_man_selected = 1;
			}
			memcpy(p_rept_tmp->cli_target_mac_2g, p_rept_tmp->cand_list_2g[cand_idx_2g].bssid, MAC_ADDR_LEN);
			hex_dump("2RootMac",p_rept_tmp->cli_target_mac_2g,6 );
		}
		ret = 1;
	}
	return ret;
}


UINT8 select_root_connection(UINT8 rssi_above_th, UINT8 ap_present)
{
	p_repeater_list_struct p_rept, p_own_rept = &g_own_rept_info;
	UINT8 i,j;
	
//	first search for third party AP with good RSSI

	printf("%s\n", __FUNCTION__);
	if(p_own_rept->is_master == 1 && ap_present)
		p_rept = p_own_rept;
	else
		p_rept = regrp_get_first_rept();
	while (p_rept != NULL)
	{
		if(p_rept->is_processed == 1)
		{
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		p_rept->non_man_selected = 0;
		
		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
			if(((ap_present &&(p_rept->cand_list_5g[i].Non_MAN == 1
				|| (p_rept->cand_list_5g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_5g[i].node_number.path_len == 6)))
				|| 
				(ap_present == 0 &&(p_rept->cand_list_5g[i].node_number.path_len == 6)))
				&& 
				((rssi_above_th == 0 && p_rept->cand_list_5g[i].rssi > DEFAULT_MAX_RSSI_THRESHOLD)|| p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD))
			{
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
	
				if(p_rept->cand_list_5g[i].Non_MAN == 1)
				{
					p_rept->non_man_selected = 1;
				}
				memcpy(p_rept->cli_target_mac_5g, p_rept->cand_list_5g[i].bssid, MAC_ADDR_LEN);
				hex_dump("ReptMAC", p_rept->br_mac, MAC_ADDR_LEN);
				hex_dump("5RootMac",p_rept->cli_target_mac_5g,6 );
				break;
			}
		}
		if(p_rept->is_processed == 1 && p_rept->cand_list_5g[i].Non_MAN == 0)
		{	
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		/*Only execute if no 5G interface*/
		if(interface_5g == '\0')
		{
		for (i=0; i< p_rept->cand_list_2g_count;i++)
		{
			if(((ap_present &&(p_rept->cand_list_2g[i].Non_MAN == 1
				|| (p_rept->cand_list_2g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_2g[i].node_number.path_len == 6)))
				|| 
				(ap_present == 0 &&(p_rept->cand_list_2g[i].node_number.path_len == 6)))
				&& 
				((rssi_above_th == 0 && p_rept->cand_list_2g[i].rssi > DEFAULT_MAX_RSSI_THRESHOLD)
				|| p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD))
			{
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
	
				if(p_rept->cand_list_5g[i].Non_MAN == 1)
				{
					p_rept->non_man_selected = 1;
				}
				memcpy(p_rept->cli_target_mac_2g, p_rept->cand_list_2g[i].bssid, MAC_ADDR_LEN);
				
				hex_dump("ReptMAC", p_rept->br_mac, MAC_ADDR_LEN);
				hex_dump("2RootMac",p_rept->cli_target_mac_2g,6 );
				break;
			}
		}
		}
		if(p_rept == &g_own_rept_info)
			p_rept = regrp_get_first_rept();
		else
			p_rept = regrp_get_next_rept(p_rept);
	}

}

void sort_cand_list_by_rssi(pntw_info cand_list, UINT8 count)
{
	int c,d;
	ntw_info swap;
	if(count <= 1)
		return;

	for (c = 0 ; c < ( count - 1 ); c++)
	{
		for (d = 0 ; d < count - c - 1; d++)
		{
		  if (cand_list[d].rssi < cand_list[d+1].rssi) /* For decreasing order use < */
		  {
			memcpy(&swap,&cand_list[d],sizeof(ntw_info));
			memcpy(&cand_list[d],&cand_list[d+1],sizeof(ntw_info));
			//cand_list[d]   = cand_list[d+1];
			//cand_list[d+1] = swap;
			memcpy(&cand_list[d+1],&swap,sizeof(ntw_info));
		  }
		}
	}

}
UINT8 select_man_internet_connection(UINT8 rssi_above_th)
{
	p_repeater_list_struct p_rept;
	UINT8 i,j;
	printf("%s\n", __FUNCTION__);
		

//	first search for third party AP with good RSSI
	p_rept = regrp_get_first_rept();
	while (p_rept != NULL)
	{
		if(p_rept->is_processed == 1)
		{
			p_rept = regrp_get_next_rept(p_rept);
			continue;
		}

		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
			if(p_rept->cand_list_5g[i].Non_MAN == 0 
				&&( rssi_above_th == 0 || p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD)
				&& p_rept->cand_list_5g[i].internet_status & 0x2)
			{
				printf("%s: 5gProcessed\n",__func__);
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
	
				memcpy(p_rept->cli_target_mac_5g, p_rept->cand_list_5g[i].bssid, MAC_ADDR_LEN);
				
				printf("Rept= %x\n", p_rept->ip_addr);
				hex_dump("5RootMac",p_rept->cli_target_mac_5g,6 );
				// 2g connection should happen on the same MAN repeater.
				break;
			}
		}
		if(p_rept->is_processed == 1)
		{
			p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		for (i=0; i< p_rept->cand_list_2g_count;i++)
		{
			if(p_rept->cand_list_2g[i].Non_MAN == 0 
				&& ( rssi_above_th == 0 || p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD)
				&& p_rept->cand_list_2g[i].internet_status & 0x2)
			{
			
			printf("%s: 2gProcessed\n",__func__);
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
	
				memcpy(p_rept->cli_target_mac_2g, p_rept->cand_list_2g[i].bssid, MAC_ADDR_LEN);
				
				printf("Rept= %x\n", p_rept->ip_addr);
				hex_dump("5RootMac",p_rept->cli_target_mac_5g,6 );
			}
		}
		
		p_rept = regrp_get_next_rept(p_rept);
	}

}

UINT8 is_node_number_same(EZ_NODE_NUMBER node_num_1, EZ_NODE_NUMBER node_num_2)
{
	if(node_num_1.path_len == node_num_2.path_len
		&& !memcmp(node_num_1.root_mac,node_num_2.root_mac,MAC_ADDR_LEN)
		&& !memcmp(node_num_1.path, node_num_2.path, node_num_2.path_len - MAC_ADDR_LEN))
		return TRUE;
	else
		return FALSE;

}

p_repeater_list_struct search_rept_by_node_number(ntw_info candidate)
{
	p_repeater_list_struct p_rept = regrp_get_first_rept();

	if(p_rept == NULL)
		return NULL;
//	hex_dump("CandNodeNum",(UINT8 *)&candidate.node_number, sizeof(EZ_NODE_NUMBER) );
	while(p_rept)
	{
	//	hex_dump("ReptNN",(UINT8 *)&p_rept->node_number, sizeof(EZ_NODE_NUMBER) );
		if(is_node_number_same(candidate.node_number, p_rept->node_number))
		{
			//printf("isProcessed=%d\n",p_rept->is_processed);
			return p_rept;
		}
		p_rept = regrp_get_next_rept(p_rept);
	}
	//hex_dump("OwnNN", (UINT8 *)&g_own_rept_info.node_number, sizeof(EZ_NODE_NUMBER));
	if(is_node_number_same(candidate.node_number,g_own_rept_info.node_number))
		return &g_own_rept_info;
}



UINT8 select_man_processed_single(UINT8 rssi_above_th)
{
	p_repeater_list_struct p_rept, p_own_rept = &g_own_rept_info;
	UINT8 i,j=0;
	UINT8 flag = 1;
	UINT8 ret =0;
	UINT8 do_not_consider_2g = 0;

	printf("%s\n", __FUNCTION__);

	//while(flag != 0)
	{
//	first search for third party AP with good RSSI
		flag = 0;
		if(p_own_rept->is_master == 1 && regrp_is_ap_present())
			p_rept = p_own_rept;
		else
			p_rept = regrp_get_first_rept();
		
		while (p_rept != NULL)
		{
			if(p_rept->is_processed == 1)
			{
				if(p_rept == &g_own_rept_info)
					p_rept = regrp_get_first_rept();
				else
					p_rept = regrp_get_next_rept(p_rept);
				continue;
			}
			
			hex_dump("ReptMAC :", p_rept->br_mac,6);
			bzero(cand_list_5g,sizeof(cand_list_5g));
			cand_list_5g_count =0;
			for (i=0; i< p_rept->cand_list_5g_count;i++)
			{
				p_repeater_list_struct p_tmp_rept;
				if((p_tmp_rept = search_rept_by_node_number(p_rept->cand_list_5g[i])) != NULL
					&& p_tmp_rept->is_processed)
				{
					// should be in sorted order
					memcpy(&cand_list_5g[cand_list_5g_count],&p_rept->cand_list_5g[i],sizeof(ntw_info));
					printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(p_rept->cand_list_5g[i].bssid),p_rept->cand_list_5g[i].rssi);
					cand_list_5g_count++;
				}
			}
			for (i=0; i< cand_list_5g_count;i++)
			{
				if(cand_list_5g[i].Non_MAN == 0
					&&( rssi_above_th == 0 || cand_list_5g[i].rssi > CUSTOM_RSSI_TH))
				{
					
					printf(" 5G:Processed: MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(cand_list_5g[i].bssid),cand_list_5g[i].rssi);
					do_not_consider_2g = 1;
					p_rept->target_rssi_5g = cand_list_5g[i].rssi;
					p_rept->is_processed_tmp = 1;
					memcpy(p_rept->cli_target_mac_5g, cand_list_5g[i].bssid, MAC_ADDR_LEN);
					// 2g connection should happen on the same MAN repeater.
					flag = 1;
					break;
				}
			}
			if(p_rept->is_processed_tmp == 1)
			{
				if(p_rept == &g_own_rept_info)
					p_rept = regrp_get_first_rept();
				else
					p_rept = regrp_get_next_rept(p_rept);

				continue;
			}
			
			bzero(cand_list_2g,sizeof(cand_list_2g));
			cand_list_2g_count =0;
			for (i=0; i< p_rept->cand_list_2g_count;i++)
			{
				p_repeater_list_struct p_tmp_rept;
				if((p_tmp_rept = search_rept_by_node_number(p_rept->cand_list_2g[i])) != NULL
					&& p_tmp_rept->is_processed)
				{
					// should be in sorted order
					memcpy(&cand_list_2g[cand_list_2g_count],&p_rept->cand_list_2g[i],sizeof(ntw_info));
					printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(p_rept->cand_list_2g[i].bssid),p_rept->cand_list_2g[i].rssi);
					cand_list_2g_count++;
				}
			}
			for (i=0; i< cand_list_2g_count;i++)
			{
				if(cand_list_2g[i].Non_MAN == 0 
					&& ( rssi_above_th == 0 || cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD))
				{
					printf(" 2G:Processed: MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(cand_list_2g[i].bssid),cand_list_2g[i].rssi);
					p_rept->target_rssi_2g = cand_list_2g[i].rssi;					
					p_rept->target_rssi_5g = -120;
					p_rept->is_processed_tmp = 1;
					flag = 1;
					memcpy(p_rept->cli_target_mac_2g, cand_list_2g[i].bssid, MAC_ADDR_LEN);
				}
			}
			
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
		}
		//mark all repeaters which were processed in the previous iteration as processed.
		if (flag)
		{
			p_repeater_list_struct p_tmp_rept = NULL;//regrp_get_first_rept();
			p_repeater_list_struct p_best_target_rept = NULL;
			INT8 best_target_rept_rssi = -110;
			
			if(p_own_rept->is_master == 1 && regrp_is_ap_present())
				p_tmp_rept = p_own_rept;
			else
				p_tmp_rept = regrp_get_first_rept();

			while(p_tmp_rept!= NULL)
			{
			
				printf("Analyse Repeater MAC %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(p_tmp_rept->br_mac));
				if(p_tmp_rept->is_processed_tmp == 1)
				{
					if (do_not_consider_2g)
					{
						if (best_target_rept_rssi < p_tmp_rept->target_rssi_5g)
						{
							best_target_rept_rssi = p_tmp_rept->target_rssi_5g;
							p_best_target_rept = p_tmp_rept;
							printf("RSSI better than previous option, This is a better option for now\n");
						}
					} else {
						if (best_target_rept_rssi < p_tmp_rept->target_rssi_2g)
						{
							best_target_rept_rssi = p_tmp_rept->target_rssi_2g;
							p_best_target_rept = p_tmp_rept;
							printf("RSSI better than previous option, This is a better option for now\n");
						}

					}
				}
				if(p_tmp_rept == &g_own_rept_info)
					p_tmp_rept = regrp_get_first_rept();
				else
				p_tmp_rept = regrp_get_next_rept(p_tmp_rept);
			}
			if(p_best_target_rept == NULL)
			{
				printf("Best Target is NULL: Err\n");
				return 1;
			}

			printf("Best Target found for Repeater MAC %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(p_best_target_rept->br_mac));
			printf("Best target 5G MAC %02x:%02x:%02x:%02x:%02x:%02x, RSSI\n", PRINT_MAC(p_best_target_rept->cli_target_mac_5g),p_best_target_rept->target_rssi_5g);
			printf("Best target 2G MAC %02x:%02x:%02x:%02x:%02x:%02x, RSSI\n", PRINT_MAC(p_best_target_rept->cli_target_mac_2g),p_best_target_rept->target_rssi_2g);

			//! now mark just the best one as procesed, clear rest
			{
				p_repeater_list_struct p_tmp_rept = NULL;// regrp_get_first_rept();
				
				if(p_own_rept->is_master == 1 && regrp_is_ap_present())
					p_tmp_rept = p_own_rept;
				else
					p_tmp_rept = regrp_get_first_rept();
				while(p_tmp_rept!= NULL)
				{
					if(p_tmp_rept == p_best_target_rept)
					{
						p_tmp_rept->is_processed = 1;
						p_tmp_rept->is_processed_tmp = 0;
					} else {
						if(p_tmp_rept->is_processed == 0)
						{
							p_tmp_rept->is_processed_tmp = 0;
							bzero(p_tmp_rept->cli_target_mac_2g,MAC_ADDR_LEN);
							bzero(p_tmp_rept->cli_target_mac_5g,MAC_ADDR_LEN);
						}
					}
					if(p_tmp_rept == &g_own_rept_info)
						p_tmp_rept = regrp_get_first_rept();
					else
						p_tmp_rept = regrp_get_next_rept(p_tmp_rept);
				}

			}
#if 0			
			if(g_own_rept_info.is_processed_tmp == 1)
				g_own_rept_info.is_processed = 1;
#endif
		}
		if (flag == 1)
			ret =1;
	}
	return ret;
}

UINT8 select_man_processed(UINT8 rssi_above_th)
{
	p_repeater_list_struct p_rept, p_own_rept = &g_own_rept_info;
	UINT8 i,j=0;
	UINT8 flag = 1;
	UINT8 ret =0;

	printf("%s\n", __FUNCTION__);

	while(flag != 0)
	{
//	first search for third party AP with good RSSI
		flag = 0;
		if(p_own_rept->is_master == 1 && regrp_is_ap_present())
			p_rept = p_own_rept;
		else
			p_rept = regrp_get_first_rept();
		
		while (p_rept != NULL)
		{
			if(p_rept->is_processed == 1)
			{
				if(p_rept == &g_own_rept_info)
					p_rept = regrp_get_first_rept();
				else
					p_rept = regrp_get_next_rept(p_rept);
				continue;
			}
			
			hex_dump("ReptMAC :", p_rept->br_mac,6);
			bzero(cand_list_5g,sizeof(cand_list_5g));
			cand_list_5g_count =0;
			for (i=0; i< p_rept->cand_list_5g_count;i++)
			{
				p_repeater_list_struct p_tmp_rept;
				if((p_tmp_rept = search_rept_by_node_number(p_rept->cand_list_5g[i])) != NULL
					&& p_tmp_rept->is_processed)
				{
					// should be in sorted order
					memcpy(&cand_list_5g[cand_list_5g_count],&p_rept->cand_list_5g[i],sizeof(ntw_info));
					printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(p_rept->cand_list_5g[i].bssid),p_rept->cand_list_5g[i].rssi);
					cand_list_5g_count++;
				}
			}
			for (i=0; i< cand_list_5g_count;i++)
			{
				if(cand_list_5g[i].Non_MAN == 0
					&&( rssi_above_th == 0 || cand_list_5g[i].rssi > CUSTOM_RSSI_TH))
				{
					
					printf(" 5G:Processed: MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(cand_list_5g[i].bssid),cand_list_5g[i].rssi);
					p_rept->is_processed_tmp = 1;
					memcpy(p_rept->cli_target_mac_5g, cand_list_5g[i].bssid, MAC_ADDR_LEN);
					// 2g connection should happen on the same MAN repeater.
					flag = 1;
					break;
				}
			}
			if(p_rept->is_processed_tmp == 1)
			{
				if(p_rept == &g_own_rept_info)
					p_rept = regrp_get_first_rept();
				else
					p_rept = regrp_get_next_rept(p_rept);

				continue;
			}
			
			if(rssi_above_th == 0
				|| interface_5g == '\0')
			{
			bzero(cand_list_2g,sizeof(cand_list_2g));
			cand_list_2g_count =0;
			for (i=0; i< p_rept->cand_list_2g_count;i++)
			{
				p_repeater_list_struct p_tmp_rept;
				if((p_tmp_rept = search_rept_by_node_number(p_rept->cand_list_2g[i])) != NULL
					&& p_tmp_rept->is_processed)
				{
					// should be in sorted order
					memcpy(&cand_list_2g[cand_list_2g_count],&p_rept->cand_list_2g[i],sizeof(ntw_info));
					printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(p_rept->cand_list_2g[i].bssid),p_rept->cand_list_2g[i].rssi);
					cand_list_2g_count++;
				}
			}
			for (i=0; i< cand_list_2g_count;i++)
			{
				if(cand_list_2g[i].Non_MAN == 0 
					&& ( rssi_above_th == 0 || cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD))
				{
					printf(" 2G:Processed: MAC: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n", PRINT_MAC(cand_list_2g[i].bssid),cand_list_2g[i].rssi);
					p_rept->is_processed_tmp = 1;
					flag = 1;
					memcpy(p_rept->cli_target_mac_2g, cand_list_2g[i].bssid, MAC_ADDR_LEN);
					break;
				}
			}
			}
			if(p_rept == &g_own_rept_info)
				p_rept = regrp_get_first_rept();
			else
				p_rept = regrp_get_next_rept(p_rept);
		}
		//mark all repeaters which were processed in the previous iteration as processed.
		{
			p_repeater_list_struct p_tmp_rept = regrp_get_first_rept();
			while(p_tmp_rept!= NULL)
			{
				if(p_tmp_rept->is_processed_tmp == 1)
				{
					p_tmp_rept->is_processed_tmp = 0;
					p_tmp_rept->is_processed = 1;
				}
				p_tmp_rept = regrp_get_next_rept(p_tmp_rept);
			}
			if(g_own_rept_info.is_processed_tmp == 1)
			{
				g_own_rept_info.is_processed = 1;
				g_own_rept_info.is_processed_tmp = 0;
			}
		}
		if (flag == 1)
			ret =1;
	}
	return ret;
}


#if 0
void select_man_best_rssi(UINT8 rssi_above_th)
{
	p_repeater_list_struct p_rept;
	UINT8 i,j;
	INT8 max_rssi = -110, cand_idx =0;
	printf("%s\n", __FUNCTION__);
	
//	first search for third party AP with good RSSI
	p_rept = regrp_get_first_rept();
	while (p_rept != NULL)
	{
		cand_idx = 0;
		max_rssi = -110;

		if(p_rept->is_processed == 1)
		{
			printf("1\n");
			p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
			if(p_rept->cand_list_5g[i].Non_MAN == 0 
				&& (rssi_above_th == 0 || p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD)
				&& p_rept->cand_list_5g[i].rssi > max_rssi)
			{
				max_rssi = p_rept->cand_list_5g[i].rssi;
				cand_idx = i;
				
				printf("Cand 5g: %d\n", max_rssi);
				
			}
		}
		if(cand_idx != 0)
		{
			
			printf("2\n");
			p_rept->is_processed = 1;
			memcpy(p_rept->cli_target_mac_5g, p_rept->cand_list_5g[cand_idx].bssid, MAC_ADDR_LEN);
			p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		else
			{
				printf("No Cand found\n");
			}
		cand_idx = 0;
		max_rssi = -110;
		for (i=0; i< p_rept->cand_list_2g_count;i++)
		{
			if(p_rept->cand_list_2g[i].Non_MAN == 0 
				&& (rssi_above_th == 0 || p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD)
				&& p_rept->cand_list_2g[i].rssi > max_rssi)
			{
				max_rssi = p_rept->cand_list_2g[i].rssi;
				cand_idx = i;
				printf("Cand 2g: %d\n",max_rssi);
			}
		}
		
		if(cand_idx != 0)
		{
			printf("3");
			p_rept->is_processed = 1;
			memcpy(p_rept->cli_target_mac_2g, p_rept->cand_list_2g[cand_idx].bssid, MAC_ADDR_LEN);
			p_rept = regrp_get_next_rept(p_rept);
			continue;
		}
		else
			{
				printf("No Cand found\n");
			}
		p_rept = regrp_get_next_rept(p_rept);
	}

}
#endif

UINT8 regrep_send_msg_3_to_peers(UINT8 regrp_sent)
{
	
	p_repeater_list_struct p_rept;
	p_rept = regrp_get_first_rept();
//	UINT8 regrp_sent =0;
	printf("%s\n", __FUNCTION__);

	while(p_rept != NULL)
	{
		hex_dump("Rept:",p_rept->br_mac,6);
		hex_dump("Target5g:",p_rept->cli_target_mac_5g,6);
		hex_dump("Target2g:",p_rept->cli_target_mac_2g,6);
		
	//	hex_dump("5gBssid:",p_rept->cli_5g_bssid,6);
	//	hex_dump("2gBssid:",p_rept->cli_2g_bssid,6);
	//	printf("isConnectedNonMAN:%d\n",is_connected_to_non_man(p_rept));


		if(memcmp(p_rept->cli_target_mac_5g,ZERO_MAC_ADDR,MAC_ADDR_LEN) ==0
			&& memcmp(p_rept->cli_target_mac_2g,ZERO_MAC_ADDR,MAC_ADDR_LEN) ==0)
		{
			printf("No peers found for Rept: %x \n",p_rept->ip_addr);
			p_rept->regrp_state = REGRP_STATE_PEER_DONE;
			regrp_mlme_compose_send_msg_3(p_rept,0);

			p_rept = regrp_get_next_rept(p_rept);
			continue;
			
		}
#if 0
		if((!MAC_ADDR_EQUAL(p_rept->cli_target_mac_5g,ZERO_MAC_ADDR) 
			&& MAC_ADDR_EQUAL(p_rept->cli_target_mac_5g,p_rept->cli_5g_bssid))
			&& (!MAC_ADDR_EQUAL(p_rept->cli_target_mac_2g,ZERO_MAC_ADDR) 
			&& MAC_ADDR_EQUAL(p_rept->cli_target_mac_2g,p_rept->cli_2g_bssid)))

		{

		}
#endif
		if(p_rept->non_man_selected == 0 && 
			((memcmp(p_rept->cli_target_mac_5g,ZERO_MAC_ADDR,MAC_ADDR_LEN)&& (memcmp(p_rept->cli_target_mac_5g,p_rept->cli_5g_bssid,MAC_ADDR_LEN) ==0))
			|| (memcmp(p_rept->cli_target_mac_2g,ZERO_MAC_ADDR,MAC_ADDR_LEN) && memcmp(p_rept->cli_target_mac_2g,p_rept->cli_2g_bssid,MAC_ADDR_LEN) ==0)))
		{
			//hex_dump("5target",p_rept->cli_target_mac_5g,6);
			//hex_dump("2target",p_rept->cli_target_mac_2g,6);
			//hex_dump("5bssid",p_rept->cli_5g_bssid,6);
			//hex_dump("5bssid",p_rept->cli_2g_bssid,6);
			printf("Rept: %x already connected to best MAN AP\n",p_rept->ip_addr);
			p_rept->regrp_state = REGRP_STATE_PEER_DONE;
			regrp_mlme_compose_send_msg_3(p_rept,0);

			p_rept = regrp_get_next_rept(p_rept);
			continue;
			
		}
		
		if(p_rept->non_man_selected == 1 && 
			((memcmp(p_rept->cli_target_mac_5g,ZERO_MAC_ADDR,MAC_ADDR_LEN)&& (memcmp(p_rept->cli_target_mac_5g,p_rept->cli_5g_bssid,MAC_ADDR_LEN) ==0))
			|| (memcmp(p_rept->cli_target_mac_2g,ZERO_MAC_ADDR,MAC_ADDR_LEN) && memcmp(p_rept->cli_target_mac_2g,p_rept->cli_2g_bssid,MAC_ADDR_LEN) ==0)))
		{
			printf("Rept: %x already connected to best Non MAN AP\n",p_rept->ip_addr);
			p_rept->regrp_state = REGRP_STATE_PEER_DONE;
			regrp_mlme_compose_send_msg_3(p_rept,0);

			p_rept = regrp_get_next_rept(p_rept);
			continue;
			
		}
		regrp_sent ++;
		regrp_mlme_compose_send_msg_3(p_rept,regrp_sent);
		
		p_rept = regrp_get_next_rept(p_rept);
	}
	if(regrp_sent == 0)
	{
		printf("No Regrp\n");
	}
	return regrp_sent;
}
#if 0
UINT8 is_root_node(p_repeater_list_struct p_rept)
{
	if(p_rept->node_number.path_len == 6)
		return TRUE;
	return FALSE;
}
#endif

UINT8 regrp_send_msg_4_to_peers()
{
	p_repeater_list_struct p_rept;
	p_rept = regrp_get_first_rept();
//	UINT8 regrp_sent =0;
	printf("%s\n", __FUNCTION__);

	while(p_rept != NULL)
	{
		regrp_mlme_compose_send_msg_4(p_rept);
		
		p_rept = regrp_get_next_rept(p_rept);
	}
	return TRUE;
}

void regrp_complete(p_repeater_list_struct p_own_rept)
{
	eloop_cancel_timeout(regroup_timeout,NULL,NULL);
	p_own_rept->regrp_state = REGRP_STATE_DONE;
	
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);

	
}


UINT8 regrp_is_ap_present()
{
	p_repeater_list_struct p_rept = NULL, p_own_rept = &g_own_rept_info;
	UINT8 i,j;
	
//	first search for third party AP with good RSSI

//	printf("%s\n", __FUNCTION__);
	p_rept = p_own_rept;

	while (p_rept != NULL)
	{
		
		for (i=0; i< p_rept->cand_list_5g_count;i++)
		{
			if((p_rept->cand_list_5g[i].Non_MAN == 1)
				|| (p_rept->cand_list_5g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_5g[i].node_number.path_len == 6))
			{
				printf("AP present\n");
				return TRUE;
			}
		}
		for (i=0; i< p_rept->cand_list_2g_count;i++)
		{
			if((p_rept->cand_list_2g[i].Non_MAN == 1)
				|| (p_rept->cand_list_2g[i].network_wt[0] == 0xf 
				&& p_rept->cand_list_2g[i].node_number.path_len == 6))
			{
				printf("AP present\n");
				return TRUE;
			}
		}

		if(p_rept == &g_own_rept_info)
			p_rept = regrp_get_first_rept();
		else
			p_rept = regrp_get_next_rept(p_rept);
	}
	return FALSE;
}





void regrp_select_conn_candidates()
{

	UINT8 ret=1;
	UINT8 ap_present= FALSE;
	
	ap_present = regrp_is_ap_present();

	select_root_connection(TRUE, ap_present);
	//select_root_connection(TRUE, FALSE);
	//direct internet connection.
//	select_man_internet_connection(TRUE);
	select_man_processed(TRUE);
	while (ret)
	{
		//select_man_best_rssi(TRUE);
		ret = select_root_connection_single(FALSE,ap_present);
		//direct internet connection.
//		select_man_internet_connection(FALSE);
		if(ret == 1)
		 ret = select_man_processed(TRUE);
		//select_man_best_rssi(FALSE);
	}

	ret = 1;
	while (ret)
	{
		//select_man_best_rssi(TRUE);
		ret = select_man_processed_single(FALSE);
		//direct internet connection.
		//select_man_internet_connection(FALSE);
		if(ret == 1)
		 ret = select_man_processed(TRUE);
		//select_man_best_rssi(FALSE);
	}
	select_man_processed(FALSE);

}
void regrp_trigger_reconnection()
{
	UINT8 regrp_sent = 0, flag = 0;
	p_repeater_list_struct p_own_rept = &g_own_rept_info;

	regrp_select_conn_candidates();

	
	if(regrp_is_ap_present())
	{
		if((!MAC_ADDR_EQUAL(p_own_rept->cli_target_mac_5g, ZERO_MAC_ADDR)
			&& !MAC_ADDR_EQUAL(p_own_rept->cli_target_mac_5g, p_own_rept->cli_5g_bssid))
			||
				(!MAC_ADDR_EQUAL(p_own_rept->cli_target_mac_2g, ZERO_MAC_ADDR)
				&& !MAC_ADDR_EQUAL(p_own_rept->cli_target_mac_2g, p_own_rept->cli_2g_bssid)))
			{
				printf("RootReconnection\n");
			hex_dump("Target5g",p_own_rept->cli_target_mac_5g,6);
			hex_dump("Target2g",p_own_rept->cli_target_mac_2g,6);
			hex_dump("5gbssid",p_own_rept->cli_5g_bssid,6);
			hex_dump("2gbssid",p_own_rept->cli_2g_bssid,6);
		
			memcpy(regrp_current_wt, p_own_rept->network_weight, NETWORK_WEIGHT_LEN);
			regrp_fill_target_wt(regrp_expected_wt_1,regrp_expected_wt_2);
			if(regrp_is_target_wt_same(regrp_expected_wt_1,regrp_expected_wt_2))
				regrp_disconnect_sta = FALSE;
			else
				regrp_disconnect_sta = TRUE;
			regrp_sent = 1;
			flag = 1;
		}
		else
			printf("No Regrp for root rept\n");
	}


	regrp_sent = regrep_send_msg_3_to_peers(regrp_sent);

	if(flag == 1)
	{
		os_sleep(5,0);
		regrp_disable_apcli(p_own_rept);
		if(cli_interface_5g[0]!= '\0')
			set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
		if(cli_interface_2g[0]!= '\0')
			set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
		os_sleep(5,0);
		regrp_connect(p_own_rept);
		
		p_own_rept->need_retrigger_regrp = 1;
	}

	if(!regrp_is_ap_present() && regrp_sent>0 )
	{
		os_sleep(5,0);
		regrp_disable_apcli(p_own_rept);
	}
#if 0
	if(regrp_sent == 1)
	{
		printf("regroup triggered and topology changed.\n"
			"Trigger again after 1 minute\n");
		eloop_register_timeout(60, 0,retrigger_regroup, NULL, NULL);
	}
#endif
#if 0
	// if I am connected to 3rd party, trigger self scan
	if(FALSE == is_root_node(p_own_rept))
	{
		p_own_rept->scan_done_2g =0;
		p_own_rept->scan_done_5g =0;

		p_own_rept->cand_list_2g_count =0;
		memset(p_own_rept->cand_list_2g,0, sizeof(p_own_rept->cand_list_2g));
		p_own_rept->cand_list_5g_count =0;
		memset(p_own_rept->cand_list_5g,0, sizeof(p_own_rept->cand_list_2g));
		printf("start scan for own rept\n");
		regrp_start_scan(&g_own_rept_info);
		p_own_rept->regrp_state = REGRP_STATE_SCAN;

		if(regrp_sent != 0)
			p_own_rept->need_retrigger_regrp = 1;
	}
	else
#endif
	{
		if(regrp_pending == 1 || regrp_sent != 0 || p_own_rept->need_retrigger_regrp == 1)
		{
			unsigned int restart_time_sec = RESTART_REGROUP_TIME;
			eloop_cancel_timeout(regroup_timeout,NULL,NULL);
			printf("%s:Restart Regroup %d\n",__func__, regrp_sent);
			// wait for all connections to becomes stable before restart regroup
			if(regrp_sent !=0)
				restart_time_sec = regrp_sent*30 + 30;  
			eloop_register_timeout(restart_time_sec, 0, restart_regroup,NULL, NULL);
			g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
			if(regrp_is_ap_present())
			{
				printf("Enable APCLI interface: %d\n", p_own_rept->network_weight[0]);
				regrp_enable_apcli(&g_own_rept_info);
			}
		}
		else
		{
			if(p_own_rept->non_ez_connection == 0)
					regrp_enable_apcli(&g_own_rept_info);
				regrp_clear_dhcp_defer();
				regrp_send_msg_4_to_peers();
				regrp_complete(p_own_rept);
				printf("Trigger Regroup : After %d sec\n",regrp_periodic_time );
				eloop_register_timeout(regrp_periodic_time, 0, restart_regroup,NULL, NULL);
				g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
		}
	}
}

regrp_clear_rept_state(p_repeater_list_struct p_rept)
{
	printf("%s\n",__func__);
	hex_dump("ReptMAC", p_rept->br_mac,6);
	//	p_rept->regrp_state = REGRP_STATE_IDLE;
		if(p_rept == &g_own_rept_info)
			regrp_complete(p_rept);
	/*Regrp state info*/
		if(p_rept->is_master == 0)
			p_rept->regrp_state = REGRP_SLAVE_STATE_IDLE;
		else if (p_rept->regrp_state != REGRP_STATE_IDLE)
			p_rept->regrp_state = REGRP_STATE_DONE;
	//	p_rept->is_master = 0;
		//p_rept->regrp_state = REGRP_STATE_DONE;
		p_rept->scan_done_2g =0;
		p_rept->scan_done_5g = 0;
		p_rept->cand_list_2g_count = 0;
		bzero(p_rept->cand_list_2g, sizeof(p_rept->cand_list_2g));
		p_rept->cand_list_5g_count = 0;
		bzero(p_rept->cand_list_5g, sizeof(p_rept->cand_list_5g));
		p_rept->is_processed = 0;
		p_rept->is_processed_tmp = 0;
		bzero(p_rept->cli_target_mac_2g,MAC_ADDR_LEN);
		bzero(p_rept->cli_target_mac_5g,MAC_ADDR_LEN);
		p_rept->need_retrigger_regrp = 0;
		p_rept->non_man_selected = 0;

		p_rept->udp_count =0;
		p_rept->tcp_link_done =0;
		//if(p_rept->tcp_rx_sock >= 0)
		//	close(p_rept->tcp_rx_sock);
		//if(p_rept->tcp_tx_sock >= 0)
		//	close(p_rept->tcp_tx_sock);

		p_rept->tcp_rx_sock=-1;
		p_rept->tcp_tx_sock =-1;
		p_rept->last_frame_time.sec = p_rept->last_frame_time.usec = 0;
		p_rept->ip_addr = 0;
		if(p_rept == &g_own_rept_info)
		{
	//		regrp_close_all_tcp_listners();
			if(eloop_is_timeout_registered(regroup_timeout,NULL,NULL))
				eloop_cancel_timeout(regroup_timeout,NULL,NULL);
			if(eloop_is_timeout_registered(restart_regroup,NULL, NULL))
				eloop_cancel_timeout(restart_regroup,NULL, NULL);

			if(eloop_is_timeout_registered(regrp_scan_timeout,NULL,p_rept))
				eloop_cancel_timeout(regrp_scan_timeout,NULL,p_rept);


			if(eloop_is_timeout_registered(regrp_msg_2_timeout, NULL, p_rept))
				eloop_cancel_timeout(regrp_msg_2_timeout, NULL, p_rept);

			if(eloop_is_timeout_registered(regrp_msg_3_timeout, NULL, NULL))
				eloop_cancel_timeout(regrp_msg_3_timeout, NULL, NULL);

			if(eloop_is_timeout_registered(slave_regrp_timeout, NULL, NULL))
				eloop_cancel_timeout(slave_regrp_timeout, NULL, NULL);

			//!TODO use NVRAM check here
			regrp_enable_apcli(&g_own_rept_info);

		}
}

regrp_clean_all_states()
{
	p_repeater_list_struct p_rept;
	regrp_clear_rept_state(&g_own_rept_info);

	p_rept = regrp_get_first_rept();
	while(p_rept != NULL)
	{
		regrp_clear_rept_state(p_rept);
		p_rept = regrp_get_next_rept(p_rept);
	}
}

void regrp_mlme_handle_msg_2(p_repeater_list_struct p_rept, p_msg_2_struct p_msg_2, UINT8 msg_len)
{

	p_repeater_list_struct p_next_rept = NULL;

	printf("%s\n", __FUNCTION__);
	hex_dump("MAC",p_rept->br_mac,6);

	if(p_msg_2->dialog_token != p_rept->dialog_token)
	{
		printf("%s: dialog token mismatch %d:%d\n",__func__, p_msg_2->dialog_token,p_rept->dialog_token);
		return;
	}
	if(p_rept->regrp_state != REGRP_STATE_PEER_WAIT_MSG_2)
	{
		printf("%s: Wrong state : %d\n", __func__, p_rept->regrp_state);
		return;
	}
	//msg_2 contains the scan list.
	//store the scan list for this repeater and change the state to REGRP_STATE_SCAN_DONE
	eloop_cancel_timeout(regrp_msg_2_timeout,NULL,p_rept);

	p_rept->cand_list_2g_count = p_msg_2->cand_list_2g_count;
	memcpy(p_rept->cand_list_2g,p_msg_2->cand_list_2g,p_msg_2->cand_list_2g_count*sizeof(ntw_info));

	p_rept->cand_list_5g_count = p_msg_2->cand_list_5g_count;
	memcpy(p_rept->cand_list_5g,p_msg_2->cand_list_5g,p_msg_2->cand_list_5g_count*sizeof(ntw_info));

	memcpy(p_rept->cli_2g_bssid, p_msg_2->cli_2g_bssid, MAC_ADDR_LEN);
	memcpy(p_rept->cli_5g_bssid, p_msg_2->cli_5g_bssid, MAC_ADDR_LEN);

	sort_cand_list_by_rssi(p_rept->cand_list_5g,p_rept->cand_list_5g_count);
	sort_cand_list_by_rssi(p_rept->cand_list_2g,p_rept->cand_list_2g_count);

	p_rept->regrp_state =	REGRP_STATE_PEER_SCAN_DONE;
	//fetch the next rept and 

	if(p_msg_2->cand_list_5g_count)
	{
		int i;
		for(i=0; i< p_msg_2->cand_list_5g_count; i++)
		{
			printf("5g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(p_rept->cand_list_5g[i].bssid),p_rept->cand_list_5g[i].rssi,p_rept->cand_list_5g[i].Non_MAN);
		}
	}
	
	if(p_msg_2->cand_list_2g_count)
	{
		int i;
		for(i=0; i< p_msg_2->cand_list_2g_count; i++)
		{
			printf("2g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(p_rept->cand_list_2g[i].bssid),p_rept->cand_list_2g[i].rssi,p_rept->cand_list_2g[i].Non_MAN);
		}
	}

	p_next_rept = regrp_get_next_rept(p_rept);

	if(p_next_rept == NULL)
	{
		// scan on all repeaters complete.
		os_sleep(0,200000);
		regrp_trigger_reconnection();
	}
	else
	{
		regrp_mlme_compose_send_msg_1(&g_own_rept_info, p_next_rept);
	}
}

regrp_disable_apcli(p_repeater_list_struct p_own_rept)
{
	UINT8 command[128] = {0};
	if(cli_interface_5g[0]!= '\0')
	{
		sprintf(command, "iwpriv %s set ApCliEnable=0\n",
												cli_interface_5g);
		
		printf("<%s>\n", command);
		system(command);
	}
	if(cli_interface_2g[0]!= '\0')
	{
		sprintf(command, "iwpriv %s set ApCliEnable=0\n",
												cli_interface_2g);
		
		printf("<%s>\n", command);
		
		system(command);
	}
}



regrp_enable_apcli(p_repeater_list_struct p_own_rept)
{
	UINT8 command[128] = {0};

	if (p_own_rept->network_weight[0] == 0xf)
	{
		return;
	}
	if(cli_interface_5g[0]!= '\0')
	{
		sprintf(command, "iwpriv %s set ApCliEnable=1\n",
												cli_interface_5g);
		
		printf("<%s>\n", command);
		system(command);
	}
	if(cli_interface_2g[0]!= '\0')
	{
		sprintf(command, "iwpriv %s set ApCliEnable=1\n",
												cli_interface_2g);
		
		printf("<%s>\n", command);
		
		system(command);
	}
}


char regrp_is_target_wt_same(UINT8 *target_wt_1,UINT8 *target_wt_2)
{
//	hex_dump("target_1", target_wt_1,6);
//	hex_dump("target_2", target_wt_2,6);
//	hex_dump("ownWt",g_own_rept_info.network_weight, NETWORK_WEIGHT_LEN);

	if(MAC_ADDR_EQUAL(target_wt_1,&g_own_rept_info.network_weight[1])
		|| MAC_ADDR_EQUAL(target_wt_2,&g_own_rept_info.network_weight[1]))
	{
//		printf("Raghav: target wt same\n");
		return TRUE;
	}
	return FALSE;
}
void regrp_mlme_handle_msg_3(p_repeater_list_struct p_rept,p_msg_3_struct p_msg_3, UINT8 msg_len)
{
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	if(p_msg_3->dialog_token != p_own_rept->dialog_token)
	{
		printf("%s: dialog token mismatch %d:%d\n",__func__, p_msg_3->dialog_token,p_own_rept->dialog_token);
		return;
	}
	if(p_own_rept->regrp_state != REGRP_SLAVE_STATE_WAIT_MSG_3)
	{
		printf("%s: Wrong state : %d\n", __func__, p_own_rept->regrp_state);
		return;
	}
	printf("%s\n", __FUNCTION__);
	hex_dump("MAC",p_rept->br_mac,6);

	hex_dump("Target 5gMAC",p_msg_3->target_5g_mac,6);
	hex_dump("Target 2gMAC",p_msg_3->target_2g_mac,6);
	hex_dump("5gBSSID",p_own_rept->cli_5g_bssid,6);
	hex_dump("2gBSSID",p_own_rept->cli_2g_bssid,6);
	hex_dump("targetWt_1",p_msg_3->target_wt_1,6);
	hex_dump("targetWt_2",p_msg_3->target_wt_2,6);

	if(MAC_ADDR_EQUAL(p_msg_3->target_wt_1,ZERO_MAC_ADDR) 
		&& MAC_ADDR_EQUAL(p_msg_3->target_wt_2,ZERO_MAC_ADDR))
	{
		printf("Err: both target wt 0\n");
	}
	else
	{
		memcpy(regrp_current_wt, p_own_rept->network_weight, NETWORK_WEIGHT_LEN);
		memcpy(regrp_expected_wt_1,p_msg_3->target_wt_1, MAC_ADDR_LEN);
		memcpy(regrp_expected_wt_2,p_msg_3->target_wt_2, MAC_ADDR_LEN);

		if(regrp_is_target_wt_same(p_msg_3->target_wt_1,p_msg_3->target_wt_2))
		{
			regrp_disconnect_sta = FALSE;
		}
		else
		{
			regrp_disconnect_sta = TRUE;
		}
		regrp_dhcp_defer = 1;
	}
	if(p_msg_3->status != 0)
	{
		os_sleep(5,0);
		regrp_disable_apcli(p_own_rept);
#if 1
		int i = (p_msg_3->status -1)*1;
		printf("Sleep for %d sec\n", i);
		while (i)
		{
			os_sleep(1,0);
			i-=1;
		//	regrp_udp_client();
			printf("SleepRem: %d\n",i);
		}
#endif
	}
	
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);

	if(p_msg_3->status != 0)
	{
		memcpy(p_own_rept->cli_target_mac_5g,p_msg_3->target_5g_mac, MAC_ADDR_LEN);
		memcpy(p_own_rept->cli_target_mac_2g,p_msg_3->target_2g_mac, MAC_ADDR_LEN);
		regrp_connect(p_own_rept);
	}
	
	regrp_enable_cli_if(cli_interface_5g);
	regrp_enable_cli_if(cli_interface_5g);

	p_own_rept->regrp_state = REGRP_SLAVE_STATE_IDLE;

	eloop_cancel_timeout(regrp_msg_3_timeout,NULL,NULL);
	eloop_cancel_timeout(slave_regrp_timeout,NULL,NULL);
}



int regrp_get_cand_list(UCHAR *ifname, UCHAR *cand_list, UCHAR *cand_count)
{
	struct _regrp_command *cmd = NULL;
	struct _regrp_cmd_cand_list *pcand_data = NULL;
	char *buf = NULL;
	int buf_len;
	int sd;

	buf_len = (sizeof(regrp_cmd_cand_list) + (MAX_VIRTUAL_REPT_NUM* sizeof(ntw_info)));
	buf = os_zalloc(buf_len);
		
	if (buf == NULL) {
		printf("Couldnt get candidate list 1\n");
		return -1;
	}	

	cmd = (struct _regrp_command *)buf;
	cmd->command_id = OID_REGROUP_CMD_CAND_LIST;
	cmd->command_len = 0;

#if 0
	if(driver_wext_get_oid(drv_data,(char *)ifname,OID_WH_EZ_REGROUP_COMMAND, (char *)cmd, (size_t *)&buf_len) < 0){
        DBGPRINT(DEBUG_ERROR, "%s: oid=0x%x failed: %d\n",
               __FUNCTION__, OID_REGROUP_CMD_CAND_LIST, errno);
        os_free(buf);
        return -1;
	}
#endif
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	if (ap_oid_query_info(OID_WH_EZ_REGROUP_COMMAND, sd, ifname, cmd, &buf_len) < 0)
	{
		printf("Couldnt get candidate list 2: %s\n", ifname);
		
        os_free(buf);
        return -1;
	}
	close(sd);

	if((buf_len < sizeof(struct _regrp_cmd_cand_list)))
	{
	
		printf("Couldnt get candidate list 3, read buffer length = %d, _regrp_cmd_cand_list = %d\n", buf_len,sizeof(struct _regrp_cmd_cand_list));
		return -1;
	}
	buf_len -= sizeof(struct _regrp_command);

	pcand_data = (pregrp_cmd_cand_list)cmd;

	*cand_count = 0;

	if(cand_list && pcand_data->cand_count){
		memcpy(cand_list,pcand_data->list,(pcand_data->cand_count * sizeof(ntw_info)));
		*cand_count = pcand_data->cand_count;
	}

	os_free(buf);

	return 0;
}

UINT8 regrp_find_rept_candidate(p_repeater_list_struct p_rept, enum CAND_ACTION ACTION)
{
	
	UINT8 i,j;
	UINT8 ret = FALSE;
	switch (ACTION)
	{
		case NON_MAN_ABOVE_TH:
		case NON_MAN_BELOW_TH:
		case INTERNET_ABOVE_TH:
		case INTERNET_BELOW_TH:
		{
			for (i=0; i< p_rept->cand_list_5g_count;i++)
			{
				if((p_rept->cand_list_5g[i].Non_MAN == 1 && 
					(ACTION == NON_MAN_BELOW_TH ||
					(ACTION == NON_MAN_ABOVE_TH 
						&& p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD)))
					||
					(p_rept->cand_list_5g[i].internet_status== 1 && 
					(ACTION == INTERNET_BELOW_TH ||
					(ACTION == INTERNET_ABOVE_TH 
						&& p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD)))
				)
				{
					p_rept->is_processed = 1;
					p_rept->is_processed_tmp = 0;
					memcpy(p_rept->cli_target_mac_5g, p_rept->cand_list_5g[i].bssid, MAC_ADDR_LEN);
					ret = TRUE;
					break;
				}
			}
			// if easy connection made, then attempt 2g on the same rept.
			if(p_rept->is_processed == 1 
				&& p_rept->cand_list_5g[i].Non_MAN == 0)
			{
				ret = TRUE;
				break;
			}

			for (i=0; i< p_rept->cand_list_2g_count;i++)
			{
				if((p_rept->cand_list_2g[i].Non_MAN == 1 && 
					(ACTION == NON_MAN_BELOW_TH ||
					(ACTION == NON_MAN_ABOVE_TH 
						&& p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD)))
					||
					(p_rept->cand_list_2g[i].internet_status== 1 && 
					(ACTION == INTERNET_BELOW_TH ||
					(ACTION == INTERNET_ABOVE_TH 
						&& p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD)))
				)
				{
					p_rept->is_processed = 1;
					p_rept->is_processed_tmp = 0;
					memcpy(p_rept->cli_target_mac_2g, p_rept->cand_list_2g[i].bssid, MAC_ADDR_LEN);
					ret = TRUE;
				}
			}
		}
		break;
		case BEST_RSSI_ABOVE_TH:
		case BEST_RSSI_BELOW_TH:
		{
			UINT8 cand_idx = 0;
			INT8 max_rssi = -110;

			for (i=0; i< p_rept->cand_list_5g_count;i++)
			{
				if(p_rept->cand_list_5g[i].Non_MAN == 0 
					&& (ACTION == BEST_RSSI_BELOW_TH ||
						(ACTION == BEST_RSSI_ABOVE_TH 
							&& p_rept->cand_list_5g[i].rssi > DEFAULT_RSSI_THRESHOLD))
					&& p_rept->cand_list_5g[i].rssi > max_rssi)
				{
					max_rssi = p_rept->cand_list_5g[i].rssi;
					cand_idx = i;
					
				}
			}
			if(cand_idx != 0)
			{
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
				memcpy(p_rept->cli_target_mac_5g, p_rept->cand_list_5g[cand_idx].bssid, MAC_ADDR_LEN);
				ret = TRUE;
				break;
			}
			cand_idx = 0;
			max_rssi = -110;
			for (i=0; i< p_rept->cand_list_2g_count;i++)
			{
				if(p_rept->cand_list_2g[i].Non_MAN == 0 
					&& (ACTION == BEST_RSSI_BELOW_TH ||
						(ACTION == BEST_RSSI_ABOVE_TH 
							&& p_rept->cand_list_2g[i].rssi > DEFAULT_RSSI_THRESHOLD))
					&& p_rept->cand_list_2g[i].rssi > max_rssi)
				{
					max_rssi = p_rept->cand_list_2g[i].rssi;
					cand_idx = i;
				}
			}
			
			if(cand_idx != 0)
			{
				p_rept->is_processed = 1;
				p_rept->is_processed_tmp = 0;
				memcpy(p_rept->cli_target_mac_2g, p_rept->cand_list_2g[cand_idx].bssid, MAC_ADDR_LEN);
				ret = TRUE;
				break;
			}
			break;
		}
	}	

	if( (ACTION == NON_MAN_ABOVE_TH || ACTION == NON_MAN_BELOW_TH) 
		&& (memcmp(p_rept->cli_target_mac_5g,p_rept->cli_5g_bssid,MAC_ADDR_LEN) ==0
		&& memcmp(p_rept->cli_target_mac_2g,p_rept->cli_2g_bssid,MAC_ADDR_LEN) ==0))
		ret = 0;

	
	if( (ACTION != NON_MAN_ABOVE_TH && ACTION != NON_MAN_BELOW_TH) 
		&& (memcmp(p_rept->cli_target_mac_5g,p_rept->cli_5g_bssid,MAC_ADDR_LEN) ==0
		|| memcmp(p_rept->cli_target_mac_2g,p_rept->cli_2g_bssid,MAC_ADDR_LEN) ==0))
		ret = 0;

	return ret;
}

regrp_enable_cli_if(UINT8 *ifname)
{
	UINT8 command[128] = {0};

	if(ifname[0]!= '\0')
	{
		sprintf(command, "iwpriv %s set ApCliEnable=1\n",
												ifname);
		
		printf("<%s>\n", command);
		system(command);
	}
}
regrp_force_bssid(UINT8 *ifname, UINT8 *bssid)
{
	UINT8 command[128] = {0};

	sprintf(command, "iwpriv %s set ez_apcli_force_bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
											ifname,PRINT_MAC(bssid));
	printf("<%s>\n", command);
	system(command);
}

void regrp_connect(p_repeater_list_struct p_own_rept)
{
	//UINT8 command[128] = {0};
	UINT8 i=0;
	
	get_connected_ap_mac(p_own_rept->cli_5g_bssid,cli_interface_5g);
	get_connected_ap_mac(p_own_rept->cli_2g_bssid,cli_interface_2g);

	if(memcmp(p_own_rept->cli_target_mac_5g, ZERO_MAC_ADDR, MAC_ADDR_LEN)
		&& !MAC_ADDR_EQUAL(p_own_rept->cli_5g_bssid,p_own_rept->cli_target_mac_5g))
	{
		//enable APCLI
		regrp_enable_cli_if(cli_interface_5g);
		// set force bssid
		regrp_force_bssid(cli_interface_5g,p_own_rept->cli_target_mac_5g);
		//wait for 5g to connect
		i = 60;
		while(i)
		{
			get_connected_ap_mac(p_own_rept->cli_5g_bssid,cli_interface_5g);
			if (!MAC_ADDR_EQUAL(p_own_rept->cli_5g_bssid,ZERO_MAC_ADDR))
			{
				printf("5g connected\n");
				hex_dump("MAC",p_own_rept->cli_5g_bssid,6);

				//disable force bssid
				regrp_force_bssid(cli_interface_5g,ZERO_MAC_ADDR);

				regrp_enable_cli_if(cli_interface_2g);
				break;
			}
			os_sleep(1,0);
			i--;
		}
		if(i == 0)
		{
			
			regrp_force_bssid(cli_interface_5g,ZERO_MAC_ADDR);

			if(!(memcmp(p_own_rept->cli_target_mac_2g, ZERO_MAC_ADDR, MAC_ADDR_LEN)
				&& !MAC_ADDR_EQUAL(p_own_rept->cli_2g_bssid,p_own_rept->cli_target_mac_2g)))
			{
				regrp_enable_cli_if(cli_interface_2g);
			}
		}
	}
	else if(memcmp(p_own_rept->cli_target_mac_2g, ZERO_MAC_ADDR, MAC_ADDR_LEN)
		&& !MAC_ADDR_EQUAL(p_own_rept->cli_2g_bssid,p_own_rept->cli_target_mac_2g))
	{
		//enable APCLI
		regrp_enable_cli_if(cli_interface_2g);
		// set force bssid
		
		regrp_force_bssid(cli_interface_2g,p_own_rept->cli_target_mac_2g);
		i = 60;
		while(i)
		{
			get_connected_ap_mac(p_own_rept->cli_2g_bssid,cli_interface_2g);
			if (!MAC_ADDR_EQUAL(p_own_rept->cli_2g_bssid,ZERO_MAC_ADDR))
			{
				printf("2g connected\n");
				hex_dump("MAC",p_own_rept->cli_2g_bssid,6);
				
				regrp_force_bssid(cli_interface_2g,ZERO_MAC_ADDR);

				
				regrp_enable_cli_if(cli_interface_5g);
				break;
			}
			os_sleep(1,0);
			i--;
		}
		if(i ==0 )
		{
			regrp_force_bssid(cli_interface_2g,ZERO_MAC_ADDR);
			regrp_enable_cli_if(cli_interface_5g);
		}
			
	}
}

#if 0
void trigger_rept_reconnect(p_repeater_list_struct p_own_rept)
{
	UINT8 found;
	if (regrp_find_rept_candidate(p_own_rept,NON_MAN_ABOVE_TH) == TRUE)
		found =1;
	else if (regrp_find_rept_candidate(p_own_rept,INTERNET_ABOVE_TH) == TRUE)
		found = 1;
	else if (regrp_find_rept_candidate(p_own_rept,BEST_RSSI_ABOVE_TH) == TRUE)
		found = 1;
	else if (regrp_find_rept_candidate(p_own_rept,NON_MAN_BELOW_TH) == TRUE)
		found = 1;
	else if (regrp_find_rept_candidate(p_own_rept,INTERNET_BELOW_TH) == TRUE)
		found = 1;
	else if (regrp_find_rept_candidate(p_own_rept,BEST_RSSI_BELOW_TH) == TRUE)
		found = 1;

	
	hex_dump("Target5g",p_own_rept->cli_target_mac_5g,6);
	hex_dump("Target2g",p_own_rept->cli_target_mac_5g,6);
	hex_dump("5gbssid",p_own_rept->cli_5g_bssid,6);
	hex_dump("2gbssid",p_own_rept->cli_2g_bssid,6);
	printf("found : %d\n", found);

	if(found == 1)
	{
		if(p_own_rept->is_master)
		{
			regrp_connect(p_own_rept);
			p_own_rept->need_retrigger_regrp = 1;
		}
	}
	else
	{
		printf("No candidate found for own rept\n");
	}
}
#endif

void restart_regroup(void * eloop_ctx, void * user_ctx)
{
	//if connected to third party then check if highest MAC
	// if not connected to third party, then check if root node
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	printf("%s: %d\n",__func__,p_own_rept->regrp_state);
	if(is_regrp_master_candidate(p_own_rept)
		&& p_own_rept->regrp_state == REGRP_STATE_RETRY)
	{
		trigger_regroup();
	}
	else if(!is_regrp_master_candidate(p_own_rept))
	{
		p_own_rept->regrp_state = REGRP_STATE_IDLE;
		trigger_regrp = 1;
		regrp_enable_apcli(p_own_rept);
	}
}
void event_scan_comp(struct _regrp_event *regrp_event_data)	//decide when to cancel
{
	p_repeater_list_struct p_own_rept = &g_own_rept_info;
	DBGPRINT(DEBUG_OFF, "-> for : %s\n",regrp_event_data->event_body);

	if((p_own_rept->is_master == 0  && p_own_rept->regrp_state != REGRP_SLAVE_STATE_SCAN)
		||
		(p_own_rept->is_master == 1  && p_own_rept->regrp_state != REGRP_STATE_SCAN)
		)
	{
		printf("ScanDone: Wrong State: %d\n", p_own_rept->regrp_state);
		return;
	}

	if(!strcmp(interface_5g,regrp_event_data->event_body))
	{
		// 5g scan done
		
		if(regrp_get_cand_list(cli_interface_5g,(UCHAR *)p_own_rept->cand_list_5g,&p_own_rept->cand_list_5g_count) != REGRP_CODE_SUCCESS)
			 // triger cand list config in driver for conenct / roam
		{
			printf("5G:GetCandiateList failed");
		}
		else
		{
			printf("5gCandCount: %d\n", p_own_rept->cand_list_5g_count);
		}
		p_own_rept->scan_done_5g = 1;

		// 5G scan done. initiate 2g scan
		if(interface_2g[0] != '\0')
			regrp_scan_req(interface_2g);
		else
			p_own_rept->scan_done_2g = 1;
	}

	else if(!strcmp(interface_2g,regrp_event_data->event_body))
	{
		// 2g scan done
		if(regrp_get_cand_list(cli_interface_2g,(UCHAR *)p_own_rept->cand_list_2g,&p_own_rept->cand_list_2g_count) == REGRP_CODE_SUCCESS){
			 // triger cand list config in driver for conenct / roam
			 printf("2gCandCount: %d\n", p_own_rept->cand_list_2g_count);
		
		}
		else
		{
			printf("2G:GetCandiateList failed");
		}
		
		p_own_rept->scan_done_2g = 1;
	}
	else
	{
		printf("Unknown interface\n");
		return;
	}

	if(p_own_rept->scan_done_2g == 1
		&& p_own_rept->scan_done_5g == 1)
	{
		eloop_cancel_timeout(regrp_scan_timeout,NULL, p_own_rept);

		if(p_own_rept->is_master == 0)
		{
			p_repeater_list_struct p_master_rept = NULL;
			p_master_rept  = get_rept_by_br_mac(p_own_rept->master_mac);

			//scan completed on own repeater. send msg 2 to the master.
			if(p_master_rept != NULL)
				regrp_mlme_compose_send_msg_2(p_own_rept,p_master_rept);

		}
#if 1
		else
		{
			//scan complete for master. send out msg 1 for all repeaters.
			p_repeater_list_struct p_rept = regrp_get_first_rept();
			
			sort_cand_list_by_rssi(p_own_rept->cand_list_5g,p_own_rept->cand_list_5g_count);
			sort_cand_list_by_rssi(p_own_rept->cand_list_2g,p_own_rept->cand_list_2g_count);
			
			p_own_rept->regrp_state =	REGRP_STATE_PEER_SCAN_DONE;
			//fetch the next rept and 
			
			if(p_own_rept->cand_list_5g_count)
			{
				int i;
				for(i=0; i< p_own_rept->cand_list_5g_count; i++)
				{
					printf("5g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
							PRINT_MAC(p_own_rept->cand_list_5g[i].bssid),p_own_rept->cand_list_5g[i].rssi,p_own_rept->cand_list_5g[i].Non_MAN);
				}
			}
			
			if(p_own_rept->cand_list_2g_count)
			{
				int i;
				for(i=0; i< p_own_rept->cand_list_2g_count; i++)
				{
					printf("2g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
							PRINT_MAC(p_own_rept->cand_list_2g[i].bssid),p_own_rept->cand_list_2g[i].rssi,p_own_rept->cand_list_2g[i].Non_MAN);
				}
			}
			if(p_rept)
			{
				regrp_mlme_compose_send_msg_1(p_own_rept,p_rept);
				p_own_rept->regrp_state = REGRP_STATE_TRIGGERED;
			}
			else
				p_own_rept->regrp_state = REGRP_STATE_DONE;
		}
#endif
	}
}

int Set_regroup_dbglevel_event_handle(char *data)
{
	DBGPRINT(DEBUG_OFF, "=> DebugLevel %d from event DebugLevel%d\n",DebugLevel,*data);
	DebugLevel = *data;
}
int Set_regroup_threshold_event_handle(char *data)
{
	regrp_threshold_event_t *rgrp_threshold = (	regrp_threshold_event_t *)data;
	DBGPRINT(DEBUG_INFO, "=>EVENT  default_rssi_threshold %d custom_rssi_th %d \n",rgrp_threshold->default_rssi_threshold,rgrp_threshold->custom_rssi_th);	

	if(rgrp_threshold->default_rssi_threshold)
		g_default_rssi_threshold = rgrp_threshold->default_rssi_threshold;
	if(rgrp_threshold->custom_rssi_th)
		g_custom_rssi_th = rgrp_threshold->custom_rssi_th;
	DBGPRINT(DEBUG_OFF, "=> g_default_rssi_threshold %d g_custom_rssi_th %d \n",g_default_rssi_threshold,g_custom_rssi_th);	
}
int Set_regroup_show_candidate_rssi_event_handle(char *data)
{
		DBGPRINT(DEBUG_OFF,"==>\n");
		unsigned char  i=0,j=0;
		unsigned char  *IpAddr;

	if(g_own_rept_info.cand_list_5g_count)
	{
		for(i=0; i< g_own_rept_info.cand_list_5g_count; i++)
		{
			DBGPRINT(DEBUG_OFF,"5g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(g_own_rept_info.cand_list_5g[i].bssid),g_own_rept_info.cand_list_5g[i].rssi,g_own_rept_info.cand_list_5g[i].Non_MAN);

		}
	}
	
	if(g_own_rept_info.cand_list_2g_count)
	{
		for(i=0; i< g_own_rept_info.cand_list_2g_count; i++)
		{
			DBGPRINT(DEBUG_OFF,"2g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(g_own_rept_info.cand_list_2g[i].bssid),g_own_rept_info.cand_list_2g[i].rssi,g_own_rept_info.cand_list_2g[i].Non_MAN);

		}
	}
}

int regrp_event_handle(char *data)
{
	struct _regrp_event *regrp_event_data = (struct _regrp_event *)data;

	//memcpy(&msg, data, sizeof(struct bndstrg_msg));
	DBGPRINT(DEBUG_OFF, "=> \n");

	switch (regrp_event_data->event_id)
	{
		//case REGROUP_EVT_CUSTOM_DATA:
			//printf("REGROUP_EVT_CUSTOM_DATA\n");
			//event_btm_query(wnm, wnm_event_data);			
		//	break;

		case REGROUP_EVT_USER_REGROUP_REQ:
			printf("REGROUP_EVT_USER_REGROUP_REQ\n");
			//event_regroup_req(p_regrp_ctx, NULL);			
			break;

		case REGROUP_EVT_SCAN_COMPLETE:
			printf("REGROUP_EVT_SCAN_COMPLETE\n");
			event_scan_comp(regrp_event_data);			
			break;

		// other events similar to event notifier
		case REGROUP_EVT_INTF_CONNECTED:
			printf("REGROUP_EVT_INTF_CONNECTED\n");
			// add handling	as per current state in app
			break;
		
		case REGROUP_EVT_INTF_DISCONNECTED:
			printf("REGROUP_EVT_INTF_DISCONNECTED\n");
			// add handling	as per current state in app
			break;

#if 0
		case new cli conencted

		case cli disconencted
#endif

		default:
			printf("Unkown event. (%u)\n",
						regrp_event_data->event_id);
			break;
	}
	
	return 0;
}


void regroup_timeout(void *eloop_data, void *user_ctx)	//decide when to cancel
{
	printf("Timeout in regroup process!!\n");
	regrp_clean_all_states();
	
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
	
	eloop_register_timeout(10, 0, restart_regroup,NULL, NULL);
	g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
}

UINT8 regrp_initiate()
{
	p_repeater_list_struct p_rept = regrp_get_first_rept();

	while(p_rept)
	{
		{
			if (regrp_mlme_compose_send_msg_regrp_initiate(p_rept) == FALSE)
				return FALSE;
		}
		p_rept = regrp_get_next_rept(p_rept);
	}

	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,REGRP_MODE_BLOCKED);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,REGRP_MODE_BLOCKED);

	os_sleep(3,0);
	return TRUE;

}


void clear_virt_rept_state()
{
	p_repeater_list_struct p_rept = regrp_get_first_rept();
	while(p_rept)
	{
		p_rept->is_processed = 0;
		p_rept->is_processed_tmp = 0;
		bzero(p_rept->cli_target_mac_2g,MAC_ADDR_LEN);
		bzero(p_rept->cli_target_mac_5g,MAC_ADDR_LEN);
		p_rept = regrp_get_next_rept(p_rept);
	}

}


void trigger_regroup()
{
	p_repeater_list_struct p_rept_entry = regrp_get_first_rept();
	printf("Trigger regroup\n");

	if(!is_regrp_master_candidate(&g_own_rept_info))
	{
		printf("NotMasterCandidate\n");
		return;
	}
//	regrp_clean_all_states();
	clear_virt_rept_state();
	g_own_rept_info.is_processed =0;
	g_own_rept_info.is_processed_tmp = 0;
	bzero(g_own_rept_info.cli_target_mac_2g,MAC_ADDR_LEN);
	bzero(g_own_rept_info.cli_target_mac_5g,MAC_ADDR_LEN);
	if(p_rept_entry == NULL)
	{
		printf("%s: No repeaters in the network. Abort\n", __func__);
		g_own_rept_info.regrp_state = REGRP_STATE_DONE;
		return;
	}
	g_own_rept_info.need_retrigger_regrp =0;
	regrp_pending =0;

	if(g_own_rept_info.regrp_state != REGRP_STATE_IDLE &&
		g_own_rept_info.regrp_state != REGRP_STATE_DONE &&
		g_own_rept_info.regrp_state != REGRP_STATE_RETRY)
	{
		printf("Regroup Already ongoing. Queue\n ");
		regrp_pending = 1;
		return;
	}
	g_own_rept_info.is_master = 1;
	//clear_virt_rept_state();

	if(cli_interface_2g[0] != '\0') 
	{
		get_connected_ap_mac(g_own_rept_info.cli_2g_bssid,cli_interface_2g);
	}
	
	if(cli_interface_5g[0] != '\0')
	{
		get_connected_ap_mac(g_own_rept_info.cli_5g_bssid,cli_interface_5g);
	}

	if (regrp_initiate() == FALSE)
	{
		printf("Regrp initiate failed. State: %d\n",g_own_rept_info.regrp_state);
		return;
	}
	//send msg_1 after own scan complete.
	if(cli_interface_5g[0] != '\0' && regrp_get_cand_list(cli_interface_5g,(UCHAR *)g_own_rept_info.cand_list_5g,&g_own_rept_info.cand_list_5g_count) != REGRP_CODE_SUCCESS)
	 // triger cand list config in driver for conenct / roam
	{
		printf("5G:GetCandiateList failed\n");
	}
	if(cli_interface_2g[0] != '\0' && regrp_get_cand_list(cli_interface_2g,(UCHAR *)g_own_rept_info.cand_list_2g,&g_own_rept_info.cand_list_2g_count) != REGRP_CODE_SUCCESS)
	 // triger cand list config in driver for conenct / roam
	{
		printf("2G:GetCandiateList failed\n");
	}

	if(g_own_rept_info.cand_list_5g_count)
	{
		int i;
		for(i=0; i< g_own_rept_info.cand_list_5g_count; i++)
		{
			printf("5g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(g_own_rept_info.cand_list_5g[i].bssid),g_own_rept_info.cand_list_5g[i].rssi,g_own_rept_info.cand_list_5g[i].Non_MAN);
		}
	}
	
	if(g_own_rept_info.cand_list_2g_count)
	{
		int i;
		for(i=0; i< g_own_rept_info.cand_list_2g_count; i++)
		{
			printf("2g:%d: %02x:%02x:%02x:%02x:%02x:%02x : Rssi=%d, NonMAN=%d\n",i,
					PRINT_MAC(g_own_rept_info.cand_list_2g[i].bssid),g_own_rept_info.cand_list_2g[i].rssi,g_own_rept_info.cand_list_2g[i].Non_MAN);
		}
	}



	regrp_mlme_compose_send_msg_1(&g_own_rept_info,p_rept_entry);
	//g_own_rept_info.regrp_state = REGRP_STATE_SCAN;

	//g_own_rept_info.regrp_state = REGRP_STATE_TRIGGERED;
	// start regroup timer.
//	eloop_register_timeout(SCAN_TIMEOUT_SEC, 0, regrp_scan_timeout, NULL, &g_own_rept_info);
	eloop_register_timeout(REGROUP_TIMEOUT_SEC, 0, regroup_timeout, NULL, NULL);

}




#if 0
void man_driver_event_handle(char *buf)
{

	device_config_to_app_t *device_info = (device_config_to_app_t *)buf;
	UINT8 is_prev_master_cand=0;
	//char command[200];
	//int ret;
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
	
	return;
}
#endif
#endif
