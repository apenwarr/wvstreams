TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=../utils ../streams ../configfile

TESTS=hconftest itertest eventtest emutest

default: all $(TESTS)

all: hconf.a

$(TESTS): hconf.a

hconf.libs: wvhconfkey.o wvhconf.o wvhconfgen.o wvhconfiter.o \
	wvhconfevents.o wvhconfini.o \
	../configfile/configfile.libs

hconf.a: hconf.libs

LIBS = ${EFENCE}

clean:
	rm -f $(TESTS)

