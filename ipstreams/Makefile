TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=.. ../utils ../streams

default: all

all: ipstreams.libs

ipstreams.libs: wvaddr.o wvtcp.o wvudp.o wvresolver.o \
		wvurl.o wvhttp.o wvinterface.o \
		wviproute.o wvipaliaser.o wvipfirewall.o wvunixsocket.o \
		../uniconf/uniconf.libs

ipstreams.a: ipstreams.libs

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: ipstreams.libs
	$(MAKE) -C tests

