
libuniconf.so libuniconf.a: $(filter-out uniconf/daemon/daemonmain.o,$(call objects,uniconf/daemon))

uniconf/daemon/uniconfdaemon: uniconf/daemon/daemonmain.o libuniconf.so

