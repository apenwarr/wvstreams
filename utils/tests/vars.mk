
TESTS+=$(patsubst %.cc,%,$(wildcard utils/tests/*.cc))

utils/tests/encodertest-LIBS:=-lz

