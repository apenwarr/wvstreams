TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

# some crazy magic for WvLockFile to use the liblockdev library if it's
# available, or our simpler method otherwise.
#
#LOCKDEV = $(shell [ -f /usr/include/lockdev.h -a -f /usr/lib/liblockdev.a ] \
#		&& echo /usr/lib/liblockdev.a)
CFLAGS += $(shell [ -n "$(LOCKDEV)" ] && echo -DUSE_LOCKDEV=1 )

XPATH=../.. ../../utils ../../zlib

#LIBS=$(EFENCE)

default: all

all: watchtest pipetest clonetest modemtest logtest logbuftest  \
         syslogtest looptest timetest prototest

streams.a: ../streams.libs $(LOCKDEV)

watchtest pipetest clonetest modemtest logtest logbuftest syslogtest \
	looptest timetest prototest: streams.a

clean:
	rm -f watchtest pipetest clonetest modemtest logtest logbuftest \
              syslogtest looptest timetest prototest
