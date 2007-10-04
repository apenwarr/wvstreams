# Process this file with autoconf to produce a configure script.
AC_INIT(WvStreams, 4.4.1, wvstreams-devel@googlegroups.com, wvstreams)
SO_VERSION=4.4

save_args() {
    ARGS=""

    while test $# -gt 0; do
        ARGS="$ARGS \"$1\""
        shift
    done

    rm -f reconfigure
    echo $0 $ARGS '"$@"' >reconfigure
    chmod +x reconfigure
}

save_args "$@"

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

AC_CONFIG_SRCDIR(streams/wvstream.cc)

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--disable-debug],
                             [strip debug information]))

AC_ARG_ENABLE(devel,
              AC_HELP_STRING([--enable-devel],
                             [developer options]))

AC_ARG_ENABLE(efence,
              AC_HELP_STRING([--enable-efence],
                             [link against Electric Fence]))

AC_ARG_ENABLE(exceptions,
              AC_HELP_STRING([--disable-exceptions],
                             [C++ exceptions (compiler)]))

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

AC_ARG_ENABLE(rtti,
              AC_HELP_STRING([--disable-rtti],
                             [C++ run-time type identification (compiler)]))

AC_ARG_ENABLE(verbose,
              AC_HELP_STRING([--enable-verbose],
                             [verbose compilation output]))

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
xplc_version='0.3.13'
AC_ARG_WITH(xplc, AC_HELP_STRING([--with-xplc], [XPLC ${xplc_version} (required)]))
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib], [zlib (required)]))

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

# Poke gnulib
gl_EARLY

# Detect target build environment
AC_CANONICAL_TARGET
case "$target" in
    *-linux*)
        ARCH_SUBDIRS="linuxstreams"
	;;
    *-sunos*|*-solaris*)
    	ARCH_SUBDIRS=""
	ac_libs="-lstdc++ -lgcc_s -ldl -lm -lc"
	;;
    *)
    	ARCH_SUBDIRS=""
	;;
esac
AC_SUBST(target)
AC_SUBST(ARCH_SUBDIRS)

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

# Function checks
AC_FUNC_ALLOCA

AC_CHECK_HEADERS(execinfo.h)

# Initialise gnulib
gl_INIT

# Check for error_t
AC_CHECK_HEADERS([argz.h errno.h])
AC_CHECK_TYPE(error_t,, AC_DEFINE([error_t], [int],
				  [Define to a type to use for `error_t' if it is not otherwise available.]),dnl `
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

# Detect pkg-config
AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
if test "$PKGCONFIG" = "no"; then
    if test "$with_xplc" = ""; then
	AC_MSG_ERROR([pkg-config is not installed])
    else
	AC_MSG_WARN([pkg-config is not installed])
    fi
fi

# devel
if test "$enable_devel" = "yes"; then
    enable_debug=yes
    enable_fatal_warnings=yes
    enable_warnings=yes
fi

if test "$enable_debug" != "no"; then
    AC_DEFINE_UNQUOTED(VER_STRING_EXTRA, [" (`whoami`@`hostname`$VER_STRING_EXTRA)"], [Extra version string.])
fi

# exceptions
if test "$enable_exceptions" = "no"; then
    WV_APPEND(CXXFLAGS, -fno-exceptions)
fi

# resolver-fork
if test "$enable_resolver_fork" = "no"; then
    AC_DEFINE(WVRESOLVER_SKIP_FORK,,
              [Define to disable WvResolver forking for debugging with gdb.])
fi

# resolver-fork
if test "$enable_delete_detector" = "yes"; then
    AC_DEFINE(ENABLE_DELETE_DETECTOR,,
              [Define to enable the XPLC delete detector.])
fi

# rtti
if test "$enable_rtti" = "no"; then
    WV_APPEND(CXXFLAGS, -fno-rtti)
fi

# dbus
if test "$with_dbus" != "no"; then
    WV_APPEND(CPPFLAGS, "-DDBUS_API_SUBJECT_TO_CHANGE")
    if test "$with_dbus" = ""; then
        DBUS_LIBDIR=/usr/lib
        WV_APPEND(CPPFLAGS, [`pkg-config --cflags dbus-1`])
        WV_APPEND(LDFLAGS, [`pkg-config --libs-only-L dbus-1`])
    else
        DBUS_LIBDIR=$with_dbus/dbus/.libs
    	WV_APPEND(CPPFLAGS, -I$with_dbus)
    	WV_APPEND(LDFLAGS, -L$DBUS_LIBDIR)
    fi

    AC_CHECK_HEADERS(dbus/dbus.h,, [with_dbus=no])
    LIBS_save="$LIBS"
    LIBS="$LIBS $DBUS_LIBDIR/libdbus-1.a"
    with_dbus=no
    AC_TRY_LINK([extern "C" void dbus_message_new(); ],
                [dbus_message_new(); ],
                [with_dbus=])
    LIBS="$LIBS_save"

    if test "$with_dbus" != "no"; then
        AC_DEFINE(WITH_DBUS,,
                  [Define to enable DBUS support.])
        LIBS_DBUS=$DBUS_LIBDIR/libdbus-1.a
    fi
fi

# gtk
#if test "$with_gtk" != "no"; then
#    CPPFLAGS_save="$CPPFLAGS"
#    WV_APPEND(CPPFLAGS,`pkg-config --cflags gtk+ 2>/dev/null`)
#    AC_CHECK_HEADERS(gtk/gtk.h,, [with_gtk=no])
#    AC_CHECK_LIB(gdk, gdk_init,, [with_gtk=no])
#    AC_CHECK_LIB(gtk, gtk_main,, [with_gtk=no])
#    test "$with_gtk" = no && CPPFLAGS="$CPPFLAGS_save"
#fi

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
        for wv_qtdir in $with_qt $QTDIR $QT_SEARCH_PATH; do
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
        WV_APPEND(LDFLAGS, -L$with_qt/lib)
        WV_APPEND(LIBS_QT, -lqt-mt)
    fi
    AC_PATH_PROG(MOC, moc, [moc not found], $with_qt/bin)
fi

# valgrind
AC_CHECK_PROG(VALGRIND, valgrind, valgrind)
AC_CHECK_HEADERS(valgrind/memcheck.h)

# xplc
if test "$with_xplc" != "no"; then
    if test "$with_xplc" = ""; then
	if pkg-config --modversion xplc-${xplc_version}; then
            WV_APPEND(CPPFLAGS, [`pkg-config --cflags xplc-${xplc_version}`])
	    WV_APPEND(LDFLAGS, [`pkg-config --libs-only-L xplc-${xplc_version}`])
	    AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no], [#define UNSTABLE])
	    LIBS_save="$LIBS"
	    AC_CHECK_LIB(xplc, XPLC_getServiceManager,
	    			[LIBS_XPLC="-lxplc-cxx -lxplc"],
				[with_xplc=no])
	    LIBS="$LIBS_save"
	    xplc_pc="xplc-${xplc_version}"
	else
	    with_xplc='no'
	fi
    else
        WV_APPEND(CPPFLAGS, [-I$with_xplc/include])
	WV_APPEND(LDFLAGS, [-L$with_xplc])
	AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no], [#define UNSTABLE])
	LIBS_save="$LIBS"
	AC_CHECK_LIB(xplc, XPLC_getServiceManager,
			[LIBS_XPLC="-lxplc-cxx -lxplc"],
			[with_xplc=no])
	LIBS="$LIBS_save"
	xplc_pc="xplc-${xplc_version}"
    fi
    # Not already on the system?  Try building our own.
    if test "$with_xplc" = "no" && test -d xplc; then
        WV_APPEND(CPPFLAGS, [-I\$(WVSTREAMS)/xplc/include])
        WV_APPEND(LDFLAGS, [-L\$(WVSTREAMS)/xplc])
        AC_CONFIG_SUBDIRS(xplc)
        build_xplc=yes
        with_xplc='$(WVSTREAMS)/xplc'
	xplc_pc='wvxplc'
	LIBS_XPLC="-lxplc-cxx -lxplc"
    fi
fi

# zlib
if test "$with_zlib" != "no"; then
    AC_CHECK_HEADERS(zlib.h,, [with_zlib=no])
    AC_CHECK_LIB(z, compress,, [with_zlib=no])
fi

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
if test "$with_xplc" = "no"; then
    AC_MSG_WARN([XPLC is missing.])
    missing_required="$missing_required XPLC"
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

if test -n "$missing_required"; then
    AC_MSG_ERROR([Required dependencies missing:$missing_required])
fi

if test "$enable_devel" = "yes"; then
    if test -n "$missing_devel"; then
        AC_MSG_ERROR([Some packages required in developer mode are missing.])
    fi
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

AC_SUBST(enable_debug)
AC_SUBST(enable_devel)
AC_SUBST(enable_efence)
AC_SUBST(enable_exceptions)
AC_SUBST(enable_fatal_warnings)
AC_SUBST(enable_optimization)
AC_SUBST(enable_resolver_fork)
AC_SUBST(enable_delete_detector)
AC_SUBST(enable_rtti)
AC_SUBST(enable_verbose)
AC_SUBST(enable_warnings)
AC_SUBST(enable_testgui)

AC_SUBST(with_dbus)
AC_SUBST(with_gtk)
AC_SUBST(with_openssl)
AC_SUBST(with_pam)
AC_SUBST(with_readline)
AC_SUBST(with_qt)
AC_SUBST(with_tcl)
AC_SUBST(with_xplc)
AC_SUBST(with_zlib)

AC_SUBST(LIBS_XPLC)
AC_SUBST(LIBS_DBUS)
AC_SUBST(LIBS_QT)
AC_SUBST(LIBS_PAM)
AC_SUBST(LIBS_TCL)

AC_SUBST(build_xplc)
AC_SUBST(xplc_pc)

AC_SUBST(ac_libs)
AC_SUBST(COMPILER_STANDARD)

AC_DEFINE_UNQUOTED([VERBOSE_PACKAGE_VERSION],["$PACKAGE_VERSION$WEAVER_BUILD_INFO"],[Verbose package version])

AC_CONFIG_FILES(config.mk)
AC_CONFIG_FILES(gnulib/Makefile)
AC_CONFIG_FILES([pkgconfig/libuniconf.pc
                 pkgconfig/libuniconf-uninstalled.pc
		 pkgconfig/libwvbase.pc
		 pkgconfig/libwvbase-uninstalled.pc
		 pkgconfig/libwvqt.pc
		 pkgconfig/libwvqt-uninstalled.pc
		 pkgconfig/libwvstreams.pc
		 pkgconfig/libwvstreams-uninstalled.pc
		 pkgconfig/libwvutils.pc
		 pkgconfig/libwvutils-uninstalled.pc
		 pkgconfig/libwvtest.pc
                 pkgconfig/libwvtest-uninstalled.pc])

if test "$xplc_pc" = "wvxplc"; then
    AC_CONFIG_FILES([pkgconfig/wvxplc.pc])
fi
AC_CONFIG_HEADERS(include/wvautoconf.h)

AC_OUTPUT

# Now remove PACKAGE_* macros from include/wvautoconf.h
sed 's,^#.*define.*PACKAGE_.*$,/* & */,' include/wvautoconf.h > include/wvautoconf.h.new
if test "x$?" = "x0"; then
    if ! diff include/wvautoconf.h include/wvautoconf.h >/dev/null; then
	mv include/wvautoconf.h.new include/wvautoconf.h
    else
	rm include/wvautoconf.h.new
    fi
fi
