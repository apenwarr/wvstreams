include $(TOPDIR)/rules.mk

XPATH=.. ../utils

LIBS=$(EFENCE)

default: all watchtest pipetest clonetest modemtest \
		logtest logbuftest syslogtest looptest timetest prototest

all: streams.a

streams.a: wvstream.o wvstreamlist.o wvsplitstream.o wvstreamclone.o \
	wvlog.o wvlogbuffer.o wvsyslog.o wvwatcher.o wvpipe.o \
	wvloopback.o wvmodem.o wvtimestream.o wvprotostream.o

watchtest pipetest clonetest modemtest logtest logbuftest syslogtest \
	looptest timetest prototest: streams.a ../utils/utils.a

clean:
	rm -f watchtest pipetest clonetest modemtest \
		logtest logbuftest syslogtest looptest timetest prototest
