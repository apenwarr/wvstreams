TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

XPATH=.. ../utils ../streams ../ipstreams ../crypto

default: all

all: urlget.libs

urlget.libs: wvhttppool.o wvhttpstream.o wvftpstream.o ftpparse.o \
	../crypto/crypto.libs

LIBS=-lssl

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: urlget.libs
	$(MAKE) -C tests

