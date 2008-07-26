WVSTREAMS=.

include wvrules.mk

%: %.in
	@sed -e 's/#VERSION#/$(PACKAGE_VERSION)/g' < $< > $@

ifeq ("$(enable_testgui)", "no")
  WVTESTRUN=env
endif

LIBS += $(LIBS_XPLC) -lm

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

#
# libwvutils: handy utility library for C++
#
TARGETS += libwvutils.so
TESTS += $(call tests_cc,utils/tests)
libwvutils_OBJS += $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: $(libwvutils_OBJS) $(LIBWVBASE)
libwvutils.so-LIBS += -lz -lcrypt $(LIBS_PAM)
utils/tests/%: LIBS+=$(LIBWVSTREAMS)

#
# libwvstreams: stream/event handling library
#
TARGETS += libwvstreams.so
TARGETS += crypto/tests/ssltest ipstreams/tests/unixtest
TARGETS += crypto/tests/printcert
ifneq ("$(with_readline)", "no")
  TARGETS += ipstreams/tests/wsd
endif
TESTS += $(call tests_cc,configfile/tests)
TESTS += $(call tests_cc,streams/tests)
TESTS += $(filter-out ipstreams/tests/wsd, $(call tests_cc,ipstreams/tests))
TESTS += $(call tests_cc,crypto/tests)
TESTS += $(call tests_cc,urlget/tests)
TESTS += $(call tests_cc,linuxstreams/tests)
libwvstreams_OBJS += $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: $(libwvstreams_OBJS) $(LIBWVUTILS)
libwvstreams.so-LIBS += -lz -lssl -lcrypto $(LIBS_PAM)
ipstreams/tests/wsd: LIBS+=-lreadline
configfile/tests/% streams/tests/% ipstreams/tests/% crypto/tests/% \
  urlget/tests/% linuxstreams/tests/%: LIBS+=$(LIBWVSTREAMS)

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
uniconf/tests/%: LIBS+=$(LIBUNICONF)

#
# libwvdbus: C++ DBus library based on wvstreams
#
ifneq ("$(with_dbus)", "no")
  TARGETS += dbus/tests/wvdbus dbus/tests/wvdbusd
  TARGETS += libwvdbus.so
  TESTS += $(call tests_cc,dbus/tests)
  libwvdbus_OBJS += $(call objects,dbus)
  libwvdbus.so: $(libwvdbus_OBJS) $(LIBWVSTREAMS)
  libwvdbus.so-LIBS += $(LIBS_DBUS)
  dbus/tests/%: LIBS+=$(LIBWVDBUS)
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
  qt/tests/%: LIBS+=$(LIBWVQT)

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
	uniconf/unigenhack_s.o

#
# libwvtest: the WvTest tools for writing C++ unit tests
#
TARGETS += wvtestmain.o libwvtest.a
TESTOBJS = utils/wvtest.o
libwvtest.a: wvtestmain.o $(TESTOBJS)

#
# Some example programs
#
TARGETS += examples/wvgrep/wvgrep examples/wvgrep/wvegrep
examples/wvgrep/wvgrep: examples/wvgrep/wvgrep.o $(LIBWVSTREAMS)
examples/wvgrep/wvegrep: examples/wvgrep/wvgrep
	ln -f $< $@


TARGETS_SO = $(filter %.so,$(TARGETS))
TARGETS_A = $(filter %.a,$(TARGETS))

all: $(TARGETS)

TESTS += wvtestmain
$(addsuffix .o,$(TESTS)):
tests: $(TESTS)

test: all tests qtest

qtest: all wvtestmain
	LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(WVSTREAMS_LIB)" $(WVTESTRUN) $(MAKE) runtests

runtests:
	$(VALGRIND) ./wvtestmain '$(TESTNAME)'
ifeq ("$(TESTNAME)", "unitest")
	cd uniconf/tests && DAEMON=0 ./unitest.sh
	cd uniconf/tests && DAEMON=1 ./unitest.sh
endif

wvtestmain: \
	$(call objects, $(filter-out ./win32/%, \
		$(shell find . -type d -name t))) \
	$(LIBWVDBUS) $(LIBUNICONF) $(LIBWVSTREAMS) $(LIBWVTEST)

distclean: clean
	rm -f uniconf/daemon/uniconfd.8 uniconf/tests/uni
	rm -f config.mk config.log config.status \
		include/wvautoconf.h config.cache reconfigure \
		stamp-h.in configure include/wvautoconf.h.in
	rm -rf autom4te.cache
	rm -f pkgconfig/*.pc

clean:
	$(subdirs)
	@rm -fv .junk $(TARGETS) uniconf/daemon/uniconfd \
		$(TESTS) tmp*.ini uniconf/daemon/uniconfd.ini \
		.wvtest-total \
		$(shell find . -name '*.o' -o -name '.*.d' \
			-o -name '*~' -o -name '*.moc')
		
clean-targets:
	rm -fv $(TARGETS)
	
clean-tests:
	rm -fv $(TESTS)

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

.PHONY: \
	clean distclean \
	kdoc doxygen \
	install install-shared install-dev uninstall \
	tests test
