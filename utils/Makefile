TOPDIR=../..
include $(TOPDIR)/rules.mk

default: all teststr testlist testhash stresshash testbuffer \
		testbase64 testver testtask testsort
all: utils.a

utils.a: base64.o verstring.o strutils.o wvstring.o wvstringtable.o \
		wvlinklist.o wvhashtable.o wvbuffer.o wvtask.o


LIBS=${EFENCE}
teststr testlist testhash stresshash testbuffer testbase64 \
	testver testtask testsort: utils.a

clean:
	rm -f teststr testlist testhash stresshash testbuffer \
		testbase64 testver testtask testsort
