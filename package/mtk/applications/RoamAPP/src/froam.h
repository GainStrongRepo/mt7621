#ifndef FROAM_H
#define FROAM_H

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

#define UP				1
#define DOWN			0
#define SUPP_BAND		2
#define INTF2			0
#define INTF5			1
#define MAX_AP_INTF_LEN	4

#define CARD1_AP1		"ra0"
#define CARD1_AP2		"ra1"
#define CARD1_AP2_MP	"rax0"
#define CARD2_AP		"rai0"

#define MAX_SUPP_STA		16
#define MAX_SUPP_MNTR_ENTRY	16
#define MAX_SUPP_ACL_ENTRY	16

#define FROAM_SUPP_DEF			FALSE
#define	STALIST_AGEOUT_TIME 	5	// sec
#define	MNTRLIST_AGEOUT_TIME 	4	// sec
#define	MNTR_MIN_PKT_COUNT 		5
#define	MNTR_MIN_TIME 			1	// sec
#define	AVG_RSSI_PKT_COUNT 		5
#define STA_DETECT_RSSI			55	// absolute
#define	ACLLIST_AGEOUT_TIME 	4	// sec
#define	ACLLIST_HOLD_TIME 		2	// sec

#define MAC_ADDR_LEN	6

typedef enum froam_status_code {	
	FROAM_CODE_SUCCESS = 0,
	FROAM_CODE_FAILURE = 1,
	FROAM_CODE_COMPLETED = 2,
	FROAM_CODE_REJECTED = 3,
	FROAM_CODE_ABORTED = 4,
	FROAM_CODE_INVALID = 5,
	FROAM_CODE_NOT_READY = 6,
	FROAM_CODE_TIMEOUT = 7,
	FROAM_CODE_TABLE_FULL = 8,
	FROAM_CODE_INVALID_ARG = 9,
	FROAM_CODE_RESOURCE_ALLOC_FAIL = 10,
	FROAM_CODE_EMPTY = 11,
	FROAM_CODE_NOT_INITIALIZED = 12
}froam_status;

/* ----------- Air Monitor Related---------------------------*/
#define LENGTH_802_3    14

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */

#ifndef ETH_P_AIR_MONITOR
#define ETH_P_AIR_MONITOR 0x51A0
#endif /* ETH_P_AIR_MONITOR */


#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

#define BTYPE_MGMT                  0
#define BTYPE_CNTL                  1
#define BTYPE_DATA                  2

/* value domain of 802.11 MGMT frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK       14

/* value domain of 802.11 CNTL frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_WRAPPER       	7
#define SUBTYPE_BLOCK_ACK_REQ       8
#define SUBTYPE_BLOCK_ACK           9
#define SUBTYPE_PS_POLL             10
#define SUBTYPE_RTS                 11
#define SUBTYPE_CTS                 12
#define SUBTYPE_ACK                 13
#define SUBTYPE_CFEND               14
#define SUBTYPE_CFEND_CFACK         15

/* value domain of 802.11 DATA frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_DATA                0
#define SUBTYPE_DATA_CFACK          1
#define SUBTYPE_DATA_CFPOLL         2
#define SUBTYPE_DATA_CFACK_CFPOLL   3
#define SUBTYPE_NULL_FUNC           4
#define SUBTYPE_CFACK               5
#define SUBTYPE_CFPOLL              6
#define SUBTYPE_CFACK_CFPOLL        7
#define SUBTYPE_QDATA               8
#define SUBTYPE_QDATA_CFACK         9
#define SUBTYPE_QDATA_CFPOLL        10
#define SUBTYPE_QDATA_CFACK_CFPOLL  11
#define SUBTYPE_QOS_NULL            12
#define SUBTYPE_QOS_CFACK           13
#define SUBTYPE_QOS_CFPOLL          14
#define SUBTYPE_QOS_CFACK_CFPOLL    15

/* ACK policy of QOS Control field bit 6:5 */
#define NORMAL_ACK                  0x00	/* b6:5 = 00 */
#define NO_ACK                      0x20	/* b6:5 = 01 */
#define NO_EXPLICIT_ACK             0x40	/* b6:5 = 10 */
#define BLOCK_ACK                   0x60	/* b6:5 = 11 */


/* 2-byte Frame control field */
typedef struct _FRAME_CONTROL {
#ifdef __BIG_ENDIAN__
	uint16_t Order:1;		/* Strict order expected */
	uint16_t Wep:1;		/* Wep data */
	uint16_t MoreData:1;	/* More data bit */
	uint16_t PwrMgmt:1;	/* Power management bit */
	uint16_t Retry:1;		/* Retry status bit */
	uint16_t MoreFrag:1;	/* More fragment bit */
	uint16_t FrDs:1;		/* From DS indication */
	uint16_t ToDs:1;		/* To DS indication */
	uint16_t SubType:4;	/* MSDU subtype */
	uint16_t Type:2;		/* MSDU type */
	uint16_t Ver:2;		/* Protocol version */
#else
    uint16_t Ver:2;		/* Protocol version */
	uint16_t Type:2;		/* MSDU type */
	uint16_t SubType:4;	/* MSDU subtype */
	uint16_t ToDs:1;		/* To DS indication */
	uint16_t FrDs:1;		/* From DS indication */
	uint16_t MoreFrag:1;	/* More fragment bit */
	uint16_t Retry:1;		/* Retry status bit */
	uint16_t PwrMgmt:1;	/* Power management bit */
	uint16_t MoreData:1;	/* More data bit */
	uint16_t Wep:1;		/* Wep data */
	uint16_t Order:1;		/* Strict order expected */
#endif	/* __BIG_ENDIAN__ */
} FRAME_CONTROL;

typedef struct _HEADER_802_11_4_ADDR {
    FRAME_CONTROL           FC;
    unsigned short          Duration;
    unsigned short			SN;
    unsigned char           FN;
    unsigned char           Addr1[MAC_ADDR_LEN];
    unsigned char           Addr2[MAC_ADDR_LEN];
	unsigned char			Addr3[MAC_ADDR_LEN];
    unsigned char	        Addr4[MAC_ADDR_LEN];
} HEADER_802_11_4_ADDR, *PHEADER_802_11_4_ADDR;

typedef struct _AIR_RADIO_INFO{
	char PHYMODE;
    char SS;
	char MCS;
	char BW;
	char ShortGI;
	unsigned long RATE;
    char RSSI[4];
	unsigned char Channel;	// keep without easy check?
} AIR_RADIO_INFO, *PAIR_RADIO_INFO;

typedef struct _AIR_RAW{
	 AIR_RADIO_INFO wlan_radio_tap;
	 HEADER_802_11_4_ADDR wlan_header;
} AIR_RAW, *PAIR_RAW;

#define BIT(n)                 ((UINT8) 1 << (n))
#define RULE_CTL				BIT(0)
#define RULE_MGT				BIT(1)
#define RULE_DATA				BIT(2)
#define RULE_A1					BIT(3)
#define RULE_A2					BIT(4)

#define FROAM_MNTR_RULE	(RULE_DATA | RULE_A2) // monitor all data frames from STA, optionally RULE_MGT

typedef struct GNU_PACKED _mntr_entry_info {
	u8 mac[MAC_ADDR_LEN];
	u8 channel;	// for driver to choose correct band 
	u8 index; // to modify corrsponding driver mntr table entry
}mntr_entry_info, *pmntr_entry_info;

#define OID_AIR_MNTR_ADD_ENTRY   		0x0980
#define OID_AIR_MNTR_DEL_ENTRY			0x0981
#define OID_AIR_MNTR_SET_RULE			0x0982

/* ----------- ACL Related---------------------------*/

enum _ACL_POLICY {
    ACL_POLICY_DISABLE = 0,	// No ACL checks
    ACL_POLICY_POSITIVE_LIST = 1,	// only ACL entries allowed to connect
    ACL_POLICY_NEGATIVE_LIST = 2	// ACL entries are banned from connection
};

#define OID_802_11_ACL_ADD_ENTRY		0x052B
#define OID_802_11_ACL_DEL_ENTRY		0x052C
#define OID_802_11_ACL_SET_POLICY		0x052D

/* ----------- Force Roam Oid & Events for driver---------------------------*/

#define OID_FROAM_COMMAND				0x0983
#define OID_FROAM_EVENT					0x0984

enum FROAM_COMMAND {
	OID_FROAM_CMD_FROAM_ENABLED = 0x1,
	OID_FROAM_CMD_GET_THRESHOLD = 0x2,
};

typedef struct GNU_PACKED _froam_command_hdr {
	u8 command_id;
	u8 command_len;
}froam_command_hdr,*pfroam_command_hdr;

typedef struct GNU_PACKED _cmd_froam_supp{
	struct _froam_command_hdr hdr;
	u8 froam_supp;
	u8 channel;
}cmd_froam_supp,*pcmd_froam_supp;

typedef struct GNU_PACKED _threshold_info{
	u8 sta_ageout_time;
	u8 mntr_ageout_time;
	u8 mntr_min_pkt_count;
	u8 mntr_min_time;
	u8 mntr_avg_rssi_pkt_count;
	char sta_detect_rssi_threshold;
	u8 acl_ageout_time;
	u8 acl_hold_time;
}threshold_info,*pthreshold_info;

typedef struct GNU_PACKED _cmd_threshold{
	struct _froam_command_hdr hdr;
	struct _threshold_info info;
}cmd_threshold,*pcmd_threshold;

enum FROAM_EVENT {	
	FROAM_EVT_STA_RSSI_LOW 	 = 0x01,
	FROAM_EVT_STA_RSSI_GOOD	 = 0x02,
	FROAM_EVT_STA_DISCONNECT = 0x03,
	FROAM_EVT_CLEAR_MNTR_LIST = 0x04,
	FROAM_EVT_CLEAR_ACL_LIST = 0x05,
	FROAM_EVT_STA_AGEOUT_TIME = 0x06,
	FROAM_EVT_MNTR_AGEOUT_TIME = 0x07,
	FROAM_EVT_ACL_AGEOUT_TIME = 0x08,
	FROAM_EVT_STA_DETECT_RSSI_THRESHOLD = 0x09,
	FROAM_EVT_MNTR_MIN_PKT_COUNT = 0x0A, // all new below
	FROAM_EVT_MNTR_MIN_TIME = 0x0B,
	FROAM_EVT_STA_AVG_RSSI_PKT_COUNT = 0x0D,
	FROAM_EVT_ACL_HOLD_TIME = 0x0C,
	FROAM_EVT_FROAM_SUPP = 0x0E,
};

typedef struct GNU_PACKED _froam_event {
	u8 event_id;
	u8 event_len;
	u8 event_body[0];
}froam_event,*pfroam_event;

typedef struct GNU_PACKED _froam_event_hdr {
	u8 event_id;
	u8 event_len;
}froam_event_hdr,*p_froam_event_hdr;

typedef struct GNU_PACKED _froam_event_sta_low_rssi {
	struct _froam_event_hdr hdr;
	u8 mac[MAC_ADDR_LEN];
	u8 channel;
}froam_event_sta_low_rssi, *pfroam_event_sta_low_rssi;

typedef struct GNU_PACKED _froam_event_sta_good_rssi {
	struct _froam_event_hdr hdr;
	u8 mac[MAC_ADDR_LEN];
}froam_event_sta_good_rssi, *pfroam_event_sta_good_rssi;

typedef struct GNU_PACKED _froam_event_sta_disconn {
	struct _froam_event_hdr hdr;
	u8 mac[MAC_ADDR_LEN];
}froam_event_sta_disconn, *pfroam_event_sta_disconn;

/* ----------- App to App Interaction---------------------------*/

#define FROAM_PORT 5006		// assuming available	todo: enahnce to find unused port

#define BUFLEN 512  //Max length of buffer

enum FROAM_MSG {
	FROAM_MSG_STA_MNTR	= 0x01,
	FROAM_MSG_STA_DETECT = 0x02,
};

typedef struct GNU_PACKED _froam_header{
	u8	Identifier[4];	/* should be the string ROAM*/
	u8  br0_mac[MAC_ADDR_LEN];
	u8	cmd;
	u8	payloadLen;
} froam_hdr,*p_froam_hdr;

#define FROAM_HDR_IDENTIFIER	"ROAM"
#define FROAM_HDR_LEN	(sizeof(struct _froam_header))

typedef struct GNU_PACKED _froam_msg_sta_mntr{
	struct _froam_header hdr;
	u8 mac[MAC_ADDR_LEN];
	u8 channel;
	u8 rsvd;
} froam_msg_sta_mntr,*pfroam_msg_sta_mntr;

typedef struct GNU_PACKED _froam_msg_sta_detect{
	struct _froam_header hdr;
	u8 mac[MAC_ADDR_LEN];
	u8 channel;
	u8 rsvd;
} froam_msg_sta_detect,*pfroam_msg_sta_detect;

/* ----------- App Context---------------------------*/

typedef struct GNU_PACKED _sta_entry{
	BOOLEAN valid;	// active
	BOOLEAN processed;	// peers informed to monitor
	u8	mac[MAC_ADDR_LEN];
	u8	channel;
	u8	timeticks;	// 1 sec ticks since processed
} sta_entry,*p_staentry;

enum MNTRY_ENTRY_STATE {
	NOT_PROCESSED	= 0,
	DRVR_ENTRY_ADDED = 1,
	DRVR_ENTRY_REMOVED = 2,
};

typedef struct GNU_PACKED _mnt_sta_entry{
	BOOLEAN valid;	// active
	u8 processed;	// mntr entry added
	u8	mac[MAC_ADDR_LEN];
	//u8	src_mac[MAC_ADDR_LEN];
	u8	channel;
	u8	timeticks;	// time since processed
	u8 pkt_count;
	char rssi[5];
	struct in_addr PeerAddr;
} mnt_sta_entry,*pmnt_staentry;

typedef struct GNU_PACKED _acl_sta_entry{
	BOOLEAN valid;		// active
	BOOLEAN processed;		// acl entry added
	u8	mac[MAC_ADDR_LEN];
	u8	channel;
	u8	timeticks;	// time since processed
} acl_sta_entry,*pacl_staentry;

typedef struct _froam_ctxt{
	/* driver interface operation */
	const struct froam_drv_ops *drv_ops;
	/* event operation */
	const struct _froam_event_ops *event_ops;
	/* driver interface data */
	void *drv_data;
	/* for rcving custom air mntr driver pkts*/
    int  air_mntr_sock;

	/* Sta List holds station entry for which low rssi detected
	  Peers would be informed to monitor these Sta*/
	pthread_mutex_t stalock;
	sta_entry stalist[MAX_SUPP_STA];
	u8 sta_count;

	/* Mntr list holds sta entry of sta to be monitored for good rssi */
	pthread_mutex_t mntrlock;
	mnt_sta_entry mntr_stalist[MAX_SUPP_STA];
	u8 mntr_sta_count;

	/* ACL list holds sta entries that are to be added to driver negative ACL list
	  This is to force roam them to other devices */
	pthread_mutex_t acllock;
	acl_sta_entry acl_stalist[MAX_SUPP_STA];
	u8 acl_sta_count;

	BOOLEAN en_force_roam_supp; // new

	u8 sta_ageout_time;
	u8 mntr_ageout_time;
	u8 mntr_min_pkt_count;
	u8 mntr_min_time;
	u8 mntr_avg_rssi_pkt_count;
	char sta_detect_rssi_threshold;
	u8 acl_ageout_time;
	u8 acl_hold_time;

	u8 terminated;
}froam_ctx,*pfroam_ctx;

int froam_init(struct _froam_ctxt *pfroam,
	struct _froam_event_ops *event_ops,
	int drv_mode);

void froam_run(struct _froam_ctxt *pfroam);

#endif
