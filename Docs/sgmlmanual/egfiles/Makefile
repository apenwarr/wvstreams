include $(TOPDIR)/rules.mk

W = ../../weaver/src
XFILES=	$W/fastforward/fastforward.a \
	$W/webconfig/webconfig.a \
	$W/netmap/netmap.a \
	$W/ipstreams/ipstreams.a \
	$W/lcdconfig/lcdconfig.a \
	$W/wvdial/wvdial.a \
	$W/configfile/configfile.a \
	$W/lcd/lcd.a \
	$W/streams/streams.a \
	$W/doublev/libdoublev.a \
	$W/utils/utils.a

XPATH=. $W $W/tunnelv $W/fastforward $W/webconfig $W/netmap $W/ipstreams \
	$W/lcdconfig $W/wvdial $W/configfile $W/lcd $W/streams $W/utils

PROGS = $(shell echo *.cc | sed 's/\.cc//g')

default: all

all: $(PROGS)

$(PROGS): $(XFILES)

clean:
	rm -f $(PROGS)

