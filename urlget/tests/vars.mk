
TESTS+=$(patsubst %.cc,%,$(wildcard urlget/tests/*.cc))

urlget/tests/http2test-LIBS:=-lssl -rdynamic

