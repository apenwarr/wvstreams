
GARBAGE+=bindings/uniconf_tcl.c

bindings/libuniconf_tcl.so: bindings/uniconf_tcl.o libuniconf.so -ltcl8.3

bindings/uniconf_tcl.c: include/uniconf.h
	$(SWIG) -tcl -o $@ $^

