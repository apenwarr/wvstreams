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

-include wvrules.mk

SUBDIRS=utils streams configfile ipstreams crypto

all: $(SUBDIRS) include libwvutils.so libwvstreams.so libwvcrypto.so \
	libwvutils.a libwvstreams.a libwvcrypto.a

include:
	rm -rf $@
	mkdir $@.new
	cd $@.new && for d in $(SUBDIRS); do ln -s ../$$d/*.h .; done
	mv $@.new $@

libwvutils.so: utils/utils.libs
libwvutils.a: utils/utils.libs

libwvstreams.so-LIBS=libwvutils.so
libwvstreams.so: ipstreams/ipstreams.libs
libwvstreams.a: ipstreams/ipstreams.libs

libwvcrypto.so-LIBS=libwvstreams.so
libwvcrypto.so: crypto/crypto.libs
libwvcrypto.a: crypto/crypto.libs

wvrules.mk:
	[ -e ../../wvrules.mk ] && ln -s ../../wvrules.mk .

genkdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install:
	$(subdirs)

uninstall:
	$(subdirs)

clean:
	rm -rf include Docs/doxy-html Docs/kdoc-html
	$(subdirs)
	[ -L wvrules.mk ] && rm -f wvrules.mk
