# Process this file with autoconf to produce a configure script.
AC_INIT(wvstreams, 3.71, wvstreams-dev@nit.ca)

AC_REVISION($Id: configure.ac,v 1.12 2003/01/06 20:28:01 pphaneuf Exp $)

echo $0 "$*" > reconfigure
chmod +x reconfigure

AC_CONFIG_SRCDIR(streams/wvstream.cc)

AC_ARG_ENABLE(devel,
              AC_HELP_STRING([--enable-devel],
                             [developer options]))

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [debugging options]))

AC_ARG_ENABLE(fatal-warnings,
              AC_HELP_STRING([--enable-fatal-warnings],
                             [turn warnings into errors]))

AC_ARG_ENABLE(verbose,
              AC_HELP_STRING([--enable-verbose],
                             [verbose compilation output]))

AC_ARG_ENABLE(optimization,
              AC_HELP_STRING([--disable-optimization],
                             [optimization options]))

AC_ARG_ENABLE(warnings,
              AC_HELP_STRING([--disable-warnings],
                             [extra warnings]))

AC_ARG_ENABLE(rtti,
              AC_HELP_STRING([--enable-rtti],
                             [C++ run-time type identification (compiler)]))

AC_ARG_ENABLE(exceptions,
              AC_HELP_STRING([--enable-exceptions],
                             [C++ exceptions (compiler)]))

AC_ARG_ENABLE(efence,
              AC_HELP_STRING([--enable-efence],
                             [Electric Fence]))

AC_ARG_ENABLE(resolver-fork,
              AC_HELP_STRING([--disable-resolver-fork],
                             [WvResolver background name resolution (debugging)]))

AC_ARG_WITH(openssl,
            AC_HELP_STRING([--with-openssl],
                           [OpenSSL]))

AC_ARG_WITH(zlib,
            AC_HELP_STRING([--with-zlib],
                           [zlib]))

AC_ARG_WITH(ogg,
            AC_HELP_STRING([--with-ogg],
                           [Ogg]))

AC_ARG_WITH(vorbis,
            AC_HELP_STRING([--with-vorbis],
                           [Vorbis]))

AC_ARG_WITH(speex,
            AC_HELP_STRING([--with-speex],
                           [Speex]))

AC_ARG_WITH(fftw,
            AC_HELP_STRING([--with-fftw],
                           [FFTW]))

AC_ARG_WITH(qt,
            AC_HELP_STRING([--with-qt],
                           [Qt]))

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

AC_LANG(C++)

# system crypt function
AC_CHECK_LIB(crypto, crypt)
AC_CHECK_HEADERS(crypt.h)

# open ssl
if test "$with_openssl" != "no"; then
    AC_CHECK_LIB(ssl, SSL_CTX_new,, [with_openssl=no])
    AC_CHECK_HEADERS(openssl/ssl.h,, [with_openssl=no])
fi
if test "$with_openssl" == "no"; then
    AC_MSG_ERROR([OpenSSL is missing and required.])
fi

# zlib
if test "$with_zlib" != "no"; then
    AC_CHECK_LIB(z, compress,, [with_zlib=no])
    AC_CHECK_HEADERS(zlib.h,, [with_zlib=no])
fi
if test "$with_zlib" == "no"; then
    AC_MSG_ERROR([zlib is missing and required.])
fi

# ogg
if test "$with_ogg" != "no"; then
AC_CHECK_LIB(ogg, ogg_stream_init,, [with_ogg=no])
AC_CHECK_HEADERS(ogg/ogg.h,, [with_ogg=no])
fi

# vorbis
if test "$with_ogg" != "no"; then
if test "$with_vorbis" != "no"; then
AC_CHECK_LIB(vorbis, vorbis_info_init,, [with_vorbis=no])
AC_CHECK_LIB(vorbisenc, vorbis_encode_init,, [with_vorbis=no])
AC_CHECK_HEADERS(vorbis/codec.h vorbis/vorbisenc.h,, [with_vorbis=no])
fi
else
with_vorbis="no"
fi
    
# speex
if test "$with_ogg" != "no"; then
if test "$with_speex" != "no"; then
AC_CHECK_LIB(speex, speex_encoder_init,, [with_speex=no])
AC_CHECK_HEADERS(speex.h speex_header.h,, [with_speex=no])
fi
else
with_speex="no"
fi

# fftw
if test "$with_fftw" != "no"; then
AC_CHECK_LIB(fftw, fftw,, [with_fftw=no])
AC_CHECK_LIB(rfftw, rfftw,, [with_fftw=no])
AC_CHECK_HEADERS(rfftw.h,, [with_fftw=no])
fi

# qt
if test "$with_qt" != "no"; then
AC_CHECK_LIB(qt, main,, [with_qt=no])
if test -n "$with_qt"; then
    QTDIR=$with_qt
fi
if test -n "$QTDIR"; then
    QTINCLUDE="$QTDIR/include"
else
    QTINCLUDE="/usr/include/qt"
fi
CPPFLAGS="$CPPFLAGS -I$QTINCLUDE"
AC_CHECK_HEADERS(qt/qobject.h,, [with_qt=no])
fi

# resolver fork
if test "$enable_resolver_fork" == no; then
AC_DEFINE(WVRESOLVER_SKIP_FORK,,
          [Define to disable WvResolver forking for debugging with gdb.])
fi

if test "$enable_devel" == "yes"; then
    enable_debug=yes
    enable_fatal_warnings=yes
    enable_warnings=yes
    if test "$with_openssl" == "no"; then
	AC_MSG_ERROR([OpenSSL is missing and required.])
    fi
    if test "$with_zlib" == "no"; then
	AC_MSG_ERROR([zlib is missing and required.])
    fi
    if test "$with_ogg" == "no"; then
	AC_MSG_ERROR([Ogg is missing and required.])
    fi
    if test "$with_vorbis" == "no"; then
	AC_MSG_ERROR([Vorbis is missing and required.])
    fi
    if test "$with_speex" == "no"; then
	AC_MSG_ERROR([Speex is missing and required.])
    fi
    if test "$with_fftw" == "no"; then
	AC_MSG_ERROR([FFTW is missing and required.])
    fi
    if test "$with_qt" == "no"; then
	AC_MSG_ERROR([Qt is missing and required.])
    fi
fi

AC_SUBST(enable_debug)
AC_SUBST(enable_fatal_warnings)
AC_SUBST(enable_verbose)
AC_SUBST(enable_optimization)
AC_SUBST(enable_warnings)
AC_SUBST(enable_rtti)
AC_SUBST(enable_exceptions)
AC_SUBST(enable_efence)
AC_SUBST(enable_resolver_fork)
AC_SUBST(with_openssl)
AC_SUBST(with_zlib)
AC_SUBST(with_ogg)
AC_SUBST(with_vorbis)
AC_SUBST(with_speex)
AC_SUBST(with_fftw)
AC_SUBST(with_qt)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

