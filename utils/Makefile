TOPDIR=../..
include $(TOPDIR)/rules.mk

TESTS=teststr testlist testhash stresshash testbuffer testbase64 testver \
		testtask testsort testproc testfork

default: all $(TESTS)

all: utils.a

utils.a: base64.o verstring.o strutils.o \
		wvstring.o wvstringlist.o wvstringtable.o \
		wvlinklist.o wvhashtable.o wvbuffer.o wvtask.o wvfork.o wvsubproc.o


LIBS=${EFENCE}
$(TESTS): utils.a

clean:
	rm -f $(TESTS)
