
GARBAGE+=bindings/uniconf_tcl.c

bindings/uniconf_tcl.c: include/uniconf.h
	$(SWIG) -tcl -o $@ $^

