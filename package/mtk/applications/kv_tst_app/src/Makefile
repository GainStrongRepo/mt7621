CFLAGS += -I$(LINUX_DIR)/drivers/char
LDFLAGS += -lpthread
WNM_RRM_EXEC = kv
WNM_RRM_CTRL_EXEC = kvctrl
CONF_H	= $(LINUX_DIR)/include/linux/autoconf.h
#UCONF_H	= $(ROOTDIR)/config/autoconf.h
PLATFORM = mipsel

CFLAGS += -O2 -Wall -g -lrt

WNM_RRM_OBJS = driver_wext.o eloop.o os_internal.o main.o util.o debug.o netlink.o rrm_wnm.o ctrl_iface_unix.o
WNM_RRM_CTRL_OBJS = hotspot_ctrl/hotspot_cli.o hotspot_ctrl/hotspot_ctrl.o os_internal.o debug.o

all: $(WNM_RRM_EXEC) $(WNM_RRM_CTRL_EXEC)

$(WNM_RRM_EXEC): $(WNM_RRM_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(WNM_RRM_EXEC) $(WNM_RRM_OBJS)
$(WNM_RRM_CTRL_EXEC): $(WNM_RRM_CTRL_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(WNM_RRM_CTRL_EXEC) $(WNM_RRM_CTRL_OBJS)

rrm_wnm.o: rrm_wnm.c $(CONF_H) #$(UCONF_H) 
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c rrm_wnm.c

romfs_dir = $(ROOTDIR)/romfs

romfs_dirs = etc_ro/wifi
   
romfs:
	[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)
	for i in $(romfs_dirs); do \
		[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)/$$i; \
	done 
	$(ROMFSINST) /bin/$(WNM_RRM_EXEC)
	$(ROMFSINST) /bin/$(WNM_RRM_CTRL_EXEC)

clean:
	rm -f *.o hotspot_ctrl/*.o $(WNM_RRM_EXEC) $(WNM_RRM_CTRL_EXEC)
