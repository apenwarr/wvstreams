TOPDIR=.

DEBUG=0
PREFIX=/usr/local
LIBDIR=${PREFIX}/lib
INCDIR=${PREFIX}/include/wvstreams
MANDIR=${PREFIX}/man

export TOPDIR DEBUG PREFIX LIBDIR INCDIR MANDIR

CC=${CXX}
#CXXOPTS += -fno-implement-inlines

export CC CXX CXXOPTS

-include rules.mk

SUBDIRS=utils streams configfile ipstreams crypto

all: $(SUBDIRS) libwvutils.so libwvstreams.so libwvcrypto.so

libwvutils.so: utils/utils.libs

libwvstreams.so-LIBS=libwvutils.so
libwvstreams.so: ipstreams/ipstreams.libs

libwvcrypto.so-LIBS=libwvstreams.so
libwvcrypto.so: crypto/crypto.libs

rules.mk:
	[ -e ../../rules.mk ] && ln -s ../../rules.mk .

genkdoc:
	kdoc -f html -d doc --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install:
	$(subdirs)

uninstall:
	$(subdirs)

clean:
	$(subdirs)
