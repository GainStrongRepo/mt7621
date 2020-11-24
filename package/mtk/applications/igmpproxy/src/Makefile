EXE:=igmpproxy

SRC = igmpproxy.c config.c confread.c request.c udpsock.c mcgroup.c rttable.c \
		igmp.c ifvc.c callout.c kern.c syslog.c lib.c mroute-api.c

CONF_H = $(LINUX_DIR)/include/linux/autoconf.h
#KERNEL_H = $(LINUX_DIR)/include/

ifeq ($(CONFIG_RALINK_MT7620),y)
    SRC += ralink.c rtGSW.c rt3052_reg.c
    TARGET_CFLAGS += -DRT3052_SUPPORT -DCONFIG_RALINK_MT7620
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
    SRC += ralink.c rtGSW.c rt3052_reg.c
    TARGET_CFLAGS += -DRT3052_SUPPORT -DCONFIG_RALINK_MT7621
endif

ifeq ($(CONFIG_RALINK_MT7628),y)
    SRC += ralink.c rt3052.c rt3052_reg.c
    TARGET_CFLAGS += -DRT3052_SUPPORT -DCONFIG_RALINK_MT7628
endif


TARGET_CFLAGS += -I$(LINUX_DIR)/drivers/net/raeth
CFLAGS = $(TARGET_CFLAGS)

all: $(EXE) 

igmpproxy: build.h $(SRC) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

romfs:
	$(ROMFSINST) /bin/$(EXE)
	$(ROMFSINST) /bin/igmpproxy.sh

clean:
	rm -f *.o *.asm build.h $(EXEC)  

build.h	:  
	echo '#define BUILD "' `date +%y%m%d` '"' >build.h
