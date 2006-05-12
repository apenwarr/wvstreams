
CXXFLAGS+=-DWVSTREAMS_RELEASE=\"$(PACKAGE_VERSION)\"

libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))
ifneq ("$(with_openslp)", "no")
  uniconf/daemon/uniconfd-LIBS+=-lslp
endif
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)

%: %.in
	@sed -e "s/#VERSION#/$(PACKAGE_VERSION)/g" < $< > $@
