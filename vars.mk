
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

TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libwvutils.so libwvutils.a
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
  CPPFLAGS+=-DUNSTABLE
  ifneq ("$(with_xplc)", "yes")
    VPATH+=$(with_xplc)
    LDFLAGS+=-L$(with_xplc)
    CPPFLAGS+=-I$(with_xplc)/include
    libwvstreams.so: -lxplc -lxplc-cxx
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

