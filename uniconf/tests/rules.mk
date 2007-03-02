
.PHONY: uniconf/tests
uniconf/tests: $(patsubst %.cc,%,$(wildcard uniconf/tests/*.cc))

%: %.in
	@sed -e 's/#VERSION#/$(PACKAGE_VERSION)/g' < $< > $@
