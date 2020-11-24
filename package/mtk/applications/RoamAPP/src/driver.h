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
    	driver.h
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "bndstrg.h"
#include "wnm.h"
#include "froam.h"

struct bndstrg;
struct bndstrg_cli_entry;

struct bndstrg_drv_ops {
	int (*drv_test)(void *drv_data, const char *ifname);
	void * (*drv_inf_init)(struct bndstrg *bndstrg, const int opmode, const int drv_mode);
	int (*drv_inf_exit)(struct bndstrg *bndstrg);
	int (*drv_accessible_cli)(void *drv_data, const char *ifname, struct bndstrg_cli_entry *entry, u8 action);
	int (*drv_inf_status_query)(void *drv_data, const char *ifname);
	int (*drv_bndstrg_onoff)(void *drv_data, const char *ifname, u8 onoff);
};

struct wnm;

struct wnm_drv_ops {
	void * (*drv_inf_init)(struct wnm *wnm, const int opmode, const int drv_mode);
	int (*drv_inf_exit)(struct wnm *wnm);
	int (*drv_wnm_onoff)(void *drv_data, const char *ifname, u8 onoff);
	
	int (*drv_send_btm_req)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_req,
							size_t btm_req_len);
	int (*drv_send_btm_query)(void *drv_data, const char *ifname,
							  const char *peer_sta_addr, const char *btm_query,
							  size_t btm_query_len);
	int (*drv_send_btm_rsp)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_rsp,
							size_t btm_rsp_len);
	int (*drv_btm_onoff)(void *drv_data, const char *ifname,
                             int onoff);
};

struct _froam_ctxt;
struct _threshold_info;

struct froam_drv_ops {
	void * (*drv_inf_init)(struct _froam_ctxt *pfroam);

	int (*drv_inf_exit)(struct _froam_ctxt *pfroam);

	int (*drv_get_froam_supp)(void *drv_data, UCHAR *ifname, BOOLEAN *pfroam_supp, u8 *pIntf_channel);

	int (*drv_add_acl_entry_req)(void * drv_data, UCHAR *ifname, UCHAR *peer_addr);

	int (*drv_del_acl_entry_req)(void *drv_data, UCHAR *ifname, UCHAR *peer_addr);

	int (*drv_set_acl_policy)(void *drv_data, UCHAR *ifname, u8 policy);

	int (*drv_add_mntr_entry_req)(void * drv_data, UCHAR *ifname, UCHAR *peer_addr, u8 index, u8 channel);

	int (*drv_del_mntr_entry_req)(void *drv_data, UCHAR *ifname, UCHAR *peer_addr, u8 index, u8 channel);

	int (*drv_set_mntr_rule)(void *drv_data, UCHAR *ifname, u8 rule);

	int (*drv_get_thresholds)(void *drv_data, UCHAR *ifname, struct _threshold_info *pthr_info);
};

#endif /* __DRIVER_H__ */
