TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

# some crazy magic for WvLockFile to use the liblockdev library if it's
# available, or our simpler method otherwise.
#
#LOCKDEV = $(shell [ -f /usr/include/lockdev.h -a -f /usr/lib/liblockdev.a ] \
#		&& echo /usr/lib/liblockdev.a)
CFLAGS += $(shell [ -n "$(LOCKDEV)" ] && echo -DUSE_LOCKDEV=1 )

XPATH=.. ../utils ../zlib

#LIBS=$(EFENCE)

default: all

all: report-lock-mode streams.libs

# more crazy lockfile magic
#
.PHONY: report-lock-mode
report-lock-mode:
	@if [ -n "$(LOCKDEV)" ]; then \
		echo Using liblockdev.a for device locking in wvlockfile.; \
	else \
		echo Using my own device locking functions in wvlockfile.; \
	fi

streams.libs: wvstream.o wvstreamlist.o wvsplitstream.o wvstreamclone.o \
	wvfile.o wvlog.o wvlogbuffer.o wvsyslog.o wvwatcher.o wvpipe.o \
	wvloopback.o wvmodem.o wvtimestream.o wvprotostream.o wvbufstream.o \
	wvlockfile.o wvdailyevent.o wvtimeoutstream.o wvlogfile.o wvtundev.o \
	$(LOCKDEV) ../utils/utils.libs

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: streams.libs
	$(MAKE) -C tests