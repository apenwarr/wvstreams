TOPDIR=../..
include $(TOPDIR)/rules.mk

default: all teststr testlist testhash stresshash testbuffer \
		testbase64 testver
all: utils.a

utils.a: base64.o verstring.o strutils.o wvstring.o \
		wvlinklist.o wvhashtable.o wvbuffer.o


LIBS=${EFENCE}
teststr testlist testhash stresshash testbuffer testbase64 testver: utils.a

clean:
	rm -f teststr testlist testhash stresshash testbuffer \
		testbase64 testver
