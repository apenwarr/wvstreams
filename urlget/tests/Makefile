TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../.. ../../utils ../../streams ../../ipstreams ../../crypto

TESTS=http2test

default: all

all: $(TESTS)

urlget.a: ../urlget.libs

ipstreams.a: ../../ipstreams/ipstreams.libs

LIBS=-lssl
#LIBS+=$(EFENCE)
$(TESTS): urlget.a ipstreams.a

clean:
	rm -f $(TESTS)

