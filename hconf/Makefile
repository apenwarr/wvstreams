TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=../utils ../streams ../configfile

default: all hconftest

all:

hconftest: hconftest.o hconf.a

hconf.libs: wvhconfkey.o wvhconf.o wvhconfgen.o \
	../streams/streams.libs ../utils/utils.libs

hconf.a: hconf.libs

LIBS = ${EFENCE}

clean:
	rm -f hconftest
