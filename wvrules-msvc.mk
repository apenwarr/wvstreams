LIBWVBASE=$(shell cygpath -m $(WVSTREAMS)/libwvwin32.a)
LIBWVUTILS=$(LIBWVBASE)
LIBWVSTREAMS=$(LIBWVBASE)
LIBUNICONF=$(LIBWVBASE)
LIBWVTEST=$(LIBWVBASE) $(shell cygpath -m $(WVSTREAMS)/wvtestmain.o)

define wvcc_base
	@rm -f "$1"
	$(COMPILE_MSG)$4 $5 $2 /Fo$1
	@# The Perl script here generates the proper dependencies, including
	@# null dependencies so Make doesn't complain
	$(DEPEND_MSG)gcc -MM $(DEPFLAGS) -E $< \
                | perl -we \
                '$$a = '"'"'$1'"'"'; \
                $$\ = $$/; \
                local $$/; \
                while (<>) { \
                    for (split(/(?<!\\)$$/m)) { \
                        s/^[^:]+:\s*/$$a: /; \
                        print; \
                        if (s/^$$a: //) { \
			    map {print "$$_:" unless m/^\\$$/} (split(/\s+/));\
                        } \
                    } \
                }' >$(DEPFILE)
endef

define wvlink_ar
	lib $2 /OUT:$1
endef

wvlink=$(LINK_MSG)$(CC) $(LDFLAGS) $($1-LDFLAGS) /Fe$1 $(filter %.o %.a %.so, $2) $($1-LIBS) $(LIBS) $(XX_LIBS) $(LDLIBS)
