
default: $(TARGETS)

ifdef VERBOSE
PREPROC_MSG =
COMPILE_MSG =
LINK_MSG =
EMPTY_MSG =
DEPEND_MSG = @
else
PREPROC_MSG = @echo preprocessing $@;
COMPILE_MSG = @echo compiling $@;
LINK_MSG = @echo linking $@;
EMPTY_MSG = @
DEPEND_MSG = @
endif

SONAMEOPT=-Wl,-soname,$(SONAME)
SOFLAGS=-shared $(if $(SONAME),$(SONAMEOPT))

DEPFILE = $(notdir $(@:.o=.d))
CPPFLAGS += $(patsubst %,-I%,$(XPATH))

%: %.o
	$(LINK_MSG)$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.cc
	$(COMPILE_MSG)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(DEPEND_MSG)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -M -E $< | \
		sed -e 's|^$(notdir $@)|$@|' > $(dir $@).$(DEPFILE)

%.o: %.c
	$(COMPILE_MSG)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(DEPEND_MSG)$(CC) $(CFLAGS) $(CPPFLAGS) -M -E $< | \
		sed -e 's|^$(notdir $@)|$@|' > $(dir $@).$(DEPFILE)

%.a:
	$(LINK_MSG)$(AR) $(ARFLAGS) $@ $^

%.so: SONAME=$@.$(RELEASE)
%.so:
	$(LINK_MSG)$(CC) $(LDFLAGS) $(SOFLAGS) $^ $(LDLIBS) -o $@
	$(EMPTY_MSG)$(LN_S) -f $@ $(SONAME)

%.moc: %.h
	$(COMPILE_MSG)$(MOC) $< -o $@

.PHONY: ChangeLog clean depend dust kdoc doxygen install install-shared install-dev uninstall tests dishes dist distclean realclean

# FIXME: little trick to ensure that the wvautoconf.h.in file is there
.PHONY: dist-hack-clean
dist-hack-clean:
	rm -f stamp-h.in

dist: dist-hack-clean configure distclean ChangeLog
	rm -rf autom4te.cache

runconfigure: config.mk include/wvautoconf.h

config.mk include/wvautoconf.h: configure config.mk.in include/wvautoconf.h.in
	$(error Please run the "configure" script)

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

ChangeLog:
	rm -f ChangeLog ChangeLog.bak
	cvs2cl --utc

define wild_clean
	@list=`echo $(wildcard $(1))` ; test -z "$${list}" || sh -cx "rm -rf $${list}"
endef

realclean: distclean
	$(call wild_clean,$(REALCLEAN))


distclean: clean
	$(call wild_clean,$(DISTCLEAN))

clean: depend dust
	$(call wild_clean,$(TARGETS) $(GARBAGE) $(TESTS) $(shell find . -name '*.o' -o -name '*.moc'))

depend:
	$(call wild_clean,$(shell find . -name '.*.d'))

dust:
	$(call wild_clean,$(shell find . -name 'core' -o -name '*~' -o -name '.#*') $(wildcard *.d))

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install: install-shared install-dev
#FIXME: We need to install uniconfd somewhere.

install-shared: $(TARGETS_SO)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_SO); do \
	    $(INSTALL_PROGRAM) $$i.$(RELEASE) $(DESTDIR)$(libdir)/ ; \
	done

install-dev: $(TARGETS_SO) $(TARGETS_A)
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL_DATA) $(wildcard include/*.h) $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_A); do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(libdir); \
	done
	for i in $(TARGETS_SO); do \
	    cd $(DESTDIR)$(libdir) && $(LN_S) $$i.$(RELEASE) $$i; \
	done

uninstall:
	$(tbd)

$(TESTS): libwvstreams.so libwvutils.so libuniconf.so
$(addsuffix .o,$(TESTS)):
tests: $(TESTS)

dishes:
	@echo "aww, later..."

include $(wildcard */rules.mk */*/rules.mk) /dev/null

-include $(shell find . -name '.*.d') /dev/null

