
.PHONY: configfile/tests
configfile/tests: $(patsubst %.cc,%,$(wildcard configfile/tests/*.cc))

