WVSTREAMS=.
WVSTREAMS_SRC= # Clear WVSTREAMS_SRC so wvrules.mk uses its WVSTREAMS_foo
include wvrules.mk
override enable_efence=no

export WVSTREAMS

XPATH=include

include vars.mk

all: config.mk xplc $(TARGETS)

.PHONY: xplc xplc/clean install-xplc
xplc:
xplc/clean:
install-xplc:

ifeq ("$(build_xplc)", "yes")

xplc:
	$(MAKE) -C xplc

xplc/clean:
	$(MAKE) -C xplc clean

install-xplc: xplc
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL_DATA) $(wildcard xplc/include/xplc/*.h) $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL_DATA) xplc/libxplc-cxx.a $(DESTDIR)$(libdir)

endif

%.so: SONAME=$@.$(RELEASE)

.PHONY: clean depend dust kdoc doxygen install install-shared install-dev install-xplc uninstall tests dishes dist distclean realclean test

# FIXME: little trick to ensure that the wvautoconf.h.in file is there
.PHONY: dist-hack-clean
dist-hack-clean:
	rm -f stamp-h.in

dist-hook: dist-hack-clean configure
	rm -rf autom4te.cache
	if test -d .xplc; then \
	    $(MAKE) -C .xplc clean patch; \
	    cp -Lpr .xplc/build/xplc .; \
	fi

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

# FIXME: there is some confusion here
ifdef WE_ARE_DIST
configure: configure.ac config.mk.in include/wvautoconf.h.in
	$(warning "$@" is old, please run "autoconf")

include/wvautoconf.h.in: configure.ac
	$(warning "$@" is old, please run "autoheader")
else
configure: configure.ac
	autoheader
	autoconf

include/wvautoconf.h.in:
	autoheader
endif

define wild_clean
	@list=`echo $(wildcard $(1))`; \
		test -z "$${list}" || sh -cx "rm -rf $${list}"
endef

realclean: distclean
	$(call wild_clean,$(REALCLEAN))


distclean: clean
	$(call wild_clean,$(DISTCLEAN))
	@rm -f pkgconfig/*.pc
	@rm -f .xplc

clean: depend dust xplc/clean
	$(call wild_clean,$(TARGETS) uniconf/daemon/uniconfd \
		$(GARBAGE) $(TESTS) tmp.ini \
		$(shell find . -name '*.o' -o -name '*.moc'))

depend:
	$(call wild_clean,$(shell find . -name '.*.d'))

dust:
	$(call wild_clean,$(shell find . -name 'core' -o -name '*~' -o -name '.#*') $(wildcard *.d))

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install: install-shared install-dev install-xplc install-uniconfd

install-shared: $(TARGETS_SO)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_SO); do \
	    $(INSTALL_PROGRAM) $$i.$(RELEASE) $(DESTDIR)$(libdir)/ ; \
	done
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) uniconf/daemon/uniconf.conf $(DESTDIR)$(sysconfdir)/

install-dev: $(TARGETS_SO) $(TARGETS_A)
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL_DATA) $(wildcard include/*.h) $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_A); do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(libdir); \
	done
	cd $(DESTDIR)$(libdir) && for i in $(TARGETS_SO); do \
	    rm -f $$i; \
	    $(LN_S) $$i.$(RELEASE) $$i; \
	done
	$(INSTALL) -d $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(wildcard pkgconfig/*.pc) $(DESTDIR)$(libdir)/pkgconfig

uniconfd: uniconf/daemon/uniconfd uniconf/daemon/uniconfd.ini \
          uniconf/daemon/uniconfd.8

install-uniconfd: uniconfd uniconf/tests/uni uniconf/tests/uni.8
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) uniconf/tests/uni $(DESTDIR)$(bindir)/
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL_PROGRAM) uniconf/daemon/uniconfd $(DESTDIR)$(sbindir)/
	$(INSTALL) -d $(DESTDIR)$(localstatedir)/lib/uniconf
	touch $(DESTDIR)$(localstatedir)/lib/uniconf/uniconfd.ini
	$(INSTALL) -d $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/daemon/uniconfd.8 $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/tests/uni.8 $(DESTDIR)$(mandir)/man8

uninstall:
	$(tbd)

$(TESTS): $(LIBUNICONF)
$(addsuffix .o,$(TESTS)):
tests: $(TESTS)

include $(filter-out xplc%,$(wildcard */rules.mk */*/rules.mk)) /dev/null

-include $(shell find . -name '.*.d') /dev/null

test: runconfigure all tests wvtestmain
	LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(WVSTREAMS_LIB)" $(WVTESTRUN) $(MAKE) runtests

runtests:
	$(VALGRIND) ./wvtestmain '$(TESTNAME)'
ifeq ("$(TESTNAME)", "unitest")
	cd uniconf/tests && DAEMON=0 ./unitest.sh
	cd uniconf/tests && DAEMON=1 ./unitest.sh
endif

wvtestmain: wvtestmain.o \
	$(call objects, $(shell find . -type d -name t)) \
	$(LIBUNICONF) $(LIBWVSTREAMS)

