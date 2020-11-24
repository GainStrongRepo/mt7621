/**
*kv_def.h --- 80211 kv related macro and struct 
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifndef GNU_PACKED
#define GNU_PACKED	__attribute__ ((packed))
#endif


typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

#define MAX_CANDIDATE_NUM 5
#define OP_LEN 16
#define CH_LEN 30
#define REQ_LEN 30
#define MAC_ADDR_LEN 6
#define SSID_LEN 33
#define URL_LEN 40

#define RT_PRIV_IOCTL			0x8BE1
#define OID_GET_SET_TOGGLE		0x8000
#define	OID_GET_SET_FROM_UI		0x4000
#define OID_802_11_WNM_COMMAND  0x094A
#define OID_802_11_WNM_EVENT	0x094B
#define OID_802_11_RRM_COMMAND  0x094C
#define OID_802_11_RRM_EVENT	0x094D

#define RT_QUERY_WNM_CAPABILITY	\
	(OID_GET_SET_FROM_UI|OID_802_11_WNM_COMMAND)
#define RT_QUERY_RRM_CAPABILITY	\
			(OID_GET_SET_FROM_UI|OID_802_11_RRM_COMMAND)

enum rrm_cmd_subid {
	OID_802_11_RRM_CMD_ENABLE = 0x01,
	OID_802_11_RRM_CMD_CAP,
	OID_802_11_RRM_CMD_SEND_BEACON_REQ,
	OID_802_11_RRM_CMD_QUERY_CAP,
	OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM,
	OID_802_11_RRM_CMD_SEND_NEIGHBOR_REPORT,
	OID_802_11_RRM_CMD_SET_NEIGHBOR_REPORT_PARAM,
	OID_802_11_RRM_CMD_HANDLE_NEIGHBOR_REQUEST_BY_DAEMON,
};

enum rrm_event_subid {
    OID_802_11_RRM_EVT_BEACON_REPORT = 0x01,
    OID_802_11_RRM_EVT_NEIGHBOR_REQUEST,
};

enum wnm_cmd_subid {
	OID_802_11_WNM_CMD_ENABLE = 0x01,
	OID_802_11_WNM_CMD_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ,
	OID_802_11_WNM_CMD_QUERY_BTM_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ_IE,
	OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM,
};
  
enum wnm_event_subid {
	OID_802_11_WNM_EVT_BTM_QUERY = 0x01,
	OID_802_11_WNM_EVT_BTM_RSP,
};


typedef struct GNU_PACKED rrm_command_s {
	UINT8 command_id;
	UINT32 command_len;
	UINT8 command_body[0];
} rrm_command_t, *p_rrm_command_t;
  
typedef struct GNU_PACKED rrm_event_s {
	UINT8 event_id;
	UINT32 event_len;
	UINT8 event_body[0];
} rrm_event_t, *p_rrm_event_t;

struct GNU_PACKED nr_req_data {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT32 nr_req_len;
	UINT8 nr_req[0];
};

typedef struct GNU_PACKED nr_rsp_data_s {
	UINT32 ifindex;
	UINT8 dialog_token;
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT32 nr_rsp_len;
	UINT8 nr_rsp[0];
} nr_rsp_data_t, *p_nr_rsp_data_t;


struct GNU_PACKED wnm_command {
	UINT8 command_id;
	UINT32 command_len;
	UINT8 command_body[0];
};
  
  struct GNU_PACKED wnm_event {
	UINT8 event_id;
	UINT32 event_len;
	UINT8 event_body[0];
};

typedef struct GNU_PACKED bcn_req_data_s {
	UINT32 ifindex;
	UINT8 dialog_token;
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT32 bcn_req_len;
	UINT8 bcn_req[0];
} bcn_req_data_t, *p_bcn_req_data_t;
  
typedef struct GNU_PACKED bcn_rsp_data_s { 
	UINT8   dialog_token;
	UINT32  ifindex;
	UINT8   peer_address[6];
	UINT32  bcn_rsp_len;
	UINT8   bcn_rsp[0];
} bcn_rsp_data_t, *p_bcn_rsp_data_t;

struct GNU_PACKED btm_req_data {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT32 btm_req_len;
	UINT8 btm_req[0];
};

typedef struct GNU_PACKED btm_req_ie_data_s {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT8 dialog_token;
	UINT32 timeout;
	UINT32 btm_req_len;
	UINT8 btm_req[0];
}btm_req_ie_data_t, *p_btm_req_ie_data_t;


  
struct GNU_PACKED btm_query_data {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT32 btm_query_len;
	UINT8 btm_query[0];
};
  
struct GNU_PACKED btm_rsp_data {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT32 btm_rsp_len;
	UINT8 btm_rsp[0];
};

/** @peer_address: mandatory; sta to send beacon request frame;
      @num_rpt: optional; number of repetitions;
      @regclass: only mandatory when channel is set to 0; operating class;
      @channum: mandatory; channel number;
      @random_ivl: optional; randomization interval; unit ms;
      	the upper bound of the random delay to be used prior to make measurement;
      @duration: optional; measurement duration; unit ms;
      @bssid: optional;
      @mode: optional; measurement mode;
      As default value 0 is a valid value in spec, so here need remap the value and the meaning;
      1 for passive mode; 2 for active mode; 3 for beacon table;
      @req_ssid: optional; subelement SSID;
      @timeout: optional; unit s;
      @rep_conditon: optional; subelement Beacon Reporting Information;
      @ref_value: optional; subelement Beacon Reporting Information;
      condition for report to be issued;
      driver will send timeout event after timeout value if no beacon report received;
      @detail: optional; subelement Reporting Detail;
       As default value 0 is a valid value in spec, so here need remap the value and the meaning;
      1 for no fixed length fields or elements;
      2 for all fixed length fields and any requested elements in the request IE;
      3 for all fixed length fields and elements      
      @op_class_len:  mandatory only when channel is set to 255; 
      @op_class_list: subelement Ap Channel Report; 
      @ch_list_len: mandatory only when channel is set to 255;
      @ch_list: subelement Ap Channel Report; 
      if you want use all the channels in operating classes then use default value 
      otherwise specify all channels you want sta to do measurement
      @request_len: optional;
      @request: subelement Request; only vaild when you specify request IDs
*/
typedef struct GNU_PACKED bcn_req_info_s { 
  UINT8 peer_address[6]; 
  UINT16 num_rpt;
  UINT8 regclass;
  UINT8 channum;
  UINT16 random_ivl;
  UINT16 duration; 
  UINT8 bssid[MAC_ADDR_LEN];
  UINT8 mode;
  UINT8 req_ssid_len;
  UINT8 req_ssid[SSID_LEN];
  UINT32 timeout;
  UINT8 rep_conditon;
  UINT8 ref_value;
  UINT8 detail;
  UINT8 op_class_len;
  UINT8 op_class_list[OP_LEN];
  UINT8 ch_list_len;
  UINT8 ch_list[CH_LEN];
  UINT8 request_len;
  UINT8 request[REQ_LEN];
} bcn_req_info, *p_bcn_req_info; 
  

/**
 @channum: mandatory; channel number;
 @phytype: optional; PHY type;
 @regclass: optional; operating class;
 @capinfo: optional; Same as AP's Capabilities Information field in Beacon;
 @bssid: mandatory;
 @preference: not used in neighbor report; optional in btm request;
	 indicates the network preference for BSS transition to the BSS listed in this
	 BSS Transition Candidate List Entries; 0 is a valid value in spec, but here
	 need remap its meaning to not include preference IE in neighbor report
	 response frame;
 @is_ht:  optional; High Throughput;
 @is_vht: optional; Very High Throughput;
 @ap_reachability: optional; indicates whether the AP identified by this BSSID is
 	reachable by the STA that requested the neighbor report. For example,
 	the AP identified by this BSSID is reachable for the exchange of
 	preauthentication frames;
 @security: optional;  indicates whether the AP identified by this BSSID supports
 	the same security provisioning as used by the STA in its current association;
 @key_scope: optional; indicates whether the AP indicated by this BSSID has the
 	same authenticator as the AP sending the report;
 @Mobility: optional; indicate whether the AP represented by this BSSID is
 	including an MDE in its Beacon frames and that the contents of that MDE are
 	identical to the MDE advertised by the AP sending the report;
*/
struct GNU_PACKED nr_info {
UINT8 channum;
UINT8 phytype;
UINT8 regclass;
UINT16 capinfo;
UINT8 bssid[MAC_ADDR_LEN];
UINT8 preference;
UINT8 is_ht;
UINT8 is_vht;
UINT8 ap_reachability;
UINT8 security;
UINT8 key_scope;
UINT8 mobility;
};

/**
	@dialogtoken: mandatory; must the same with neighbor request from sta
	or 0 on behalf of automatic report
@nrresp_info_count: mandatory; the num of  neighbor elements;must bigger 
	than 0 and not exceeds 5;
@nrresp_info: info of neighbor elements; mandatory;
*/
typedef struct GNU_PACKED rrm_nrrsp_info_custom_s {
UINT8 peer_address[MAC_ADDR_LEN];
UINT8 dialogtoken;
UINT8 nrresp_info_count;
struct nr_info nrresp_info[0];
} rrm_nrrsp_info_custom_t, *p_rrm_nrrsp_info_custom_t;

/**
	@sta_mac: mandatory; mac of sta sending the frame;
@dialogtoken: optional; dialog token;
@reqmode: optional; request mode;
@disassoc_timer: optional; the time(TBTTs) after which the AP will issue 
	a Disassociation frame to this STA;
@valint: optional;  the number of beacon transmission times (TBTTs) until
	the BSS transition candidate list is no longer valid;
@timeout: optional; driver will send timeout event after timeout value 
	if no beacon report received; unit s;
@TSF: optional; BSS Termination TSF;
@duration: optional; number of minutes for which the BSS is not present;
@url_len: optional;
@url: optional; only vaild when you specify url;
@num_candidates: mandatory; num of candidates;
@candidates: mandatory; request mode; the num of candidate is no larger
	than 5;
*/
typedef struct GNU_PACKED btm_reqinfo_s {
UINT8 sta_mac [MAC_ADDR_LEN];
UINT8 dialogtoken;
UINT8 reqmode;
UINT16 disassoc_timer;
UINT8 valint;
UINT32 timeout;
UINT64 TSF;
UINT16 duration;
UINT8 url_len;
UINT8 url[URL_LEN];
UINT8 num_candidates;
struct nr_info candidates[0];
}btm_reqinfo_t, *p_btm_reqinfo_t;



#endif /* _INTERFACE_H_ */

