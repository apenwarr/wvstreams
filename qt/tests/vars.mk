
ifneq ("$(with_qt)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
endif

