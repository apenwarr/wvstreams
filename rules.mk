
libwvstreams.a libwvstreams.so: $(libwvstreams.so-OBJECTS) libwvutils.so

libwvutils.a libwvutils.so: $(libwvutils.so-OBJECTS)

DEPFILE = $(notdir $(@:.o=.d))

%: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) $($@-LIBS) -o $@
	@sed -e 's|^$(notdir $@)|$@|' $(notdir $@).d > $(dir $@).$(notdir $@).d
	@rm -f $(notdir $@).d

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@sed -e 's|^$(notdir $@)|$@|' $(DEPFILE) > $(dir $@).$(DEPFILE)
	@rm -f $(DEPFILE)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	@sed -e 's|^$(notdir $@)|$@|' $(DEPFILE) > $(dir $@).$(DEPFILE)
	@rm -f $(DEPFILE)

%.a:
	$(AR) $(ARFLAGS) $@ $^

%.so:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $($@-LIBS) -shared $^ -o $@

.PHONY: ChangeLog clean depend dust kdoc doxygen install uninstall tests dishes

config.mk include/wvautoconf.h: config.mk.in include/wvautoconf.h.in configure
	$(error Please run the "configure" script)

ifdef WE_ARE_DIST
configure: configure.ac
	$(warning "$@" is old, please run "autoconf")

include/wvautoconf.h.in: configure.ac
	$(warning "$@" is old, please run "autoheader")
else
configure: configure.ac
	autoconf

include/wvautoconf.h.in: stamp-h.in
stamp-h.in: configure.ac
	autoheader
	echo timestamp > $@
endif

ChangeLog:
	rm -f ChangeLog ChangeLog.bak
	cvs2cl --utc

clean: depend dust
	rm -f $(shell find . -name '*.o')
	rm -rf $(wildcard $(TARGETS) $(GARBAGES) $(TESTS))

depend:
	rm -f $(shell find . -name '.*.d')

dust:
	rm -f $(shell find . -name 'core' -o -name '*~' -o -name '.#*')

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install: all
	$(tbd)

uninstall:
	$(tbd)

$(TESTS): libwvstreams.a libwvutils.a
tests: $(TESTS)

dishes:
	@echo "aww, okay, but not now..."

include $(call doinclude,rules.mk)

-include $(shell find . -name '.*.d') /dev/null

