DAEMONLIBS := libwvstreams.so libwvutils.so

ifneq ("$(with_pam)", "no")
DAEMONLIBS += -lpam
endif

uniconf/daemon/uniconfdaemon: $(call objects,uniconf/daemon) \
	$(DAEMONLIBS)
