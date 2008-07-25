WVSTREAMS=.
include wvrules.mk

libwvqt.so-LIBS: $(LIBS_QT)

qt/wvqtstreamclone.o: include/wvqtstreamclone.moc
qt/wvqthook.o: include/wvqthook.moc

ifneq ("$(with_qt)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
endif

qt/tests/qtstringtest: libwvqt.a
qt/tests/%: LDLIBS+=libwvqt.a
qt/tests/%: LDLIBS+=$(LIBS_QT)


libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))

ifeq ($(EXEEXT),.exe)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o libwvwin32.a
else
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)
endif

%: %.in
	@sed -e 's/#VERSION#/$(PACKAGE_VERSION)/g' < $< > $@


ifeq ("$(enable_testgui)", "no")
WVTESTRUN=env
endif

ifneq ("$(with_pam)", "no")
  libwvstreams.so: -lpam
endif

TARGETS += libwvbase.so libwvbase.a
TARGETS += libwvutils.so libwvutils.a
TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libuniconf.so libuniconf.a
TARGETS += wvtestmain.o libwvtest.a
TARGETS += uniconf/daemon/uniconfd uniconf/tests/uni
TARGETS += crypto/tests/ssltest ipstreams/tests/unixtest
TARGETS += crypto/tests/printcert
ifneq ("$(with_dbus)", "no")
TARGETS += dbus/tests/wvdbus dbus/tests/wvdbusd
endif
ifneq ("$(with_readline)", "no")
TARGETS += ipstreams/tests/wsd
endif
GARBAGE += wvtestmain.o tmp.ini .wvtest-total

ifneq ("$(with_qt)", "no")
  TARGETS += libwvqt.so libwvqt.a
endif

ifneq ("$(with_dbus)", "no")
  TARGETS += libwvdbus.so libwvdbus.a
endif

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGE += $(wildcard lib*.so.*)

DISTCLEAN += uniconf/daemon/uniconfd.8
DISTCLEAN += uniconf/tests/uni
DISTCLEAN += autom4te.cache config.mk config.log config.status \
		include/wvautoconf.h config.cache reconfigure \
		stamp-h.in configure include/wvautoconf.h.in

ifeq ("$(enable_testgui)", "no")
WVTESTRUN=env
endif

ifneq ("$(with_xplc)", "no")
LIBS+=$(LIBS_XPLC) -lm
endif

libwvutils.so-LIBS+=$(LIBS_PAM)

BASEOBJS= \
	utils/wvbuffer.o utils/wvbufferstore.o \
	utils/wvcont.o \
	utils/wverror.o \
	streams/wvfdstream.o \
	utils/wvfork.o \
	utils/wvhash.o \
	utils/wvhashtable.o \
	utils/wvlinklist.o \
	utils/wvmoniker.o \
	utils/wvregex.o \
	utils/wvscatterhash.o utils/wvsorter.o \
	utils/wvstring.o utils/wvstringlist.o \
	utils/wvstringmask.o \
	utils/strutils.o \
	utils/wvtask.o \
	utils/wvtimeutils.o \
	streams/wvistreamlist.o \
	utils/wvstreamsdebugger.o \
	streams/wvlog.o \
	streams/wvstream.o \
	uniconf/uniconf.o \
	uniconf/uniconfgen.o uniconf/uniconfkey.o uniconf/uniconfroot.o \
	uniconf/unihashtree.o \
	uniconf/unimountgen.o \
	uniconf/unitempgen.o \
	utils/wvbackslash.o \
	utils/wvencoder.o \
	utils/wvtclstring.o \
	utils/wvstringcache.o \
	uniconf/uniinigen.o \
	uniconf/unigenhack.o \
	uniconf/unilistiter.o \
	streams/wvfile.o \
	streams/wvstreamclone.o  \
	streams/wvconstream.o \
	utils/wvcrashbase.o

TESTOBJS = utils/wvtest.o

libwvbase.a libwvbase.so: $(filter-out uniconf/unigenhack.o,$(BASEOBJS))
libwvbase.a: uniconf/unigenhack_s.o
libwvbase.so: uniconf/unigenhack.o
libwvbase.so: LIBS+=$(LIBXPLC)

libwvutils.a libwvutils.so: $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: libwvbase.so
libwvutils.so: -lz -lcrypt

libwvstreams.a libwvstreams.so: $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: libwvutils.so libwvbase.so
libwvstreams.so: LIBS+=-lz -lssl -lcrypto 

libuniconf.a libuniconf.so: $(filter-out $(BASEOBJS), \
	$(call objects,uniconf))
libuniconf.a: uniconf/uniconfroot.o
libuniconf.so: libwvstreams.so libwvutils.so libwvbase.so

libwvdbus.a libwvdbus.so: $(call objects,dbus)
libwvdbus.so: libwvstreams.so libwvutils.so libwvbase.so
libwvdbus.so: LIBS+=$(LIBS_DBUS)

libwvtest.a: wvtestmain.o $(TESTOBJS)

ifeq ("$(wildcard /usr/lib/libqt-mt.so)", "/usr/lib/libqt-mt.so")
  libwvqt.so-LIBS+=-lqt-mt
else 
  # RedHat has a pkgconfig file we can use to sort out this mess..
  ifeq ("$(wildcard /usr/lib/pkgconfig/qt-mt.pc)", "/usr/lib/pkgconfig/qt-mt.pc")
    libwvqt.so-LIBS+=`pkg-config --libs qt-mt`
  else
    libwvqt.so-LIBS+=-lqt
  endif
endif
libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvutils.so libwvstreams.so libwvbase.so

ifneq (${_WIN32},)
  $(error "Use 'make -f Makefile-win32' instead!")
endif

export WVSTREAMS

XPATH=include

SUBDIRS =

all: runconfigure $(TARGETS)

.PHONY: clean depend dust kdoc doxygen install install-shared install-dev uninstall tests dishes dist distclean realclean test

runconfigure: config.mk include/wvautoconf.h

ifndef CONFIGURING
configure=$(error Please run the "configure" script)
else
configure:=
endif

config.mk: configure config.mk.in
	$(call configure)

include/wvautoconf.h: include/wvautoconf.h.in
	$(call configure)

aclocal.m4: acinclude.m4
	aclocal
	@touch $@

configure: configure.ac include/wvautoconf.h.in aclocal.m4
	autoconf
	@rm -f config.mk include/wvautoconf.h
	@touch $@

include/wvautoconf.h.in: configure.ac aclocal.m4
	autoheader
	@touch $@

distclean: clean
	@rm -rfv .junk $(DISTCLEAN)
	@rm -rf autom4te.cache
	@rm -f pkgconfig/*.pc

clean:
	$(subdirs)
	@rm -rfv .junk $(TARGETS) uniconf/daemon/uniconfd \
		$(GARBAGE) $(TESTS) tmp.ini \
		$(shell find . -name '*.o' -o -name '*.moc'))

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

uniconfd: uniconf/daemon/uniconfd uniconf/daemon/uniconfd.ini \
          uniconf/daemon/uniconfd.8

