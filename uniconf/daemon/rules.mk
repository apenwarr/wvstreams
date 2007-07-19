
DISTCLEAN+=uniconf/daemon/uniconfd.8

libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))

uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)

%: %.in
	@sed -e "s/#VERSION#/$(PACKAGE_VERSION)/g" < $< > $@
