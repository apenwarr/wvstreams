TOPDIR=../..
include $(TOPDIR)/rules.mk

XPATH=../utils ../streams

default: all initest delsection
all: configfile.a

configfile.a: wvconf.o cfgsection.o cfgentry.o

#LIBS = ${EFENCE}
initest: initest.o configfile.a ../streams/streams.a ../utils/utils.a

delsection: delsection.o configfile.a ../streams/streams.a ../utils/utils.a

clean:
	rm -f initest delsection
