
TESTS+=$(patsubst %.cc,%,$(wildcard crypto/tests/*.cc))

crypto/tests/reqtest-LIBS:=-lssl
crypto/tests/md5test-LIBS:=-lssl
crypto/tests/cryptotest-LIBS:=-lssl
crypto/tests/certtest-LIBS:=-lssl
crypto/tests/ssltest-LIBS:=-lssl
crypto/tests/sslsrvtest-LIBS:=-lssl

