bindings/tcl: bindings/tcl/uniconf.so
bindings/python: bindings/python/_uniconf.so
bindings/php: bindings/php/php_uniconf.so

GARBAGE+=bindings/tcl/uniconf.cc bindings/python/uniconf.cc bindings/php/uniconf.cc

SWIGOPTS = -c++ -Iinclude

bindings/tcl/uniconf.so: bindings/tcl/uniconf.o libuniconf.so -ltcl8.3

bindings/python/_uniconf.so: bindings/python/_uniconf.o libuniconf.so -lpython2.1

bindings/php/php_uniconf.so: bindings/php/uniconf.o libuniconf.so

bindings/tcl/uniconf.cc: bindings/uniconf.i
	$(SWIG) -tcl $(SWIGOPTS) -o $@ $^

bindings/python/_uniconf.cc: bindings/uniconf.i
	$(SWIG) -python $(SWIGOPTS) -o $@ $^

bindings/php/uniconf.cc: bindings/uniconf.i
	$(SWIG) -php $(SWIGOPTS) -o $@ $^
