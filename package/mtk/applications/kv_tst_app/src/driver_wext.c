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
    	driver_wext.c
*/

#include "rrm_wnm.h"
#include "driver_wext.h"
#include "priv_netlink.h"
#include "wireless_copy.h"
#include "netlink.h"
#include <sys/ioctl.h>
#include <error.h>

extern u8 terminate;

static void wnm_driver_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct rrm_wnm *rrm_wnm = (struct rrm_wnm *)drv_data->priv;

	rrm_wnm->event_ops->wnm_event_handle(rrm_wnm, buf);
}

static void rrm_driver_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct rrm_wnm *rrm_wnm = (struct rrm_wnm *)drv_data->priv;

	rrm_wnm->event_ops->rrm_event_handle(rrm_wnm, buf);
}

static void driver_wext_event_wireless(struct driver_wext_data *drv,
                 void *ctx, char *data, int len)
{               
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *buf /*,*assoc_info_buf, *info_pos */;

    /* info_pos = NULL; */
	/* assoc_info_buf = NULL; */
    pos = data;
    end = data + len;   
    
    while (pos + IW_EV_LCP_LEN <= end) {
        /* 
 		 * Event data may be unaligned, so make a local, aligned copy
         * before processing. 
         */
        os_memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		
		if (iwe->u.data.flags != 1)
			DBGPRINT(DEBUG_INFO, "cmd = 0x%x len = %d : pos = %p, end= %p\n", iwe->cmd, iwe->len, pos, end);
        
		if (iwe->len <= IW_EV_LCP_LEN)
            return;

        custom = pos + IW_EV_POINT_LEN;

        //if (drv->we_version_compiled > 18 && iwe->cmd == IWEVCUSTOM) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            os_memcpy(dpos, pos + IW_EV_LCP_LEN,
                  sizeof(struct iw_event) - dlen);
        //} else {
            //os_memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            //custom += IW_EV_POINT_OFF;
		//}
		
		switch (iwe->cmd) {
        case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
               	return;
           	buf = os_malloc(iwe->u.data.length + 1);
            if (buf == NULL)
                return;
            os_memcpy(buf, custom, iwe->u.data.length);
            buf[iwe->u.data.length] = '\0';

            switch (iwe->u.data.flags) {
			case OID_802_11_WNM_EVENT:
				DBGPRINT(DEBUG_TRACE, "WNM_EVENT\n");
				wnm_driver_event_handle(drv,buf);
				break;
			case OID_802_11_RRM_EVENT:
				DBGPRINT(DEBUG_TRACE, "RRM_EVENT\n");
				rrm_driver_event_handle(drv,buf);
				break;
			
			default:
				if (iwe->u.data.flags != 1)
					DBGPRINT(DEBUG_INFO, "unkwnon event type(%d)\n", iwe->u.data.flags);
				break; 
			}

           	os_free(buf);
            break;
        }

        pos += iwe->len;
    }
}

 int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t len)    
{
    char *buf;                             
    struct iwreq iwr;
	
    buf = os_zalloc(len);

    os_memset(&iwr, 0, sizeof(iwr));       
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.flags = oid;
    iwr.u.data.flags |= OID_GET_SET_TOGGLE;

    if (data)
        os_memcpy(buf, data, len);

	if (buf) {
    	iwr.u.data.pointer = (caddr_t)buf;    
    	iwr.u.data.length = len;
	} else {
    	iwr.u.data.pointer = NULL;    
    	iwr.u.data.length = 0;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(DEBUG_ERROR, "%s: oid=0x%x len (%d) failed: %d\n",
               __FUNCTION__, oid, (int)len, errno);
        os_free(buf);
        return -1;
    }

    os_free(buf);
    return 0;
}

int driver_wext_get_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t len)    
{
    char *buf;                             
    struct iwreq iwr;
	
    buf = os_zalloc(len);

    os_memset(&iwr, 0, sizeof(iwr));       
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.flags = oid;
    iwr.u.data.flags |= OID_GET_SET_FROM_UI;

    if (data)
        os_memcpy(buf, data, len);

	if (buf) {
    	iwr.u.data.pointer = (caddr_t)buf;    
    	iwr.u.data.length = len;
	} else {
    	iwr.u.data.pointer = NULL;    
    	iwr.u.data.length = 0;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(DEBUG_ERROR, "%s: oid=0x%x len (%d) failed: %d\n",
               __FUNCTION__, oid, (int)len, errno);
        os_free(buf);
        return -1;
    }

	memcpy(data, buf, len);
	
    os_free(buf);
    return 0;
}

static void driver_wext_event_rtm_newlink(void *ctx, struct ifinfomsg *ifi,
                    					  u8 *buf, size_t len)
{
    struct driver_wext_data *drv = ctx;
    int attrlen, rta_len;
    struct rtattr *attr;
    
    attrlen = len;

   	DBGPRINT(DEBUG_INFO, "attrlen=%d", attrlen);

    attr = (struct rtattr *) buf;
    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen)) {
        DBGPRINT(DEBUG_INFO, "rta_type=%02x\n", attr->rta_type);
        if (attr->rta_type == IFLA_WIRELESS) {
            driver_wext_event_wireless(
                drv, ctx,
                ((char *) attr) + rta_len,
                attr->rta_len - rta_len);
        }
        attr = RTA_NEXT(attr, attrlen);
    }
}

static void *driver_wext_init(struct rrm_wnm *rrm_wnm, 
						      const int opmode,
							  const int drv_mode)
{
	struct driver_wext_data *drv_wext_data;
#if 1
	struct netlink_config *cfg;
#endif
	DBGPRINT(DEBUG_TRACE, "\n");
	
	drv_wext_data = calloc(1, sizeof(*drv_wext_data));
	
	if (!drv_wext_data) {
		DBGPRINT(DEBUG_ERROR, "No avaliable memory for driver_wext_data\n");
		goto err1;

	}

	DBGPRINT(DEBUG_OFF, "Initialize ralink wext interface\n");

	drv_wext_data->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (drv_wext_data->ioctl_sock < 0) {
		DBGPRINT(DEBUG_ERROR, "socket(PF_INET,SOCK_DGRAM)");
		goto err2;
	}
#if 1
	cfg = os_zalloc(sizeof(*cfg));

    if (!cfg) {
		DBGPRINT(DEBUG_ERROR, "No avaliable memory for netlink cfg\n");
        goto err3;
    }

	cfg->ctx = drv_wext_data;
	cfg->newlink_cb = driver_wext_event_rtm_newlink;

	drv_wext_data->netlink = netlink_init(cfg);

	if (!drv_wext_data->netlink) {
		DBGPRINT(DEBUG_ERROR, "wext netlink init fail\n");
		goto err3;
	}
#endif

	drv_wext_data->priv = (void *)rrm_wnm;

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return NULL;
}

static int driver_wext_exit(struct rrm_wnm *rrm_wnm)
{
	struct driver_wext_data *drv_wext_data = rrm_wnm->drv_data;

	DBGPRINT(DEBUG_TRACE, "\n");

	netlink_deinit(drv_wext_data->netlink);

	close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}

int driver_wnm_onoff(void * drv_data, const char *ifname, u8 onoff)
{
	struct wnm_command *cmd;
	int cmd_len;

	cmd_len = sizeof(struct wnm_command) + 1;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);
	cmd->command_id = OID_802_11_WNM_CMD_ENABLE;
	cmd->command_len = 1;
	cmd->command_body[0]= onoff;

	driver_wext_set_oid(drv_data,ifname,OID_802_11_WNM_COMMAND, (char *)cmd, cmd_len);

	os_free(cmd);
	return 0;
}

int driver_wnm_btm_onoff(void * drv_data, const char *ifname, int onoff)
{
	struct wnm_command *cmd;
	int cmd_len;

	cmd_len = sizeof(struct wnm_command) + 2;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);
	cmd->command_id = OID_802_11_WNM_CMD_CAP;
	cmd->command_len = 2;
	cmd->command_body[0] = (cmd->command_body[0] & !(1<<0)) | onoff ;
	cmd->command_body[1]= 0;

	driver_wext_set_oid(drv_data,ifname,OID_802_11_WNM_COMMAND, (char *)cmd, cmd_len);

	os_free(cmd);
	return 0;

}

int driver_btm_query_cap(void * drv_data, const char *ifname, u8 *mac, u8 *cap)
{
	struct wnm_command *cmd;
	int cmd_len;

	cmd_len = sizeof(struct wnm_command) + MAC_ADDR_LEN+1;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);
	cmd->command_id = OID_802_11_WNM_CMD_QUERY_BTM_CAP;
	cmd->command_len = MAC_ADDR_LEN+1;
	memcpy(cmd->command_body, mac, MAC_ADDR_LEN);

	driver_wext_get_oid(drv_data,ifname,OID_802_11_WNM_COMMAND, (char *)cmd, cmd_len);
	memcpy(cap, (cmd->command_body+MAC_ADDR_LEN), 1);
	os_free(cmd);
	return 0;

}



int driver_wnm_send_btm_req(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_req,
							size_t btm_req_len)
{
	int len;
	struct wnm_command *cmd_data;
	struct btm_req_data *btm_req_data;

	len = sizeof(struct wnm_command)+ sizeof(struct btm_req_data) +btm_req_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);
	cmd_data->command_id = OID_802_11_WNM_CMD_SEND_BTM_REQ;
	cmd_data->command_len = sizeof(struct btm_req_data)+ btm_req_len;


	btm_req_data = (struct btm_req_data *)cmd_data->command_body;
	//btm_req_data->ifindex = ifindex;
	os_memcpy(btm_req_data->peer_mac_addr, peer_sta_addr,6);
	btm_req_data->btm_req_len = btm_req_len;
	os_memcpy(btm_req_data->btm_req,btm_req,btm_req_len );

	hex_dump("btm_req",cmd_data->command_body, cmd_data->command_len);
	//send_btm_req();
	driver_wext_set_oid(drv_data,ifname, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_wnm_send_btm_req_param(void *drv_data, const char *ifname,
							const char *btm_req_param, u32 param_len)
{
	int len;
	struct wnm_command *cmd_data;

	len = sizeof(struct wnm_command)+ param_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);
	cmd_data->command_id = OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, btm_req_param,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_wnm_send_btm_req_raw(void *drv_data, const char *ifname,
							const char *btm_req_raw, u32 param_len)
{
	int len;
	struct wnm_command *cmd_data;

	len = sizeof(struct wnm_command)+ param_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);
	cmd_data->command_id = OID_802_11_WNM_CMD_SEND_BTM_REQ_IE;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, btm_req_raw,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_rrm_send_nr_rsp_param(void *drv_data, const char *ifname,
							const char *nr_rsp_param, u32 param_len)
{
	int len;
	p_rrm_command_t cmd_data;

	len = sizeof(*cmd_data)+ param_len;
	cmd_data = (p_rrm_command_t)os_zalloc(len);
	cmd_data->command_id = OID_802_11_RRM_CMD_SET_NEIGHBOR_REPORT_PARAM;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, nr_rsp_param, param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_rrm_send_nr_rsp_raw(void *drv_data, const char *ifname,
							const char *nr_rsp_raw, u32 param_len)
{
	int len;
	p_rrm_command_t cmd_data;

	len = sizeof(*cmd_data)+ param_len;
	cmd_data = (p_rrm_command_t)os_zalloc(len);
	cmd_data->command_id = OID_802_11_RRM_CMD_SEND_NEIGHBOR_REPORT;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, nr_rsp_raw, param_len);
	driver_wext_set_oid(drv_data,ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_rrm_send_nr_req_handle_way(void *drv_data, const char *ifname,
							u8 way)
{
	int len;
	struct wnm_command *cmd_data;

	len = sizeof(struct wnm_command)+ 1;
	cmd_data = (struct wnm_command *)os_zalloc(len);
	cmd_data->command_id = OID_802_11_RRM_CMD_HANDLE_NEIGHBOR_REQUEST_BY_DAEMON;
	cmd_data->command_len = 1;

	os_memcpy(cmd_data->command_body, &way,1);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_rrm_onoff(void *drv_data, const char *ifname, int onoff)
{
	struct rrm_command_s *cmd = NULL;
	int cmd_len = 0;

	cmd_len = sizeof(struct rrm_command_s) + 1;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);
	cmd->command_id = OID_802_11_RRM_CMD_ENABLE;
	cmd->command_len = 1;
	cmd->command_body[0] = onoff;

	driver_wext_set_oid(drv_data,ifname,OID_802_11_RRM_COMMAND, (char *)cmd, cmd_len);

	os_free(cmd);
	return 0;

}

int driver_rrm_query_cap(void * drv_data, const char *ifname, u8 *mac, u8 *cap)
{
	struct rrm_command_s *cmd = NULL;
	int cmd_len = 0;

	cmd_len = sizeof(struct rrm_command_s) + MAC_ADDR_LEN+8;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);
	cmd->command_id = OID_802_11_RRM_CMD_QUERY_CAP;
	cmd->command_len = MAC_ADDR_LEN+8;
	memcpy(cmd->command_body, mac, MAC_ADDR_LEN);

	driver_wext_get_oid(drv_data,ifname,OID_802_11_RRM_COMMAND, (char *)cmd, cmd_len);
	memcpy(cap, (cmd->command_body+MAC_ADDR_LEN), 8);
	os_free(cmd);
	return 0;
}

int driver_rrm_send_bcn_req_param(void *drv_data, const char *ifname,
							const char *bcn_req_param, u32 param_len)
{
	int len;
	p_rrm_command_t cmd_data;

	len = sizeof(*cmd_data)+ param_len;
	cmd_data = (p_rrm_command_t)os_zalloc(len);
	cmd_data->command_id = OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, bcn_req_param,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}

int driver_rrm_send_bcn_req_raw(void *drv_data, const char *ifname,
							const char *bcn_req_raw, u32 param_len)
{
	int len;
	p_rrm_command_t cmd_data;

	len = sizeof(*cmd_data)+ param_len;
	cmd_data = (p_rrm_command_t)os_zalloc(len);
	cmd_data->command_id = OID_802_11_RRM_CMD_SEND_BEACON_REQ;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, bcn_req_raw,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;

}


const struct rrm_wnm_drv_ops rrm_wnm_drv_wext_ops = {
	.drv_inf_init = driver_wext_init,
	.drv_inf_exit = driver_wext_exit,
	.drv_wnm_onoff = driver_wnm_onoff,
	.drv_send_btm_req = driver_wnm_send_btm_req,
	.drv_send_btm_req_raw = driver_wnm_send_btm_req_raw,
	.drv_send_btm_req_param = driver_wnm_send_btm_req_param,
	.drv_btm_onoff     = driver_wnm_btm_onoff,
	.drv_btm_query_cap  = driver_btm_query_cap,
	.drv_send_nr_rsp_param = driver_rrm_send_nr_rsp_param,
	.drv_send_nr_rsp_raw = driver_rrm_send_nr_rsp_raw,
	.drv_send_nr_req_handle_way = driver_rrm_send_nr_req_handle_way,
	.drv_rrm_onoff     = driver_rrm_onoff,
	.drv_rrm_query_cap  = driver_rrm_query_cap,
	.drv_send_bcn_req_param = driver_rrm_send_bcn_req_param,
	.drv_send_bcn_req_raw = driver_rrm_send_bcn_req_raw,
};

