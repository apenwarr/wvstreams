TOPDIR=../..
include $(TOPDIR)/rules.mk

XPATH=.. ../utils ../streams

default: all tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest

all: ipstreams.a

ipstreams.a: wvaddr.o wvtcp.o wvudp.o wvresolver.o wvhttp.o wvinterface.o \
		wviproute.o wvipaliaser.o wvipfirewall.o wvunixsocket.o

#LIBS=$(EFENCE)
tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest: \
		ipstreams.a ../streams/streams.a ../utils/utils.a

simpleconn: ../configfile/configfile.a


clean:
	rm -f tcptest udptest udp2test listentest resolvertest routetest \
		conttest aliastest stresstest simpleconn httptest

