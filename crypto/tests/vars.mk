
TESTS+=$(patsubst %.cc,%,$(wildcard crypto/tests/*.cc))

crypto/tests/ssltest-LIBS:=-lssl
crypto/tests/sslsrvtest-LIBS:=-lssl

