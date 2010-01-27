
default:

%:
	@if [ ! -d lib ]; then \
		echo "Not configured!  Please run ./configure before running make"; \
		exit 1; \
	else \
		make -C lib $@; \
	fi;
