TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=../utils ../streams

default: all tests

all: configfile.libs

configfile.libs: wvconf.o cfgsection.o cfgentry.o ../streams/streams.libs

configfile.a: configfile.libs

#LIBS = ${EFENCE}

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: configfile.libs
	$(MAKE) -C tests

