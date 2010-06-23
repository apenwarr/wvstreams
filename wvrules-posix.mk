ifdef _WIN32
  LIBWVSTATIC=$(WVSTREAMS_LIB)libwvstatic.a
  LIBWVBASE=$(LIBWVSTATIC)
  LIBWVUTILS=$(LIBWVSTATIC)
  LIBWVSTREAMS=$(LIBWVSTATIC)
  LIBUNICONF=$(LIBWVSTATIC)
  LIBWVDBUS=$(LIBWVSTATIC)
  LIBWVQT=$(LIBWVSTATIC)
  LIBWVTEST=$(WVSTREAMS_LIB)libwvtest.a $(LIBWVUTILS)
else
  LIBWVSTATIC=$(WVSTREAMS_LIB)libwvstatic.a
  LIBWVBASE=$(WVSTREAMS_LIB)libwvbase.so
  LIBWVUTILS=$(WVSTREAMS_LIB)libwvutils.so $(LIBWVBASE)
  LIBWVSTREAMS=$(WVSTREAMS_LIB)libwvstreams.so $(LIBWVUTILS)
  LIBUNICONF=$(WVSTREAMS_LIB)libuniconf.so $(LIBWVSTREAMS)
ifneq ("$(with_dbus)", "no")
  LIBWVDBUS=$(WVSTREAMS_LIB)libwvdbus.so $(LIBWVSTREAMS)
endif
ifneq ("$(with_qt)", "no")
  LIBWVQT=$(WVSTREAMS_LIB)libwvqt.so $(LIBWVSTREAMS)
endif
  LIBWVTEST=$(WVSTREAMS_LIB)libwvtest.a $(LIBWVUTILS)
endif

#
# Initial C compilation flags
#
INCFLAGS=$(addprefix -I,$(WVSTREAMS_INC) $(XPATH))

CPPFLAGS += $(CPPOPTS)
CFLAGS += $(COPTS)
CXXFLAGS += $(CXXOPTS)
LDFLAGS += $(LDOPTS) -L$(if $(WVSTREAMS_LIB),$(WVSTREAMS_LIB),.)

# Default compiler we use for linking
WVLINK_CC = $(CXX)

ifneq ("$(enable_optimization)", "no")
  CXXFLAGS+=-O2
  CFLAGS+=-O2
endif

ifneq ("$(enable_warnings)", "no")
  CXXFLAGS+=-Wall -Woverloaded-virtual
  CFLAGS+=-Wall
endif

DEBUG:=$(filter-out no 0,$(enable_debug))
ifdef DEBUG
  CPPFLAGS += -ggdb -DDEBUG=1 $(patsubst %,-DDEBUG_%,$(DEBUG))
  LDFLAGS += -ggdb
else
  CPPFLAGS += -DDEBUG=0
  LDFLAGS += 
endif

define wvlink_ar
	$(LINK_MSG)set -e; rm -f $1 $(patsubst %.a,%.libs,$1); \
	echo $(patsubst $O%,%,$2 $($1-EXTRA)) >$(patsubst %.a,%.libs,$1); \
	$(AR) q $1 $(filter %.o,$2 $($1-EXTRA)); \
	echo $(patsubst %,$(shell pwd)/%,$(filter %.o,$2 $($1-EXTRA))) \
	     >$(patsubst %.a,%.list,$1); \
	for d in $(filter %.libs,$2 $($1-EXTRA)); do \
	    cd $$(dirname "$$d"); \
	    for c in $$(cat $$(basename "$$d")); do \
		if echo $$c | grep -q "\.list$$"; then \
		    for i in $$(cat $$c); do \
			$(AR) q $(shell pwd)/$1 $$i; \
			echo $$i >>$(patsubst %.a,$(shell pwd)/%.list,$1); \
		    done; \
		else \
		    $(AR) q $(shell pwd)/$1 $$c; \
		    echo $$(pwd)/$$c >>$(patsubst %.a,$(shell pwd)/%.list,$1); \
		fi; \
	    done; \
	    cd $(shell pwd); \
	done; \
	for l in $(filter %.list,$2 $($1-EXTRA)); do \
	    for i in $$(cat $$l | grep -vE '__.SYMDEF|SORTED'); do \
		$(AR) q $1 $$i; \
		echo $$i >>$(patsubst %.a,%.list,$1); \
	    done; \
	done; \
	$(AR) s $1
endef

wvlink=$(LINK_MSG)$(WVLINK_CC) $(LDFLAGS) $($1-LDFLAGS) -o $1 $(filter %.o %.a %.so, $2) $($1-LIBS) $(XX_LIBS) $(LDLIBS) $(PRELIBS) $(LIBS)
