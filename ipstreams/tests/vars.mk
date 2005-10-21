
TESTS+=$(patsubst %.cc,%,$(wildcard ipstreams/tests/*.cc))

ipstreams/tests/xplctest: LIBS+=-lxplc-cxx
