TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=../utils ../streams ../configfile

default: all

all: uniconf.libs

uniconf.libs: unikey.o uniconf.o unigen.o uniconfiter.o \
	unievents.o uniconfini.o wvconfemu.o \
	../configfile/configfile.libs

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: uniconf.libs
	$(MAKE) -C tests

