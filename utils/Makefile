TOPDIR=../..
include $(TOPDIR)/rules.mk

TESTS=teststr testlist testhash stresshash testbuffer testbase64 testver \
		testtask testsort testproc testfork testencoder

default: all $(TESTS)

all: utils.a

testencoder-LIBS=-lz

utils.a: base64.o verstring.o strutils.o strcrypt.o \
		wvstring.o wvstringlist.o wvstringtable.o \
		wvlinklist.o wvhashtable.o wvbuffer.o wvtask.o \
		wvfork.o wvsubproc.o wvencoder.o wvgzip.o wvcrash.o


LIBS=${EFENCE}
$(TESTS): utils.a

clean:
	rm -f $(TESTS)
