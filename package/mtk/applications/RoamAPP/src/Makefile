#NVDIR = $(ROOTDIR)/lib/libnvram
#CFLAGS += -I$(NVDIR) -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
rDFLAGS += -lpthread
#LDFLAGS += -lnvram
ROAM_EXEC = RoamApp
#LIBNVRAM = $(ROOTDIR)/lib/lib/libnvram-0.9.28.so

all: clean $(ROAM_EXEC)

RoamApp: bndstrg.o driver_wext.o eloop.o os_internal.o main.o util.o debug.o netlink.o wnm.o rrm.o internetchk.o froam.o
	$(CC) $(CFLAGS) $(rDFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o $(ROAM_EXEC)

romfs:
	for i in $(ROAM_EXEC) ; do $(ROMFSINST) /bin/$$i ; done
