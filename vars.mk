
# debugging function
showvar = @echo \"'$(1)'\" =\> \"'$($(1))'\"
tbd = $(error "$@" not implemented yet)

# expands to the object files in the directories
objects=$(sort $(foreach type,c cc,$(call objects_$(type),$1)))
objects_c=$(patsubst %.c,%.o,$(wildcard $(addsuffix /*.c,$1)))
objects_cc=$(patsubst %.cc,%.o,$(wildcard $(addsuffix /*.cc,$1)))

# initialization
TARGETS:=
GARBAGES:=
DISTCLEAN:=
REALCLEAN:=
TESTS:=
NO_CONFIGURE_TARGETS:=

NO_CONFIGURE_TARGETS+=clean ChangeLog depend dust configure dist distclean realclean

ifneq "$(filter-out $(NO_CONFIGURE_TARGETS),$(if $(MAKECMDGOALS),$(MAKECMDGOALS),default))" ""
include config.mk
endif

TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libwvutils.so libwvutils.a

ifneq ("$(with_oggvorbis)", "no")
TARGETS += libwvoggvorbis.so libwvoggvorbis.a
endif

ifneq ("$(with_fftw)", "no")
TARGETS += libwvfft.so libwvfft.a
endif

ifneq ("$(with_qt)", "no")
TARGETS += libwvqt.so libwvqt.a
endif

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGES += ChangeLog

DISTCLEAN += autom4te.cache config.mk config.log config.status include/wvautoconf.h

REALCLEAN += stamp-h.in configure include/wvautoconf.h.in

CPPFLAGS += -Iinclude -pipe
ARFLAGS = rs

libwvstreams.so: LDFLAGS+=-lssl

libwvutils.so: LDFLAGS+=-lz -lcrypto

libwvoggvorbis.so: LDFLAGS+=-logg -lvorbis -lvorbisenc

libwvfft.so: LDFLAGS+=-lfftw -lrfftw

libwvqt.so: LDFLAGS+=-lqt

DEBUG:=$(filter-out no,$(enable_debug))

# for O_LARGEFILE
CXXFLAGS+=-D_GNU_SOURCE
CFLAGS+=-D_GNU_SOURCE

ifndef enable_debug
CXXFLAGS+=-g
CFLAGS+=-g
endif

ifdef DEBUG
CXXFLAGS+=-ggdb -DDEBUG$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))
CFLAGS+=-ggdb -DDEBUG$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))
endif

ifeq ("$(enable_debug)", "no")
#CXXFLAGS+=-fomit-frame-pointer
# -DNDEBUG is disabled because we like assert() to crash
#CXXFLAGS+=-DNDEBUG
#CFLAGS+=-DNDEBUG
endif

ifeq ("$(enable_fatal_warnings)", "yes")
CXXFLAGS+=-Werror
# FIXME: not for C, because our only C file, crypto/wvsslhack.c, has
#        a few warnings.
#CFLAGS+=-Werror
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

ifneq ("$(enable_rtti)", "yes")
CXXFLAGS+=-fno-rtti
endif

ifneq ("$(enable_exceptions)", "yes")
CXXFLAGS+=-fno-exceptions
endif

ifeq ("$(enable_efence)", "yes")
LDFLAGS+=-lefence
endif

ifeq ("$(enable_verbose)", "yes")
VERBOSE:=yes
endif

# Some other generally useful optimizations
CXXFLAGS+=-fnonnull-objects

RELEASE?=$(PACKAGE_VERSION)

include $(wildcard */vars.mk */*/vars.mk) /dev/null

