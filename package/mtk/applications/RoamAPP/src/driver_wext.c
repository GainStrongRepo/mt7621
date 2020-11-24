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

#include "bndstrg.h"
#include "rrm.h"
#include "driver_wext.h"
#include "priv_netlink.h"
#include "wireless_copy.h"
#include "netlink.h"
#include <sys/ioctl.h>
#include <error.h>
#include "froam.h"

static void event_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct bndstrg *bndstrg = (struct bndstrg *)drv_data->priv;

	bndstrg->event_ops->event_handle(bndstrg, buf);
}
static void wnm_driver_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct wnm *wnm = (struct wnm *)drv_data->priv;

	wnm->event_ops->event_handle(wnm, buf);
}
static void rrm_driver_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	//struct wnm *wnm = (struct wnm *)drv_data->priv;
	p_rrm_event_t p_rrm_event = (p_rrm_event_t)buf;
	//p_bcn_rsp_data_t p_bcn_rsp_data = (p_bcn_rsp_data_t) p_rrm_event->event_body;
	hex_dump("RRM_EVENT", (unsigned char *)buf, ((unsigned int)(sizeof(rrm_event_t) + p_rrm_event->event_len)));
}

static void froam_driver_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	DBGPRINT(DEBUG_INFO, "-->\n");

	pfroam_ctx pfroam = (pfroam_ctx)drv_data->priv;

	pfroam->event_ops->event_handle(pfroam, buf);

	DBGPRINT(DEBUG_INFO, "<--\n");
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
		
		//if (iwe->u.data.flags != 1)
		//	DBGPRINT(DEBUG_TRACE, "cmd = 0x%x len = %d : pos = %p, end= %p\n", iwe->cmd, iwe->len, pos, end);
        
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
			case OID_BNDSTRG_MSG:
				event_handle(drv, buf); 
				break;
			case OID_802_11_WNM_EVENT:
				DBGPRINT(DEBUG_TRACE, "WNM_EVENT\n");
				wnm_driver_event_handle(drv,buf);
				break;
			case OID_802_11_RRM_EVENT:
				DBGPRINT(DEBUG_TRACE, "RRM_EVENT\n");
				rrm_driver_event_handle(drv,buf);
				break;

			case OID_FROAM_EVENT:
				DBGPRINT(DEBUG_TRACE, "FROAM_EVENT\n");
				froam_driver_event_handle(drv,buf);
				break;

			default:
				if (iwe->u.data.flags != 1)
					DBGPRINT(DEBUG_ERROR, "unkwnon event type(%d)\n", iwe->u.data.flags);
				break; 
			}

           	os_free(buf);
            break;
        }

        pos += iwe->len;
    }

}

 int driver_wext_get_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t *len)    
{
    struct iwreq iwr;

	DBGPRINT(DEBUG_INFO, "--> oid=0x%x, len=%d, iface=%s \n",oid, (int)(*len), ifname);
	
    os_memset(&iwr, 0, sizeof(iwr));       
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.flags = oid;

	if (data && len){
		iwr.u.data.pointer = (caddr_t)data;    
		iwr.u.data.length = *len;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(DEBUG_ERROR, ": oid=0x%x failed: %d\n",oid, errno);
        return -1;
    }

	*len = iwr.u.data.length;

	DBGPRINT(DEBUG_INFO, "<-- len=%d\n", (int)(*len));

    return 0;
}

 int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t len)    
{
    char *buf = NULL;                             
    struct iwreq iwr;

	DBGPRINT(DEBUG_INFO, "--> oid=0x%x, len=%d, iface=%s \n",oid, (int)len, ifname);

    os_memset(&iwr, 0, sizeof(iwr));       
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.flags = oid;
    iwr.u.data.flags |= OID_GET_SET_TOGGLE;

    if (data && len){
    	buf = os_zalloc(len);
        os_memcpy(buf, data, len);
	}

	if (buf) {
    	iwr.u.data.pointer = (caddr_t)buf;    
    	iwr.u.data.length = len;
	} else {
    	iwr.u.data.pointer = NULL;    
    	iwr.u.data.length = 0;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(DEBUG_ERROR, ": oid=0x%x len (%d) failed: %d\n",
               oid, (int)len, errno);
        os_free(buf);
        return -1;
    }

    os_free(buf);

	DBGPRINT(DEBUG_INFO, "<--\n");
    return 0;
}

/*
 * Test bndstrg cmd
 */
static int driver_wext_test(void *drv_data, const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	

	DBGPRINT(DEBUG_OFF, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_BNDSTRG_TEST, "123", 4);

	return ret;
}

static int driver_wext_accessible_cli(
				void *drv_data,
				const char *ifname,
				struct bndstrg_cli_entry *entry,
				u8 action)
{
	int ret;
	struct driver_wext_data *drv_wext_data = \
					(struct driver_wext_data *)drv_data;	
	struct bndstrg_msg msg;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	memcpy(msg.Addr, entry->Addr, MAC_ADDR_LEN);
	msg.TalbeIndex = entry->TableIndex;
	msg.Action = action;

	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_BNDSTRG_MSG,
				(char *) &msg,
				sizeof(struct bndstrg_msg));

	return ret;
}

static int driver_wext_inf_status_query(
				void *drv_data,
				const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = \
					(struct driver_wext_data *)drv_data;	
	struct bndstrg_msg msg;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	msg.Action = INF_STATUS_QUERY;

	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_BNDSTRG_MSG,
				(char *) &msg,
				sizeof(struct bndstrg_msg));
	DBGPRINT(DEBUG_TRACE, "ret = %u\n", ret);
	return ret;
}

static int driver_wext_bndstrg_onoff(
				void *drv_data,
				const char *ifname,
				u8 onoff)
{
	int ret;
	struct driver_wext_data *drv_wext_data = \
					(struct driver_wext_data *)drv_data;	
	struct bndstrg_msg msg;

	DBGPRINT(DEBUG_OFF, "%s\n", __FUNCTION__);
	msg.Action = BNDSTRG_ONOFF;
	msg.OnOff = onoff;

	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_BNDSTRG_MSG,
				(char *) &msg,
				sizeof(struct bndstrg_msg));

	return ret;
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

static void *driver_wext_init(struct bndstrg *bndstrg, 
						      const int opmode,
							  const int drv_mode)
{
	struct driver_wext_data *drv_wext_data;
#if 1
	struct netlink_config *cfg;
#endif
	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);
	
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

	drv_wext_data->priv = (void *)bndstrg;

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return NULL;
}

static int driver_wext_exit(struct bndstrg *bndstrg)
{
	struct driver_wext_data *drv_wext_data = bndstrg->drv_data;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	netlink_deinit(drv_wext_data->netlink);

	close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}

const struct bndstrg_drv_ops bndstrg_drv_wext_ops = {
	.drv_inf_init = driver_wext_init,
	.drv_inf_exit = driver_wext_exit,
	.drv_test = driver_wext_test,
	.drv_accessible_cli = driver_wext_accessible_cli,
	.drv_inf_status_query = driver_wext_inf_status_query,
	.drv_bndstrg_onoff = driver_wext_bndstrg_onoff,
};

static void *driver_wnm_wext_init(struct wnm *wnm, 
						      const int opmode,
							  const int drv_mode)
{
	struct driver_wext_data *drv_wext_data;
#if 1
	struct netlink_config *cfg;
#endif
	
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

	drv_wext_data->priv = (void *)wnm;

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return NULL;
}



static int driver_wnm_wext_exit(struct wnm *wnm)
{
	struct driver_wext_data *drv_wext_data = wnm->drv_data;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	netlink_deinit(drv_wext_data->netlink);

	close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}

int driver_wnm_onoff(void * drv_data, const char *ifname,u8 onoff)
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

int driver_wnm_btm_onoff(void * drv_data, const char *ifname,int onoff)
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




const struct wnm_drv_ops wnm_drv_wext_ops = {
	.drv_inf_init = driver_wnm_wext_init,
	.drv_inf_exit = driver_wnm_wext_exit,
	.drv_wnm_onoff = driver_wnm_onoff,
	.drv_send_btm_req = driver_wnm_send_btm_req,
	.drv_btm_onoff     = driver_wnm_btm_onoff,
};

static void *driver_froam_wext_init(pfroam_ctx pfroam)
{
	struct driver_wext_data *drv_wext_data;
	struct netlink_config *cfg;

	DBGPRINT(DEBUG_INFO, "-->\n");
	
	drv_wext_data = calloc(1, sizeof(*drv_wext_data));
	
	if (!drv_wext_data) {
		DBGPRINT(DEBUG_ERROR, "No avaliable memory for driver_wext_data\n");
		goto err1;
	}

	DBGPRINT(DEBUG_INFO, "Initialize wext interface\n");

	drv_wext_data->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (drv_wext_data->ioctl_sock < 0) {
		DBGPRINT(DEBUG_ERROR, "socket(PF_INET,SOCK_DGRAM) failed");
		goto err2;
	}

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

	drv_wext_data->priv = (void *)pfroam;

	DBGPRINT(DEBUG_INFO, "<--\n");

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	DBGPRINT(DEBUG_ERROR, "<-- error\n");

	return NULL;
}

static int driver_froam_wext_exit(pfroam_ctx pfroam)
{
	struct driver_wext_data *drv_wext_data = pfroam->drv_data;

	DBGPRINT(DEBUG_OFF, "-->\n");

	netlink_deinit(drv_wext_data->netlink);

	close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	DBGPRINT(DEBUG_OFF, "<--\n");
	return 0;
}

int driver_get_froam_supp(void *drv_data, UCHAR *ifname, BOOLEAN *pfroam_supp, u8 *pIntf_channel){
	struct _cmd_froam_supp cmd;
	int buf_len = 0;
	DBGPRINT(DEBUG_INFO, "--> \n");

	if(!pfroam_supp){
        DBGPRINT(DEBUG_ERROR, ": Invalid params\n");
		return -1;
	}

	memset(&cmd,0x0,sizeof(struct _cmd_threshold));
	cmd.hdr.command_id = OID_FROAM_CMD_FROAM_ENABLED;
	cmd.hdr.command_len = sizeof(threshold_info);

	buf_len = sizeof(cmd_froam_supp);

	if(driver_wext_get_oid(drv_data,(char *)ifname,OID_FROAM_COMMAND, (char *)&cmd, (size_t *)&buf_len) < 0){
        DBGPRINT(DEBUG_ERROR, ": oid=0x%x failed: %d\n",
               OID_FROAM_CMD_GET_THRESHOLD, errno);
        return -1;
	}

	if(buf_len < sizeof(cmd_froam_supp)){
        DBGPRINT(DEBUG_ERROR, ": Unexpected size =0x%x\n",buf_len);
		return -1;
	}

	*pfroam_supp = cmd.froam_supp;
	*pIntf_channel = cmd.channel;

	DBGPRINT(DEBUG_TRACE, "Froam Support:%d\n", cmd.froam_supp);

	DBGPRINT(DEBUG_INFO, "<--\n");
	return 0;

}

int driver_add_acl_entry_req(void * drv_data, UCHAR *ifname, UCHAR *peer_addr)
{
	int ret = 0;
	DBGPRINT(DEBUG_INFO, "-->\n");

	if(peer_addr){
		DBGPRINT(DEBUG_TRACE, "IF:%s ADDR: %02x-%02x-%02x-%02x-%02x-%02x\n", ifname,
		*peer_addr,*(peer_addr+1),*(peer_addr+2),*(peer_addr+3),*(peer_addr+4),*(peer_addr+5));
	}
	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_802_11_ACL_ADD_ENTRY, (char *)peer_addr, MAC_ADDR_LEN);

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_del_acl_entry_req(void *drv_data, UCHAR *ifname, UCHAR *peer_addr)
{
	int ret = 0;
	DBGPRINT(DEBUG_INFO, "-->\n");

	if(peer_addr){
		DBGPRINT(DEBUG_TRACE, "IF:%s ADDR: %02x-%02x-%02x-%02x-%02x-%02x\n",ifname,
		*peer_addr,*(peer_addr+1),*(peer_addr+2),*(peer_addr+3),*(peer_addr+4),*(peer_addr+5));
	}
	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_802_11_ACL_DEL_ENTRY, (char *)peer_addr, MAC_ADDR_LEN);

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_set_acl_policy(void *drv_data, UCHAR *ifname, u8 policy)
{
	int ret = 0;
	DBGPRINT(DEBUG_TRACE, "--> Policy: 0x%x\n", policy);

	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_802_11_ACL_SET_POLICY, (char *)&policy, sizeof(u8));

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_add_mntr_entry_req(void * drv_data, UCHAR *ifname, UCHAR *peer_addr, u8 index, u8 channel)
{
	mntr_entry_info entry;
	int ret = 0;
	DBGPRINT(DEBUG_INFO, "-->\n");

	memset(&entry,0x0,sizeof(mntr_entry_info));
	memcpy(entry.mac, peer_addr, MAC_ADDR_LEN);
	entry.index = index;
	entry.channel = channel;

	if(peer_addr){
		DBGPRINT(DEBUG_TRACE, "IF:%s Idx:0x%d, Chan:%d, ADDR: %02x-%02x-%02x-%02x-%02x-%02x\n",
		ifname, index, channel,
		*peer_addr,*(peer_addr+1),*(peer_addr+2),*(peer_addr+3),*(peer_addr+4),*(peer_addr+5));
	}
	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_AIR_MNTR_ADD_ENTRY, (char *)&entry, sizeof(mntr_entry_info));

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_del_mntr_entry_req(void *drv_data, UCHAR *ifname, UCHAR *peer_addr, u8 index, u8 channel)
{
	mntr_entry_info entry;
	int ret = 0;
	DBGPRINT(DEBUG_INFO, "-->\n");
	
	memset(&entry,0x0,sizeof(mntr_entry_info));
	memcpy(entry.mac, peer_addr, MAC_ADDR_LEN);
	entry.index = index;
	entry.channel = channel;

	if(peer_addr){
		DBGPRINT(DEBUG_TRACE, "IF:%s Idx:0x%d, Chan:%d, ADDR: %02x-%02x-%02x-%02x-%02x-%02x\n",
		ifname, index, channel,
		*peer_addr,*(peer_addr+1),*(peer_addr+2),*(peer_addr+3),*(peer_addr+4),*(peer_addr+5));
	}
	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_AIR_MNTR_DEL_ENTRY, (char *)&entry, sizeof(mntr_entry_info));

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_set_mntr_rule(void *drv_data, UCHAR *ifname, u8 rule)
{
	int ret = 0;
	DBGPRINT(DEBUG_TRACE, "--> Rule : 0x%x\n", rule);
	
	ret = driver_wext_set_oid(drv_data,(char *)ifname, OID_AIR_MNTR_SET_RULE, (char *)&rule, sizeof(u8));

	DBGPRINT(DEBUG_INFO, "<--\n");
	return ret;
}

int driver_get_thresholds(void *drv_data, UCHAR *ifname, struct _threshold_info *pthr_info){
	struct _cmd_threshold cmd;
	int buf_len = 0;
	DBGPRINT(DEBUG_INFO, "-->\n");

	if(!pthr_info){
        DBGPRINT(DEBUG_ERROR, ": Invalid params\n");
		return -1;
	}

	memset(&cmd,0x0,sizeof(struct _cmd_threshold));
	cmd.hdr.command_id = OID_FROAM_CMD_GET_THRESHOLD;
	cmd.hdr.command_len = sizeof(threshold_info);

	buf_len = sizeof(cmd_threshold);

	if(driver_wext_get_oid(drv_data,(char *)ifname,OID_FROAM_COMMAND, (char *)&cmd, (size_t *)&buf_len) < 0){
        DBGPRINT(DEBUG_ERROR, ": oid=0x%x failed: %d\n",
               OID_FROAM_CMD_GET_THRESHOLD, errno);
        return -1;
	}

	if(buf_len < sizeof(cmd_threshold)){
        DBGPRINT(DEBUG_ERROR, ": Unexpected size =0x%x\n",buf_len);
		return -1;
	}

	pthr_info->sta_ageout_time = cmd.info.sta_ageout_time;
	pthr_info->mntr_ageout_time = cmd.info.mntr_ageout_time;
	pthr_info->mntr_min_pkt_count = cmd.info.mntr_min_pkt_count;
	pthr_info->mntr_min_time = cmd.info.mntr_min_time;
	pthr_info->mntr_avg_rssi_pkt_count = cmd.info.mntr_avg_rssi_pkt_count;
	pthr_info->sta_detect_rssi_threshold = cmd.info.sta_detect_rssi_threshold;
	pthr_info->acl_ageout_time = cmd.info.acl_ageout_time;
	pthr_info->acl_hold_time = cmd.info.acl_hold_time;

	DBGPRINT(DEBUG_TRACE, "StaAgeTime:%d, StaDetectRSSI:%d, \n",
		pthr_info->sta_ageout_time, pthr_info->sta_detect_rssi_threshold);
	DBGPRINT(DEBUG_TRACE, "MntrAgeTime:%d, MntrMinPktCount:%d, MntrMinTime:%d, AvgRSSIPktCount:%d, \n",
		pthr_info->mntr_ageout_time, pthr_info->mntr_min_pkt_count, pthr_info->mntr_min_time,
		pthr_info->mntr_avg_rssi_pkt_count);
	DBGPRINT(DEBUG_TRACE, "ACLAgeTime:%d, ACLHoldTime:%d\n",
		pthr_info->acl_ageout_time, pthr_info->acl_hold_time);

	DBGPRINT(DEBUG_INFO, "<--\n");
	return 0;

}

const struct froam_drv_ops froam_drv_wext_ops = {
	.drv_inf_init = driver_froam_wext_init,
	.drv_inf_exit = driver_froam_wext_exit,
 	.drv_get_froam_supp = driver_get_froam_supp,
	.drv_add_acl_entry_req = driver_add_acl_entry_req,
	.drv_del_acl_entry_req = driver_del_acl_entry_req,
	.drv_set_acl_policy = driver_set_acl_policy,
	.drv_add_mntr_entry_req = driver_add_mntr_entry_req,
	.drv_del_mntr_entry_req = driver_del_mntr_entry_req,
	.drv_set_mntr_rule = driver_set_mntr_rule,
	.drv_get_thresholds = driver_get_thresholds
};

