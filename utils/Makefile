TOPDIR=../..
include $(TOPDIR)/rules.mk

default: all teststr testlist testhash stresshash testbuffer \
		testbase64 testver testtask testsort testproc

all: utils.a

utils.a: base64.o verstring.o strutils.o \
		wvstring.o wvstringlist.o wvstringtable.o \
		wvlinklist.o wvhashtable.o wvbuffer.o wvtask.o wvsubproc.o


LIBS=${EFENCE}
teststr testlist testhash stresshash testbuffer testbase64 \
	testver testtask testsort testproc: utils.a

clean:
	rm -f teststr testlist testhash stresshash testbuffer \
		testbase64 testver testtask testsort testproc
