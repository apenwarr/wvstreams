# wvrules.mk:  2003 09 09
#
# Copyright (C) 1998-2003 by Avery Pennarun <apenwarr@worldvisions.ca>.
#   Use, distribute, modify, and redistribute freely.  (But if you're nice,
#   you'll send all your changes back to me.)
#
# This is a complicated-looking set of Makefile rules that should make your
# own Makefiles simpler, by providing for several useful features (like
# autodependencies and a 'clean' target) without any extra effort.
#
# It will only work with GNU make.
#

# if WVSTREAMS_SRC is set assume everything else is set.
# For packages that use WvStreams use WVSTREAMS_SRC=. for distribution.
ifeq ($(WVSTREAMS_SRC),)
  ifeq ($(WVSTREAMS),)
    $(error The WVSTREAMS variable is not defined)
  endif
  WVSTREAMS_SRC=$(WVSTREAMS)
  WVSTREAMS_LIB=$(WVSTREAMS)
  WVSTREAMS_INC=$(WVSTREAMS)/include
  WVSTREAMS_BIN=$(WVSTREAMS)
endif
export WVSTREAMS WVSTREAMS_SRC WVSTREAMS_LIB WVSTREAMS_INC WVSTREAMS_BIN

SHELL=/bin/bash

ifneq ($(wildcard $(WVSTREAMS_SRC)/config.mk),)
  include $(WVSTREAMS_SRC)/config.mk
endif

ifeq (${EXEEXT},.exe)
  include $(WVSTREAMS_SRC)/wvrules-win32.mk
endif

ifeq (${WVTESTRUN},)
  WVTESTRUN=$(WVSTREAMS_BIN)/wvtesthelper
endif

ifneq ("$(with_xplc)", "no")
ifneq ("$(with_xplc)", "")
  LDFLAGS:=-L$(with_xplc) $(LDFLAGS)
endif
  LIBXPLC=-lxplc-cxx -lxplc
endif

LIBWVBASE=$(WVSTREAMS_LIB)/libwvbase.so $(LIBXPLC)
LIBWVUTILS=$(WVSTREAMS_LIB)/libwvutils.so $(LIBWVBASE)
LIBWVSTREAMS=$(WVSTREAMS_LIB)/libwvstreams.so $(LIBWVUTILS)
LIBWVOGG=$(WVSTREAMS_LIB)/libwvoggvorbis.so $(LIBWVSTREAMS)
LIBUNICONF=$(WVSTREAMS_LIB)/libuniconf.so $(LIBWVSTREAMS)
LIBWVQT=$(WVSTREAMS_LIB)/libwvqt.so $(LIBWVSTREAMS)

#
# Initial C compilation flags
#
CPPFLAGS += $(CPPOPTS)
C_AND_CXX_FLAGS += -D_BSD_SOURCE -D_GNU_SOURCE $(OSDEFINE) \
		  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS += $(COPTS) $(C_AND_CXX_FLAGS) 
CXXFLAGS += $(CXXOPTS) $(C_AND_CXX_FLAGS)
LDFLAGS += $(LDOPTS) -L$(WVSTREAMS_LIB)

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

STRIP=strip --remove-section=.note --remove-section=.comment
#STRIP=echo

# macros that expand to the object files in the given directories
objects=$(sort $(foreach type,c cc,$(call objects_$(type),$1)))
objects_c=$(patsubst %.c,%.o,$(wildcard $(addsuffix /*.c,$1)))
objects_cc=$(patsubst %.cc,%.o,$(wildcard $(addsuffix /*.cc,$1)))

# macro that expands to the subdir.mk files to include
xsubdirs=$(sort $(wildcard $1/*/subdir.mk)) /dev/null

# we need a default rule, since the 'includes' below causes trouble
.PHONY: default all
default: all

# default "test" rule does nothing...
.PHONY: test runtests
test:
runtests:

%/test:
	$(MAKE) -C $(dir $@) test

$(LIBXPLC):

$(WVSTREAMS_SRC)/rules.local.mk:
	@true

-include $(WVSTREAMS_SRC)/rules.local.mk

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

INCFLAGS=$(addprefix -I,$(WVSTREAMS_INC) $(XPATH))
CPPFLAGS+=$(INCFLAGS)
CFLAGS+=$(CPPFLAGS)
CXXFLAGS+=$(CPPFLAGS)

ifeq ($(VERBOSE),1)
  COMPILE_MSG :=
  LINK_MSG :=
  DEPEND_MSG :=
  SYMLINK_MSG :=
else
  COMPILE_MSG = @echo compiling $@...;
  LINK_MSG = @echo linking $@...;
  #DEPEND_MSG = @echo "   depending $@...";
  DEPEND_MSG := @
  SYMLINK_MSG := @
endif

# any rule that depends on FORCE will always run
.PHONY: FORCE
FORCE:

ifeq ($(LN_S),)
LN_S := ln -s
endif
ifeq ($(LN),)
LN := ln
endif

# Create symbolic links
# usage: $(wvlns,source,dest)
wvlns=$(SYMLINK_MSG)$(LN_S) -f $1 $2

# Create hard links
# usage: $(wvln,source,dest)
wvln=$(SYMLINK_MSG)$(LN) -f $1 $2

# usage: $(wvcc_base,outfile,infile,stem,compiler cflags,mode)
#    eg: $(wvcc,foo.o,foo.cc,foo,$(CC) $(CFLAGS) -fPIC,-c)
DEPFILE = $(if $(filter %.o,$1),$(dir $1).$(notdir $(1:.o=.d)),/dev/null)
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
wvcc=$(call wvcc_base,$1,$2,$3,$(CC) $(CFLAGS) $($1-CPPFLAGS) $($1-CFLAGS) $4,$(if $5,$5,-c))
wvcxx=$(call wvcc_base,$1,$2,$3,$(CXX) $(CFLAGS) $(CXXFLAGS) $($1-CPPFLAGS) $($1-CFLAGS) $($1-CXXFLAGS) $4,$(if $5,$5,-c))

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
wvsoname=$(if $($1-SONAME),$($1-SONAME),$(if $(SONAME),$(SONAME),$1))
define wvlink_so
	$(LINK_MSG)$(CC) $(LDFLAGS) $($1-LDFLAGS) -Wl,-soname,$(call wvsoname,$1) -shared -o $1 $(filter %.o %.a %.so,$2) $($1-LIBS) $(LIBS) $(XX_LIBS)
	$(if $(filter-out $(call wvsoname,$1),$1),$(call wvlns,$1,$(call wvsoname,$1)))
endef

wvlink=$(LINK_MSG)$(CC) $(LDFLAGS) $($1-LDFLAGS) -o $1 $(filter %.o %.a %.so, $2) $($1-LIBS) $(LIBS) $(XX_LIBS) $(LDLIBS)

../%.so:;	@echo "Shared library $@ does not exist!"; exit 1
../%.a:;	@echo "Library $@ does not exist!"; exit 1
../%.o:;	@echo "Object $@ does not exist!"; exit 1
/%.a:;		@echo "Library $@ does not exist!"; exit 1

%.o: %.c;	$(call wvcc ,$@,$<,$*)
%.fpic.o: %.c;	$(call wvcc ,$@,$<,$*,-fPIC)
%.o: %.cc;	$(call wvcxx,$@,$<,$*)
%.fpic.o: %.cc;	$(call wvcxx,$@,$<,$*,-fPIC)
%.o: %.cpp;	$(call wvcxx,$@,$<,$*)
%.fpic.o:%.cpp; $(call wvcxx,$@,$<,$*,-fPIC)
%.s: %.c;	$(call wvcc ,$@,$<,$*,,-S)
%.s: %.cc;	$(call wvcxx,$@,$<,$*,,-S)
%.s: %.cpp;	$(call wvcxx,$@,$<,$*,,-S)
%.E: %.c;	$(call wvcc,$@,$<,$*,,-E)
%.E: %.cc;	$(call wvcxx,$@,$<,$*,,-E)
%.E: %.cpp;	$(call wvcxx,$@,$<,$*,,-E)

%.moc: %.h;	moc -o $@ $<

%: %.o;		$(call wvlink,$@,$^) 
%.t: %.t.o;	$(call wvlink,$@,$(WVSTREAMS_SRC)/wvtestmain.o $(call reverse,$(filter %.o,$^)) $(filter-out %.o,$^) $(LIBWVUTILS))
%.a %.libs:;	$(call wvlink_ar,$@,$^)
%.so:;		$(call wvlink_so,$@,$^)

# Force objects to be built before final binaries	
$(addsuffix .o,$(basename $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp))):

%.gz: FORCE %
	@rm -f $@
	gzip -f $*
	@ls -l $@

#
# Header files for tcl/tk packages
#
pkgIndex.tcl: $(filter-out pkgIndex.tcl,$(wildcard *.tcl))
	@echo Generating pkgIndex.tcl...
	@rm -f $@
	@echo pkg_mkIndex . \
		$$(echo $^ | sed 's,\.tcl_paths,,') | tclsh

pkgIndex.tcl $(wildcard *.tcl): .tcl_paths
.tcl_paths:
	@echo Generating .tcl_paths...
	@rm -f $@
	@find . $(TOPDIR) -name '*.tcl' -printf '%h\n' | sort | uniq | \
		(echo lappend auto_path \\; sed 's/^.*$$/	& \\/'; echo) >$@.tmp
	@mv $@.tmp $@

#
# We automatically generate header dependencies for .c and .cc files.  The
# dependencies are stored in the file ".filename.d"
#
depfiles_sf = $(wildcard .*.d t/.*.d)

ifneq ($(depfiles_sf),)
-include $(depfiles_sf)
endif


#
# A macro for compiling subdirectories listed in the SUBDIRS variable.
# Tries to make the target ($@) in each subdir, unless the target is called
# "subdirs" in which case it makes "all" in each subdir.
#
define subdirs_func
	+@OLDDIR="$$(pwd)"; set -e; \
	for d in __fx__ $2; do \
		if [ "$$d" = "__fx__" ]; then continue; fi; \
		cd "$$d"; \
		echo ; \
		echo "--> Making $1 in $$(pwd)..."; \
		$(MAKE) --no-print-directory $1 || exit 1; \
		cd "$$OLDDIR"; \
	done
	@echo
	@echo "--> Back in $$(pwd)..."
endef

subdirs = $(call subdirs_func,$(subst subdirs,all,$(if $1,$1,$@)),$(if $2,$2,$(SUBDIRS)))

# # $(call reverse,$(SUBDIRS)) works since GNU make 3.80 only
# reverse = \
# 	$(if $(1),$(call reverse,$(wordlist 2, 999, $(1))) $(firstword $(1)))

define shell_reverse
	revlist="" ; \
	for word in $(1) ; do \
		revlist="$${word} $${revlist}"; \
	done ; \
	echo "$${revlist}"
endef

reverse = $(shell $(call shell_reverse,$(1)))

clean_subdirs = $(call subdirs,clean,$(call reverse,$(SUBDIRS)),keep)

%: %/Makefile FORCE
	@cd "$@"; echo; echo "--> Making all in $$(pwd)..."; \
		$(MAKE) --no-print-directory all

subdirs: ${SUBDIRS}

#
# Auto-clean rule.  Feel free to append to this in your own directory, by
# defining your own "clean" rule.
#
.PHONY: clean _wvclean

clean: _wvclean

_wvclean:
	@echo '--> Cleaning $(shell pwd)...'
	@rm -f *~ *.tmp *.o *.a *.so *.so.* *.libs *.moc *.d .*.d .depend \
		 .\#* .tcl_paths pkgIndex.tcl gmon.out core build-stamp \
		 wvtestmain
	@rm -f $(patsubst %.t.cc,%.t,$(wildcard *.t.cc) $(wildcard t/*.t.cc)) \
		t/*.o t/*~ t/.*.d t/.\#*
	@rm -f semantic.cache tags
	@rm -rf debian/tmp

#
# default dist rules.
distclean: clean

PKGNAME := $(notdir $(shell pwd))
PPKGNAME := $(shell echo $(PKGNAME) | tr a-z A-Z | tr - _)
PKGVER := $(shell test -f wvver.h \
	    && cat wvver.h | sed -ne "s/\#define $(PPKGNAME)_VER_STRING.*\"\([^ ]*\).*\"/\1/p")
ifneq ($(PKGVER),)
PKGDIR := $(PKGNAME)-$(PKGVER)
else
PKGDIR := $(PKGNAME)
endif
ifneq ($(PKGSNAPSHOT),)
PKGDIR := $(PKGDIR)+$(shell date +%Y%m%d)
endif
dist-dir:
	@echo $(PKGDIR)

dist-hook:

dist: dist-hook ChangeLog
	@echo '--> Making dist in ../build/$(PKGDIR)...'
	@test -d ../build || mkdir ../build
	@rsync -a --delete --force '$(shell pwd)/' '../build/$(PKGDIR)'
	@find '../build/$(PKGDIR)' -name CVS -type d -print0 | xargs -0 rm -rf --
	@find '../build/$(PKGDIR)' -name .cvsignore -type f -print0 | xargs -0 rm -f --
	@$(MAKE) -C '../build/$(PKGDIR)' distclean
	@rm -f '../build/$(PKGDIR).tar.gz'
	@cd ../build; tar -zcf '$(PKGDIR).tar.gz' '$(PKGDIR)'
	@echo '--> Created tarball in ../build/$(PKGDIR).tar.gz.'

ChangeLog: FORCE
	@echo '--> Generating ChangeLog from CVS...'
	@rm -f ChangeLog ChangeLog.bak
	@cvs2cl --utc

#
# Make 'tags' file using the ctags program - useful for editing
#
#tags: $(shell find -name '*.cc' -o -name '*.[ch]')
#	@echo '(creating "tags")'
#	@if [ -x /usr/bin/ctags ]; then /usr/bin/ctags $^; fi
