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
#include "driver_wext.h"
#ifdef REGROUP_SUPPORT
int ap_oid_set_info(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
        struct iwreq wrq;
        strcpy(wrq.ifr_name, DeviceName);
        wrq.u.data.length = PtrLength;
        wrq.u.data.pointer = (caddr_t) ptr;
        wrq.u.data.flags = OidQueryCode | OID_GET_SET_TOGGLE;
        return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}


int ap_oid_query_info(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
        struct iwreq wrq;
        strcpy(wrq.ifr_name, DeviceName);
        wrq.u.data.length = PtrLength;
        wrq.u.data.pointer = (caddr_t) ptr;
        wrq.u.data.flags = OidQueryCode;
        return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}

void regrp_get_own_ap_info(p_ap_info_struct p_ap_info, char *intf_name)
{
	int sd;
	char command_buf[256] = {0};
	struct _regrp_command *cmd = (struct _regrp_command *)command_buf;
	cmd->command_id = OID_REGROUP_QUERY_INTERFACE_DETAILS;
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_WH_EZ_REGROUP_COMMAND, sd, intf_name, command_buf, sizeof(command_buf)))
	{
		memcpy(p_ap_info, cmd, sizeof(ap_info_struct));
	}
	close(sd);
}
#if 0
void regrp_handle_driver_event(p_regrp_event_t regrp_event)
{
	switch(regrp_event->event_id)
	{
		case OID_VR_EVENT_TRIGGER_HANDOFF_MR:
		{
			printf("OID_VR_EVENT_TRIGGER_HANDOFF_MR\n");
			//regrp_handle_trigger_handoff_event(regrp_event);
			break;
		}
#if 0
		case OID_VR_SEND_SNIFF_RESULT:
		{
			p_peer_sniff_result_t sniff_result = regrp_event->event_body;
			p_repeater_list_struct rept_entry = get_rept_by_br_mac(event_sniff_result->mr_mac);
			UINT8 buffer[256] = {0};

			if (rept_entry)
			{
				memcpy(buffer, sniff_result, sizeof(peer_sniff_result_t));
				regrp_tcp_send(rept_entry->tcp_tx_sock, buffer, sizeof(peer_sniff_result_t));
			}
			break;
		}
#endif
		case OID_VR_EVENT_TRIGGER_HANDOFF_VR:
		{
			break;
		}
		case OID_VR_EVENT_TRIGGER_REVERSE_HANDOFF:
		{
			break;
		}
	}
}
#endif
#if 0
void regrp_dr_reset_handoff_trigger(p_sta_entry_struct sta_entry, UINT8 reason_code)
{
	int sd;
	char command_buf[256] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;
	regrp_command->command_id = OID_VR_QUERY_CMD_RESET_HANDOFF_TRIGGER;
	memcpy(regrp_command->command_body, sta_entry->mac_addr, MAC_ADDR_LEN);
	regrp_command->command_body[MAC_ADDR_LEN] = reason_code;
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sta_entry->band == Band_2G)
	{
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_2g, command_buf, sizeof(command_buf)))
		{
		}
	} else {
	
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_5g, command_buf, sizeof(command_buf)))
		{
		}
	}
	close(sd);


}
void regrp_dr_get_current_rssi(p_sta_entry_struct sta_entry)
{
	int sd;
	char command_buf[256] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;
	regrp_command->command_id = OID_VR_QUERY_STA_CURRENT_RSSI;
	memcpy(regrp_command->command_body, sta_entry->mac_addr, MAC_ADDR_LEN);
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sta_entry->band == Band_2G)
	{
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_2g, command_buf, sizeof(command_buf)))
		{
			sta_entry->last_rssi_sample = command_buf[0];
		}
	} else {
	
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_5g, command_buf, sizeof(command_buf)))
		{
			sta_entry->last_rssi_sample = command_buf[0];
		}
	}
	close(sd);

}

void regrp_dr_send_sniff_request(p_regrp_sniff_req_t sniff_request)
{
	int sd;
	char command_buf[256] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;
	regrp_command->command_id = OID_VR_CONFIG_SNIFF_MODE;
	memcpy(regrp_command->command_body, sniff_request, sizeof(regrp_sniff_request_t));
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sniff_request->band == Band_2G)
	{
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_2g, command_buf, sizeof(command_buf)))
		{
			sta_entry->last_rssi_sample = command_buf[0];
		}
	} else {
	
		if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, interface_5g, command_buf, sizeof(command_buf)))
		{
			sta_entry->last_rssi_sample = command_buf[0];
		}
	}
	close(sd);

}

#if 0
int regrp_drv_update_sta(p_regrp_sta_entry_struct p_entry)
{
		//Maybe no need. can be done after VR3
}


void regrp_drv_update_activate_regrp_sta(p_regrp_sta_entry_struct p_entry)
{
	//update the PN number and activate the STA.
	
	int sd;
	UINT8 intf_name[IFNAMSIZ];
	char command_buf[2048] = {0};
	UINT16 buf_len=0;
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;
	p_regrp_sta_update_struct p_regrp_update = (p_regrp_sta_update_struct)regrp_command->command_body;
	
	if(if_indextoname(p_entry->regrp_rept_inf_idx, intf_name) == NULL)
	{
		perror("if_indextoname");
		return;
	}
	
	regrp_command->command_id = OID_VR_UPDATE_VR_STA;
	memcpy(p_regrp_update->mac_addr,p_entry->mac_addr, MAC_ADDR_LEN);
	p_regrp_update->mlme_state_len = p_entry->mlme_state_len;
	memcpy(p_regrp_update->sta_mlme_state, p_entry->sta_mlme_state, p_entry->mlme_state_len);

	buf_len = sizeof (regrp_command)+ sizeof(regrp_sta_update_struct) + p_entry->mlme_state_len;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, intf_name, command_buf, buf_len))
	{
		printf("STA: Activated %x:%x:%x:%x:%x:%x: IfIdx: %s",PRINT_MAC(p_entry->mac_addr), intf_name);
	}

	close(sd);
}

void regrp_drv_disable_sta(p_sta_entry_struct p_entry)
{
	int sd;
	UINT8 intf_name[IFNAMSIZ];
	char command_buf[10] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;

	if(if_indextoname(p_entry->ifindex, intf_name) == NULL)
	{
		perror("if_indextoname");
		return;
	}
	
	regrp_command->command_id = OID_VR_DISABLE_STA;
	memcpy(regrp_command->command_body,p_entry->mac_addr, MAC_ADDR_LEN);
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, intf_name, command_buf, sizeof(command_buf)))
	{
		printf("STA: disabled %x:%x:%x:%x:%x:%x",PRINT_MAC(p_entry->mac_addr));
	}
	close(sd);
}

void regrp_drv_get_pn(p_sta_entry_struct p_entry)
{
	int sd;
	UINT8 intf_name[IFNAMSIZ];
	char command_buf[10] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;

	if(if_indextoname(p_entry->ifindex, intf_name) == NULL)
	{
		perror("if_indextoname");
		return;
	}
	
	regrp_command->command_id = OID_VR_GET_STA_PN;
	memcpy(regrp_command->command_body,p_entry->mac_addr, MAC_ADDR_LEN);
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, intf_name, command_buf, sizeof(command_buf)))
	{
		memcpy(p_entry->PN, command_buf, PN_LEN);
	}
	close(sd);
}

void regrp_drv_get_sta_mlme_state(UINT8* mac_addr, UINT8 ifindex, UINT8 * mlme_state, UINT16 *mlme_state_len)
{
	int sd;
	UINT8 intf_name[IFNAMSIZ];
	char command_buf[1024] = {0};
	p_regrp_command_t regrp_command = (p_regrp_command_t)command_buf;

	if(if_indextoname(ifindex, intf_name) == NULL)
	{
		perror("if_indextoname");
		return;
	}
	
	regrp_command->command_id = OID_VR_GET_STA_MLME_STATE;
	memcpy(regrp_command->command_body,mac_addr, MAC_ADDR_LEN);
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 == ap_oid_query_info(OID_VIRTUAL_ROAM_COMMAND, sd, intf_name, command_buf, sizeof(command_buf)))
	{
		*mlme_state_len = command_buf[0];
		memcpy(mlme_state, &command_buf[1], command_buf[0]);
	}
	close(sd);
}

#endif

#endif
#endif
