TESTS+=utils/plugtests

.PHONY: utils/plugtests
utils/plugtests:
	$(MAKE) -C utils/plugtests all
