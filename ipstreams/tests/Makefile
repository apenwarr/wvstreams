TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../.. ../../utils ../../streams

TESTS=tcptest udptest udp2test listentest unixtest ulistentest \
	resolvertest routetest \
	conttest aliastest simpleconn httptest urltest wvaddrcomp

default: all

all: $(TESTS)

ipstreams.a: ../ipstreams.libs

#LIBS=$(EFENCE)
$(TESTS): ipstreams.a


clean:
	rm -f $(TESTS)

