# Process this file with autoconf to produce a configure script.
AC_INIT(WvStreams, 3.74, wvstreams-dev@nit.ca, wvstreams)

AC_REVISION($Id: configure.ac,v 1.35 2003/09/10 03:42:20 apenwarr Exp $)

rm -f reconfigure
echo $0 "$@" '"$@"' >reconfigure
chmod +x reconfigure

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

AC_ARG_WITH(fam, AC_HELP_STRING([--with-fam], [FAM]))
AC_ARG_WITH(fftw, AC_HELP_STRING([--with-fftw], [FFTW]))
AC_ARG_WITH(gdbm, AC_HELP_STRING([--with-gdbm], [GDBM]))
AC_ARG_WITH(gtk, AC_HELP_STRING([--with-gtk], [GTK+]))
AC_ARG_WITH(ogg, AC_HELP_STRING([--with-ogg], [Ogg]))
AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl], [OpenSSL]))
AC_ARG_WITH(pam, AC_HELP_STRING([--with-pam], [PAM]))
AC_ARG_WITH(qt, AC_HELP_STRING([--with-qt], [Qt]))
AC_ARG_WITH(speex, AC_HELP_STRING([--with-speex], [Speex]))
AC_ARG_WITH(vorbis, AC_HELP_STRING([--with-vorbis], [Vorbis]))
AC_ARG_WITH(xplc, AC_HELP_STRING([--with-xplc], [XPLC]))
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib], [zlib]))

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

# fam
if test "$with_fam" != "no"; then
    AC_CHECK_HEADERS(fam.h,, [with_fam=no])
    AC_CHECK_LIB(fam, FAMOpen,, [with_fam=no])
fi

# fftw
if test "$with_fftw" != "no"; then
    AC_CHECK_HEADERS(rfftw.h,, [with_fftw=no])
    AC_CHECK_LIB(fftw, fftw,, [with_fftw=no])
    AC_CHECK_LIB(rfftw, rfftw,, [with_fftw=no])
fi

# gdbm
if test "$with_gdbm" != "no"; then
    AC_CHECK_HEADERS(gdbm.h,, [with_gdbm=no])
    AC_CHECK_LIB(gdbm, gdbm_open,, [with_gdbm=no])
fi

# gtk
if test "$with_gtk" != "no"; then
    CPPFLAGS_save="$CPPFLAGS"
    WV_APPEND(CPPFLAGS,`pkg-config --cflags gtk+ 2>/dev/null`)
    AC_CHECK_HEADERS(gtk/gtk.h,, [with_gtk=no])
    AC_CHECK_LIB(gdk, gdk_init,, [with_gtk=no])
    AC_CHECK_LIB(gtk, gtk_main,, [with_gtk=no])
    test "$with_gtk" = no && CPPFLAGS="$CPPFLAGS_save"
fi

# ogg
if test "$with_ogg" != "no"; then
    AC_CHECK_HEADERS(ogg/ogg.h,, [with_ogg=no])
    AC_CHECK_LIB(ogg, ogg_stream_init,, [with_ogg=no])
fi

# openssl
if test "$with_openssl" != "no"; then
    AC_CHECK_HEADERS(openssl/ssl.h,, [with_openssl=no])
    AC_CHECK_LIB(ssl, SSL_CTX_new,, [with_openssl=no])
fi

# pam
if test "$with_pam" != "no"; then
    AC_CHECK_HEADERS(security/pam_appl.h,, [with_pam=no])
    AC_CHECK_LIB(pam, pam_start,, [with_pam=no])
fi

# qt
if test "$with_qt" != "no"; then
    test "$with_qt" = yes && with_qt=
    AC_CACHE_CHECK([for Qt], [wv_cv_with_qt], [
        wv_cv_with_qt=no
        CPPFLAGS_save="$CPPFLAGS"
        for wv_qtdir in $with_qt $QTDIR $QT_SEARCH_PATH; do
            CPPFLAGS="$CPPFLAGS_save -I$wv_qtdir/include"
            AC_TRY_COMPILE([#include <qapplication.h>],
                           [QApplication app(0, 0); return app.exec()],
                           [wv_cv_with_qt=$wv_qtdir; break])
        done
        CPPFLAGS="$CPPFLAGS_save"
    ])
    with_qt=$wv_cv_with_qt
    if test "$with_qt" != no; then
        WV_APPEND(CPPFLAGS, -I$with_qt/include)
        WV_APPEND(LDFLAGS, -L$with_qt/lib)
    fi
    AC_PATH_PROG(MOC, moc, [moc not found], $with_qt/bin)
fi

# speex
if test "$with_speex" != "no"; then
    AC_CHECK_HEADERS(speex.h speex_header.h,, [with_speex=no])
    AC_CHECK_LIB(speex, speex_encoder_init,, [with_speex=no])
fi

# vorbis
if test "$with_vorbis" != "no"; then
    AC_CHECK_HEADERS(vorbis/codec.h vorbis/vorbisenc.h,, [with_vorbis=no])
    AC_CHECK_LIB(vorbis, vorbis_info_init,, [with_vorbis=no])
    AC_CHECK_LIB(vorbisenc, vorbis_encode_init,, [with_vorbis=no])
fi

# xplc
if test "$with_xplc" != "no"; then
    AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no])
    AC_CHECK_LIB(xplc, XPLC_getServiceManager,, [with_xplc=no], -lxplc-cxx)
fi

# zlib
if test "$with_zlib" != "no"; then
    AC_CHECK_HEADERS(zlib.h,, [with_zlib=no])
    AC_CHECK_LIB(z, compress,, [with_zlib=no])
fi

# check for missing packages
missing_required=
missing_devel=
if test "$with_fam" = "no"; then
    AC_MSG_WARN([FAM is missing.])
    missing_devel=yes
fi
if test "$with_fftw" = "no"; then
    AC_MSG_WARN([FFTW is missing.])
    missing_devel=yes
fi
if test "$with_gdbm" = "no"; then
    AC_MSG_WARN([GDBM is missing.])
    missing_devel=yes
fi
if test "$with_gtk" = "no"; then
    AC_MSG_WARN([GTK+ is missing.])
    missing_devel=yes
fi
if test "$with_ogg" = "no"; then
    AC_MSG_WARN([Ogg is missing.])
    missing_devel=yes
fi
if test "$with_openssl" = "no"; then
    AC_MSG_WARN([OpenSSL is missing and is required.])
    missing_required=yes
fi
if test "$with_pam" = "no"; then
    AC_MSG_WARN([PAM is missing.])
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
fi
if test "$with_zlib" = "no"; then
    AC_MSG_WARN([zlib is missing and is required.])
    missing_required=yes
fi

if test -n "$missing_required"; then
    AC_MSG_ERROR([Required dependencies missing.])
fi

if test "$enable_devel" = "yes"; then
    if test -n "$missing_devel"; then
        AC_MSG_ERROR([Some packages required in developer mode are missing.])
    fi
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

AC_SUBST(with_fam)
AC_SUBST(with_fftw)
AC_SUBST(with_gdbm)
AC_SUBST(with_gtk)
AC_SUBST(with_ogg)
AC_SUBST(with_openssl)
AC_SUBST(with_pam)
AC_SUBST(with_qt)
AC_SUBST(with_speex)
AC_SUBST(with_vorbis)
AC_SUBST(with_xplc)
AC_SUBST(with_zlib)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

touch include/wvautoconf.h

