
TESTS+=$(filter-out ipstreams/tests/wsd, $(patsubst %.cc,%,$(wildcard ipstreams/tests/*.cc)))
ifneq ("$(with_readline)", "no")
TESTS+=ipstreams/tests/wsd
endif

ipstreams/tests/xplctest: LIBS+=-lxplc-cxx -lxplc
ipstreams/tests/wsd: LIBS+=-lxplc-cxx -lxplc -lreadline

