#
# wvrules.mk:  2001 11 12
#
# Copyright (C) 1998-2001 by Avery Pennarun <apenwarr@worldvisions.ca>.
#   Use, distribute, modify, and redistribute freely.  (But if you're nice,
#   you'll send all your changes back to me.)
#
# This is a complicated-looking set of Makefile rules that should make your
# own Makefiles simpler, by providing for several useful features (like
# autodependencies and a 'clean' target) without any extra effort.
#
# It will only work with GNU make.
#
SHELL=/bin/bash

STRIP=strip --remove-section=.note --remove-section=.comment
#STRIP=echo

# we need a default rule, since the 'includes' below causes trouble
default: all

${TOPDIR}/rules.local.mk:
	@true

-include ${TOPDIR}/rules.local.mk

#
# Figure out which OS we're running (for now, only picks out Linux or BSD)
#
OS=$(shell uname -a | awk '{print $$1}' | sed -e 's/^.*BSD/BSD/g' )

#
# (Just BSD and LINUX clash with other symbols, so us ISLINUX and ISBSD)
#
ifeq ($(OS),Linux)
  OSDEFINE=-DISLINUX
endif

ifeq ($(OS),BSD)
  OSDEFINE=-DISBSD
endif

#
# Define DEBUG flag if not already set.
#
ifeq ($(DEBUG),)
DEBUG=1
endif

#
# Point to the electric fence library, assuming DEBUG=1.
#
ifneq ($(NOEFENCE),1)
 ifeq ($(DEBUG),1)
   EFENCE=-lefence
 else
   EFENCE=
 endif
endif

ifeq ($(CCMALLOC),1)
 ifeq ($(DEBUG),1)
   XX_LIBS += -lccmalloc -ldl
 endif
endif

#
# C compilation flags (depends on DEBUG setting)
#
CPPFLAGS = $(CPPOPTS)
CFLAGS = $(COPTS) -Wall -D_BSD_SOURCE -D_GNU_SOURCE $(OSDEFINE)
CXXFLAGS = $(CXXOPTS) -Woverloaded-virtual
LDFLAGS = $(LDOPTS)

ifeq ($(DEBUG),1)
CFLAGS += -ggdb -DDEBUG=1
CXXFLAGS +=-fno-rtti -fno-exceptions
else
CFLAGS += -g -O -DDEBUG=0
#CFLAGS += -DNDEBUG    # I don't like disabling assertions...
#CFLAGS += -fomit-frame-pointer  # really evil
#CXXFLAGS += -fno-implement-inlines  # causes trouble with egcs 1.0
CXXFLAGS += -fno-rtti -fno-exceptions
endif

ifeq ($(PROFILE),1)
CFLAGS += -pg
LDFLAGS += -pg
endif

ifeq ($(STATIC),1)
LDFLAGS += -static
endif


# any rule that depends on FORCE will always run
.PHONY: FORCE
FORCE:

%.gz: FORCE %
	@rm -f $@
	gzip -f $*
	@ls -l $@

ALLDIRS = $(XPATH)
#VPATH = $(shell echo $(ALLDIRS) | sed 's/[ 	][ 	]*/:/g')
INCFLAGS = $(addprefix -I,$(ALLDIRS))

#
# Typical compilation rules.
#
override _R_CFLAGS=$(CPPFLAGS) $(CFLAGS) $(INCFLAGS) -MD
override _R_CXXFLAGS=$(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(INCFLAGS) -MD
export _R_CFLAGS
export _R_CXXFLAGS

%.o: %.c
	@rm -f .$*.d $@
	$(CC) $$_R_CFLAGS -c $<
	@mv $*.d .$*.d

%.fpic.o: %.c
	@rm -f .$*.d $@
	$(CC) -fPIC $$_R_CFLAGS -c $<
	@mv $*.d .$*.d

%.o: %.cc
	@rm -f .$*.d $@
	$(CXX) $$_R_CXXFLAGS -c $<
	@mv $*.d .$*.d

%.fpic.o: %.cc
	@rm -f .$*.d $@
	$(CXX) -fPIC $$_R_CXXFLAGS -c $<
	@mv $*.d .$*.d

%.o: %.cpp
	@rm -f .$*.d $@
	$(CXX) $$_R_CXXFLAGS -c $<
	@mv $*.d .$*.d

%.fpic.o: %.cpp
	@rm -f .$*.d $@
	$(CXX) -fPIC $$_R_CXXFLAGS -c $<
	@mv $*.d .$*.d

%.s: %.c
	$(CC) $$_R_CFLAGS -S $<

%.s: %.cc
	$(CC) $$_R_CFLAGS -S $<

%.s: %.cpp
	$(CC) $$_R_CFLAGS -S $<

%.E: %.c
	$(CC) $$_R_CFLAGS -E $< >$@

%.E: %.cc
	$(CC) $$_R_CFLAGS -E $< >$@

%.E: %.cpp
	$(CC) $$_R_CFLAGS -E $< >$@

../%.libs:
	@echo "Library list $@ does not exist!"; exit 1

../%.so:
	@echo "Shared library $@ does not exist!"; exit 1

../%.a:
	@echo "Library $@ does not exist!"; exit 1

../%.o:
	@echo "Object $@ does not exist!"; exit 1

%.libs:
	rm -f $@
	@set -e; \
	SIMPLE_OBJS="$(filter %.o %.libs,$^)"; \
	echo "echo $$SIMPLE_OBJS >$@"; \
	for d in $$SIMPLE_OBJS; do \
		echo $$d >>$@; \
	done
	
# define a shell command "libexpand" to expand out .libs file references.
# usage: libexpand BASEDIR file file file file
# eg. libexpand mydir foo.o blah.o weasels.o frogs/silly.libs
#  If frogs/silly.libs contains "a.o b.o"
#  Prints the following to stdout:
#            mydir/foo.o mydir/blah.o mydir/weasels.o 
#            mydir/frogs/a.o mydir/frogs/b.o
#
define define_libexpand
	libexpand_list() \
	{ \
		for d in $$*; do \
			set -e; \
			case "$$d" in \
			    *.o|*.a) echo "$$BASEDIR$$d" ;; \
			    *.libs)  SUBF=$$(cat "$$BASEDIR$$d") || exit 1; \
			    	     libexpand $$BASEDIR$$(dirname "$$d") \
			    			$$SUBF || exit 1 ;; \
			esac; \
		done; \
	}; \
	\
	libexpand() \
	{ \
		set -e; \
		true "libexpand: ($$*)" >&2; \
		BASEDIR="$$1/"; \
		[ "$$BASEDIR" = "./" ] && BASEDIR=""; \
		shift; \
		true "libexpand2: ($$BASEDIR) ($$*)" >&2; \
		BIGLIST=$$(libexpand_list $$*) || exit 1; \
		echo "$$BIGLIST" \
			| sed 's,\(^\|/\)[^.][^/]*/../,\1,g' \
			| sort | uniq; \
		true "libexpand3: ($$BASEDIR) ..." >&2; \
	}
endef

# we need to do some magic in order to make a .a file that contains another
# .a file.  Otherwise this would be easy...
%.a:
	rm -f $@
	@set -e; \
	$(define_libexpand); \
	EXPAND=$$(libexpand . $(filter %.libs,$^)) || exit 1; \
	SIMPLE_OBJS="$(filter %.o,$^) $$EXPAND"; \
	AR_FILES="$(filter %.a,$^)"; \
	echo "ar q $@ $(filter %.o %.libs,$^)"; \
	ar q $@ $$SIMPLE_OBJS; \
	for d in $$AR_FILES; do \
		echo "ar q $@ $$d (magic library merge)"; \
		BASE=$$(basename $$d); \
		for e in $$(ar t $$d); do \
			OBJNAME="ar_$${BASE}_$$e"; \
			ar p $$d $$e >$$OBJNAME; \
			ar q $@ $$OBJNAME; \
			rm -f $$OBJNAME; \
		done; \
	done
	ranlib $@
	
%.so:
	@echo $(CC) $(LDFLAGS) $($@-LDFLAGS) -shared -o $@ $^ $($@-LIBS) $(LIBS) $(XX_LIBS)
	@set -e; \
	$(define_libexpand); \
	$(CC) $(LDFLAGS) $($@-LDFLAGS) -shared -o $@ \
		$(filter %.o %.a %.so,$^) \
		$$(libexpand . $(filter %.libs,$^)) \
		$($@-LIBS) $(LIBS) $(XX_LIBS)

%: %.o
	$(CC) $(LDFLAGS) $($@-LDFLAGS) -o $@ $(filter %.o %.a %.so, $^) $($@-LIBS) $(LIBS) $(XX_LIBS)

# Force objects to be built before final binaries	
$(addsuffix .o,$(basename $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp))):

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
# Automatically generate header dependencies for .c and .cc files.  The
# dependencies are stored in the file ".depend"
#

depfiles_sf = $(wildcard .*.d)

ifneq ($(depfiles_sf),)
-include $(depfiles_sf)
endif


#
# A macro for compiling subdirectories listed in the SUBDIRS variable.
# Tries to make the target ($@) in each subdir, unless the target is called
# "subdirs" in which case it makes "all" in each subdir.
#
define subdirs
	+@OLDDIR="$$(pwd)"; set -e; \
	for d in __fx__ ${SUBDIRS}; do \
		if [ "$$d" = "__fx__" ]; then continue; fi; \
		cd "$$d"; \
		echo ; \
		echo "--> Making $(subst subdirs,all,$@) in $$(pwd)..."; \
		$(MAKE) --no-print-directory $(subst subdirs,all,$@); \
		cd "$$OLDDIR"; \
	done
	@echo
	@echo "--> Back in $$(pwd)..."
endef

%: %/Makefile FORCE
	@cd "$@"; echo; echo "--> Making all in $$(pwd)..."; \
		$(MAKE) --no-print-directory all

subdirs: ${SUBDIRS}

#
# Auto-clean rule.  Feel free to append to this in your own directory, by
# defining your own "clean" rule.
#
clean: FORCE cleanrule

cleanrule: FORCE
	rm -f *~ *.tmp *.o *.a *.so *.so.* *.libs *.d .*.d .depend .\#* \
		.tcl_paths pkgIndex.tcl gmon.out core

#
# Make 'tags' file using the ctags program - useful for editing
#
#tags: $(shell find -name '*.cc' -o -name '*.[ch]')
#	@echo '(creating "tags")'
#	@if [ -x /usr/bin/ctags ]; then /usr/bin/ctags $^; fi
