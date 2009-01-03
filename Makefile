WVSTREAMS:=.

include wvrules.mk
include install.mk

ifdef _WIN32
    include win32.mk
endif

ifdef _SOLARIS
    RM = rm -f
    CFLAGS += -DSOLARIS
    CXXFLAGS += -DSOLARIS
else 
    ifdef _MACOS
    	CFLAGS+= -DMACOS
    	CXXFLAGS+= -DMACOS
    else
    	RM = rm -fv
    endif 
endif

%: %.subst
	sed -e 's/#VERSION#/$(WVPACKAGE_VERSION)/g' < $< > $@

ifeq ("$(enable_testgui)", "no")
  WVTESTRUN=env
endif

LIBS += -lm

ifeq ($(USE_WVSTREAMS_ARGP),1)
  utils/wvargs.o-CPPFLAGS += -Iargp
  libwvutils.so-LIBS += -Largp -largp

# argp does its own dependency checking, so let's call it once per wvstreams
# build, in case some system dependencies have changed and argp should be
# recompiled
  ARGP_TARGET=argp/libargp.list
  TARGETS += $(ARGP_TARGET)
.PHONY: argp/all
argp/all:
	$(MAKE) -C argp
# Warning!  Crucial!  KEEP THIS SEMICOLON HERE!  Without it, if you blow away
# $(ARGP_TARGET), $(MAKE) will get confused.
$(ARGP_TARGET): argp/all;
else
  ARGP_TARGET=
endif

#
# libwvbase: a the minimal code needed to link a wvstreams program.
#
BASEOBJS = \
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
	utils/wvattrs.o \
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
	utils/wvcrashbase.o \
	xplc-cxx/factory.o \
	xplc-cxx/getiface.o \
	xplc-cxx/strtouuid.o \
	xplc-cxx/uuidtostr.o \
	xplc-cxx/xplc.o \
	xplc/category.o \
	xplc/catiter.o \
	xplc/catmgr.o \
	xplc/loader.o \
	xplc/moduleloader.o \
	xplc/modulemgr.o \
	xplc/monikers.o \
	xplc/new.o \
	xplc/servmgr.o \
	xplc/statichandler.o 

TARGETS += libwvbase.so
libwvbase_OBJS += $(filter-out uniconf/unigenhack.o $(WV_EXCLUDES),$(BASEOBJS))
libwvbase.so: $(libwvbase_OBJS) uniconf/unigenhack.o
libwvbase.so-LIBS += $(LIBXPLC)

#
# libwvutils: handy utility library for C++
#
TARGETS += libwvutils.so
TESTS += $(call tests_cc,utils/tests)
libwvutils_OBJS += $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: $(libwvutils_OBJS) $(LIBWVBASE) $(ARGP_TARGET)
ifndef _MACOS
libwvutils.so-LIBS += -lz -lcrypt $(LIBS_PAM)
else
libwvutils.so-LIBS += -lz $(LIBS_PAM)
endif

utils/tests/%: PRELIBS+=$(LIBWVSTREAMS)

#
# libwvstreams: stream/event handling library
#
TARGETS += libwvstreams.so
TARGETS += crypto/tests/ssltest ipstreams/tests/unixtest
TARGETS += crypto/tests/printcert
ifneq ("$(with_readline)", "no")
  TARGETS += ipstreams/tests/wsd
  ipstreams/tests/wsd-LIBS += -lreadline
else
  TEST_SKIP_OBJS += ipstreams/tests/wsd
endif
TESTS += $(call tests_cc,configfile/tests)
TESTS += $(call tests_cc,streams/tests)
TESTS += $(call tests_cc,ipstreams/tests)
TESTS += $(call tests_cc,crypto/tests)
TESTS += $(call tests_cc,urlget/tests)
TESTS += $(call tests_cc,linuxstreams/tests)
libwvstreams_OBJS += $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: $(libwvstreams_OBJS) $(LIBWVUTILS)
libwvstreams.so-LIBS += -lz -lssl -lcrypto $(LIBS_PAM)
configfile/tests/% streams/tests/% ipstreams/tests/% crypto/tests/% \
  urlget/tests/% linuxstreams/tests/%: PRELIBS+=$(LIBWVSTREAMS)

#
# libuniconf: unified configuration system
#
TARGETS += libuniconf.so
TARGETS += uniconf/daemon/uniconfd uniconf/tests/uni
TESTS += $(call tests_cc,uniconf/tests) uniconf/tests/uni
libuniconf_OBJS += $(filter-out $(BASEOBJS) uniconf/daemon/uniconfd.o, \
	$(call objects,uniconf uniconf/daemon))
libuniconf.so: $(libuniconf_OBJS) $(LIBWVSTREAMS)
uniconf/daemon/uniconfd uniconf/tests/uni: $(LIBUNICONF)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.ini \
          uniconf/daemon/uniconfd.8
uniconf/tests/%: PRELIBS+=$(LIBUNICONF)

#
# libwvdbus: C++ DBus library based on wvstreams
#
ifneq ("$(with_dbus)", "no")
  TARGETS += libwvdbus.so
  TARGETS += dbus/tests/wvdbus dbus/tests/wvdbusd
  TESTS += $(call tests_cc,dbus/tests)
  libwvdbus_OBJS += $(call objects,dbus)
  libwvdbus.so: $(libwvdbus_OBJS) $(LIBWVSTREAMS)
  libwvdbus.so-LIBS += $(LIBS_DBUS)
  dbus/tests/%: PRELIBS+=$(LIBWVDBUS)
else
  TEST_SKIP_OBJS += dbus/t/%
endif

#
# libwvqt: helper library to make WvStreams work better in Qt event loops
#
ifneq ("$(with_qt)", "no")
  TARGETS += libwvqt.so
  TESTS += $(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
  libwvqt_OBJS += $(call objects,qt)
  libwvqt.so: $(libwvqt_OBJS) $(LIBWVSTREAMS)
  libwvqt.so-LIBS += $(LIBS_QT)
  qt/tests/%: PRELIBS+=$(LIBWVQT)

  qt/wvqtstreamclone.o: include/wvqtstreamclone.moc
  qt/wvqthook.o: include/wvqthook.moc
endif

#
# libwvstatic.a: all the wvstreams libraries in one static .a file, to make
# it easy to link your programs statically to wvstreams.
#
TARGETS += libwvstatic.a
libwvstatic.a: \
	$(libwvbase_OBJS) \
	$(libwvutils_OBJS) \
	$(libwvstreams_OBJS) \
	$(libuniconf_OBJS) \
	$(libwvdbus_OBJS) \
	$(libwvqt_OBJS) \
	uniconf/unigenhack_s.o \
	$(ARGP_TARGET)

#
# libwvtest: the WvTest tools for writing C++ unit tests
#
TARGETS += wvtestmain.o libwvtest.a
TESTOBJS = utils/wvtest.o
libwvtest.a: wvtestmain.o $(TESTOBJS)

TARGETS_SO = $(filter %.so,$(TARGETS))
TARGETS_A = $(filter %.a,$(TARGETS))

all: config.mk $(filter-out $(WV_EXCLUDES), $(TARGETS))

TESTS += wvtestmain

REAL_TESTS = $(filter-out $(TEST_SKIP_OBJS), $(TESTS))
$(addsuffix .o,$(REAL_TESTS)):
tests: $(REAL_TESTS)

test: all tests qtest

qtest: all wvtestmain
	LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(shell pwd)" $(WVTESTRUN) $(MAKE) runtests

runtests:
	$(VALGRIND) ./wvtestmain '$(TESTNAME)'
ifeq ("$(TESTNAME)", "")
	cd uniconf/tests && DAEMON=0 ./unitest.sh
	cd uniconf/tests && DAEMON=1 ./unitest.sh
endif


ifndef _MACOS
TEST_TARGETS = 	$(filter-out $(TEST_SKIP_OBJS), $(call objects, $(filter-out win32/%, \
		$(shell find . -type d -name t -printf "%P\n"))))
else
TEST_TARGETS = 	$(filter-out $(TEST_SKIP_OBJS), $(call objects, $(filter-out win32/%, \
		$(shell find . -type d -name t -print))))
endif

wvtestmain: $(TEST_TARGETS) $(LIBWVDBUS) $(LIBUNICONF) $(LIBWVSTREAMS) $(LIBWVTEST)

distclean: clean
	rm -f uniconf/daemon/uniconfd.8 uniconf/tests/uni
	rm -f config.mk config.log config.status \
		include/wvautoconf.h config.cache \
		stamp-h.in 
	rm -rf autom4te.cache
	rm -rf argp/autom4te.cache
	rm -rf argp/config.status argp/config.log
	rm -rf argp/configure.lineno argp/config.h
	rm -rf argp/Makefile argp/testsuite/Makefile
	rm -rf argp/testsuite/ex1 argp/testsuite/ex2 argp/testsuite/ex3
	rm -rf argp/libargp.a argp/stamp-h1
	rm -f pkgconfig/*.pc

clean:
	$(subdirs)
	@$(RM) .junk $(TARGETS) uniconf/daemon/uniconfd \
		$(TESTS) tmp*.ini uniconf/daemon/uniconfd.ini \
		.wvtest-total \
		$(shell find . -name '*.o' -o -name '.*.d' \
			-o -name '*~' -o -name '*.moc')
		
clean-targets:
	$(RM) $(TARGETS)
	
clean-tests:
	$(RM) $(TESTS)

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

.PHONY: \
	clean distclean \
	kdoc doxygen \
	install install-shared install-dev uninstall \
	tests test
