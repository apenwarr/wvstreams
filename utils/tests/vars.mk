
TESTS+=$(patsubst %.cc,%,$(wildcard utils/tests/*.cc))

utils/tests/testencoder-LIBS:=-lz

