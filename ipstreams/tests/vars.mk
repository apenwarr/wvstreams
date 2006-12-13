
TESTS+=$(patsubst %.cc,%,$(wildcard ipstreams/tests/*.cc))

ipstreams/tests/xplctest: LIBS+=-lxplc-cxx -lxplc
ipstreams/tests/wsd: LIBS+=-lxplc-cxx -lxplc -lreadline

