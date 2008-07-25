LIBWVBASE=$(WVSTREAMS_LIB)/libwvbase.so $(LIBXPLC)
LIBWVUTILS=$(WVSTREAMS_LIB)/libwvutils.so $(LIBWVBASE)
LIBWVSTREAMS=$(WVSTREAMS_LIB)/libwvstreams.so $(LIBWVUTILS)
LIBWVOGG=$(WVSTREAMS_LIB)/libwvoggvorbis.so $(LIBWVSTREAMS)
LIBUNICONF=$(WVSTREAMS_LIB)/libuniconf.so $(LIBWVSTREAMS)
LIBWVDBUS=$(WVSTREAMS_LIB)/libwvdbus.so $(LIBWVSTREAMS)
LIBWVQT=$(WVSTREAMS_LIB)/libwvqt.so $(LIBWVSTREAMS)
LIBWVTEST=$(WVSTREAMS_LIB)/libwvtest.a $(LIBWVUTILS)

#
# Initial C compilation flags
#
INCFLAGS=$(addprefix -I,$(WVSTREAMS_INC) $(XPATH))

CPPFLAGS += $(CPPOPTS)
CFLAGS += $(COPTS)
CXXFLAGS += $(CXXOPTS)
LDFLAGS += $(LDOPTS) -L$(WVSTREAMS_LIB)

# Default compiler we use for linking
WVLINK_CC = $(CXX)

ifeq ("$(enable_debug)", "yes")
  DEBUG:=1
else
  DEBUG:=0
endif

ifneq ("$(enable_optimization)", "no")
  CXXFLAGS+=-O2
  #CXXFLAGS+=-felide-constructors
  CFLAGS+=-O2
endif

ifneq ("$(enable_warnings)", "no")
  CXXFLAGS+=-Wall -Woverloaded-virtual
  CFLAGS+=-Wall
endif

ifeq ($(DEBUG),1)
  CFLAGS += -ggdb -DDEBUG=1
  CXXFLAGS += -ggdb -DDEBUG=1
  LDFLAGS += -ggdb
else
  CFLAGS += -DDEBUG=0
  CXXFLAGS += -DDEBUG=0
  #CFLAGS += -DNDEBUG    # I don't like disabling assertions...
  #CFLAGS += -fomit-frame-pointer  # really evil
  #CXXFLAGS += -fno-implement-inlines  # causes trouble with egcs 1.0
  LDFLAGS += 
endif

ifeq ($(PROFILE),1)
  CFLAGS += -pg
  LDFLAGS += -pg
endif

ifeq ($(STATIC),1)
  LDFLAGS += -static
endif

define wvlink_ar
	$(LINK_MSG)set -e; rm -f $1 $(patsubst %.a,%.libs,$1); \
	echo $2 >$(patsubst %.a,%.libs,$1); \
	$(AR) q $1 $(filter %.o,$2); \
	for d in "" $(filter %.libs,$2); do \
	    if [ "$$d" != "" ]; then \
			cd $$(dirname "$$d"); \
			$(AR) q $(shell pwd)/$1 $$(cat $$(basename $$d)); \
			cd $(shell pwd); \
		fi; \
	done; \
	$(AR) s $1
endef

define wvcc_base
	@rm -f "$1"
	$(COMPILE_MSG)$4 $5 $2 -o $1
	@# The Perl script here generates the proper dependencies, including
	@# null dependencies so Make doesn't complain
	$(DEPEND_MSG)$4 -M -E $< \
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
	$(LINK_MSG)set -e; rm -f $1 $(patsubst %.a,%.libs,$1); \
	echo $2 >$(patsubst %.a,%.libs,$1); \
	$(AR) q $1 $(filter %.o,$2); \
	for d in "" $(filter %.libs,$2); do \
	    if [ "$$d" != "" ]; then \
			cd $$(dirname "$$d"); \
			$(AR) q $(shell pwd)/$1 $$(cat $$(basename $$d)); \
			cd $(shell pwd); \
		fi; \
	done; \
	$(AR) s $1
endef

wvlink=$(LINK_MSG)$(CC) $(LDFLAGS) $($1-LDFLAGS) -o $1 $(filter %.o %.a %.so, $2) $($1-LIBS) $(LIBS) $(XX_LIBS) $(LDLIBS)
