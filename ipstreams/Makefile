include $(TOPDIR)/rules.mk

XPATH=.. ../utils ../streams

default: all tcptest udptest listentest resolvertest routetest aliastest

all: ipstreams.a

ipstreams.a: wvaddr.o wvtcp.o wvudp.o wvresolver.o wvhttp.o wvinterface.o \
		wviproute.o wvipaliaser.o wvipfirewall.o

LIBS=../streams/streams.a ../utils/utils.a $(EFENCE)
tcptest udptest listentest resolvertest routetest aliastest: ipstreams.a

clean:
	rm -f tcptest udptest listentest resolvertest routetest aliastest
