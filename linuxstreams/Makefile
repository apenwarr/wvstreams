TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=.. ../utils ../streams ../ipstreams

default: all

all: linuxstreams.libs

linuxstreams.libs: wviproute.o wvipaliaser.o wvipfirewall.o \
    wvinterface.o

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: linuxstreams.libs
	$(MAKE) -C tests

