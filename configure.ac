# Process this file with autoconf to produce a configure script.
AC_INIT(wvstreams, 3.71, wvstreams-dev@nit.ca)

AC_REVISION($Id: configure.ac,v 1.2 2002/09/09 19:45:02 pphaneuf Exp $)

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

AC_PROG_CXX
AC_PROG_CC

AC_CHECK_LIB(crypto, crypt)
AC_CHECK_LIB(ssl, SSL_CTX_new)
AC_CHECK_LIB(z, compress)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

