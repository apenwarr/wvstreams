ifeq ("$(with_readline)", "no")
install: install-shared install-dev install-uniconfd
else
install: install-shared install-dev install-uniconfd install-wsd
endif

install-shared: $(TARGETS_SO)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_SO); do \
	    $(INSTALL_PROGRAM) $$i.$(SO_VERSION) $(DESTDIR)$(libdir)/ ; \
	done
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) uniconf/daemon/uniconf.conf $(DESTDIR)$(sysconfdir)/

install-dev: $(TARGETS_SO) $(TARGETS_A)
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL_DATA) $(wildcard include/*.h) $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL_DATA) $(wildcard include/xplc/*.h) $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_A); do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(libdir); \
	done
	cd $(DESTDIR)$(libdir) && for i in $(TARGETS_SO); do \
	    rm -f $$i; \
	    $(LN_S) $$i.$(SO_VERSION) $$i; \
	done
	$(INSTALL) -d $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(filter-out %-uninstalled.pc, $(wildcard pkgconfig/*.pc)) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) wvtestrun $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)/valgrind
	$(INSTALL) wvstreams.supp $(DESTDIR)$(libdir)/valgrind

install-uniconfd: uniconf/daemon/uniconfd uniconf/tests/uni uniconf/tests/uni.8
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) uniconf/tests/uni $(DESTDIR)$(bindir)/
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL_PROGRAM) uniconf/daemon/uniconfd $(DESTDIR)$(sbindir)/
	$(INSTALL) -d $(DESTDIR)$(localstatedir)/lib/uniconf
	touch $(DESTDIR)$(localstatedir)/lib/uniconf/uniconfd.ini
	$(INSTALL) -d $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/daemon/uniconfd.8 $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/tests/uni.8 $(DESTDIR)$(mandir)/man8

install-wsd: ipstreams/tests/wsd
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) ipstreams/tests/wsd $(DESTDIR)$(bindir)/
