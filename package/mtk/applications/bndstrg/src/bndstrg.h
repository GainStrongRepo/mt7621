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
    	bndstrg.h
*/

#ifndef __BNDSTRG_H__
#define __BNDSTRG_H__

#include <net/if.h>
#include <stdint.h>
#include "types.h"
#include "os.h"
#include "util.h"
#include "driver.h"
#include "event.h"
#include "debug.h"
#include "eloop.h"
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>


#define IFNAME_2G "ra0"
#define IFNAME_5G "rai0"

#define HASH_TABLE_SIZE                 256
#define MAC_ADDR_LEN				6
#define BAND_5G		1
#define BAND_2G		2
#define APMT2_PEER_PROBE_REQ		0
#define APMT2_PEER_AUTH_REQ		3

#define BND_STRG_MAX_TABLE_SIZE	256
#define BND_STRG_TIMER_PERIOD	1000
#define BND_STRG_AGE_TIME		150000
#define BND_STRG_HOLD_TIME		90000
#define BND_STRG_CHECK_TIME_5G	30000
#define BND_STRG_CHECK_TIME_2G	30000
#define BND_STRG_RSSI_DIFF		30
#define BND_STRG_RSSI_LOW		-70
#define BND_STRG_N_DIFF			3
#define BND_STRG_NSS_THR		3
#define NVRAM_TABLE_SIZE 		128

#ifndef TRUE
#define TRUE						1
#endif
#ifndef FALSE
#define FALSE					0
#endif

enum BND_STRG_PRIORITY_FLAGS {
	fBND_STRG_PRIORITY_RSSI_DIFF		,
	fBND_STRG_PRIORITY_BAND_PERSIST		,
	fBND_STRG_PRIORITY_HT_SUPPORT		,
	fBND_STRG_PRIORITY_5G_RSSI			,
	fBND_STRG_PRIORITY_VHT_SUPPORT		,
	fBND_STRG_PRIORITY_NSS_SUPPORT		,
	fBND_STRG_PRIORITY_LOAD_BALANCE 	,
	fBND_STRG_PRIORITY_DEFAULT_2G		,
	fBND_STRG_PRIORITY_DEFAULT_5G		,
	fBND_STRG_PRIORITY_5G_RSSI_DYNAMIC	,
	fBND_STRG_PRIORITY_MAX				, //always to be the last
};

enum bndstrg_return_code {
	BND_STRG_SUCCESS = 0,
	BND_STRG_INVALID_ARG,
	BND_STRG_RESOURCE_ALLOC_FAIL,
	BND_STRG_TABLE_FULL,
	BND_STRG_TABLE_IS_NULL,
	BND_STRG_NOT_INITIALIZED,
	BND_STRG_2G_INF_NOT_READY,
	BND_STRG_5G_INF_NOT_READY,
	BND_STRG_STA_IS_CONNECTED,
	BND_STRG_UNEXP,
};

enum bndstrg_control_flags {
	fBND_STRG_CLIENT_SUPPORT_2G			= (1 << 0),
	fBND_STRG_CLIENT_SUPPORT_5G			= (1 << 1),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G	= (1 << 2),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_5G	= (1 << 3),
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_2G		= (1 << 4),	
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_5G		= (1 << 5),
	fBND_STRG_CLIENT_LOW_RSSI_2G		= (1 << 6),
	fBND_STRG_CLIENT_LOW_RSSI_5G		= (1 << 7),
	fBND_STRG_CLIENT_IS_2G_ONLY			= (1 << 8),
	fBND_STRG_CLIENT_IS_5G_ONLY			= (1 << 9),
	fBND_STRG_CLIENT_SUPPORT_VHT		= (1 << 10),
};

enum bndstrg_frame_check_flags {
	fBND_STRG_FRM_CHK_PRB_REQ			= (1 << 0),
	fBND_STRG_FRM_CHK_ATH_REQ			= (1 << 1),
	fBND_STRG_FRM_CHK_ASS_REQ			= (1 << 2),
};

enum bndstrg_condition_check_flags {
	fBND_STRG_CND_RSSI_DIFF			= (1 << 0),
	fBND_STRG_CND_BAND_PERSIST			= (1 << 1),
	fBND_STRG_CND_HT_SUPPORT		= (1 << 2),
	fBND_STRG_CND_5G_RSSI			= (1 << 3),
	fBND_STRG_CND_VHT_SUPPORT			= (1 << 4),
	fBND_STRG_CND_NSS_SUPPORT			= (1 << 5),
	fBND_STRG_CND_LOAD_BALANCE 			= (1 << 6),
	fBND_STRG_CND_DEFAULT_2G			= (1 << 7),
	fBND_STRG_CND_DEFAULT_5G			= (1 << 8),
	fBND_STRG_CND_5G_RSSI_DYNAMIC		= (1 << 9),
};

struct bndstrg_entry_stat {
	char 	Rssi;
	char	FirstAuthTime[16];
	u8 		AuthReqCount;
	char	FirstProbeTime[16];
	u8 		ProbeReqCount;
	char	FirstAssocTime[16];
	u8 		AssocCount;
	char	FirstClientDelTime[16];
	u8 		MacDelCount;
};

enum PhyMode {
	fPhyMode_Legacy,
	fPhyMode_HT,
	fPhyMode_VHT,
};

enum STATUS {
	CLI_INIT = 0,
	CLI_ACTIVE = 1,
	CLI_NOTACTIVE = 2,
};

struct bndstrg_nvram_client{
	u8 Addr[MAC_ADDR_LEN];
	u8 Manipulable;
	u8 PhyMode;
	u8 Band;
	u8 Nss;
};

struct bndstrg_cli_entry {
	u8 				bValid;
	u32				TableIndex;
	u8				bActiveStatus;
	u8				AgingConfirmed[2]; /* confirm this sta is not connected to our 2.4G or 5G interface before aging */
	u32 				Control_Flags;
	struct timespec 	tp;		/* timestamp when insert-entry */
	u32  			elapsed_time; /* ms */
	u8 				Addr[MAC_ADDR_LEN];
	struct bndstrg_entry_stat statistics[2]; /* 2.4G/5G */
	struct bndstrg_cli_entry *pNext;
	u8              Nss;
	u8 				Manipulable;
	u8 				band;
};

/* for setting different band steering algorithms */
struct bndstrg_alg_control {
	u32 		FrameCheck;
	u32		ConditionCheck;
};

enum ACTION_CODE{
	CONNECTION_REQ = 1,
	CLI_ADD,
	CLI_UPDATE,
	CLI_DEL,
	CLI_AGING_REQ,
	CLI_AGING_RSP,
	INF_STATUS_QUERY,
	INF_STATUS_RSP_2G,
	INF_STATUS_RSP_5G,
	TABLE_INFO,
	ENTRY_LIST,
	BNDSTRG_ONOFF,
	SET_RSSI_DIFF,
	SET_RSSI_LOW,
	SET_AGE_TIME,
	SET_HOLD_TIME,
	SET_CHECK_TIME,
	SET_MNT_ADDR,
	SET_CHEK_CONDITIONS,
	INF_STATUS_RSP_DBDC,
	SET_CND_PRIORITY,
	NVRAM_UPDATE
};

struct bndstrg_cli_table {
	u8		 bInitialized;
	u8		 bEnabled;
	u8		 Band;
	u8		 b2GInfReady;
	u8		 b5GInfReady;
	u8		 b2GInfDiable_by_drv;
	u8		 b5GInfDiable_by_drv;
	char		 RssiDiff;			/* if Rssi2.4G > Rssi5G by RssiCheck, then allow client to connect 2.4G */
	char		 RssiLow;			/* if Rssi5G < RssiLow, then this client cannot connect to 5G */
	u32		AgeTime;			/* Entry Age Time (ms) */
	u32		HoldTime;		/* Time for holding 2.4G connection rsp (ms) */
	u32		CheckTime_5G;	/* Time for deciding if a client is 2.4G only (ms) */
	u32		CheckTime_2G;	/* Time for deciding if a client is 5G only (ms) */
	u32 		Size;
	u8		uc2GIfName[32];
	u8		uc5GIfName[32];
	u8		status_queried;
	u8		status_queried_cnt;
	u8		table_enable_cnt;
	u8		dbdc_mode;
	struct bndstrg_alg_control	AlgCtrl;
	u32 sent_action_code_counter[NVRAM_UPDATE]; //NVRAM_UPDATE = 22 in ACTION_CODE
	u32 received_action_code_counter[NVRAM_UPDATE]; //NVRAM_UPDATE = 22 in ACTION_CODE
	struct bndstrg_cli_entry 	Entry[BND_STRG_MAX_TABLE_SIZE];
	struct bndstrg_cli_entry* 	Hash[HASH_TABLE_SIZE];
	u32 PriorityList[fBND_STRG_PRIORITY_MAX];
	u8  PriorityListSize;
	u8 	active_client_2G;	
	u8 	active_client_5G;
	u8	n_diff;
	struct bndstrg_nvram_client nvram_entry[NVRAM_TABLE_SIZE];
	u8 bndstrg_nvram_client_count;
#ifdef BND_STRG_DBG
	unsigned char MonitorAddr[MAC_ADDR_LEN];
#endif /* BND_STRG_DBG */
};

/* Use for I/O between driver and daemon */
/* Must make sure the structure is the same as the one in driver */
struct bndstrg_msg{
	u8	Action;
	u8	ReturnCode;
	u8	TableIndex;
	u8 	OnOff;
	u8	Band;
	u8	b2GInfReady;
	u8	uc2GIfName[32];
	u8	b5GInfReady;
	u8	uc5GIfName[32];
	s8 	Rssi[4];
	s8	RssiDiff;
	s8 	RssiLow;
	u8	FrameType;
	u32	Time;
	u32	ConditionCheck;
	unsigned char Addr[MAC_ADDR_LEN];
	u8	bAllowStaConnectInHt;

	//for entry updateuse
	u32 Control_Flags;
	u32 elapsed_time; /* ms */
	u8	bConnStatus;
	u8	bVHTCapable;
	u8 	Nss;
	u32 PriorityList[fBND_STRG_PRIORITY_MAX];
	u8 	PriorityListSize;
	struct bndstrg_nvram_client nvram_entry;
};

struct bndstrg {
	/* driver interface operation */
	const struct bndstrg_drv_ops *drv_ops;

	/* event operation */
	const struct bndstrg_event_ops *event_ops;

	/* driver interface data */
	void *drv_data;

	/* control interface */
	struct bndstrg_ctrl_iface *bndstrg_ctrl_iface;
	
	u8 version;

	struct bndstrg_cli_table table;
	
};

int bndstrg_init(struct bndstrg *bndstrg, struct bndstrg_event_ops *event_ops, int drv_mode, int opmode, int version);
int bndstrg_deinit(struct bndstrg *bndstrg);
void bndstrg_run(struct bndstrg *bndstrg);

#define IS_BND_STRG_DUAL_BAND_CLIENT(_Control_Flags) \
	((_Control_Flags & fBND_STRG_CLIENT_SUPPORT_2G) && (_Control_Flags & fBND_STRG_CLIENT_SUPPORT_5G))

#endif /* __BNDSTRG_H__ */

