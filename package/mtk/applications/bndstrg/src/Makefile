LDFLAGS += -lpthread
BNDSTRG_EXEC = bndstrg

#PLATFORM = x86
PLATFORM = mipsel

ifeq ($(PLATFORM),x86)
CROSS_COMPILE=""
endif

ifeq ($(PLATFORM),mipsel)
CROSS_COMPILE = mipsel-linux-
endif

CFLAGS += -O2 -Wall -g -lrt -I$(NVDIR) -DNVRAM_LIB_SUPPORT
CC = $(CROSS_COMPILE)gcc

BNDSTRG_OBJS = driver_wext.o eloop.o os_internal.o main.o util.o debug.o netlink.o
#ctrl_iface_unix.o
BNDSTRG_OBJS += bndstrg.o

all: $(BNDSTRG_EXEC)

$(BNDSTRG_EXEC): $(BNDSTRG_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(LIBNVRAM) -o $(BNDSTRG_EXEC) $(BNDSTRG_OBJS)

bndstrg.o: bndstrg.c $(CONF_H) $(UCONF_H)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c bndstrg.c

romfs_dir = $(ROOTDIR)/romfs

romfs_dirs = etc/wifi



clean:
	rm -f *.o $(BNDSTRG_EXEC)
