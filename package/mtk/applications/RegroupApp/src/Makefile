CFLAGS += -g -O2 -Wall -Wstrict-prototypes -pipe
LDFLAGS += -s
LDLIBS += -lpthread

EXEC = RegroupApp

all: clean $(EXEC)

RegroupApp: eloop.o os_internal.o main.o util.o debug.o netlink.o driver_wext.o vr.o vr_mlme.o vr_drv.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o $(EXEC)