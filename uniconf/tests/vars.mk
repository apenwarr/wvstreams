
TESTS+=$(patsubst %.cc,%,$(wildcard uniconf/tests/*.cc)) uniconf/tests/uni

DISTCLEAN+=uniconf/tests/uni
