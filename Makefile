TOPDIR=.

DEBUG=0
PREFIX=/usr/local
LIBDIR=${PREFIX}/lib
INCDIR=${PREFIX}/include/wvstreams
MANDIR=${PREFIX}/share/man

RELEASE=$(shell cat wvver.h \
	| awk '/\#define[ 	][ 	]*WVSTREAMS_VER[ 	]/ { print $$3 }' \
	| sed -e 's,0x\(....\)\(....\),\1.\2,' \
		-e 's/^0*//' \
		-e 's,\([^._]\)0*$$,\1,' )

export RELEASE

export TOPDIR DEBUG PREFIX LIBDIR INCDIR MANDIR

CC=${CXX}
CXXOPTS += -D_REENTRANT 
CXXOPTS-LIB=-fPIC

export CC CXX CXXOPTS

include $(TOPDIR)/wvrules.mk

SUBDIRS=src

all: $(SUBDIRS)

genkdoc:
	kdoc -f html -d doc --name wvstreams --strip-h-path src/*/*.h

doxygen:
	doxygen

install:
	$(subdirs)

installdev:
	$(subdirs)

installshared:
	$(subdirs)

uninstall:
	$(subdirs)

clean:
	$(subdirs)
