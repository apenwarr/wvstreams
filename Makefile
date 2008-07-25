WVSTREAMS=.
include wvrules.mk


%: %.in
	@sed -e 's/#VERSION#/$(PACKAGE_VERSION)/g' < $< > $@


ifeq ("$(enable_testgui)", "no")
  WVTESTRUN=env
endif

GARBAGE += wvtestmain.o tmp.ini .wvtest-total

TARGETS_SO = $(filter %.so,$(TARGETS))
TARGETS_A = $(filter %.a,$(TARGETS))

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

TARGETS += libwvbase.so
libwvbase_OBJS += $(filter-out uniconf/unigenhack.o,$(BASEOBJS))
libwvbase.so: $(libwvbase_OBJS) uniconf/unigenhack.o
libwvbase.so-LIBS += $(LIBXPLC)

TARGETS += libwvutils.so
libwvutils.so-LIBS+=$(LIBS_PAM)
libwvutils_OBJS += $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: $(libwvutils_OBJS) $(LIBWVBASE)
libwvutils.so-LIBS += -lz -lcrypt

TARGETS += libwvstreams.so
TARGETS += crypto/tests/ssltest ipstreams/tests/unixtest
TARGETS += crypto/tests/printcert
crypto/tests/% ipstreams/tests/%: LIBS+=$(LIBWVSTREAMS)
libwvstreams_OBJS += $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: $(libwvstreams_OBJS) $(LIBWVUTILS)
libwvstreams.so-LIBS += -lz -lssl -lcrypto 
ifneq ("$(with_pam)", "no")
  libwvstreams.so-LIBS += -lpam
endif
ifneq ("$(with_readline)", "no")
  TARGETS += ipstreams/tests/wsd
endif

TARGETS += libuniconf.so
TARGETS += uniconf/daemon/uniconfd uniconf/tests/uni
uniconf/daemon/uniconfd uniconf/tests/uni: $(LIBUNICONF)
libuniconf_OBJS += $(filter-out $(BASEOBJS) uniconf/daemon/uniconfd.o, \
	$(call objects,uniconf uniconf/daemon))
libuniconf.so: $(libuniconf_OBJS) $(LIBWVSTREAMS)
libuniconf.a: uniconf/uniconfroot.o
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.ini \
          uniconf/daemon/uniconfd.8

ifneq ("$(with_dbus)", "no")
  TARGETS += dbus/tests/wvdbus dbus/tests/wvdbusd
  TARGETS += libwvdbus.so
  dbus/tests/wvdbus dbus/tests/wvdbusd: $(LIBWVDBUS)
  libwvdbus_OBJS += $(call objects,dbus)
  libwvdbus.so: $(libwvdbus_OBJS) $(LIBWVSTREAMS)
  libwvdbus.so-LIBS += $(LIBS_DBUS)
endif

ifneq ("$(with_qt)", "no")
  TARGETS += libwvqt.so
  TESTS += $(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
  libwvqt_OBJS += $(call objects,qt)
  libwvqt.so: $(libwvqt_OBJS) $(LIBWVSTREAMS)
  libwvqt.so-LIBS += $(LIBS_QT)

  qt/wvqtstreamclone.o: include/wvqtstreamclone.moc
  qt/wvqthook.o: include/wvqthook.moc

  qt/tests/qtstringtest: $(LIBWVQT)
  qt/tests/%: LIBS+=$(LIBWVQT)
  qt/tests/%: LIBS+=$(LIBS_QT)
endif

TARGETS += libwvstatic.a
libwvstatic.a: \
	$(libwvbase_OBJS) \
	$(libwvutils_OBJS) \
	$(libwvstreams_OBJS) \
	$(libuniconf_OBJS) \
	$(libwvdbus_OBJS) \
	$(libwvqt_OBJS) \
	uniconf/unigenhack_s.o

TARGETS += wvtestmain.o libwvtest.a
TESTOBJS = utils/wvtest.o
libwvtest.a: wvtestmain.o $(TESTOBJS)

export WVSTREAMS

all: $(TARGETS)

.PHONY: clean depend dust kdoc doxygen install install-shared install-dev uninstall tests dishes dist distclean realclean test

distclean: clean
	@rm -rfv .junk $(DISTCLEAN)
	@rm -rf autom4te.cache
	@rm -f pkgconfig/*.pc

clean:
	$(subdirs)
	@rm -rfv .junk $(TARGETS) uniconf/daemon/uniconfd \
		$(GARBAGE) $(TESTS) tmp.ini \
		$(shell find . -name '*.o' -o -name '*.moc'))
		
clean-targets:
	rm -fv $(TARGETS)

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

