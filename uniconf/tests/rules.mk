
.PHONY: uniconf/tests
uniconf/tests: $(patsubst %.cc,%,$(wildcard uniconf/tests/*.cc))

