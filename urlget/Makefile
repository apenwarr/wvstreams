TOPDIR=..
include $(TOPDIR)/wvrules.mk

XPATH=.. ../utils ../streams ../ipstreams ../crypto

TESTS=http2test

default: all $(TESTS)

all: urlget.libs

urlget.a: urlget.libs ../ipstreams/ipstreams.libs

urlget.libs: wvhttppool.o ../crypto/crypto.libs

LIBS=-lssl -lcrypto
#LIBS+=$(EFENCE)
$(TESTS): wvhttppool.o urlget.a

clean:
	rm -f $(TESTS)

