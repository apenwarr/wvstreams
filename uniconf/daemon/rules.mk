
libuniconfdaemon.so: $(filter-out uniconf/daemon/daemonmain.o,$(call objects,uniconf/daemon)) libwvstreams.so libwvutils.so libuniconf.so

uniconf/daemon/uniconfdaemon: libuniconfdaemon.so uniconf/daemon/daemonmain.o

