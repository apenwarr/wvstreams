TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=../utils ../streams

default: all hconftest

all:

hconftest: hconftest.o hconf.a

hconf.libs: ../streams/streams.libs ../utils/utils.libs

hconf.a: hconf.libs

#LIBS = ${EFENCE}

clean:
	rm -f hconftest
