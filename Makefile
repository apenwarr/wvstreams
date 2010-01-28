
default:

pristine:
	rm -rf $(shell readlink -n lib)
	rm -f lib configure.real include/wvautoconf.h.in

%:
	@if [ ! -d lib ]; then \
		echo "Not configured!  Please run ./configure before running make"; \
		exit 1; \
	else \
		$(MAKE) -C lib $@; \
	fi;
