include $(TOPDIR)/rules.mk

default: all testlist testhash stresshash testbuffer testbase64 testver
all: utils.a

utils.a: base64.o verstring.o strutils.o wvstring.o \
		wvlinklist.o wvhashtable.o \
		wvlockfile.o wvbuffer.o

LIBS=${EFENCE}
testlist testhash stresshash testbuffer testbase64 testver: utils.a

clean:
	rm -f testlist testhash stresshash testbuffer testbase64 testver
