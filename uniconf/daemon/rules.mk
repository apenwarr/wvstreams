
libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))

uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o libuniconf.so
