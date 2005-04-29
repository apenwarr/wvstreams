LIBWVBASE=$(WVSTREAMS_LIB)/libwvbase.so $(LIBXPLC)
LIBWVUTILS=$(WVSTREAMS_LIB)/libwvutils.so $(LIBWVBASE)
LIBWVSTREAMS=$(WVSTREAMS_LIB)/libwvstreams.so $(LIBWVUTILS)
LIBWVOGG=$(WVSTREAMS_LIB)/libwvoggvorbis.so $(LIBWVSTREAMS)
LIBUNICONF=$(WVSTREAMS_LIB)/libuniconf.so $(LIBWVSTREAMS)
LIBWVQT=$(WVSTREAMS_LIB)/libwvqt.so $(LIBWVSTREAMS)
LIBWVTEST=$(WVSTREAMS_LIB)/libwvtest.a $(LIBWVUTILS)

#
# Initial C compilation flags
#
INCFLAGS=$(addprefix -I,$(WVSTREAMS_INC) $(XPATH))

CPPFLAGS += $(CPPOPTS)
C_AND_CXX_FLAGS += -D_BSD_SOURCE -D_GNU_SOURCE $(OSDEFINE) \
		  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS += $(COPTS) $(C_AND_CXX_FLAGS) 
CXXFLAGS += $(CXXOPTS) $(C_AND_CXX_FLAGS)
LDFLAGS += $(LDOPTS) -L$(WVSTREAMS_LIB)

# Default compiler we use for linking
WVLINK_CC = gcc

# FIXME: what does this do??
XX_LIBS := $(XX_LIBS) $(shell $(CC) -lsupc++ -lgcc_eh 2>&1 | grep -q "undefined reference" && echo " -lsupc++ -lgcc_eh")

ifeq ("$(enable_debug)", "yes")
  DEBUG:=1
else
  DEBUG:=0
endif

ifeq ("$(enable_fatal_warnings)", "yes")
  CXXFLAGS+=-Werror
  # FIXME: not for C, because our only C file, crypto/wvsslhack.c, has
  #        a few warnings on purpose.
  #CFLAGS+=-Werror
endif

ifneq ("$(enable_optimization)", "no")
  CXXFLAGS+=-O2
  #CXXFLAGS+=-felide-constructors
  CFLAGS+=-O2
endif

ifneq ("$(enable_warnings)", "no")
#WLACH:FIXME: Conditional on using MSVC
#  CXXFLAGS+=-Wall -Woverloaded-virtual
#  CFLAGS+=-Wall
endif

ifneq ("$(enable_rtti)", "yes")
  CXXFLAGS+=-fno-rtti
endif

ifneq ("$(enable_exceptions)", "yes")
  CXXFLAGS+=-fno-exceptions
endif

ifeq ("$(enable_efence)", "yes")
  EFENCE:=-lefence
  USE_EFENCE:=1
endif

ifeq (USE_EFENCE,1)
  LDLIBS+=$(EFENCE)
endif

ifeq ("$(enable_verbose)", "yes")
  VERBOSE:=1
endif

ifdef DONT_LIE
  VERBOSE:=1 $(warning DONT_LIE is deprecated, use VERBOSE instead)
endif

#
# Figure out which OS we're running (for now, only picks out Linux or BSD)
#
OS:=$(shell uname -a | awk '{print $$1}' | sed -e 's/^.*BSD/BSD/g' )

#
# (Just BSD and LINUX clash with other symbols, so use ISLINUX and ISBSD)
# This sucks.  Use autoconf for most things!
#
ifeq ($(OS),Linux)
  OSDEFINE:=-DISLINUX
endif

ifeq ($(OS),BSD)
  OSDEFINE:=-DISBSD
endif

ifeq ($(CCMALLOC),1)
 ifeq ($(DEBUG),1)
   XX_LIBS += -lccmalloc -ldl
 endif
endif

ifeq ($(DEBUG),1)
  C_AND_CXX_FLAGS += -ggdb -DDEBUG=1
  LDFLAGS += -ggdb
else
  C_AND_CXX_FLAGS += -g -DDEBUG=0
  #CFLAGS += -DNDEBUG    # I don't like disabling assertions...
  #CFLAGS += -fomit-frame-pointer  # really evil
  #CXXFLAGS += -fno-implement-inlines  # causes trouble with egcs 1.0
  LDFLAGS += -g
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
