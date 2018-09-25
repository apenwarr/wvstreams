# wvrules.mk
#
# Copyright (C) 1998-2007 by Avery Pennarun <apenwarr@alumnit.ca>
#   and contributors.
#   Use, distribute, modify, and redistribute freely.  (But if you're nice,
#   you'll send all your changes back to me.)
#
# This is a complicated-looking set of Makefile rules that should make your
# own Makefiles simpler, by providing for several useful features (like
# autodependencies and a 'clean' target) without any extra effort.
#
# It will only work with GNU make.
#

# we need a default rule, since the 'includes' below can cause trouble
.PHONY: default all
default: all

all:

# if WVSTREAMS_SRC is set assume everything else is set.
# For packages that use WvStreams use WVSTREAMS_SRC=. for distribution.
ifneq ($(WVSTREAMSOBJ),)
  WVSTREAMS_LIB=$(WVSTREAMSOBJ)/
  WVSTREAMS_BIN=$(WVSTREAMSOBJ)
  WVSTREAMS_INC=$(WVSTREAMSOBJ)/include
  WVSTREAMS_OUT=$(WVSTREAMSOBJ)
else
  WVSTREAMS_LIB=
  WVSTREAMS_BIN=.
  WVSTREAMS_INC=./include
  WVSTREAMS_OUT=.
endif
ifneq ($(WVSTREAMS),)
  WVSTREAMS_SRC=$(WVSTREAMS)
  WVSTREAMS_INC+=$(WVSTREAMS)/include
endif
export WVSTREAMS WVSTREAMSOBJD WVSTREAMS_SRC WVSTREAMS_LIB WVSTREAMS_INC WVSTREAMS_BIN

SHELL=/bin/bash

include $(WVSTREAMS_SRC)/config.defaults.mk
-include $(WVSTREAMS_OUT)/config.mk
-include $(WVSTREAMS_SRC)/config.overrides.mk
-include $(WVSTREAMS_SRC)/local.mk

ifeq (${OS},SOLARIS)
  _SOLARIS= _SOLARIS
  AR=gar
endif

ifeq (${OS},MACOS)
  _MACOS=_MACOS
endif

ifeq (${OS},WIN32)
  _WIN32=_WIN32
endif

ifdef _WIN32
  XPATH += $(WVSTREAMS)/win32 $(WVSTREAMS)/win32/cominclude
  AR=i586-mingw32msvc-ar
  LIBS += -lssl -lcrypto -lz -lole32 -lrpcrt4 -lwsock32 -lgdi32 -limagehlp \
  	  -lstdc++
else
  CFLAGS += -fPIC
  CXXFLAGS += -fPIC
endif

include $(WVSTREAMS_SRC)/wvrules-$(COMPILER_STANDARD).mk

ifeq (${WVTESTRUN},)
  WVTESTRUN=$(WVSTREAMS_SRC)/wvtestrun
endif

# macros that expand to the object files in the given directories
objects=$(sort $(foreach type,c cc,$(call objects_$(type),$1)))
objects_c=$(filter-out $(WV_EXCLUDES), \
		$(patsubst $(WVSTREAMS)/%.c,%.o,$(wildcard $(addprefix $(WVSTREAMS)/,$(addsuffix /*.c,$1)))))
objects_cc=$(filter-out $(WV_EXCLUDES), \
		$(patsubst $(WVSTREAMS)/%.cc,%.o,$(wildcard $(addprefix $(WVSTREAMS)/,$(addsuffix /*.cc,$1)))))
tests_cc=$(filter-out $(WV_EXCLUDES), \
		$(patsubst $(WVSTREAMS)/%.cc,%,$(wildcard $(addprefix $(WVSTREAMS)/,$(addsuffix /*.cc,$1)))))

# default "test" rule does nothing...
.PHONY: test runtests
test:
runtests:

%/test:
	$(MAKE) -C $(dir $@) test

INCFLAGS=$(addprefix -I,$(WVSTREAMS_INC) $(XPATH))
CPPFLAGS+=$(INCFLAGS) \
	-D_DEFAULT_SOURCE -D_GNU_SOURCE $(OSDEFINE) \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	-DUNSTABLE

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

# usage: $(wvcc,outfile,infile,stem,extra_cflags,mode)
#    eg: $(wvcc,foo.o,foo.cc,foo,-fPIC,-c)

define compile
	$(COMPILE_MSG)$($(if $(subst C,,$6),$6,CC)) $(if $5,$5,-c) -o $1 $2 \
	  -MMD -MF \
	  $(shell dirname $1)/.$(shell basename $1 .o).d -MP -MQ $1 \
	  $(CPPFLAGS) $($6FLAGS) $($1-$6FLAGS) \
	  $($1-CPPFLAGS) $4
endef

define wvcc
	$(call compile,$1,$2,$3,$4,$5,C)
endef

define wvcxx
	$(call compile,$1,$2,$3,$4,$5,CXX)
endef

%.so: SONAME=$@$(if $(SO_VERSION),.$(SO_VERSION))

wvsoname=$(if $($1-SONAME),$($1-SONAME),$(if $(SONAME),$(SONAME),$1))

ifdef _WIN32
  define wvlink_so
	@echo "Skipping $@ on win32 (can't build shared libraries)"
  endef
else ifdef _SOLARIS
  define wvlink_so
	$(LINK_MSG)$(WVLINK_CC) $(LDFLAGS) $($1-LDFLAGS) -shared -o $1 $(filter %.o %.a %.so,$2) $($1-LIBS) $(LIBS) $(XX_LIBS)
  endef
else ifdef _MACOS
  define wvlink_so
	$(LINK_MSG)$(WVLINK_CC) $(LDFLAGS) $($1-LDFLAGS) -dynamiclib -o $1 $(filter %.o %.a %.so,$2) $($1-LIBS) $(LIBS) $(XX_LIBS)
  endef
else
  define wvlink_so
	$(LINK_MSG)$(WVLINK_CC) $(LDFLAGS) $($1-LDFLAGS) -Wl,-z,defs -Wl,-soname,$(call wvsoname,$1) -shared -o $1 $(filter %.o %.a %.so,$2) $($1-LIBS) $(LIBS) $(XX_LIBS)
	$(if $(filter-out $(call wvsoname,$1),$1),$(call wvlns,$1,$(call wvsoname,$1)))
  endef
endif

/%.a:;		@echo "Library $@ does not exist!"; exit 1

VPATH=$(WVSTREAMS)/

$O%.o: %.c;		$(call wvcc ,$@,$<,$*)
$O%.fpic.o: %.c;	$(call wvcc ,$@,$<,$*,-fPIC)
$O%.o: %.cc;		$(call wvcxx,$@,$<,$*)
$O%.fpic.o: %.cc;	$(call wvcxx,$@,$<,$*,-fPIC)
$O%.o: %.cpp;		$(call wvcxx,$@,$<,$*)
$O%.fpic.o: %.cpp;	$(call wvcxx,$@,$<,$*,-fPIC)
$O%.s: %.c;		$(call wvcc ,$@,$<,$*,,-S)
$O%.s: %.cc;		$(call wvcxx,$@,$<,$*,,-S)
$O%.s: %.cpp;		$(call wvcxx,$@,$<,$*,,-S)
$O%.E: %.c;		$(call wvcc,$@,$<,$*,,-E)
$O%.E: %.cc;		$(call wvcxx,$@,$<,$*,,-E)
$O%.E: %.cpp;		$(call wvcxx,$@,$<,$*,,-E)

$O%.moc: %.h;	$(MOC) -o $@ $<

%: %.o;		$(call wvlink,$@,$^) 
%.t: %.t.o;	$(call wvlink,$@,$(call reverse,$(filter %.o,$^)) $(filter-out %.o,$^) $(LIBWVTEST))
%.a %.libs:;	$(call wvlink_ar,$@,$^)
%.so:;		$(call wvlink_so,$@,$^)

# Force objects to be built before final binaries	
$(addsuffix .o,$(basename $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp))):

%.gz: FORCE %
	@rm -f $@
	gzip -f $*
	@ls -l $@

#
# We automatically generate header dependencies for .c and .cc files.  The
# dependencies are stored in the file ".filename.d", and we include them
# automatically here if they exist.
#
-include $(shell find $(if $O,$O,.) -name '.*.d') /dev/null


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
# defining your own "clean" and/or "distclean" rules.
#
.PHONY: clean _wvclean

clean: _wvclean

_wvclean:
	@echo '--> Cleaning $(shell pwd)...'
	@rm -f *~ *.tmp *.o *.a *.so *.so.* *.libs *.dll *.lib *.moc *.d .*.d .depend *.list \
		 .\#* .tcl_paths pkgIndex.tcl gmon.out core build-stamp \
		 wvtestmain
	@rm -f $(patsubst %.t.cc,%.t,$(wildcard *.t.cc) $(wildcard t/*.t.cc)) \
		t/*.o t/*~ t/.*.d t/.\#*
	@rm -f valgrind.log.pid*
	@rm -f semantic.cache tags
	@rm -rf debian/tmp

dist-hook:

PKGDIR=$(WVPACKAGE_TARNAME)-$(WVPACKAGE_VERSION)

dist: config.mk dist-hook
	@echo '--> Making dist in ../build/$(PKGDIR)...'
	@test -d ../build || mkdir ../build
	@rsync -a --delete --force '$(shell pwd)/' '../build/$(PKGDIR)'
	cd ../build/$(PKGDIR) && git clean -d -f -x
	cd ../build/$(PKGDIR) && git log > ChangeLog
	cd ../build/$(PKGDIR) && ./autogen.sh
	@find '../build/$(PKGDIR)' -name .git -type d -print0 | xargs -0 rm -rf --
	@find '../build/$(PKGDIR)' -name .gitignore -type f -print0 | xargs -0 rm -f --
	@rm -f '../build/$(PKGDIR).tar.gz'
	@cd ../build; tar -zcf '$(PKGDIR).tar.gz' '$(PKGDIR)'
	@echo '--> Created tarball in ../build/$(PKGDIR).tar.gz.'
