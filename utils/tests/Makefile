TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

TESTS=teststr testlist testhash stresshash testbuffer testbase64 testver \
		testtask testsort testproc testfork testencoder testdiriter \
		testcallback testcrash testtcl dirnametest replacetest

default: all

all: $(TESTS)

testencoder-LIBS=-lz
testcrash-LIBS=-rdynamic

utils.a: ../utils.libs

LIBS=${EFENCE}
$(TESTS): utils.a

clean:
	rm -f $(TESTS)
