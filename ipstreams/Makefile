TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=.. ../utils ../streams ../linuxstreams

default: all

all: ipstreams.libs

ipstreams.libs: wvaddr.o wvtcp.o wvudp.o wvresolver.o \
		wvurl.o wvhttp.o wvunixsocket.o \
		../uniconf/uniconf.libs \
                ../linuxstreams/linuxstreams.libs

ipstreams.a: ipstreams.libs

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: ipstreams.libs
	$(MAKE) -C tests

