TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=../utils ../streams ../configfile

default: all hconftest emutest

all: hconf.a

emutest hconftest: hconf.a

hconf.libs: wvhconfkey.o wvhconf.o wvhconfgen.o wvhconfini.o \
	../configfile/configfile.libs

hconf.a: hconf.libs

LIBS = ${EFENCE}

clean:
	rm -f hconftest emutest
