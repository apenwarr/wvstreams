TOPDIR=..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../include

MODULES=base64.o verstring.o strutils.o strcrypt.o wvtimeutils.o \
		wvstring.o wvstringlist.o wvstringtable.o \
		wvtclstring.o \
		wvlinklist.o wvhashtable.o wvsorter.o wvbuffer.o wvtask.o \
		wvfork.o wvsubproc.o wvencoder.o wvgzip.o wvcrash.o \
		wvdiriter.o

default: all

all: utils.libs

utils.libs: $(MODULES)

clean:
	$(MAKE) -C tests clean

.PHONY: tests
tests: utils.libs
	$(MAKE) -C tests

