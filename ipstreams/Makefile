TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=.. ../utils ../streams

TESTS=tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest

default: all $(TESTS)

all: ipstreams.libs

ipstreams.libs: wvaddr.o wvtcp.o wvudp.o wvresolver.o wvhttp.o wvinterface.o \
		wviproute.o wvipaliaser.o wvipfirewall.o wvunixsocket.o \
		../configfile/configfile.libs

ipstreams.a: ipstreams.libs

#LIBS=$(EFENCE)
tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest simpleconn: \
		ipstreams.a

clean:
	rm -f $(TESTS)

