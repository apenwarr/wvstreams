
GARBAGE+=bindings/tcl/uniconf.cc bindings/python/uniconf.cc

SWIGOPTS = -c++ -Iinclude

bindings/tcl/uniconf.so: bindings/tcl/uniconf.o libuniconf.so -ltcl8.3

bindings/python/_uniconf.so: bindings/python/_uniconf.o libuniconf.so -lpython2.1

bindings/tcl/uniconf.cc: bindings/uniconf.i
	$(SWIG) -tcl $(SWIGOPTS) -o $@ $^

bindings/python/_uniconf.cc: bindings/uniconf.i
	$(SWIG) -python $(SWIGOPTS) -o $@ $^
