include $(TOPDIR)/rules.mk

XPATH=.. ../utils

default: all watchtest pipetest clonetest modemtest logtest logbuftest \
		looptest timetest

all: streams.a

streams.a: wvstream.o wvsplitstream.o wvstreamclone.o wvlog.o wvlogbuffer.o \
	wvwatcher.o wvpipe.o wvloopback.o wvmodem.o wvtimestream.o

watchtest pipetest clonetest modemtest logtest logbuftest \
	looptest timetest: streams.a ../utils/utils.a

clean:
	rm -f watchtest pipetest clonetest modemtest logtest logbuftest \
		looptest timetest
