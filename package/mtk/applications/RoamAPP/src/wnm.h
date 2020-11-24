
#ifndef WNM_H
#define WNM_H

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

enum WNM_COMMAND {
	OID_802_11_WNM_CMD_ENABLE 	= 0x01,
	OID_802_11_WNM_CMD_CAP 		= 0x02,
	OID_802_11_WNM_CMD_SEND_BTM_REQ = 0x03,
};

enum WNM_EVENT {
	OID_802_11_WNM_EVT_BTM_QUERY    = 0x01,
	OID_802_11_WNM_EVT_BTM_RSP    	= 0x02
};




struct GNU_PACKED btm_req_frame{
			u8 request_mode;
			u16 disassociation_timer;
			u8 validity_interval;
			/* 
 			 * Following are BSS Termination Duration, Session Information URL,
 			 * and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
	} ;


struct GNU_PACKED btm_req_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_req_len;
	u8 btm_req[0];
};

struct GNU_PACKED btm_query_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_query_len;
	u8 btm_query[0];
};

struct GNU_PACKED btm_rsp_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_rsp_len;
	u8 btm_rsp[0];
};

struct GNU_PACKED wnm_command {
	u8 command_id;
	u8 command_len;
	u8 command_body[0];
};

struct GNU_PACKED wnm_event {
	u8 event_id;
	u8 event_len;
	u8 event_body[0];
};


#define OID_802_11_WNM_COMMAND					0x094A
#define OID_802_11_WNM_EVENT					0x094B


struct wnm {
	/* driver interface operation */
	const struct wnm_drv_ops *drv_ops;

	/* event operation */
	const struct wnm_event_ops *event_ops;

	/* driver interface data */
	void *drv_data;

	/* control interface */
	
	u8 version;	
};

int wnm_init(struct wnm *wnm, 
				 struct wnm_event_ops *event_ops,
				 int drv_mode,
				 int opmode,
				 int version);
void wnm_run(struct wnm *wnm);


#endif

