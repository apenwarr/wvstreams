TOPDIR=../..
include $(TOPDIR)/rules.mk

XPATH=../utils ../streams

default: all initest delsection

all: configfile.libs

configfile.libs: wvconf.o cfgsection.o cfgentry.o ../streams/streams.libs

configfile.a: configfile.libs

#LIBS = ${EFENCE}

initest delsection: configfile.a

clean:
	rm -f initest delsection
