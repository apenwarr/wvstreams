
libuniconfdaemon.so: $(filter-out uniconf/daemon/daemonmain.o,$(call objects,uniconf/daemon)) -luniconf

uniconf/daemon/uniconfdaemon: libwvstreams.so libwvutils.so libuniconf.so libuniconfdaemon.so uniconf/daemon/daemonmain.o

