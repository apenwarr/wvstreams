TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=.. ../utils ../streams

TESTS=tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest urltest

default: all $(TESTS)

all: ipstreams.libs

ipstreams.libs: wvaddr.o wvtcp.o wvudp.o wvresolver.o \
		wvurl.o wvhttp.o wvinterface.o \
		wviproute.o wvipaliaser.o wvipfirewall.o wvunixsocket.o \
		../uniconf/uniconf.libs

ipstreams.a: ipstreams.libs

#LIBS=$(EFENCE)
$(TESTS): ipstreams.a


clean:
	rm -f $(TESTS)

