
.PHONY: default all

default:

all: default

test: wvtestmain
	./wvtestmain

include vars.mk

wvtestmain: wvtestmain.o $(call objects, $(shell find -type d -name t)) \
	libwvstreams.so libwvutils.so

include rules.mk

