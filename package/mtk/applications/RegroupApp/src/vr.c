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

#include "man.h"

/*Global Definitions*/
repeater_list_struct g_virtual_rept_list[MAX_VIRTUAL_REPT_NUM];
//sta_entry_struct g_station_list[MAX_STA_SUPPORT];
repeater_list_struct g_own_rept_info;
UINT32 tcp_listeners[MAX_VIRTUAL_REPT_NUM];
UINT32 g_virtual_repeater_count;
struct os_time last_entry_added_time;
UINT8 trigger_regrp =1;
/*************************************************************************************************/

/*Extern*/
extern unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern char interface_5g[16];
extern char interface_2g[16];
extern UINT8 regrp_pending;
extern char cli_interface_5g[16];
extern char cli_interface_2g[16];

/*************************************************************************************************/


/*Functions*/
#if 0
void get_node_number_network_wt(p_repeater_list_struct p_own_rept)
{
	int sd;
	device_info_to_app_t device_info_local;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(ap_oid_query_info(OID_WH_EZ_GET_DEVICE_INFO, sd, interface_2g, &device_info_local, sizeof(device_info_to_app_t)) < 0)
	{
		printf("get rept info failed\n");
		return;
	}
	memcpy(p_own_rept->network_weight, 
		device_info_local.network_weight, NETWORK_WEIGHT_LEN);
	memcpy(&p_own_rept->node_number,&device_info_local.node_number, sizeof(EZ_NODE_NUMBER));
}
#endif

void regrp_get_node_number_wt(p_repeater_list_struct p_rept)
{
	int sd;
	UINT8 intf_name[IFNAMSIZ];
	node_num_wt node_wt;
	char command_buf[256] = {0};
	struct _regrp_command *cmd = (struct _regrp_command *)command_buf;
	//p_vr_command_t vr_command = (p_vr_command_t)command_buf;
	cmd->command_id = OID_REGROUP_QUERY_NODE_NUMBER_WT;
	cmd->command_len = 0;
	printf("%s\n",__func__);
	if(interface_2g[0] != '\0')
	{
		strcpy(intf_name,interface_2g);
		intf_name[strlen(interface_2g)] = '\0';
	}
	if(interface_5g[0] != '\0')
	{
		strcpy(intf_name,interface_5g);
		intf_name[strlen(interface_5g)] = '\0';
	}
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_WH_EZ_REGROUP_COMMAND, sd, intf_name, command_buf, sizeof(command_buf)))
	{
		memcpy(&node_wt, cmd, sizeof(node_num_wt));
		memcpy(&p_rept->node_number,&node_wt.node_number,sizeof(EZ_NODE_NUMBER));
		memcpy(p_rept->network_weight,node_wt.network_wt,NETWORK_WEIGHT_LEN);
		hex_dump("OwnNN", (UINT8 *)&p_rept->node_number,sizeof(EZ_NODE_NUMBER));
	}
	close(sd);

}
void vr_set_own_rept_info()
{
	memzero(&g_own_rept_info, sizeof (g_own_rept_info));
	if(interface_5g[0] != '\0')
	{
		vr_get_own_ap_info(&g_own_rept_info.ap_info_5g, interface_5g);
		ez_hex_dump("5G Info",&g_own_rept_info.ap_info_5g,sizeof(ap_info_struct));
		
	}
	if(interface_2g[0] != '\0')
	{
		vr_get_own_ap_info(&g_own_rept_info.ap_info_2g, interface_2g);
		ez_hex_dump("2G Info",&g_own_rept_info.ap_info_2g,sizeof(ap_info_struct));
	}

	g_own_rept_info.valid = 1;
	get_ifip("br-lan", &g_own_rept_info.ip_addr);
	get_mac(g_own_rept_info.br_mac,"br-lan");
	//get_node_number_network_wt(&g_own_rept_info);

	regrp_get_node_number_wt(&g_own_rept_info);
	hex_dump("---------->OwnNN",(UINT8 *)&g_own_rept_info.node_number, sizeof(EZ_NODE_NUMBER));
//	regrp_get_node_number_wt(&g_own_rept_info,);
#if 0
	memcpy(&g_own_rept_info.network_weight[1], &g_own_rept_info.br_mac[0], MAC_ADDR_LEN);
	g_own_rept_info.node_number.path_len = 6;
	memcpy(g_own_rept_info.node_number.root_mac, &g_own_rept_info.br_mac[0], MAC_ADDR_LEN);
#endif

	g_own_rept_info.regrp_state = REGRP_STATE_IDLE;
}


int vr_tcp_disconnect(p_repeater_list_struct p_rept_entry)
{
	printf("%s: %x\n",__func__,p_rept_entry->ip_addr);
	
	close(p_rept_entry->tcp_tx_sock);
	p_rept_entry->tcp_tx_sock = 0;
	p_rept_entry->tcp_link_done = 0;
}

int vr_tcp_connect(p_repeater_list_struct p_rept_entry)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	fd_set fdset;
	struct timeval tv;

	//portno = PORT_VR + 1;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("ERROR opening socket");
		return -1;
	}
	printf("Connect to IP: %x: %d\n",p_rept_entry->ip_addr, sockfd);
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(p_rept_entry->ip_addr);
	serv_addr.sin_port = htons(PORT_VR);

	fcntl(sockfd, F_SETFL, O_NONBLOCK); // setup non blocking socket



#if 1
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		if(errno != EINPROGRESS)
		{
			perror("ERROR connecting");
			close(sockfd);
			return -1;
		}
	} else {
		p_rept_entry->tcp_link_done = 1;
		//fcntl(sockfd, F_SETFL, O_BLOCK); // setup blocking socket
	//	return sockfd;
	}
#endif
#if 1
	FD_ZERO(&fdset);
   FD_SET(sockfd, &fdset);
   tv.tv_sec = 10;			   /* 10 second timeout */
   tv.tv_usec = 0;

   if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1)
   {
	   int so_error;
	   socklen_t len = sizeof so_error;
	   int opts = fcntl(sockfd,F_GETFL);
	   getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

	   if (so_error == 0) {
		   printf(" Socket is open\n");
		   p_rept_entry->tcp_link_done = 1;
		   opts = opts & (~O_NONBLOCK);
		   fcntl(sockfd, F_SETFL, opts); // setup blocking socket
		   return sockfd;
	   }
	   else
	   {
		   perror("ERROR connecting");
		   close(sockfd);
		   return -1;
	   }
   }
#endif
}

int vr_tcp_send(int sockfd, UINT8 *buffer)
{
}
#if 0
void vr_send_command_repeater_config(p_repeater_list_struct p_rept_entry, UINT8 band)
{
	UINT8 buffer[256] = {0};
	p_vr_command_t vr_command = (p_vr_command_t)buffer;
	p_vr_config_interface_t vr_config = (p_vr_config_interface_t)vr_command->command_body;
	int sd;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	
	vr_command->command_id =  OID_VR_CONFIG_VIRTUAL_INTERFACE;
	if (band == Band_2G)
	{
		vr_config->main_idx = g_own_rept_info.ap_info_2g.wdev_id;
		vr_config->vr_idx = p_rept_entry->ap_info_2g.wdev_id;
		vr_config->vr_init = 1;
		memcpy(vr_config->vr_mac, p_rept_entry->ap_info_2g.mac_addr, MAC_ADDR_LEN);		
		ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_2g, buffer, sizeof(*vr_config) + 1);
		memzero(buffer, sizeof(buffer));
		sprintf(buffer,"ifconfig %s%d down; ifconfig %s%d up", 
			g_own_rept_info.ap_info_2g.intf_prefix, p_rept_entry->ap_info_2g.wdev_id,
			g_own_rept_info.ap_info_2g.intf_prefix, p_rept_entry->ap_info_2g.wdev_id);
	} else {
		vr_config->main_idx = g_own_rept_info.ap_info_5g.wdev_id;
		vr_config->vr_idx = p_rept_entry->ap_info_5g.wdev_id;
		vr_config->vr_init = 1;
		memcpy(vr_config->vr_mac, p_rept_entry->ap_info_5g.mac_addr, MAC_ADDR_LEN);
		if (ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_5g, buffer, sizeof(*vr_config) + 1))
		{
			printf("error encountered while configuring VR\n");
		}
		memzero(buffer, sizeof(buffer));
		sprintf(buffer,"ifconfig %s%d down; ifconfig %s%d up", 
			g_own_rept_info.ap_info_5g.intf_prefix, p_rept_entry->ap_info_5g.wdev_id,
			g_own_rept_info.ap_info_5g.intf_prefix, p_rept_entry->ap_info_5g.wdev_id);
	}
	system(buffer);

}
#endif
void vr_set_peer_device_shared_info (p_device_shared_info_struct p_new_rept, p_repeater_list_struct rept_entry)
{
	rept_entry->ip_addr = p_new_rept->ip_addr;
	memcpy(rept_entry->br_mac, p_new_rept->br_mac, MAC_ADDR_LEN);

	memcpy(rept_entry->ap_info_2g.mac_addr, p_new_rept->ap_shared_info_2g.mac_addr, MAC_ADDR_LEN);
	memcpy(rept_entry->ap_info_5g.mac_addr, p_new_rept->ap_shared_info_5g.mac_addr, MAC_ADDR_LEN);

	rept_entry->ap_info_2g.ssid_len = p_new_rept->ap_shared_info_2g.ssid_len;
	rept_entry->ap_info_5g.ssid_len = p_new_rept->ap_shared_info_5g.ssid_len;
	
	memcpy(rept_entry->ap_info_2g.ssid, p_new_rept->ap_shared_info_2g.ssid, p_new_rept->ap_shared_info_2g.ssid_len);
	memcpy(rept_entry->ap_info_5g.ssid, p_new_rept->ap_shared_info_5g.ssid, p_new_rept->ap_shared_info_5g.ssid_len);

}

void vr_add_rept_entry (p_device_shared_info_struct new_rept_entry)
{
	p_repeater_list_struct p_rept_entry = g_virtual_rept_list;
	int i = 0;
	// Add repeater entry to the global list
	// if a new one, then configure the device for the new mbss
	// can be optimized that only the nearest ones are chosen if entries full.
	//create tcp tx with that repeater entry.

	for(i=0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (p_rept_entry->valid)
		{
			p_rept_entry++;
			continue;
		}
		printf("Add Rept Entry : ipaddr : %x\n",new_rept_entry->ip_addr);
		hex_dump("br_mac",new_rept_entry->br_mac,6);
		vr_set_peer_device_shared_info(new_rept_entry, p_rept_entry);
#if 0
		if (!MAC_ADDR_EQUAL(p_rept_entry->ap_info_5g.mac_addr,ZERO_MAC_ADDR))
		{
			p_rept_entry->ap_info_5g.wdev_id = FIRST_VR_WDEV_ID + 2*i;
			vr_send_command_repeater_config(p_rept_entry, Band_5G);
		}
		
		if (!MAC_ADDR_EQUAL(p_rept_entry->ap_info_2g.mac_addr,ZERO_MAC_ADDR))
		{
			p_rept_entry->ap_info_2g.wdev_id = FIRST_VR_WDEV_ID + 2*i + 1;
			vr_send_command_repeater_config(p_rept_entry, Band_2G);
		}
#endif
		p_rept_entry->vr_init_done = 1;
		p_rept_entry->valid = 1;
		p_rept_entry->entry_idx = i;
#if 1		
	//	p_rept_entry->tcp_tx_sock = vr_tcp_connect(p_rept_entry);
#endif
		os_get_time(&last_entry_added_time);

		// if entry is added after regroup done then trigger another regroup
		if(is_regrp_master_candidate(&g_own_rept_info) 
			&& g_own_rept_info.regrp_state == REGRP_STATE_DONE)
		{
		
			printf("%s:Restart Regroup\n",__func__);
			eloop_register_timeout(RESTART_REGROUP_TIME, 0, restart_regroup,NULL, NULL);
			g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
			
		}
		else if(is_regrp_master_candidate(&g_own_rept_info) && 
			g_own_rept_info.regrp_state != REGRP_STATE_DONE
			&& g_own_rept_info.regrp_state != REGRP_STATE_IDLE)
		{
			regrp_pending =1;
		}
		os_get_time(&p_rept_entry->last_frame_time);
		break;
		
	}
	
	if (i == MAX_VIRTUAL_REPT_NUM)
	{
		printf("Couldnt allocate new VR");
	}
}

void vr_delete_rept_entry(p_repeater_list_struct p_rept_entry)
{
	printf("%s: %x\n", p_rept_entry->ip_addr);
	if(p_rept_entry->tcp_rx_sock >= 0)
		close(p_rept_entry->tcp_rx_sock);
	if(p_rept_entry->tcp_tx_sock >= 0)
		close(p_rept_entry->tcp_tx_sock);
	printf("%s: %x\n", p_rept_entry->ip_addr);
	/*Make sure all the STA transferred to this VR are handled.*/
	memzero(p_rept_entry, sizeof(repeater_list_struct));

}

void regrp_delete_listner(int sock)
{
	int i = 0;
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (tcp_listeners[i] == sock)
		{
			tcp_listeners[i] = 0;
		}
	}
}

void vr_close_all_tcp_listners()
{
	int i = 0;
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (tcp_listeners[i])
		{
			eloop_unregister_read_sock(tcp_listeners[i]);
			close(tcp_listeners[i]);
			tcp_listeners[i] = 0;
		}
	}
}

void vr_close_all_tcp_tx_sockets()
{
	int i = 0;

	p_repeater_list_struct p_rept = regrp_get_first_rept();
	while(p_rept != NULL)
	{
		if(p_rept->tcp_link_done == 1 && p_rept->tcp_tx_sock >= 0)
		{
			hex_dump("Close TcP TX with", p_rept->br_mac,6);
			close(p_rept->tcp_tx_sock);
			p_rept->tcp_tx_sock =-1;
			p_rept->tcp_link_done =0;
		}
		p_rept = regrp_get_next_rept(p_rept);
		
	}

}


void vr_get_own_device_shared_info (p_device_shared_info_struct p_device_info)
{
	UINT32 local_ip;
	
	get_ifip("br-lan",&local_ip);
	if (local_ip != g_own_rept_info.ip_addr)
	{
		printf("My IP changed, change all listners\n");
		g_own_rept_info.ip_addr = local_ip;
		vr_close_all_tcp_listners();
		vr_close_all_tcp_tx_sockets();
		os_get_time(&last_entry_added_time);
	}

	p_device_info->ip_addr = local_ip;
	memcpy(p_device_info->network_weight,
		g_own_rept_info.network_weight, NETWORK_WEIGHT_LEN);

	memcpy(&p_device_info->node_number,
		&g_own_rept_info.node_number, sizeof(EZ_NODE_NUMBER));

	memcpy(p_device_info->br_mac, g_own_rept_info.br_mac, MAC_ADDR_LEN);

	memcpy(p_device_info->ap_shared_info_2g.mac_addr, g_own_rept_info.ap_info_2g.mac_addr, MAC_ADDR_LEN);
	memcpy(p_device_info->ap_shared_info_5g.mac_addr, g_own_rept_info.ap_info_5g.mac_addr, MAC_ADDR_LEN);

	p_device_info->ap_shared_info_2g.ssid_len = g_own_rept_info.ap_info_2g.ssid_len;
	p_device_info->ap_shared_info_5g.ssid_len = g_own_rept_info.ap_info_5g.ssid_len;
	
	memcpy(p_device_info->ap_shared_info_2g.ssid, g_own_rept_info.ap_info_2g.ssid,g_own_rept_info.ap_info_2g.ssid_len);
	memcpy(p_device_info->ap_shared_info_5g.ssid, g_own_rept_info.ap_info_5g.ssid,g_own_rept_info.ap_info_5g.ssid_len);

}




p_repeater_list_struct get_rept_by_br_mac(UINT8 *mac)
{
#if 1
	int i=0, j=0;
	p_repeater_list_struct p_rept = g_virtual_rept_list;
	while(i < MAX_VIRTUAL_REPT_NUM)
	{
		if(p_rept[i].valid)
		{
			 if (MAC_ADDR_EQUAL(p_rept[i].br_mac,mac))
				return &p_rept[i];
		}
		i++;
	}
	printf("No rept found with Bridge MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(mac));
	return NULL;
#endif
}

UINT8 is_rept_info_changed(p_repeater_list_struct p_rept,
	p_device_shared_info_struct new_rept_info)
{
	UINT8 change_bitmap = 0;
	if(p_rept->ip_addr != new_rept_info->ip_addr)
	{
		printf("IP changed: old: %x, New %x\n", p_rept->ip_addr, new_rept_info->ip_addr);
		change_bitmap |= IP_CHANGED;
	}

	if(memcmp(&p_rept->node_number,&new_rept_info->node_number,sizeof(EZ_NODE_NUMBER)))
	{
		printf("Node Number changed\n");
		hex_dump("oldNN",(UINT8 *)&p_rept->node_number, sizeof(EZ_NODE_NUMBER));
		hex_dump("NewNN",(UINT8 *)&new_rept_info->node_number, sizeof(EZ_NODE_NUMBER));
		change_bitmap |= NODE_NUMBER_CHANGED;

	}

	if (memcmp(p_rept->network_weight,new_rept_info->network_weight, NETWORK_WEIGHT_LEN))
	{
	
		printf("Weight changed\n");
		
		hex_dump("oldWt",(UINT8 *)p_rept->network_weight, NETWORK_WEIGHT_LEN);
		hex_dump("NewWt",(UINT8 *)new_rept_info->network_weight, NETWORK_WEIGHT_LEN);
		change_bitmap |= WEIGHT_CHANGED;
	}
	if(change_bitmap)
	{
		hex_dump("rept_mac:", p_rept->br_mac,6);
		printf("ReptIdx:%d", p_rept->entry_idx);
	}
	return change_bitmap;
}
static void vr_server(int sock, void *eloop_ctx, void *sock_ctx)
{
	int server_socket = sock;
	struct sockaddr_in client_address;
	char buf[512];
	unsigned int clientLength;
	int message;
	char command[BUFLEN];
	
	//printf("%s\n", __FUNCTION__);

	{
		clientLength = sizeof(client_address);
		memset(buf, '\0', BUFLEN);
		message = 0;
		//printf("receive msg\n");		
		message = recvfrom(server_socket, buf, BUFLEN, 0,
		(struct sockaddr*) &client_address, &clientLength);
		//printf("msg received\n");		
				

		if(message == -1)
			perror("Error: recvfrom call failed");

		//printf("SERVER: read %d bytes from IP %s(%s)\n", message,
		//inet_ntoa(client_address.sin_addr), buf);
		//printf("UDP msg = :\n");
		if (!memcmp(&buf[0], REPT_INFO_TAG, REPT_INFO_TAG_LEN))
		{
			UINT8 msg_len = message - REPT_INFO_TAG_LEN;
			p_device_shared_info_struct rept_info = NULL;
			p_repeater_list_struct virtual_rept_info = NULL;
			rept_info = (p_device_shared_info_struct)(&buf[REPT_INFO_TAG_LEN]);
			if (!MAC_ADDR_EQUAL(rept_info->br_mac, g_own_rept_info.br_mac))
			{
			//	hex_dump("UDP_MSG", &buf[REPT_INFO_TAG_LEN],6);

				virtual_rept_info = get_rept_by_br_mac(rept_info->br_mac);
				
				if (virtual_rept_info == NULL)
				{
					printf("create a new VR interface\n");
					vr_add_rept_entry(rept_info);
				} else {
					char new_rept_info = is_rept_info_changed(virtual_rept_info, rept_info);
					if(new_rept_info)
					{
					
						if (new_rept_info & IP_CHANGED) {
							printf("change in IP found:%d, %d \n",virtual_rept_info->tcp_link_done,virtual_rept_info->tcp_tx_sock);
							virtual_rept_info->ip_addr = rept_info->ip_addr;
							//vr_tcp_disconnect(virtual_rept_info);
						}
						if (new_rept_info & NODE_NUMBER_CHANGED)
						{
							memcpy(&virtual_rept_info->node_number,&rept_info->node_number,sizeof(EZ_NODE_NUMBER));
						}
						
						if (new_rept_info & WEIGHT_CHANGED)
						{
							memcpy(virtual_rept_info->network_weight,rept_info->network_weight,NETWORK_WEIGHT_LEN);
						}
						
						os_get_time(&last_entry_added_time);

						if(is_regrp_master_candidate(&g_own_rept_info)
							&& g_own_rept_info.regrp_state == REGRP_STATE_DONE
							&& new_rept_info)
						{
						
							printf("%s:Restart Regroup\n",__func__);
							eloop_register_timeout(RESTART_REGROUP_TIME, 0, restart_regroup,NULL, NULL);
							g_own_rept_info.regrp_state = REGRP_STATE_RETRY;
						} else if (new_rept_info && is_regrp_master_candidate(&g_own_rept_info))
						{
							if(g_own_rept_info.regrp_state == REGRP_STATE_TRIGGERED)
								g_own_rept_info.need_retrigger_regrp = 1;
							else
								printf("No regrp Trigger: State: %d\n",g_own_rept_info.regrp_state);
						}

					}
					os_get_time(&virtual_rept_info->last_frame_time);
					if (new_rept_info 
						&& virtual_rept_info->tcp_link_done 
						&& virtual_rept_info->tcp_tx_sock)
						{

							if (new_rept_info & IP_CHANGED) {
								printf("change in IP found. disconnect previous connection\n");
								virtual_rept_info->ip_addr = rept_info->ip_addr;
								vr_tcp_disconnect(virtual_rept_info);
								//vr_tcp_connect(virtual_rept_info);
								printf("Disconnect done");
							}
#if 0
							if (new_rept_info & NODE_NUMBER_CHANGED)
							{
								memcpy(&virtual_rept_info->node_number,&rept_info->node_number,sizeof(EZ_NODE_NUMBER));
							}

							if (new_rept_info & WEIGHT_CHANGED)
							{
								memcpy(virtual_rept_info->network_weight,rept_info->network_weight,NETWORK_WEIGHT_LEN);
							}
#endif
						}
					virtual_rept_info->udp_count++;
				}
			}
		}
#if 0
		else if (!memcmp(&buf[0], SNIFF_REQ_TAG, SNIFF_REQ_TAG_LEN))
		{
			UINT8 msg_len = message - REPT_INFO_TAG_LEN;
			p_vr_sniff_req_t sniff_request = (p_vr_sniff_req_t)(&buf[SNIFF_REQ_TAG_LEN]);
			p_repeater_list_struct virtual_rept_info = NULL;
			if (!MAC_ADDR_EQUAL(sniff_request->br_mac, g_own_rept_info.br_mac))
			{
				virtual_rept_info = get_rept_by_br_mac(sniff_request->br_mac);
				if (virtual_rept_info)
				{
					vr_dr_send_sniff_request(sniff_request);
				} else {
					printf("Error: Sniff request from un-registered VR\n");
				}
			}			

		}
#endif
	}
}



p_repeater_list_struct get_rept_by_ip(UINT32 ip_addr)
{
#if 0
	int i=0, j=0;
	p_repeater_list_struct p_rept = g_virtual_rept_list;
	while(i < MAX_VIRTUAL_REPT_NUM)
	{
		if(p_rept[i].valid)
		{
			 if (p_rept[i].ap_info[j].ip_addr == ip_addr)
				return &p_rept[i];
		}
		i++;
	}
	printf("err no rept found with IP : %x\n", ip_addr);
	return NULL;
#endif
return NULL;

}


void vr_rept_update_tcp_sock(int sock, UINT8 add)

{
	struct sockaddr_in address;
	UINT32 addrlen=0;
	p_repeater_list_struct p_rept_list = NULL;
	addrlen = sizeof(address);

	if (sock == 0)
	{
		printf("%s:Invalid Socket\n", __FUNCTION__);
		return;
	}
	if (getpeername(sock , (struct sockaddr*)&address , (socklen_t*)&addrlen)<0)
	{
		perror("getpeername");
	}
	if((p_rept_list = vr_mlme_search_rept_by_ip(address.sin_addr.s_addr)) != NULL)
	{
		if (add)
			p_rept_list->tcp_rx_sock = sock;
		else
			p_rept_list->tcp_rx_sock = 0;
	}
		

}

static void vr_handle_rx_tcp_pkt(p_repeater_list_struct p_rept,UINT8 *buffer, UINT32 valread)
{
	switch (buffer[MAC_ADDR_LEN])
	{
		case MSG_REGRP_INIT:
			vr_mlme_handle_msg_initiate(p_rept,buffer,valread);
			break;
		case MSG_1:  // start_regroup and scan procedure
			vr_mlme_handle_msg_1(p_rept,buffer,valread);
			break;
		case MSG_2: // handle scan results of all repeaters
			vr_mlme_handle_msg_2(p_rept,buffer,valread);
			break;
		case MSG_3: // handle the connect command. 
			vr_mlme_handle_msg_3(p_rept,buffer,valread);
			break;
		default:
			printf("Err: Unknown Message");
	}
}

static void vr_tcp_cli_server(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct sockaddr_in address;
	UINT32 addrlen;
	ssize_t  valread;
	UINT8 buffer[1024];
	UINT8 *sock_addr_raw;
	UINT32 ip;
	p_repeater_list_struct p_rept = NULL;

	printf("received data for Socket %x\n", sock);
	if (getpeername(sock , (struct sockaddr*)&address , (socklen_t*)&addrlen)<0)
	{
		perror("getpeername");
		close( sock );
		eloop_unregister_read_sock(sock);
		return;
	}

	hex_dump("PeerName", &address, sizeof(address));
	sock_addr_raw = (UINT8 *)&address;
	ip = *(UINT32 *)(sock_addr_raw + 12);	

	if ((valread = read( sock , buffer, 1024)) < 0)
	{
	   //Somebody disconnected , get his details and print
	//   addrlen = sizeof(address);
	//   printf("Host disconnected , ip %s , port %d \n" , ip, ntohs(address.sin_port));
	   //vr_rept_update_tcp_sock(sock, FALSE);
	   //Close the socket and mark as 0 in list for reuse
	   
	   regrp_delete_listner(sock);
	   eloop_unregister_read_sock(sock);
	   
	   close( sock );	  
	}
	if(valread == 0)
	{
		printf("zero bytes received\n");
		
		//addrlen = sizeof(address);
	//	printf("Host disconnected , ip %s , port %d \n" , ip, ntohs(address.sin_port));
		//vr_rept_update_tcp_sock(sock, FALSE);
		//Close the socket and mark as 0 in list for reuse
		eloop_unregister_read_sock(sock);
		close( sock );
		regrp_delete_listner(sock);
		printf("closeSocket\n");
		return;
	}

	if(valread < sizeof(msg_1_struct))
	{
		printf("Msg too small : %d\n", valread);
		return;
	}
	if((p_rept = get_rept_by_br_mac(buffer)) != NULL)
	{
		vr_handle_rx_tcp_pkt(p_rept,buffer, valread);
		p_rept->tcp_rx_sock = sock;
	}

}


/*Main TCP server which accepts client connections*/
static void vr_tcp_main_server(int sock, void *eloop_ctx, void *sock_ctx)
{
	int tcp_cli_srv_sock;
	struct sockaddr_in client_address;
	int i=0;
	//char buf[512];
	unsigned int clientLength;

	printf("%s\n", __FUNCTION__);
	clientLength = sizeof(client_address);
	if ((tcp_cli_srv_sock = accept(sock, (struct sockaddr *)&client_address, (socklen_t*)&clientLength))<0)
	{
	   perror("accept");
	   return;
//	   exit(EXIT_FAILURE);
	}
	printf("Accept connection from: %x:%x : SockFd:%d\n",client_address.sin_addr.s_addr,
		client_address.sin_port, tcp_cli_srv_sock);
	//vr_rept_update_tcp_sock(tcp_cli_srv_sock, TRUE);
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if (tcp_listeners[i] == 0)
		{
			tcp_listeners[i] = tcp_cli_srv_sock;
			
			break;
		}
	}
	printf("register a listen socket %x\n", tcp_cli_srv_sock);
	eloop_register_read_sock(tcp_cli_srv_sock,vr_tcp_cli_server,NULL,NULL);
}

/*Register the TCP and UDP server to listen for messages from other repeaters.*/
void vr_register_server()
{
	int server_socket, server_tcp_socket;
	struct sockaddr_in server_address;
	int checkCall;
	int broadcast = 1;
	char netif1[] = "br-lan";
	UINT32 ip_addr;
	int opt = TRUE;

	printf("%s\n", __FUNCTION__);
	/*Create socket for listening to broadcast frames */
	server_socket=socket(AF_INET, SOCK_DGRAM, 0);

	if(server_socket == -1)
		perror("Error: socket failed");

	setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST,	&broadcast, sizeof(broadcast));

	bzero((char*) &server_address, sizeof(server_address));

	/*Fill in server's sockaddr_in*/
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(PORT_VR);

	setsockopt(server_socket, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));

	/*Bind server socket and listen for incoming clients*/
	checkCall = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	if(checkCall == -1)
		perror("Error: bind call failed");

	eloop_register_read_sock(server_socket,vr_server,NULL,NULL);

	/*create socket to listen to unicast frames*/
	
	server_tcp_socket=socket(AF_INET, SOCK_STREAM, 0);

	if(server_tcp_socket == -1)
		perror("Error: TCP socket failed");

	//setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST,	&broadcast, sizeof(broadcast));
    
    if( setsockopt(server_tcp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

	bzero((char*) &server_address, sizeof(server_address));

	/*Fill in server's sockaddr_in*/
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(PORT_VR);

	setsockopt(server_tcp_socket, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));
	/*Bind server socket and listen for incoming clients*/
	checkCall = bind(server_tcp_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	if(checkCall == -1)
		perror("Error: bind call failed");

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(server_tcp_socket, MAX_VIRTUAL_REPT_NUM) < 0)
	{
		perror("listen");
		//exit(EXIT_FAILURE);
		return;
	}
	  


	eloop_register_read_sock(server_tcp_socket,vr_tcp_main_server,NULL,NULL);

}

UINT8 is_connected_to_non_man(p_repeater_list_struct p_rept)
{
	if(p_rept->network_weight[0] == 0x0f
		&& p_rept->node_number.path_len == 7)
		return TRUE;

	return FALSE;
}
// if mac_1 > mac_2 return 1, else return 0
is_mac_higher(UINT8 *mac_1, UINT8* mac_2)
{
	int i;
	for (i= 5; i >= 0; i--)
	{
		if (mac_1[i] > mac_2[i])
			return 1;
		if((mac_1[i] == mac_2[i]))
			continue;
		if(mac_1[i] < mac_2[i])
			return 0;
	}
	return 0;
}
UINT8 is_mac_highest(p_repeater_list_struct p_own_rept)
{
	UINT8 i = 0;
	UINT8 mac_highest =1;
	p_repeater_list_struct virtual_rept = g_virtual_rept_list;
	//hex_dump("OwnMAC:", p_own_rept->br_mac,6);
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if( virtual_rept[i].valid && is_connected_to_non_man(&virtual_rept[i]))
		{
		//	hex_dump("VirtMAC:", virtual_rept[i].br_mac,6);
			if(is_mac_higher(virtual_rept[i].br_mac, p_own_rept->br_mac))
			{
	//			printf("Mac Not Highest\n");
				mac_highest = 0;
				return mac_highest;
			}
		}
	}
	printf("Mac Highest: %d\n",mac_highest);
	return mac_highest;
}

UINT8 is_regrp_master_candidate(p_repeater_list_struct p_own_rept)
{
	if(is_root_node(p_own_rept))
	{
		//printf("Is root Node\n");
		//p_own_rept->is_master = 1;
		return 1;
	}
#if 1	
	else if (p_own_rept->non_ez_connection && p_own_rept->node_number.path_len == 7)
	{
		printf("Connected to Non MAN : IP : %x: vrtRcnt: %d\n",g_own_rept_info.ip_addr,g_virtual_repeater_count);
		//p_own_rept->is_master = is_mac_highest(p_own_rept);
		return is_mac_highest(p_own_rept);
	}
#endif	
}

UINT8 should_trigger_regrp_first_time(p_repeater_list_struct p_own_rept)
{
	struct os_time now;
	os_get_time(&now);
#if 1
		if(((g_virtual_repeater_count > 1)
			&&
			(now.sec > last_entry_added_time.sec
				&& now.sec - last_entry_added_time.sec > 30))
			||
			((g_virtual_repeater_count == 1) &&
			(p_own_rept->non_ez_connection == 1) &&
			(now.sec > last_entry_added_time.sec
				&& now.sec - last_entry_added_time.sec > 180)))
#else
		if((g_virtual_repeater_count > 1 && 
			(now.sec > last_entry_added_time.sec
				&& now.sec - last_entry_added_time.sec > 45)))
#endif
		return TRUE;

return FALSE;


}

UINT8 should_trigger_regrp(p_repeater_list_struct p_own_rept)
{
	UINT8 is_candidate;
	struct os_time now;
	os_get_time(&now);

	if (trigger_regrp == 0)
		return;

	is_candidate = is_regrp_master_candidate(p_own_rept);
//	printf("%s: %d %d %d %d %d",__func__,is_candidate,first_time,);
	if(is_candidate == 0)
		return;

	if(p_own_rept->regrp_state != REGRP_STATE_IDLE && p_own_rept->regrp_state != REGRP_SLAVE_STATE_IDLE){
		printf("Regroup State is not idle :%d\n",p_own_rept->regrp_state);
		return;
	}

	if(should_trigger_regrp_first_time(p_own_rept))
	{
		trigger_regrp = 0;
		printf("Trigger Regroup first time %d\n",g_virtual_repeater_count);
		trigger_regroup();
		return;
	}
		

}

void vr_maintainance()
{
	int i = 0;
	p_repeater_list_struct virtual_rept = g_virtual_rept_list;
	struct os_time now;

	os_get_time(&now);

	g_virtual_repeater_count = 0;
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		if(virtual_rept->valid &&
			now.sec - virtual_rept->last_frame_time.sec > 30
		)
		{
			printf("No Broadcast received for 30s, Delete entry\n");
			hex_dump("ReptMac",virtual_rept->br_mac,6 );
			regrp_clear_rept_state(virtual_rept);
			vr_delete_rept_entry(virtual_rept);
			printf("Abc\n");
			os_get_time(&last_entry_added_time);
		}
		if(virtual_rept->valid 
					&& virtual_rept->vr_init_done 
					/*&& virtual_rept->tcp_link_done*/)
			{
				g_virtual_repeater_count++;
			}
		if (virtual_rept->valid 
			&& virtual_rept->vr_init_done 
			&& (!virtual_rept->tcp_link_done || !virtual_rept->tcp_tx_sock)
			&& (virtual_rept->udp_count > 5))
			{
			//	printf("Active VR does not have communication channel, create one:");
			//	virtual_rept->tcp_tx_sock = vr_tcp_connect(virtual_rept);	
			//	printf("%d\n",virtual_rept->tcp_tx_sock);
			//	virtual_rept->udp_count =0;
			}
		virtual_rept++;
	}
}

void vr_udp_client()
{

	int sock1, sinlen, config_len;
	char broadcast_mac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	char config[256] = {"\0"};
	struct sockaddr_in sock_in;

	int yes = 1;
	char netif1[] = "br-lan";
	int i;
	//printf("udp send\n");
	{
		sinlen = sizeof(struct sockaddr_in);
		memset(&sock_in, 0, sinlen);
		
		sock1 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );

		setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));


		/* -1 = 255.255.255.255 this is a BROADCAST address,
		 a local broadcast address could also be used.
		 you can comput the local broadcat using NIC address and its NETMASK 
		*/ 

		memcpy(config, REPT_INFO_TAG, REPT_INFO_TAG_LEN);
		//memcpy(&config[REPT_INFO_TAG_LEN], vr_get_own_device_shared_info(), sizeof(repeater_list_struct));
		vr_get_own_device_shared_info(&config[REPT_INFO_TAG_LEN]);
		config_len = REPT_INFO_TAG_LEN + sizeof(repeater_list_struct);
		//ez_hex_dump("send_to:",config,config_len);
		sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
		sock_in.sin_port = htons(PORT_VR); /* port number */
		sock_in.sin_family = AF_INET;
		if(sendto(sock1, config, config_len, 0, (struct sockaddr *)&sock_in, sinlen) < 0)
		{
			printf("Udp fail\n");
		}
	}


		close(sock1);
		shutdown(sock1, 2);
}

void vr_periodic_exec()
{
	//printf("vr_periodic_exec\n");
	vr_udp_client();
	vr_maintainance();
	
	should_trigger_regrp(&g_own_rept_info);
	eloop_register_timeout(1, 0, vr_periodic_exec, NULL, NULL);
}

void vr_init_globals()
{
	// init the global variables for the functioning of virtual repeaters
	// list of repeaters in the network.
	// structure for all connected STA
	// structure for all transfered STA

	memzero(g_virtual_rept_list, sizeof (g_virtual_rept_list));
	//memzero(g_station_list, sizeof(g_station_list));
	memzero(&g_own_rept_info, sizeof (g_own_rept_info));
	vr_set_own_rept_info();
	
	if(cli_interface_5g[0]!= '\0')
		set_regrp_mode(cli_interface_5g,NON_REGRP_MODE);
	if(cli_interface_2g[0]!= '\0')
		set_regrp_mode(cli_interface_2g,NON_REGRP_MODE);
}
#if 0

void vr_send_sniff_req(p_vr_sniff_req_t sniff_request)
{
	int sock1, sinlen, config_len;
	char broadcast_mac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	char config[256] = {"\0"};
	struct sockaddr_in sock_in;

	int yes = 1;
	char netif1[] = "br-lan";
	int i;
	
	{
		sinlen = sizeof(struct sockaddr_in);
		memset(&sock_in, 0, sinlen);
		
		sock1 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );

		setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE, netif1, sizeof(netif1));


		/* -1 = 255.255.255.255 this is a BROADCAST address,
		 a local broadcast address could also be used.
		 you can comput the local broadcat using NIC address and its NETMASK 
		*/ 

		memcpy(config, SNIFF_REQ_TAG, SNIFF_REQ_TAG_LEN);
		memcpy(&config[SNIFF_REQ_TAG_LEN], sniff_request, sizeof(vr_sniff_request_t));
		//vr_get_own_device_shared_info(&config[REPT_INFO_TAG_LEN]);
		config_len = SNIFF_REQ_TAG_LEN + sizeof(repeater_list_struct);
		ez_hex_dump("send_to:",config,config_len);
		sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
		sock_in.sin_port = htons(PORT_VR); /* port number */
		sock_in.sin_family = AF_INET;
		sendto(sock1, config, config_len, 0, (struct sockaddr *)&sock_in, sinlen);

	}


		close(sock1);
		shutdown(sock1, 2);
}

void vr_handle_rssi_too_low(char * buf)
{
	
}

void vr_handle_rssi_better(char * buf)
{
	
}

void vr_update_station_event(char * buf, int num_sta)
{
	int i = 0;
	int j = 0;
	unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};
	for (j = 0; j < num_sta; j ++)
	{
		for (i = 0; i < MAX_STA_SUPPORT; i++)
		{
			if (MAC_ADDR_EQUAL(g_station_list[i].mac_addr, &buf[j * MAC_ADDR_LEN]))
			{
				DBGPRINT(DEBUG_OFF, "Update station list\n");
				
				memcpy(g_station_list[i].mac_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
				break;
			}
		}
	}
}	

void vr_delete_station_event(char * buf)
{
	int i = 0;
	unsigned char ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0};
	for (i = 0; i < MAX_STA_SUPPORT; i++)
	{
		if (MAC_ADDR_EQUAL(g_station_list[i].mac_addr, &buf[2]))
		{
			DBGPRINT(DEBUG_OFF, "delete from station list\n");
			
			memcpy(g_station_list[i].mac_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
			/*Raghav: Add more things here*/
			break;
		}
	}
}


int vr_get_station_entry()
{
	int i = 0;
	for (i = 0; i < MAX_STA_SUPPORT; i++)
	{
		if (g_station_list[i].valid)
		{
			continue;
		} else {
			return i;
		}
#if 0
		if (MAC_ADDR_EQUAL(g_station_list[i].mac_addr,&buf[2]))
		{
			break;
		}
		if (MAC_ADDR_EQUAL(g_station_list[i].mac_addr,ZERO_MAC_ADDR))
		{
			printf("Add to station list\n");
			memcpy(g_station_list[i].mac_addr, &buf[2], MAC_ADDR_LEN);
			/*Raghav: Add more things here.*/
			break;
		}
#endif		
	}
	return -1;
}


int vr_find_best_ap_for_sta(p_sta_entry_struct sta_entry)
{
	int i =0;
	int target_repeater_idx = -1;
	for (i = 0; i < MAX_VIRTUAL_REPT_NUM; i++)
	{
		
		if (target_repeater_idx == 0xff)
		{
			if (sta_entry->sniff_cb.sniff_result[i].rssi > sta_entry->last_rssi_sample)
			{
				target_repeater_idx = i;
			}
		} else {
			if (sta_entry->sniff_cb.sniff_result[i].rssi > sta_entry->sniff_cb.sniff_result[target_repeater_idx].rssi)
			{
				target_repeater_idx = i;
			}
		}
	}
	return target_repeater_idx;
}


void vr_sniff_req_timeout(void *eloop_data, void *user_data)
{
	int target_repeater_idx = 0;
	p_sta_entry_struct sta_entry = user_data;
	printf("vr_sniff_req_timeout\n");
	if (sta_entry->state == VR_STA_STATE_SEARCH)
	{
		if (sta_entry->sniff_cb.sniff_results_count == g_virtual_repeater_count)
		{
			printf("Results received from all Virtual repeaters\n");
			vr_dr_get_current_rssi(sta_entry);
			target_repeater_idx  = vr_find_best_ap_for_sta(sta_entry);
			if (target_repeater_idx == -1)
			{
				printf("Current repeater is best fit for the STA, default all states and start backoff\n");
				sta_entry->state = VR_STA_STATE_OWN;
				vr_dr_reset_handoff_trigger(sta_entry, NO_BETTER_REPEATER_FOUND);
			} else {

			}
		} else {
			printf("Didnt receive results from all repeaters, default all states and start backoff\n");
			sta_entry->state = VR_STA_STATE_OWN;
			vr_dr_reset_handoff_trigger(sta_entry, RESULTS_INCOMPLETE);
		}
	} else {
		printf("sniff_req_timeout called for sta in incorrect state\n");
	}
}

void vr_force_ping_timeout(void *eloop_data, void *user_data)
{
	p_sta_entry_struct sta_entry = user_data;
	UINT8 * ip = &sta_entry->ip_addr;
	UINT8 ping[250] = {'\0'};
	//printf("vr_force_ping_timeout\n");
	sprintf(ping, "ping -c1 %d.%d.%d.%d -w 2 > t", ip[0], ip[3], ip[2], ip[3]);
	system(ping);
	if (sta_entry->state == VR_STA_STATE_SEARCH)
		eloop_register_timeout(0,200,vr_force_ping_timeout,NULL,sta_entry);
	
}
void vr_handle_trigger_handoff_event(p_vr_event_t vr_event)
{
	vr_sniff_request_t sniff_req;
	int sta_index;
	p_vr_trigger_handoff_t trigger_handoff= vr_event->event_body;
	if (vr_mlme_search_sta_by_mac(trigger_handoff->mac_addr) == NULL)
	{
		printf("Sta Entry not available\n");
		sta_index = vr_get_station_entry();
		if (sta_index != -1)
		{
			printf("Add new station on sta index %d\n", sta_index);
			memset(&g_station_list[sta_index], 0, sizeof(g_station_list[sta_index]));
			g_station_list[sta_index].band = trigger_handoff->band;
			memcpy(g_station_list[sta_index].mac_addr, trigger_handoff->mac_addr, MAC_ADDR_LEN);
			g_station_list[sta_index].ip_addr = trigger_handoff->ip_addr;
			g_station_list[sta_index].state = VR_STA_STATE_SEARCH;
			g_station_list[sta_index].valid = 1;
		} else {
			printf("Error: station Entry not available\n");
			return;
		}
	} else {
		printf("Sta entry available\n");
		if (g_station_list[sta_index].state != VR_STA_STATE_OWN)
		{
			printf("Error: trigger roam command for a station not in idle state\n");
			return;
		} else {					
			printf("update station state to VR_STA_STATE_SEARCH\n");
			g_station_list[sta_index].state = VR_STA_STATE_SEARCH;
		}
	}
	memcpy(sniff_req.br_mac, g_own_rept_info.br_mac, MAC_ADDR_LEN);
	memcpy(sniff_req.client_mac, trigger_handoff->mac_addr, MAC_ADDR_LEN);
	sniff_req.band = trigger_handoff->band;
	vr_send_sniff_req(&sniff_req);
	eloop_register_timeout(0,200,vr_force_ping_timeout,NULL,&g_station_list[sta_index]);
	eloop_register_timeout(5,0,vr_sniff_req_timeout,NULL,&g_station_list[sta_index]);
}

void vr_send_msg_to_rept(p_repeater_list_struct p_rept, UINT8 *buf, UINT8 buf_len)
{
	

}
#endif
/*************************************************************************************************/


