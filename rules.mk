
default: $(TARGETS)

libwvoggvorbis.a libwvoggvorbis.so: $(call objects,oggvorbis)
libwvoggvorbis.so: libwvstreams.so

libwvfft.a libwvfft.so: $(call objects,fft)
libwvfft.so: libwvstreams.so

libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvstreams.so

libwvstreams.a libwvstreams.so: $(call objects,configfile crypto ipstreams linuxstreams streams uniconf urlget)
libwvstreams.so: libwvutils.so

libwvutils.a libwvutils.so: $(call objects,utils)

DEPFILE = $(notdir $(@:.o=.d))

ifdef VERBOSE
COMPILE_MSG:=
LINK_MSG:=
else
COMPILE_MSG=@echo compiling $@;
LINK_MSG=@echo linking $@;
endif

%: %.o
	$(LINK_MSG)$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%: %.cc
	$(COMPILE_MSG)$(LINK.cc) -MD $< $(LOADLIBES) $(LDLIBS) -o $@
	@test -f $(notdir $@).d
	@sed -e 's|^$(notdir $@).o|$@|' $(notdir $@).d > $(dir $@).$(notdir $@).d
	@rm -f $(notdir $@).d

%.o: %.cc
	$(COMPILE_MSG)$(CXX) $(CXXFLAGS) -MD $(CPPFLAGS) -c -o $@ $<
	@test -f $(DEPFILE)
	@sed -e 's|^$(notdir $@)|$@|' $(DEPFILE) > $(dir $@).$(DEPFILE)
	@rm -f $(DEPFILE)

%.o: %.c
	$(COMPILE_MSG)$(CC) $(CFLAGS) -MD $(CPPFLAGS) -c -o $@ $<
	@test -f $(DEPFILE)
	@sed -e 's|^$(notdir $@)|$@|' $(DEPFILE) > $(dir $@).$(DEPFILE)
	@rm -f $(DEPFILE)

%.a:
	$(LINK_MSG)$(AR) $(ARFLAGS) $@ $^

%.so:
	$(LINK_MSG)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared \
		-Wl,-soname,$@ $^ -o $@

%.moc: %.h
	$(COMPILE_MSG)moc $< -o $@

.PHONY: ChangeLog clean depend dust kdoc doxygen install install-shared install-dev uninstall tests dishes dist distclean realclean

# FIXME: little trick to ensure that the wvautoconf.h.in file is there
.PHONY: dist-hack-clean
dist-hack-clean:
	rm -f stamp-h.in

dist: dist-hack-clean configure distclean
	rm -rf autom4te.cache

config.mk include/wvautoconf.h: configure config.mk.in include/wvautoconf.h.in
	$(error Please run the "configure" script)

# FIXME: there is some confusion here
ifdef WE_ARE_DIST
configure: configure.ac config.mk.in include/wvautoconf.h.in
	$(warning "$@" is old, please run "autoconf")

include/wvautoconf.h.in: configure.ac
	$(warning "$@" is old, please run "autoheader")
else
configure: configure.ac config.mk.in include/wvautoconf.h.in
	autoconf

include/wvautoconf.h.in: stamp-h.in
stamp-h.in: configure.ac
	autoheader
	echo timestamp > $@
endif

ChangeLog:
	rm -f ChangeLog ChangeLog.bak
	cvs2cl --utc

realclean: distclean
	rm -rf $(wildcard $(REALCLEAN))

distclean: clean
	rm -rf $(wildcard $(DISTCLEAN))

clean: depend dust
	rm -rf $(wildcard $(TARGETS) $(GARBAGES) $(TESTS)) $(shell find . -name '*.o' -o -name '*.moc')

depend:
	rm -rf $(shell find . -name '.*.d')

dust:
	rm -rf $(shell find . -name 'core' -o -name '*~' -o -name '.#*') $(wildcard *.d)

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install: install-shared install-dev

# FIXME: these should be built with their suffix, and the rule automated
install-shared: $(TARGETS_SO)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_SO); do \
	    $(INSTALL_PROGRAM) $$i $(DESTDIR)$(libdir)/$$i.$(RELEASE); \
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

$(TESTS): libwvstreams.a libwvutils.a
$(TESTS): LDLIBS+=libwvstreams.a libwvutils.a
tests: $(TESTS)

dishes:
	@echo "aww, okay, but not now..."

include $(wildcard */rules.mk */*/rules.mk) /dev/null

-include $(shell find . -name '.*.d') /dev/null

