
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

TARGETS += libwvbase.so libwvbase.a
TARGETS += libwvutils.so libwvutils.a
TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libuniconf.so libuniconf.a
TARGETS += wvtestmain.o libwvtest.a
TARGETS += uniconf/daemon/uniconfd uniconf/tests/uni
GARBAGE += wvtestmain.o tmp.ini .wvtest-total

#ifneq ("$(with_swig)", "no")
#  ifneq ("$(with_tcl)", "no")
#    TARGETS += bindings/tcl
#    CPPFLAGS += -I/usr/include/tcl8.3
#  endif
#  ifneq ("$(with_python)", "no")
#    TARGETS += bindings/python
#    CPPFLAGS += -I/usr/include/python2.1
#  endif
#  ifneq ("$(with_php)", "no")
#    TARGETS += bindings/php
#    CPPFLAGS += `php-config --includes`
#  endif
#endif

ifneq ("$(with_qt)", "no")
  TARGETS += libwvqt.so libwvqt.a
endif

ifneq ("$(with_telephony)", "no")
  TARGETS += libwvtelephony.so libwvtelephony.a
endif

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGE += $(wildcard lib*.so.*)

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

ifeq ("$(enable_testgui)", "no")
WVTESTRUN=env
endif

ifeq ("$(enable_rtti)", "no")
CXXFLAGS+=-fno-rtti
endif

ifeq ("$(enable_exceptions)", "no")
CXXFLAGS+=-fno-exceptions
endif

ifeq ("$(enable_efence)", "yes")
LDLIBS+=-lefence
endif

ifneq ("$(with_bdb)", "no")
  libwvutils.so-LIBS+=-ldb
endif

ifneq ("$(with_qdbm)", "no")
  libwvutils.so-LIBS+=-L. -lqdbm
endif

libwvbase.so: LIBS+=-lxplc-cxx

ifneq ("$(with_openslp)", "no")
  libwvstreams.so: -lslp
endif

ifneq ("$(with_pam)", "no")
  libwvstreams.so: -lpam
endif

LDLIBS := $(LDLIBS) \
	$(shell $(CC) -lsupc++ -lgcc_eh 2>&1 | grep -q "undefined reference" \
		&& echo " -lsupc++ -lgcc_eh")

RELEASE?=$(PACKAGE_VERSION)

include $(filter-out xplc/% linuxstreams/%,$(wildcard */vars.mk */*/vars.mk)) \
	$(wildcard $(foreach dir,$(ARCH_SUBDIRS),$(dir)/*/vars.mk)) /dev/null

# LDFLAGS+=-z defs

BASEOBJS= \
	utils/wvbuffer.o utils/wvbufferstore.o \
	utils/wvcont.o \
	utils/wverror.o \
	streams/wvfdstream.o \
	utils/wvfork.o \
	utils/wvhash.o \
	utils/wvhashtable.o \
	utils/wvlinklist.o \
	utils/wvmoniker.o \
	utils/wvregex.o \
	utils/wvscatterhash.o utils/wvsorter.o \
	utils/wvstring.o utils/wvstringlist.o \
	utils/strutils.o \
	utils/wvtask.o \
	utils/wvtimeutils.o \
	utils/wvvector.o \
	streams/wvistreamlist.o \
	streams/wvlog.o \
	streams/wvstream.o \
	uniconf/uniconf.o uniconf/uniconf_c.o \
	uniconf/uniconfgen.o uniconf/uniconfkey.o uniconf/uniconfroot.o \
	uniconf/unihashtree.o \
	uniconf/unimountgen.o \
	uniconf/unitempgen.o \
	utils/wvbackslash.o \
	utils/wvencoder.o \
	utils/wvtclstring.o \
	utils/wvstringcache.o \
	uniconf/uniinigen.o \
	uniconf/unigenhack.o \
	uniconf/unilistiter.o \
	streams/wvfile.o \
	streams/wvstreamclone.o  \
	streams/wvconstream.o

TESTOBJS = \
	utils/wvtest.o \
	utils/wvtest_filecountprefix.o

# print the sizes of all object files making up libwvbase, to help find
# optimization targets.
basesize:
	size --total $(BASEOBJS)

micro: micro.o libwvbase.so

libwvbase.a libwvbase.so: $(filter-out uniconf/unigenhack.o,$(BASEOBJS))
libwvbase.a: uniconf/unigenhack_s.o
libwvbase.so: uniconf/unigenhack.o

libwvutils.a libwvutils.so: $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: libwvbase.so
libwvutils.so: -lz -lcrypt -lpopt

libwvstreams.a libwvstreams.so: $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: libwvutils.so libwvbase.so
libwvstreams.so: LIBS+=-lssl -lcrypto

libuniconf.a libuniconf.so: $(filter-out $(BASEOBJS), \
	$(call objects,uniconf))
libuniconf.a: uniconf/uniconfroot.o
libuniconf.so: libwvstreams.so libwvutils.so libwvbase.so

libwvtelephony.a libwvtelephony.so: $(call objects,telephony)
libwvtelephony.so: 

libwvtest.a: wvtestmain.o $(TESTOBJS)

ifeq ("$(wildcard /usr/lib/libqt-mt.so)", "/usr/lib/libqt-mt.so")
  libwvqt.so-LIBS+=-lqt-mt
else 
  # RedHat has a pkgconfig file we can use to sort out this mess..
  ifeq ("$(wildcard /usr/lib/pkgconfig/qt-mt.pc)", "/usr/lib/pkgconfig/qt-mt.pc")
    libwvqt.so-LIBS+=`pkg-config --libs qt-mt`
  else
    libwvqt.so-LIBS+=-lqt
  endif
endif
libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvutils.so libwvstreams.so

libwvgtk.a libwvgtk.so: $(call objects,gtk)
libwvgtk.so: -lgtk -lgdk libwvstreams.so libwvutils.so
