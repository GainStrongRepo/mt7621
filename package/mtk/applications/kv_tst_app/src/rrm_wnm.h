
#ifndef RRM_WNM_H
#define RRM_WNM_H

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
#include "kv_def.h"
//#include "oid.h"
#include "ctrl_iface_unix.h"

enum kv_cmd {
	invalid_cmd,
	wnm_enable,
	btm_enable,
	peer_btmcap,
	btm_req,
	nr_req,
	nr_rsp,
};

struct rrm_wnm {
	/* driver interface operation */
	const struct rrm_wnm_drv_ops *drv_ops;
	/* event operation */
	const struct rrm_wnm_event_ops *event_ops;
	/* driver interface data */
	void *drv_data;
	/* control interface */
	struct hotspot_ctrl_iface *hs_ctrl_iface;
	u8 version;	
	char iface[64];
};

typedef struct {
    u8 eid;
    u8 len;
    u8 octet[0];
} eid, *p_eid;


int rrm_wnm_init(struct rrm_wnm *wnm, 
				 struct rrm_wnm_event_ops *event_ops,
				 int drv_mode,
				 int opmode,
				 int version);
void rrm_wnm_run(struct rrm_wnm *wnm);


int driver_rrm_set_capabilities(
				void *drv_data,
				const char *ifname,
				char *capabilities);

int driver_rrm_enable_disable(
				void *drv_data,
				const char *ifname,
				u8 enable);

int hex2num(char c);

int hwaddr_aton(const char *txt, u8 *addr);



#endif

