
.PHONY: utils/tests
utils/tests: $(patsubst %.cc,%,$(wildcard utils/tests/*.cc))

