TOPDIR=..
include $(TOPDIR)/wvrules.mk

TESTS=teststr testlist testhash stresshash testbuffer testbase64 testver \
		testtask testsort testproc testfork testencoder testdiriter \
		testcallback

MODULES=base64.o verstring.o strutils.o strcrypt.o wvtimeutils.o \
		wvstring.o wvstringlist.o wvstringtable.o \
		wvlinklist.o wvhashtable.o wvsorter.o wvbuffer.o wvtask.o \
		wvfork.o wvsubproc.o wvencoder.o wvgzip.o wvcrash.o \
		wvdiriter.o

default: all $(TESTS)

all: utils.libs

testencoder-LIBS=-lz

utils.libs: $(MODULES)

utils.a: utils.libs

LIBS=${EFENCE}
$(TESTS): utils.a

clean:
	rm -f $(TESTS)
