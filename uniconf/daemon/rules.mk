
libuniconfdaemon.so: \
   $(filter-out uniconf/daemon/daemonmain.o,$(call objects,uniconf/daemon)) \
   $(LIBUNICONF)

uniconf/daemon/uniconfdaemon: libuniconfdaemon.so uniconf/daemon/daemonmain.o

