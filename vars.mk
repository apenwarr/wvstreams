
# debugging function
showvar = @echo \"'$(1)'\" =\> \"'$($(1))'\"
tbd = $(error "$@" not implemented yet)

# include function
doinclude = $(wildcard $(2:%=%/)*/$(1)) /dev/null

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

GARBAGES += ChangeLog

DISTCLEAN += autom4te.cache config.mk config.log config.status include/wvautoconf.h

REALCLEAN += stamp-h.in configure include/wvautoconf.h.in

CPPFLAGS += -MD -Iinclude -pipe
ARFLAGS = rs

libwvstreams.so-OBJECTS:=
libwvutils.so-OBJECTS:=

libwvstreams.so-LIBS:=-lssl
libwvutils.so-LIBS:=-lz -lcrypto

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

RELEASE?=$(PACKAGE_VERSION)

include $(call doinclude,vars.mk)

