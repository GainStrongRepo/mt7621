OBJS_1 = eloop.o os_internal.o main.o util.o debug.o netlink.o man.o driver_wext.o regrp.o regrp_drv.o regrp_mlme.o 
OBJS_2 = get_network_weight.o
CFLAGS += -g -O2 -Wall -Wstrict-prototypes -pipe
LDFLAGS += -s

all: ManDaemon app_ez

ManDaemon: $(OBJS_1)
	$(CC) -o $@ $(OBJS_1) $(LDFLAGS)

app_ez: $(OBJS_2)
	$(CC) -o $@ $(OBJS_2) $(LDFLAGS) -lpthread

clean:
	rm -f core ManDaemon app_ez *.o
