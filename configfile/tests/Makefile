TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../../utils ../../streams

default: all

all: initest delsection nofile

configfile.a: ../configfile.libs

initest delsection nofile: configfile.a

clean:
	rm -f initest delsection nofile

