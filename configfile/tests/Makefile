TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../../utils ../../streams

default: all

all: initest delsection

configfile.a: ../configfile.libs

initest delsection: configfile.a

clean:
	rm -f initest delsection

.PHONY: tests
tests:
	$(MAKE) -C tests

