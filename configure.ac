# Process this file with autoconf to produce a configure script.
AC_INIT(wvstreams, 3.71, wvstreams-dev@nit.ca)

AC_REVISION($Id: configure.ac,v 1.6 2002/10/23 10:58:24 jbrown Exp $)

AC_CONFIG_SRCDIR(streams/wvstream.cc)

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [debugging options]))
AC_SUBST(enable_debug)

AC_ARG_ENABLE(fatal-warnings,
              AC_HELP_STRING([--enable-fatal-warnings],
                             [turn warnings into errors]))
AC_SUBST(enable_fatal_warnings)

AC_ARG_ENABLE(optimization,
              AC_HELP_STRING([--disable-optimization],
                             [optimization options]))
AC_SUBST(enable_optimization)

AC_ARG_ENABLE(warnings,
              AC_HELP_STRING([--disable-warnings],
                             [extra warnings]))
AC_SUBST(enable_warnings)

AC_ARG_ENABLE(rtti,
              AC_HELP_STRING([--enable-rtti],
                             [C++ run-time type identification (compiler)]))
AC_SUBST(enable_rtti)

AC_ARG_ENABLE(exceptions,
              AC_HELP_STRING([--enable-exceptions],
                             [C++ exceptions (compiler)]))
AC_SUBST(enable_exceptions)

AC_ARG_ENABLE(efence,
              AC_HELP_STRING([--enable-efence],
                             [Electric Fence]))
AC_SUBST(enable_efence)

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# system crypt function
AC_CHECK_LIB(crypto, crypt)

# open ssl
AC_CHECK_LIB(ssl, SSL_CTX_new)
AC_CHECK_HEADERS(openssl/ssl.h openssl/evp.h)
if test x$ac_cv_lib_ssl_SSL_CTX_new == xyes &&
    test x$ac_cv_header_openssl_ssl_h == xyes &&
    test x$ac_cv_header_openssl_evp_h == xyes; then
    INCLUDE_CRYPTO=yes
else
    AC_MSG_ERROR([Cannot find OpenSSL development package])
fi
AC_SUBST(INCLUDE_CRYPTO)

# zlib
AC_CHECK_LIB(z, compress)
AC_CHECK_HEADERS(zlib.h)
if test x$ac_cv_lib_z_compress == xyes &&
    test x$ac_cv_header_zlib_h == xyes; then
    INCLUDE_GZIP=yes
else
    AC_MSG_ERROR([Cannot find zlib development package])
fi
AC_SUBST(INCLUDE_GZIP)

# ogg vorbis
AC_CHECK_LIB(ogg, ogg_stream_init)
AC_CHECK_LIB(vorbis, vorbis_info_init)
AC_CHECK_LIB(vorbisenc, vorbis_encode_init)
AC_CHECK_HEADERS(ogg/ogg.h vorbis/codec.h vorbis/vorbisenc.h)
if test x$ac_cv_lib_ogg_ogg_stream_init == xyes &&
    test x$ac_cv_lib_vorbis_vorbis_info_init == xyes &&
    test x$ac_cv_lib_vorbisenc_vorbis_encode_init == xyes &&
    test x$ac_cv_header_ogg_ogg_h == xyes &&
    test x$ac_cv_header_vorbis_codec_h == xyes &&
    test x$ac_cv_header_vorbis_vorbisenc_h == xyes; then
    INCLUDE_OGGVORBIS=yes
else
    AC_MSG_WARN([Cannot find oggvorbis development package so excluding Ogg Vorbis support])
fi
AC_SUBST(INCLUDE_OGGVORBIS)
    
# fftw
AC_CHECK_LIB(fftw, fftw)
AC_CHECK_LIB(rfftw, rfftw)
AC_CHECK_HEADERS(fftw.h rfftw.h)
if test x$ac_cv_lib_fftw_fftw == xyes &&
    test x$ac_cv_lib_rfftw_rfftw == xyes &&
    test x$ac_cv_header_fftw_h == xyes &&
    test x$ac_cv_header_rfftw_h == xyes; then
    INCLUDE_FFT=yes
else
    AC_MSG_WARN([Cannot find fftw development package so excluding FFT support])
fi
AC_SUBST(INCLUDE_FFT)

# qt
AC_CHECK_LIB(qt, main)
#AC_CHECK_HEADERS(qt/qsocketnotifier.h)
if test x$ac_cv_lib_qt_main == xyes; then
    INCLUDE_QT=yes
else
    AC_MSG_WARN([Cannot find qt development package so exlucing Qt support])
fi
AC_SUBST(INCLUDE_QT)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

