
GARBAGE+=bindings/uniconf_tcl.c

SWIGPARAMS =
ifneq ("$(with_swig)x", "x")
	SWIGPARAMS += -I$(with_swig)/Lib -I$(with_swig)/Lib/tcl
endif

bindings/libuniconf_tcl.so: bindings/uniconf_tcl.o libuniconf.so -ltcl8.3

bindings/uniconf_tcl.c: include/uniconf.h
	$(SWIG) $(SWIGPARAMS) -tcl -o $@ $^

