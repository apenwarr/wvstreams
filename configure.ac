# Process this file with autoconf to produce a configure script.
AC_INIT(WvStreams, 4.6, wvstreams-devel@googlegroups.com, wvstreams)
SO_VERSION=4.6

# append to a variable without introducing superfluous white space
AC_DEFUN([WV_APPEND],[
    if test -z "$$1"; then
        $1="$2"
    else
        $1="$$1 $2"
    fi
])

QT_SEARCH_PATH="
$prefix
/usr
$libdir/qt-3.1
$libdir/qt3
$libdir/qt
$datadir/qt3
$datadir/qt
/usr
/usr/lib/qt3
/usr/lib/qt-3.1
/usr/share/qt3
/usr/lib/qt
/usr/share/qt
"

MYDIR=$(cd `dirname -- $0` && pwd)
if test $MYDIR = `pwd`; then
    rm -f lib;
    mkdir lib || exit 1;
    cd lib && exec ../configure "$@"
fi

AC_CONFIG_SRCDIR(streams/wvstream.cc)

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--disable-debug],
                             [strip debug information]))

AC_ARG_ENABLE(fatal-warnings,
              AC_HELP_STRING([--enable-fatal-warnings],
                             [turn warnings into errors]))

AC_ARG_ENABLE(optimization,
              AC_HELP_STRING([--disable-optimization],
                             [optimization options]))

AC_ARG_ENABLE(resolver-fork,
              AC_HELP_STRING([--disable-resolver-fork],
                             [WvResolver background name resolution (debugging)]))

AC_ARG_ENABLE(delete-detector,
              AC_HELP_STRING([--enable-delete-detector],
                             [Delete detector (reference counting)]))

AC_ARG_ENABLE(warnings,
              AC_HELP_STRING([--disable-warnings],
                             [extra warnings]))

AC_ARG_ENABLE(testgui,
              AC_HELP_STRING([--disable-testgui],
                             [GUI for unit tests]))

AC_ARG_WITH(dbus, AC_HELP_STRING([--with-dbus], [DBUS]))
AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl], [OpenSSL >= 0.9.7 (required)]))
AC_ARG_WITH(pam, AC_HELP_STRING([--with-pam], [PAM]))
AC_ARG_WITH(tcl, AC_HELP_STRING([--with-tcl], [Tcl]))
AC_ARG_WITH(qt, AC_HELP_STRING([--with-qt], [Qt]))
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib], [zlib (required)]))
AC_ARG_WITH(valgrind, AC_HELP_STRING([--with-valgrind], [Valgrind]))

AC_ARG_VAR(MOC, [Qt meta object compiler])
AC_ARG_VAR(WEAVER_BUILD_INFO, [Extra version info])

# avoid autoconf's default values, but keep those the user might have given
CFLAGS="$CFLAGS"
CXXFLAGS="$CXXFLAGS"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Detect target build environment
AC_CANONICAL_TARGET
case "$target" in
    *-linux*)
        ARCH_SUBDIRS="linuxstreams"
        OS="LINUX"
	;;
    *-sunos*|*-solaris*)
    	ARCH_SUBDIRS=""
	ac_libs="-lstdc++ -lgcc_s -ldl -lm -lc"
	OS="SOLARIS"
	;;
    *-win*|*-mingw32*)
    	ARCH_SUBDIRS="win32"
    	OS="WIN32"
    	;;
    *-apple*)
    	ARCH_SUBDIRS=""
    	OS="MACOS"
    	;;
    *)
    	ARCH_SUBDIRS=""
    	OS="OTHER"
	;;
esac

AC_SUBST(target)
AC_SUBST(ARCH_SUBDIRS)
AC_SUBST(OS)

# Detect endianness
AC_C_BIGENDIAN

# Look for __attribute__ ((deprecated))
CPPFLAGS_save="$CPPFLAGS"
WV_APPEND(CPPFLAGS, -Werror)
AC_MSG_CHECKING(for __attribute__ ((deprecated)))
AC_COMPILE_IFELSE(
[#include "confdefs.h"
void f() __attribute__ ((deprecated));
void f()
{
}

int main()
{
    return 0;
}
],
		  [AC_MSG_RESULT([yes])
		   AC_DEFINE([ATTR_DEPRECATED],
			     [__attribute__ ((deprecated))],
			     [Compiler warning on deprecated functions])],
		  [AC_MSG_RESULT([no])
		   AC_DEFINE([ATTR_DEPRECATED], [],
			     [Compiler warning on deprecated functions])])
CPPFLAGS="$CPPFLAGS_save"

# argp
USE_WVSTREAMS_ARGP=0
AC_CHECK_HEADERS(argp.h)
AC_CHECK_FUNC(argp_parse)
if test "$ac_cv_func_argp_parse" != yes \
    -o "$ac_cv_header_argp_h" != yes ; then
    (
    	echo
    	echo 'configuring argp...'
	mkdir -p argp
    	cd argp
	rm -f $MYDIR/argp/config.status
    	$MYDIR/argp/configure --host=$host_cpu-$host_os --srcdir=$MYDIR/argp || exit $?
    	echo 'argp configured.'
    	echo
    ) || exit $?
    USE_WVSTREAMS_ARGP=1
fi

# Function checks
AC_HEADER_DIRENT

AC_FUNC_ALLOCA

AC_CHECK_HEADERS(execinfo.h)

# Check for error_t
AC_CHECK_HEADERS([argz.h errno.h])
AC_CHECK_TYPE(error_t,, [AC_DEFINE([error_t], [int],
				  [Define to a type to use for `error_t' if it is not otherwise available.])
AC_DEFINE([__error_t_defined], [1], [Define so that glibc/gnulib argp.h
    does not typedef error_t.])],dnl `
[#if HAVE_ARGZ_H
# include <argz.h>
#else
# if HAVE_ERRNO_H
#  include <errno.h>
# endif
#endif])

# Check for size of ethernet addresses
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([net/if.h], [], [],
[#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])
AC_CHECK_HEADERS([net/ethernet.h], [], [],
[#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])
AC_CHECK_HEADERS([netinet/if_ether.h], [], [],
[#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
])
AC_CHECK_DECL(ETHER_ADDR_LEN,,
	      AC_CHECK_TYPE(ether_addr_t,
	      		    AC_DEFINE([ETHER_ADDR_LEN], [sizeof(ether_addr_t)],
			    	      [Size of ethernet address]),
			    AC_DEFINE([ETHER_ADDR_LEN], [6]),
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#if HAVE_NETINET_IF_ETHER_H
# include <netinet/if_ether.h>
#endif
]),
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#if HAVE_NETINET_IF_ETHER_H
# include <netinet/if_ether.h>
#endif
])

# Check for basic Internet support
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([netinet/in_systm.h])
AC_CHECK_HEADERS([netinet/ip.h], [], [],
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
# include <netinet/in_systm.h>
#endif
])
AC_CHECK_HEADERS([netinet/tcp.h])

# Check for advanced Linux-style modem support
AC_CHECK_HEADERS([linux/serial.h])
AC_CHECK_FUNCS([cfmakeraw])

# Detect hard-linking based on LN_S's behaviour
AC_MSG_CHECKING([whether ln works...])
case "$LN_S" in
    ln*) 
        LN='ln'
	AC_MSG_RESULT([yes])
	;;
    *)
	LN="$LN_S"
	AC_MSG_RESULT([no, using $LN])
	;;
esac
AC_SUBST(LN)

# Setting the default language to C++ means that CXX and CXXCPP will be 
# used for compile tests.
AC_LANG(C++)

# __libc_stack_end isn't available to shared libraries with some libc versions
AC_MSG_CHECKING([whether __libc_stack_end is public])
AH_TEMPLATE([HAVE_LIBC_STACK_END], [Whether libc supports __libc_stack_end])
LDFLAGS_save="$LDFLAGS"
WV_APPEND(LDFLAGS, [-Wl,-z,defs -shared])
HAVE_LIBC_STACK_END=no
AC_TRY_LINK([extern const void *__libc_stack_end; ],
	    [volatile const void *x = __libc_stack_end; ],
	    [HAVE_LIBC_STACK_END=yes; ])
if test "$HAVE_LIBC_STACK_END" = "yes"; then
	AC_DEFINE([HAVE_LIBC_STACK_END])
fi
AC_MSG_RESULT([$HAVE_LIBC_STACK_END])
LDFLAGS="$LDFLAGS_save"

# Detect pkg-config
AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
if test "$PKGCONFIG" = "no"; then
    AC_MSG_WARN([pkg-config is not installed])
fi

if test "$enable_debug" != "no"; then
    AC_DEFINE_UNQUOTED(VER_STRING_EXTRA, [" (`whoami`@`hostname`$VER_STRING_EXTRA)"], [Extra version string.])
fi

# resolver-fork
if test "$enable_resolver_fork" = "no"; then
    AC_DEFINE(WVRESOLVER_SKIP_FORK,,
              [Define to disable WvResolver forking for debugging with gdb.])
fi

# xplc delete detector
if test "$enable_delete_detector" = "yes"; then
    AC_DEFINE(ENABLE_DELETE_DETECTOR,,
              [Define to enable the XPLC delete detector.])
fi

# dbus
if test "$with_dbus" != "no"; then        
    if test "$with_dbus" = "" -o "$with_dbus" = "yes"; then
        AC_MSG_CHECKING([Checking that D-Bus version greater than 1.2.14 installed.])
        if pkg-config --atleast-version 1.2.14 dbus-1; then
	    WV_APPEND(CPPFLAGS, [`pkg-config --cflags dbus-1`])
            WV_APPEND(LDFLAGS, [`pkg-config --libs dbus-1`])
	    AC_MSG_RESULT([yes])
        else
	    with_dbus=no
	    AC_MSG_RESULT([no])
	fi
    else
        # no version check when doing --with-dbus
        DBUS_LIBDIR=$with_dbus/dbus/.libs
    	WV_APPEND(CPPFLAGS, -I$with_dbus)
    	WV_APPEND(LDFLAGS, -L$DBUS_LIBDIR)
    	WV_APPEND(LIBS, -ldbus-1)
    fi

    # double check that we actually have D-Bus
    if test "$with_dbus" != "no"; then
        AC_CHECK_HEADERS(dbus/dbus.h,, [with_dbus=no])
	with_dbus=no
	AC_TRY_LINK([extern "C" void dbus_message_new(); ],
                    [dbus_message_new(); ],
                    [with_dbus=])
    fi

    if test "$with_dbus" != "no"; then
        AC_DEFINE(WITH_DBUS,,
                  [Define to enable DBUS support.])
        LIBS_DBUS=$DBUS_LIBDIR/libdbus-1.a
    fi
fi

# BSD sockets, if you're on Solaris
AC_CHECK_LIB(socket, bind)

# openssl
if test "$with_openssl" != "no"; then
    if test "$with_openssl" != ""; then
        WV_APPEND(CPPFLAGS, [-I$with_openssl/include])
        WV_APPEND(LDFLAGS, [-L$with_openssl])
    fi
    AC_CHECK_HEADERS(openssl/ssl.h,, [with_openssl=no],
                     [#define OPENSSL_NO_KRB5])
    LIBS_save="$LIBS"
    AC_CHECK_LIB(crypto, X509_free)
    AC_CHECK_LIB(ssl, SSL_has_matching_session_id,, [with_openssl=no])
    AC_CHECK_LIB(ssl, POLICY_MAPPING_new,, [with_openssl_policy_mapping=no])
    if test "$with_openssl_policy_mapping" != "no"; then
        AC_DEFINE([HAVE_OPENSSL_POLICY_MAPPING], [1],
	    [Whether libssl has the POLICY_MAPPING features (0.9.8 and up)])
    fi
    LIBS="$LIBS_save"
    if test "$with_openssl" != "no"; then
        LIBS_SSL="-lcrypto -lssl"
    fi
fi

# readline
if test "$with_readline" != "no"; then
    AC_CHECK_HEADERS(readline/readline.h,, [with_readline=no])
    AC_CHECK_LIB(readline, readline,, [with_readline=no])
fi

# pam
if test "$with_pam" != "no"; then
    AC_CHECK_HEADERS(security/pam_appl.h,, [with_pam=no])
    LIBS_save="$LIBS"
    AC_CHECK_LIB(pam, pam_start,, [with_pam=no])
    AC_MSG_CHECKING(for sane PAM implementation)
    AC_COMPILE_IFELSE(
[#include "confdefs.h"
#if HAVE_SECURITY_PAM_APPL_H
# include <security/pam_appl.h>
#endif

/* noconv: null PAM conversation function */
int noconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata)
{
    // if you need to ask things, it won't work
    return PAM_CONV_ERR;
}

int main()
{
    struct pam_conv c;
    c.conv = noconv;
    return 0;
}
],
		      [AC_MSG_RESULT([yes])
		       AC_DEFINE([HAVE_BROKEN_PAM], [0])],
		      [AC_MSG_RESULT([no])
		       AC_DEFINE([HAVE_BROKEN_PAM], [1],
				 [Solaris has a broken PAM implementation])])
    LIBS="$LIBS_save"
    if test "$with_pam" != "no" -a "$HAVE_BROKEN_PAM" != "1"; then
        LIBS_PAM=-lpam
    fi
fi

# tcl
if test "$with_tcl" != "no"; then
    CPPFLAGS_save="$CPPFLAGS"
    WV_APPEND(CPPFLAGS, -I/usr/include/tcl8.3)
    AC_CHECK_HEADERS(tcl.h,, [with_tcl=no])
    LIBS_save="$LIBS"
    AC_CHECK_LIB(tcl8.3, TclInterpInit,, [with_tcl=no])
    LIBS="$LIBS_save"
    if test "$with_tcl" != "no"; then
        CPPFLAGS="$CPPFLAGS_save"
        LIBS_TCL=-ltcl8.3
    fi
fi

# qt
if test "$with_qt" != "no"; then
    test "$with_qt" = yes && with_qt=
    AC_CACHE_CHECK([for Qt], [wv_cv_with_qt], [
        wv_cv_with_qt=no
        CPPFLAGS_save="$CPPFLAGS"
        LDFLAGS_save="$LDFLAGS"
        LIBS_save="$LIBS"
        for wv_qtdir in $with_qt $QTDIR $QT_SEARCH_PATH $(pkg-config --variable=prefix qt-mt); do
            eval wv_qtdir="$wv_qtdir"
            CPPFLAGS="$CPPFLAGS_save -I$wv_qtdir/include -I$wv_qtdir/include/qt3"
            LDFLAGS="$LDFLAGS_save -L$wv_qtdir/lib"
            LIBS="$LIBS_save -lqt-mt"
            AC_TRY_LINK([#include <qstring.h>],
                        [QString x("hello"); return 0; ],
                        [wv_cv_with_qt=$wv_qtdir; break])
        done
        CPPFLAGS="$CPPFLAGS_save"
        LDFLAGS="$LDFLAGS_save"
        LIBS="$LIBS_save"
    ])
    with_qt=$wv_cv_with_qt
    if test "$with_qt" != no; then
        WV_APPEND(CPPFLAGS, -I$with_qt/include -I$with_qt/include/qt3)
        if test "$wv_qtdir" != "/usr" ; then
                # never explicitly include /usr/lib
	        WV_APPEND(LDFLAGS, -L$with_qt/lib)
	fi
        WV_APPEND(LIBS_QT, -lqt-mt)
    fi
    AC_PATH_PROG(MOC, moc, [moc not found], $with_qt/bin)
fi

# valgrind
if test "$with_valgrind" != "no"; then
    AC_CHECK_PROG(VALGRIND, valgrind, valgrind)
    AC_CHECK_HEADERS(valgrind/memcheck.h)
fi

# zlib
if test "$with_zlib" != "no"; then
    AC_CHECK_HEADERS(zlib.h,, [with_zlib=no])
    AC_CHECK_LIB(z, compress,, [with_zlib=no])
fi

# Find out whether TR1 or Boost are available.
AC_CHECK_HEADERS(tr1/functional)
AC_CHECK_HEADERS(boost/function.hpp)

# When compiling with exceptions disabled and Boost, applications need
# to provide an "exception handler", declared here.
AC_CHECK_HEADERS(boost/throw_exception.hpp)

# check for missing packages
missing_required=
missing_devel=
if test "$with_dbus" = "no"; then
    AC_MSG_WARN([DBUS is missing.])
    missing_devel=yes
fi
if test "$with_pam" = "no"; then
    AC_MSG_WARN([PAM is missing.])
    missing_devel=yes
fi
if test "$with_qt" = "no"; then
    AC_MSG_WARN([Qt is missing.])
    missing_devel=yes
fi
if test "$VALGRIND" = ""; then
    AC_MSG_WARN([Valgrind is missing.])
fi
if test "$with_openssl" = "no"; then
    AC_MSG_WARN([OpenSSL is missing.])
    missing_required="$missing_required OpenSSL>=0.9.7"
fi
if test "$with_readline" = "no"; then
    AC_MSG_WARN([readline is missing.])
fi
if test "$with_zlib" = "no"; then
    AC_MSG_WARN([zlib is missing.])
    missing_required="$missing_required zlib"
fi
if test "$ac_cv_header_tr1_functional" != "yes" \
     -a "$ac_cv_header_boost_function_hpp" != "yes"; then
    AC_MSG_WARN([both tr1/functional and boost/function.hpp are missing.])
    missing_required="$missing_required boost/function.hpp"
fi

if test -n "$missing_required"; then
    AC_MSG_ERROR([Required dependencies missing:$missing_required])
fi

if test "$VALGRIND" != ""; then
    VALGRIND="valgrind --tool=memcheck --leak-check=yes --num-callers=10 --suppressions=\$(WVSTREAMS_SRC)/wvstreams.supp"
    if valgrind --help | grep log-file >/dev/null; then
	VALGRIND="$VALGRIND --log-file=valgrind.log"
    else
	VALGRIND="$VALGRIND --logfile=valgrind.log"
    fi
fi


# Compiler is always posix if invoking this configure script
COMPILER_STANDARD=posix

AC_SUBST(SO_VERSION)

AC_SUBST(USE_WVSTREAMS_ARGP)

AC_SUBST(enable_debug)
AC_SUBST(enable_optimization)
AC_SUBST(enable_resolver_fork)
AC_SUBST(enable_delete_detector)
AC_SUBST(enable_warnings)
AC_SUBST(enable_testgui)

AC_SUBST(with_dbus)
AC_SUBST(with_openssl)
AC_SUBST(with_openssl_policy_mapping)
AC_SUBST(with_pam)
AC_SUBST(with_readline)
AC_SUBST(with_qt)
AC_SUBST(with_tcl)
AC_SUBST(with_zlib)

AC_SUBST(LIBS_DBUS)
AC_SUBST(LIBS_QT)
AC_SUBST(LIBS_PAM)
AC_SUBST(LIBS_TCL)

AC_SUBST(ac_libs)
AC_SUBST(COMPILER_STANDARD)

if test x$MYDIR != x$PWD; then
    WVSRCDIR=$MYDIR
    mkdir -p $(cd $MYDIR && find . -type d)
    WVSRCDIR_SLASH=$WVSRCDIR/
else
    WVSRCDIR=.
fi
AC_SUBST(WVSRCDIR)
AC_SUBST(WVSRCDIR_SLASH)

AC_DEFINE_UNQUOTED([VERBOSE_PACKAGE_VERSION],["$PACKAGE_VERSION$WEAVER_BUILD_INFO"],[Verbose package version])

AC_CONFIG_FILES(config.mk)
AC_CONFIG_FILES([pkgconfig/libuniconf.pc
                 pkgconfig/libuniconf-uninstalled.pc
		 pkgconfig/libwvbase.pc
		 pkgconfig/libwvbase-uninstalled.pc
		 pkgconfig/libwvdbus.pc
		 pkgconfig/libwvdbus-uninstalled.pc
		 pkgconfig/libwvqt.pc
		 pkgconfig/libwvqt-uninstalled.pc
		 pkgconfig/libwvstreams.pc
		 pkgconfig/libwvstreams-uninstalled.pc
		 pkgconfig/libwvutils.pc
		 pkgconfig/libwvutils-uninstalled.pc
		 pkgconfig/libwvtest.pc
                 pkgconfig/libwvtest-uninstalled.pc])
AC_CONFIG_FILES(Makefile)

AC_CONFIG_HEADERS(include/wvautoconf.h)

AC_OUTPUT

if test -L $MYDIR/lib; then
    rm -f $MYDIR/lib;
fi
if ! test -d $MYDIR/lib; then
    ln -s $(pwd) $MYDIR/lib;
fi

# Now convert PACKAGE_* macros into WVPACKAGE_* in include/wvautoconf.h
sed 's,\(\#define.*\)PACKAGE,\1WVPACKAGE,' include/wvautoconf.h > include/wvautoconf.h.new
if test "x$?" = "x0"; then
    if ! diff include/wvautoconf.h include/wvautoconf.h.new >/dev/null; then
	mv include/wvautoconf.h.new include/wvautoconf.h
    else
	rm include/wvautoconf.h.new
    fi
fi
