
# debugging function
showvar = @echo \"'$(1)'\" =\> \"'$($(1))'\"
tbd = $(error "$@" not implemented yet)

# expands to the object files in the directories
objects=$(sort $(foreach type,c cc,$(call objects_$(type),$1)))
objects_c=$(patsubst %.c,%.o,$(wildcard $(addsuffix /*.c,$1)))
objects_cc=$(patsubst %.cc,%.o,$(wildcard $(addsuffix /*.cc,$1)))

# initialization
TARGETS:=
GARBAGE:=
DISTCLEAN:=
REALCLEAN:=
TESTS:=
NO_CONFIGURE_TARGETS:=

NO_CONFIGURE_TARGETS+=clean ChangeLog depend dust configure dist \
		distclean realclean

ifneq "$(filter-out $(NO_CONFIGURE_TARGETS),$(if $(MAKECMDGOALS),$(MAKECMDGOALS),default))" ""
-include config.mk
endif

TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libwvutils.so libwvutils.a
TARGETS += uniconf/daemon/uniconfdaemon

ifneq ("$(with_ogg)", "no")
ifneq ("$(with_vorbis)", "no")
TARGETS += libwvoggvorbis.so libwvoggvorbis.a
endif
ifneq ("$(with_speex)", "no")
TARGETS += libwvoggspeex.so libwvoggspeex.a
endif
endif

ifneq ("$(with_fftw)", "no")
TARGETS += libwvfft.so libwvfft.a
endif

ifneq ("$(with_qt)", "no")
TARGETS += libwvqt.so libwvqt.a
endif

ifneq ("$(with_gtk)", "no")
TARGETS += libwvgtk.so libwvgtk.a
endif

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGE += ChangeLog $(wildcard libwv*.so.*)

DISTCLEAN += autom4te.cache config.mk config.log config.status \
		include/wvautoconf.h config.cache

REALCLEAN += stamp-h.in configure include/wvautoconf.h.in

CPPFLAGS += -Iinclude -pipe
ARFLAGS = rs

DEBUG:=$(filter-out no,$(enable_debug))

# for O_LARGEFILE
CXXFLAGS=${CXXOPTS}
CFLAGS=${COPTS}
CXXFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

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

ifneq ("$(with_fam)", "no")
libwvstreams.so: -lfam
endif

ifneq ("$(with_gdbm)", "no")
libwvutils.so: -lgdbm
endif

ifeq ("$(enable_verbose)", "yes")
VERBOSE:=yes
endif

ifneq ("$(with_xplc)", "no")
CPPFLAGS+=-DUNSTABLE
ifneq ("$(with_xplc)", "yes")
VPATH+=$(with_xplc)
LDFLAGS+=-L$(with_xplc)
CPPFLAGS+=-I$(with_xplc)/include
libwvstreams.so: -lxplc -lxplc-cxx
endif
endif

ifneq ("$(with_pam)", "no")
libwvstreams.so: -lpam
endif


LDLIBS := $(LDLIBS) \
	$(shell $(CC) -lsupc++ 2>&1 | grep -q "undefined reference" \
		&& echo " -lsupc++")

RELEASE?=$(PACKAGE_VERSION)

include $(wildcard */vars.mk */*/vars.mk) /dev/null

libwvoggvorbis.a libwvoggvorbis.so: $(call objects,oggvorbis)
libwvoggvorbis.so: -logg -lvorbis -lvorbisenc libwvutils.so

libwvoggspeex.a libwvoggspeex.so: $(call objects,oggspeex)
libwvoggspeex.so: -logg -lspeex libwvutils.so

libwvfft.a libwvfft.so: $(call objects,fft)
libwvfft.so: -lfftw -lrfftw libwvutils.so

libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvutils.so libwvstreams.so

libwvgtk.a libwvgtk.so: $(call objects,gtk)
libwvgtk.so: -lgtk -lgdk libwvstreams.so libwvutils.so

libwvstreams.a libwvstreams.so: $(call objects,configfile crypto ipstreams linuxstreams streams uniconf urlget)
libwvstreams.so: libwvutils.so -lssl

libwvutils.a libwvutils.so: $(call objects,utils)
libwvutils.so: -lz -lcrypt

