# Process this file with autoconf to produce a configure script.
AC_INIT(WvStreams, 3.80, wvstreams-dev@nit.ca, wvstreams)

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
AC_DEFUN(WV_APPEND,
    if test -z "$$1"; then
        $1="$2"
    else
        $1="$$1 $2"
    fi
)

QT_SEARCH_PATH="
$prefix
/usr
$libdir/qt3
$libdir/qt
$datadir/qt3
$datadir/qt
/usr
/usr/lib/qt3
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
              AC_HELP_STRING([--enable-exceptions],
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

AC_ARG_ENABLE(rtti,
              AC_HELP_STRING([--enable-rtti],
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

AC_ARG_WITH(acl, AC_HELP_STRING([--with-acl], [ACL]))
AC_ARG_WITH(fam, AC_HELP_STRING([--with-fam], [FAM]))
AC_ARG_WITH(fftw, AC_HELP_STRING([--with-fftw], [FFTW]))
AC_ARG_WITH(bdb, AC_HELP_STRING([--with-bdb], [Berkeley DB 1.x (or compatible)]))
AC_ARG_WITH(qdbm, AC_HELP_STRING([--with-qdbm], [QDBM]))
AC_ARG_WITH(ogg, AC_HELP_STRING([--with-ogg], [Ogg]))
AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl], [OpenSSL (required)]))
AC_ARG_WITH(pam, AC_HELP_STRING([--with-pam], [PAM]))
AC_ARG_WITH(tcl, AC_HELP_STRING([--with-tcl], [Tcl]))
AC_ARG_WITH(telephony, AC_HELP_STRING([--with-telephony], [Telephony]))
AC_ARG_WITH(swig, AC_HELP_STRING([--with-swig], [SWIG]))
AC_ARG_WITH(qt, AC_HELP_STRING([--with-qt], [Qt]))
AC_ARG_WITH(speex, AC_HELP_STRING([--with-speex], [Speex]))
AC_ARG_WITH(vorbis, AC_HELP_STRING([--with-vorbis], [Vorbis]))
AC_ARG_WITH(xplc, AC_HELP_STRING([--with-xplc], [XPLC (required)]))
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib], [zlib (required)]))

AC_ARG_VAR(MOC, [Qt meta object compiler])

# avoid autoconf's default values, but keep those the user might have given
CFLAGS="$CFLAGS"
CXXFLAGS="$CXXFLAGS"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LN_S

# Setting the default language to C++ means that CXX and CXXCPP will be 
# used for compile tests.
AC_LANG(C++)

# devel
if test "$enable_devel" = "yes"; then
    enable_debug=yes
    enable_fatal_warnings=yes
    enable_warnings=yes
fi

# exceptions
if test "$enable_exceptions" != "yes"; then
    WV_APPEND(CXXFLAGS, -fno-exceptions)
fi

# resolver-fork
if test "$enable_resolver_fork" = "no"; then
    AC_DEFINE(WVRESOLVER_SKIP_FORK,,
              [Define to disable WvResolver forking for debugging with gdb.])
fi

# rtti
if test "$enable_rtti" != "yes"; then
    WV_APPEND(CXXFLAGS, -fno-rtti)
fi

# acl
if test "$with_acl" != "no"; then
    test "$with_acl" = yes && with_acl=
    CPPFLAGS_save="$CPPFLAGS"
    LDFLAGS_save="$LDFLAGS"
    if test "$with_acl"; then
        WV_APPEND(CPPFLAGS, [-I$with_acl/include])
        WV_APPEND(LDFLAGS, [-L$with_acl/libacl/.libs])
    fi
    # check for the acl headers
    AC_CHECK_HEADERS(acl.h,, [with_acl=no])
    # check for acl library
    AC_CHECK_LIB(acl, acl_set_file,, [with_acl=no])

    if test "$with_acl" = "no"; then
        CPPFLAGS="$CPPFLAGS_save"
        LDFLAGS="$LDFLAGS_save"
    else
        AC_DEFINE(WITH_ACL,,
                  [Define to enable ACL support.])
    fi
fi

# fam
if test "$with_fam" != "no"; then
    AC_CHECK_HEADERS(fam.h,, [with_fam=no])
    AC_CHECK_LIB(fam, FAMOpen,, [with_fam=no])
    if test "$with_fam" != "no"; then
        AC_DEFINE(WITH_FAM,, [Define to enable FAM support.])
    fi
fi

# fftw
if test "$with_fftw" != "no"; then
    AC_CHECK_HEADERS(rfftw.h,, [with_fftw=no])
    AC_CHECK_LIB(fftw, fftw,, [with_fftw=no])
    AC_CHECK_LIB(rfftw, rfftw,, [with_fftw=no])
fi

# bdb
if test "$with_bdb" != "no"; then
    test "$with_bdb" = yes && with_bdb=
    CPPFLAGS_save="$CPPFLAGS"
    LDFLAGS_save="$LDFLAGS"
    if test "$with_bdb"; then
        WV_APPEND(CPPFLAGS, [-I$with_bdb/include])
        WV_APPEND(LDFLAGS, [-L$with_bdb])
    fi
    # check for the bdb headers
    AC_CHECK_HEADERS(db.h,, [with_bdb_header=no],
                     [#include <db.h>
                      #ifdef DB_VERSION_MAJOR
                      #error "this is Berkeley DB 2.x or newer"
                      #endif])
    if test "$with_bdb_header" = "no"; then
        AC_CHECK_HEADERS(db_185.h,, [with_bdb=no])
    fi
    # check for bdb library
    AC_CHECK_LIB(db, dbopen,, [with_bdb_lib=no])
    if test "$with_bdb_lib" = "no"; then
        AC_CHECK_LIB(db1, dbopen, [with_bdb_lib=yes],)
    fi
    if test "$with_bdb_lib" = "no"; then
        with_bdb=no
    fi

    if test "$with_bdb" = "no"; then
        CPPFLAGS="$CPPFLAGS_save"
        LDFLAGS="$LDFLAGS_save"
    else
        AC_DEFINE(WITH_BDB,,
                  [Define to enable Berkeley DB support.])
    fi
fi

# qdbm
if test "$with_qdbm" != "no"; then
    test "$with_qdbm" = yes && with_qdbm=
    CPPFLAGS_save="$CPPFLAGS"
    LDFLAGS_save="$LDFLAGS"
    if test "$with_qdbm"; then
        WV_APPEND(CPPFLAGS, [-I$with_qdbm])
        WV_APPEND(LDFLAGS, [-L$with_qdbm])
    fi
    # check for the qdbm headers
    AC_CHECK_HEADERS(villa.h,, [with_qdbm=no])
    # check for qdbm library
    AC_CHECK_LIB(qdbm, vlopen,, [with_qdbm=no])

    if test "$with_qdbm" = "no"; then
        CPPFLAGS="$CPPFLAGS_save"
        LDFLAGS="$LDFLAGS_save"
    else
        AC_DEFINE(WITH_QDBM,,
                  [Define to enable QDBM support.])
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

# ogg
if test "$with_ogg" != "no"; then
    AC_CHECK_HEADERS(ogg/ogg.h,, [with_ogg=no])
    AC_CHECK_LIB(ogg, ogg_stream_init,, [with_ogg=no])
fi

# openssl
if test "$with_openssl" != "no"; then
    AC_CHECK_HEADERS(openssl/ssl.h,, [with_openssl=no],
                     [#define OPENSSL_NO_KRB5])
    AC_CHECK_LIB(ssl, SSL_CTX_new,, [with_openssl=no])
fi

# pam
if test "$with_pam" != "no"; then
    AC_CHECK_HEADERS(security/pam_appl.h,, [with_pam=no])
    AC_CHECK_LIB(pam, pam_start,, [with_pam=no])
fi

# tcl
if test "$with_tcl" != "no"; then
    CPPFLAGS_save="$CPPFLAGS"
    WV_APPEND(CPPFLAGS, -I/usr/include/tcl8.3)
    AC_CHECK_HEADERS(tcl.h,, [with_tcl=no])
    AC_CHECK_LIB(tcl8.3, TclInterpInit,, [with_tcl=no])
    if test "$with_tcl" != "no"; then
        CPPFLAGS="$CPPFLAGS_save"
    fi
fi

# telephony
if test "$with_telephony" != "no"; then
    with_telephony=yes
else
    with_telephony=no
fi

# swig
if test "$with_swig" != "no"; then
     AC_CHECK_PROG(SWIG, swig, swig, no, $with_swig:$PATH)
     if test "$SWIG" = "no"; then
        with_swig=no
     else 
        if test "$with_swig"; then
           SWIG="$with_swig/swig"
        fi
     fi
fi

# qt
if test "$with_qt" != "no"; then
    test "$with_qt" = yes && with_qt=
    AC_CACHE_CHECK([for Qt], [wv_cv_with_qt], [
        wv_cv_with_qt=no
        CPPFLAGS_save="$CPPFLAGS"
        LDFLAGS_save="$LDFLAGS"
        for wv_qtdir in $with_qt $QTDIR $QT_SEARCH_PATH; do
            eval wv_qtdir="$wv_qtdir"
            CPPFLAGS="$CPPFLAGS_save -I$wv_qtdir/include -I$wv_qtdir/include/qt3"
            LDFLAGS="$LDFLAGS_save -L$wv_qtdir/lib -lqt-mt"
            AC_TRY_LINK([#include <qstring.h>],
                        [QString x("hello"); return 0; ],
                        [wv_cv_with_qt=$wv_qtdir; break])
        done
        CPPFLAGS="$CPPFLAGS_save"
        LDFLAGS="$LDFLAGS_save"
    ])
    with_qt=$wv_cv_with_qt
    if test "$with_qt" != no; then
        WV_APPEND(CPPFLAGS, -I$with_qt/include -I$with_qt/include/qt3)
        WV_APPEND(LDFLAGS, -L$with_qt/lib)
    fi
    AC_PATH_PROG(MOC, moc, [moc not found], $with_qt/bin)
fi

# speex
if test "$with_speex" != "no"; then
    AC_CHECK_HEADERS(speex.h speex_header.h,, [with_speex=no])
    AC_CHECK_LIB(speex, speex_encoder_init,, [with_speex=no])
fi

# valgrind
AC_CHECK_PROG(VALGRIND, valgrind, valgrind)
AC_CHECK_HEADERS(valgrind/memcheck.h)

# vorbis
if test "$with_vorbis" != "no"; then
    AC_CHECK_HEADERS(vorbis/codec.h vorbis/vorbisenc.h,, [with_vorbis=no])
    AC_CHECK_LIB(vorbis, vorbis_info_init,, [with_vorbis=no])
    AC_CHECK_LIB(vorbisenc, vorbis_encode_init,, [with_vorbis=no])
fi

# xplc
if test "$with_xplc" != "no"; then
    if test -d xplc; then
        WV_APPEND(CPPFLAGS, [-I\$(WVSTREAMS)/xplc/include])
        WV_APPEND(LDFLAGS, [-L\$(WVSTREAMS)/xplc])
        AC_CONFIG_SUBDIRS(xplc)
        build_xplc=yes
        with_xplc='$(WVSTREAMS)/xplc'
    else
        test "$with_xplc" = yes && with_xplc=
        CPPFLAGS_save="$CPPFLAGS"
        LDFLAGS_save="$LDFLAGS"
        if test "$with_xplc"; then
            WV_APPEND(CPPFLAGS, [-I$with_xplc/include])
            WV_APPEND(LDFLAGS, [-L$with_xplc])
        fi
        AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no], [#define UNSTABLE])
        AC_CHECK_LIB(xplc, XPLC_getServiceManager,, [with_xplc=no])
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
if test "$with_acl" = "no"; then
    AC_MSG_WARN([ACL is missing.])
    missing_devel=yes
fi
if test "$with_fam" = "no"; then
    AC_MSG_WARN([FAM is missing.])
    missing_devel=yes
fi
if test "$with_fftw" = "no"; then
    AC_MSG_WARN([FFTW is missing.])
    missing_devel=yes
fi
if test "$with_qdbm" = "no"; then
    AC_MSG_WARN([QDBM is missing.])
    missing_devel=yes
fi
if test "$with_ogg" = "no"; then
    AC_MSG_WARN([Ogg is missing.])
    missing_devel=yes
fi
if test "$with_pam" = "no"; then
    AC_MSG_WARN([PAM is missing.])
    missing_devel=yes
fi
if test "$with_telephony" = "no"; then
    AC_MSG_WARN([Telephony is missing.])
    missing_devel=yes
fi
if test "$with_qt" = "no"; then
    AC_MSG_WARN([Qt is missing.])
    missing_devel=yes
fi
if test "$with_speex" = "no"; then
    AC_MSG_WARN([Speex is missing.])
    missing_devel=yes
fi
if test "$with_vorbis" = "no"; then
    AC_MSG_WARN([Vorbis is missing.])
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
    missing_required="$missing_required OpenSSL"
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
    VALGRIND="valgrind --leak-check=yes --num-callers=10 --logfile-fd=-1 --suppressions=\${WVSTREAMS}/suppressions.wv"
fi

AC_SUBST(enable_debug)
AC_SUBST(enable_devel)
AC_SUBST(enable_efence)
AC_SUBST(enable_exceptions)
AC_SUBST(enable_fatal_warnings)
AC_SUBST(enable_optimization)
AC_SUBST(enable_resolver_fork)
AC_SUBST(enable_rtti)
AC_SUBST(enable_verbose)
AC_SUBST(enable_warnings)
AC_SUBST(enable_testgui)

AC_SUBST(with_acl)
AC_SUBST(with_bdb)
AC_SUBST(with_fam)
AC_SUBST(with_fftw)
AC_SUBST(with_qdbm)
AC_SUBST(with_gtk)
AC_SUBST(with_ogg)
AC_SUBST(with_openssl)
AC_SUBST(with_pam)
AC_SUBST(with_telephony)
AC_SUBST(with_qt)
AC_SUBST(with_speex)
AC_SUBST(with_swig)
AC_SUBST(with_tcl)
AC_SUBST(with_vorbis)
AC_SUBST(with_xplc)
AC_SUBST(with_zlib)

AC_SUBST(build_xplc)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

