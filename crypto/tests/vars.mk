
TESTS+=$(patsubst %.cc,%,$(wildcard crypto/tests/*.cc))

crypto/tests/certtest: LDFLAGS+=-lssl
crypto/tests/cryptotest: LDFLAGS+=-lssl
crypto/tests/md5test: LDFLAGS+=-lssl
crypto/tests/reqtest: LDFLAGS+=-lssl
crypto/tests/ssltest: LDFLAGS+=-lssl
crypto/tests/sslsrvtest: LDFLAGS+=-lssl

