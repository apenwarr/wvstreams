TOPDIR=.

-include $(TOPDIR)/wvrules.mk

SUBDIRS=utils streams configfile ipstreams crypto

INCFILES=$(wildcard include/*.h)
INCOUT=$(shell find . -type f -name '*.h' -printf "${INCDIR}/%f\n")
LIBFILES-DEV=libwvutils.a libwvstreams.a libwvcrypto.a
LIBFILES=libwvutils.so libwvstreams.so libwvcrypto.so

RELEASE=$(shell cat ../wvver.h \
	| awk '/\#define[ 	][ 	]*WVSTREAMS_VER[ 	]/ { print $$3 }' \
	| sed -e 's,0x\(....\)\(....\),\1.\2,' \
		-e 's/^0*//' \
		-e 's,\([^._]\)0*$$,\1,' )

export RELEASE

all: include $(SUBDIRS) libwvutils.a libwvutils.so \
	libwvstreams.a libwvstreams.so libwvcrypto.so libwvcrypto.a

include:
	rm -rf $@   
	mkdir $@.new
	cd $@.new && for d in $(SUBDIRS); do ln -s ../$$d/*.h .; done
	mv $@.new $@

libwvutils.so-LDFLAGS= -Wl,-soname,libwvutils.so.$(RELEASE)
libwvutils.so-LIBS=-lz -lcrypto
libwvutils.so: utils/utils.libs
libwvutils.a: utils/utils.libs

libwvstreams.so-LDFLAGS=-Wl,-soname,libwvstreams.so.$(RELEASE)
libwvstreams.so-LIBS=libwvutils.so
libwvstreams.so: ipstreams/ipstreams.libs
libwvstreams.a: ipstreams/ipstreams.libs

libwvcrypto.so-LDFLAGS=-Wl,-soname,libwvcrypto.so.$(RELEASE)
libwvcrypto.so-LIBS=libwvstreams.so -lssl -lcrypto
libwvcrypto.so: crypto/crypto.libs
libwvcrypto.a: crypto/crypto.libs

wvrules.mk:
	[ -e ../wvrules.mk ] && ln -s ../wvrules.mk .

install: installshared installdev

installshared: all
	[ -d ${LIBDIR}      ] || install -d ${LIBDIR}
	for d in ${LIBFILES}; do \
		strip --strip-unneeded $$d; \
		install -m 0644 $$d ${LIBDIR}/$$d.${RELEASE}; \
	done

installdev: all
	[ -d ${LIBDIR}      ] || install -d ${LIBDIR}
	[ -d ${INCDIR}      ] || install -d ${INCDIR}
	set -x; for d in ${INCFILES}; do \
		install -m 0644 $$d ${INCDIR}; \
	done
	for d in ${LIBFILES-DEV}; do \
		install -m 0644 $$d ${LIBDIR}; \
	done
	for d in ${LIBFILES}; do \
		( cd ${LIBDIR} ; ln -s $$d.${RELEASE} $$d ) ; \
	done

uninstall:
	cd ${LIBDIR}; rm -f ${LIBFILES}
	rm -f ${INCOUT}
	-rmdir ${INCDIR}

clean:
	$(subdirs)

