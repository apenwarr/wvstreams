TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=../utils ../streams ../configfile

TESTS=conftest itertest eventtest emutest

default: all $(TESTS)

all: uniconf.a

$(TESTS): uniconf.a

uniconf.libs: unikey.o uniconf.o unigen.o uniconfiter.o \
	unievents.o uniconfini.o wvconfemu.o \
	../configfile/configfile.libs

uniconf.a: uniconf.libs

LIBS = ${EFENCE}

clean:
	rm -f $(TESTS) *.new


