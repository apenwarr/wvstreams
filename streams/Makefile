TOPDIR=../..
include $(TOPDIR)/rules.mk

# some crazy magic for WvLockFile to use the liblockdev library if it's
# available, or our simpler method otherwise.
#
LOCKDEV = $(shell [ -f /usr/include/lockdev.h -a -f /usr/lib/liblockdev.a ] \
		&& echo /usr/lib/liblockdev.a)
CFLAGS += $(shell [ -n "$(LOCKDEV)" ] && echo -DUSE_LOCKDEV=1 )

XPATH=.. ../utils

#LIBS=$(EFENCE)

default: all watchtest pipetest clonetest modemtest \
		logtest logbuftest syslogtest looptest timetest prototest

all: report-lock-mode streams.a

# more crazy lockfile magic
#
.PHONY: report-lock-mode
report-lock-mode:
	@if [ -n "$(LOCKDEV)" ]; then \
		echo Using liblockdev.a for device locking in wvlockfile.; \
	else \
		echo Using my own device locking functions in wvlockfile.; \
	fi

streams.a: wvstream.o wvstreamlist.o wvsplitstream.o wvstreamclone.o \
	wvfile.o wvlog.o wvlogbuffer.o wvsyslog.o wvwatcher.o wvpipe.o \
	wvloopback.o wvmodem.o wvtimestream.o wvprotostream.o \
	wvlockfile.o $(LOCKDEV)

watchtest pipetest clonetest modemtest logtest logbuftest syslogtest \
	looptest timetest prototest: streams.a ../utils/utils.a

clean:
	rm -f watchtest pipetest clonetest modemtest \
		logtest logbuftest syslogtest looptest timetest prototest
