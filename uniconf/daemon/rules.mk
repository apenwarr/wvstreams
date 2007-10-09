
CXXFLAGS+=-DWVSTREAMS_RELEASE=\"$(PACKAGE_VERSION)\"
DISTCLEAN+=uniconf/daemon/uniconfd.8

libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))

ifeq ($(EXEEXT),.exe)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o libwvwin32.a
else
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)
endif

%: %.in
	@sed -e "s/#VERSION#/$(PACKAGE_VERSION)/g" < $< > $@
