
# debugging function
showvar = @echo \"'$(1)'\" =\> \"'$($(1))'\"
tbd = $(error "$@" not implemented yet)

# initialization
TARGETS:=
GARBAGE:=
DISTCLEAN:=
REALCLEAN:=
TESTS:=
NO_CONFIGURE_TARGETS:=

NO_CONFIGURE_TARGETS+=clean ChangeLog depend dust configure dist \
		distclean realclean

TARGETS += xplc-stamp
TARGETS += libwvutils.so libwvutils.a
TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libuniconf.so libuniconf.a
GARBAGE += wvtestmain.o

ifneq ("$(with_swig)", "no")
  ifneq ("$(with_tcl)", "no")
    TARGETS += libuniconf_tcl.so
    CPPFLAGS += -I/usr/include/tcl8.3
  endif
endif

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

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGE += ChangeLog $(wildcard lib*.so.*)

DISTCLEAN += autom4te.cache config.mk config.log config.status \
		include/wvautoconf.h config.cache reconfigure

REALCLEAN += stamp-h.in configure include/wvautoconf.h.in

CPPFLAGS += -Iinclude -pipe
ARFLAGS = rs

DEBUG:=$(filter-out no,$(enable_debug))

# for O_LARGEFILE
CXXFLAGS+=${CXXOPTS}
CFLAGS+=${COPTS}
CXXFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

ifeq ($(DEBUG),)
CXXFLAGS+=-g
CFLAGS+=-g
endif

ifneq ($(DEBUG),)
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
LDLIBS+=-lefence
endif

ifneq ("$(with_fam)", "no")
  libwvstreams.so: -lfam
endif

ifneq ("$(with_gdbm)", "no")
  libwvutils.so: -lgdbm
endif

ifneq ("$(with_bdb)", "no")
  libwvutils.so-LIBS+=-ldb
endif

ifneq ("$(with_xplc)", "no")
  #CPPFLAGS+=-DUNSTABLE
  ifneq ("$(with_xplc)", "yes")
    #VPATH+=$(with_xplc)
    #LDFLAGS+=-L$(with_xplc)
    #CPPFLAGS+=-I$(with_xplc)/include
    #libwvstreams.so: -lxplc -lxplc-cxx
  endif
endif

ifneq ("$(with_fam)", "no")
  libwvstreams.so: -lfam
endif

ifneq ("$(with_pam)", "no")
  libwvstreams.so: -lpam
endif

LDLIBS := -lgcc $(LDLIBS) \
	$(shell $(CC) -lsupc++ 2>&1 | grep -q "undefined reference" \
		&& echo " -lsupc++")

RELEASE?=$(PACKAGE_VERSION)

include $(wildcard */vars.mk */*/vars.mk) /dev/null

libwvutils.a libwvutils.so: $(call objects,utils)
libwvutils.so: -lz -lcrypt

libwvstreams.a libwvstreams.so: $(call objects,configfile crypto ipstreams linuxstreams streams urlget)
libwvstreams.so: libwvutils.so -lssl -lcrypto

libuniconf.a libuniconf.so: $(call objects,uniconf)
libuniconf.so: libwvstreams.so libwvutils.so

libwvoggvorbis.a libwvoggvorbis.so: $(call objects,oggvorbis)
libwvoggvorbis.so: -logg -lvorbis -lvorbisenc libwvutils.so

libwvoggspeex.a libwvoggspeex.so: $(call objects,oggspeex)
libwvoggspeex.so: -logg -lspeex libwvutils.so

libwvfft.a libwvfft.so: $(call objects,fft)
libwvfft.so: -lfftw -lrfftw libwvutils.so

ifeq ("$(wildcard /usr/lib/libqt-mt.so)", "/usr/lib/libqt-mt.so")
  libwvqt.so-LIBS+=-lqt-mt
else
  libwvqt.so-LIBS+=-lqt
endif
libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvutils.so libwvstreams.so

libwvgtk.a libwvgtk.so: $(call objects,gtk)
libwvgtk.so: -lgtk -lgdk libwvstreams.so libwvutils.so

libuniconf_tcl.so: bindings/uniconf_tcl.o -ltcl8.3 -luniconf

xplc-stamp: $(wildcard $(with_xplc)/libxplc.*)
	rm -f xplc-stamp
	rm -f libxplc.a libxplc.so* libxplc-cxx.a libxplc-cxx.so*
	if [ "$(with_xplc)" != "no" -a -n "$(with_xplc)" ]; then \
		ldconfig -n "$(with_xplc)"; \
		ln -s $(with_xplc)/libxplc-cxx.* .; \
		ln -s $(with_xplc)/libxplc.* .; \
	fi
	touch xplc-stamp
