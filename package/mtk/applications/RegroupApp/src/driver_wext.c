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
#include "os.h"
#include "driver_wext.h"
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include "priv_netlink.h"
#include "wireless_copy.h"
#include "netlink.h"
#include <sys/ioctl.h>
#include <error.h>
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "man.h"

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
		
#if 0
		if (iwe->u.data.flags != 1)
			DBGPRINT(DEBUG_TRACE, "cmd = 0x%x len = %d : pos = %p, end= %p\n", iwe->cmd, iwe->len, pos, end);
#endif        
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
		case IWEVEXPIRED:
			
			buf = os_malloc(iwe->u.data.length + 1);
						if (buf == NULL)
							return;
			os_memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';

			ez_hex_dump("received IWEVEXPIRED", buf,iwe->u.data.length);
           	os_free(buf);
            break;
        case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
               	return;
           	buf = os_malloc(iwe->u.data.length + 1);
            if (buf == NULL)
                return;
            os_memcpy(buf, custom, iwe->u.data.length);
            buf[iwe->u.data.length] = '\0';

            switch (iwe->u.data.flags) {
#if 0
			case WHC_DRVEVNT_STA_JOIN:
				hex_dump("MAN Event",buf,8);
				vr_add_station_event(buf);
				break;
#endif
#if 0
			case OID_VIRTUAL_ROAM_EVENT:
				hex_dump("VirtualRoam Event", buf, 8);
				vr_handle_driver_event(buf);
				break;				
#endif
			case OID_WH_EZ_MAN_DEAMON_EVENT:
				man_driver_event_handle(buf);
				break;
			case OID_WH_EZ_REGROUP_EVENT:
				regrp_event_handle(buf);
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
	int errno;	
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

 void driver_wext_init()
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

	drv_wext_data->priv = (void *)NULL;

	//return (void *)drv_wext_data;
	return;
err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return;
	//return NULL;
}

#if 0
static int driver_wext_exit(struct bndstrg *bndstrg)
{
	struct driver_wext_data *drv_wext_data = bndstrg->drv_data;

	DBGPRINT(DEBUG_TRACE, "%s\n", __FUNCTION__);

	netlink_deinit(drv_wext_data->netlink);

	close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}
#endif
