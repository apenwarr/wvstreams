# Process this file with autoconf to produce a configure script.
AC_INIT(wvstreams, 3.74, wvstreams-dev@nit.ca)

AC_REVISION($Id: configure.ac,v 1.29 2003/04/29 15:37:29 jim Exp $)

echo $0 "$*" > reconfigure
chmod +x reconfigure

AC_CONFIG_SRCDIR(streams/wvstream.cc)

test -z "${WVPACKAGES}" && {
  AC_MSG_ERROR([The environment variable WVPACKAGES needs to be defined.])
}

AC_ARG_ENABLE(debug,
	      AC_HELP_STRING([--enable-debug],
			     [link debug information]),
	      [],
	      [enable_debug="no"])
AC_SUBST(enable_debug)
AC_MSG_CHECKING([for --enable-debug])
AC_MSG_RESULT([$enable_debug])


AC_ARG_ENABLE(efence,
	      AC_HELP_STRING([--enable-efence],
			     [link against Electric Fence]),
	      [enable_debug="yes"],
	      [enable_efence="$enable_debug"])
AC_SUBST(enable_efence)
AC_MSG_CHECKING([for --enable-efence])
AC_MSG_RESULT([$enable_efence])


AC_ARG_ENABLE(verbose,
	      AC_HELP_STRING([--enable-verbose],
			     [verbose compilation output]),
	      [],
	      [enable_verbose="no"])
AC_SUBST(enable_verbose)
AC_MSG_CHECKING([for --enable-verbose])
AC_MSG_RESULT([$enable_verbose])

AC_ARG_ENABLE(devel,
              AC_HELP_STRING([--enable-devel],
                             [developer options]),
	      [],
	      [enable_devel="no"])

AC_ARG_ENABLE(fatal-warnings,
              AC_HELP_STRING([--enable-fatal-warnings],
                             [turn warnings into errors]),
	      [],
	      [enable_fatal_warnings="no"])

AC_ARG_ENABLE(optimization,
              AC_HELP_STRING([--disable-optimization],
                             [optimization options]),
	      [],
	      [enable_optimization="yes"])

AC_ARG_ENABLE(warnings,
              AC_HELP_STRING([--disable-warnings],
                             [extra warnings]),
	      [],
	      [enable_warnings="yes"])

AC_ARG_ENABLE(rtti,
              AC_HELP_STRING([--enable-rtti],
                             [C++ run-time type identification (compiler)]),
	      [],
	      [enable_rtti="no"])

AC_ARG_ENABLE(exceptions,
              AC_HELP_STRING([--enable-exceptions],
                             [C++ exceptions (compiler)]),
	      [],
	      [enable_exceptions="no"])

AC_ARG_ENABLE(resolver-fork,
              AC_HELP_STRING([--disable-resolver-fork],
                 [WvResolver background name resolution (debugging)]),
              [],
	      [enable_resolver_fork="yes"])

AC_ARG_WITH(openssl,
            AC_HELP_STRING([--without-openssl],
                           [OpenSSL]),
	    [],
	    [with_openssl="yes"])

AC_ARG_WITH(zlib,
            AC_HELP_STRING([--without-zlib],
                           [zlib]),
	    [],
	    [with_zlib="yes"])

AC_ARG_WITH(ogg,
            AC_HELP_STRING([--with-ogg],
                           [Ogg]),
	    [],
	    [with_ogg="no"])

AC_ARG_WITH(vorbis,
            AC_HELP_STRING([--with-vorbis],
                           [Vorbis]),
	    [with_ogg="yes"],
	    [with_vorbis="no"])

AC_ARG_WITH(speex,
            AC_HELP_STRING([--with-speex],
                           [Speex]),
	    [with_ogg="yes"],
	    [with_speex="yes"])

AC_ARG_WITH(fam,
            AC_HELP_STRING([--with-fam],
                           [FAM]),
	    [],
	    [with_fam="no"])

dnl gdbm is in use by wvpackages' vacation, slapd, imapd, yp,
dnl so no harm in allowing it for niti
AC_ARG_WITH(gdbm,
            AC_HELP_STRING([--without-gdbm],
                           [GDBM]),
	    [],
	    [with_gdbm="yes"])

AC_ARG_WITH(fftw,
            AC_HELP_STRING([--with-fftw],
                           [FFTW]),
	    [],
	    [with_fftw="no"])

dnl libpam is in use by wvpackages' afpd, proftpd, apache/libphp4.so
dnl so no harm in allowing it for niti
AC_ARG_WITH(pam,
            AC_HELP_STRING([--without-pam],
                           [PAM]),
	    [],
	    [with_pam="yes"])

AC_ARG_WITH(gtk,
            AC_HELP_STRING([--with-gtk],
                           [GTK]),
	    [with_glib="yes"],
	    [with_gtk="no"])

AC_ARG_WITH(glib,
            AC_HELP_STRING([--with-glib],
                           [Glib system include directory]),
	    [],
	    [with_glib="no"])

AC_ARG_WITH(qt,
	    AC_HELP_STRING([--with-qt],
			   [Qt system include directory]),
	    [],
	    [with_qt="no"])

AC_ARG_WITH(xplc,
            AC_HELP_STRING([--with-xplc],
                           [XPLC]),
	    [],
	    [with_xplc="no"])

# No macros relying on compile tests should be used above these
# AC_... compiler detection macros.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LN_S

# Setting the default language to C++ means that CXX and CXXCPP will be 
# used for compile tests.
AC_LANG(C++)

# system crypt function
AC_CHECK_LIB(crypt, crypt)
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

# fam
if test "$with_fam" != "no"; then
    AC_CHECK_LIB(fam, FAMOpen,, [with_fam=no])
    AC_CHECK_HEADERS(fam.h,, [with_fam=no])
fi
if test "$with_fam" != "no"; then
    AC_DEFINE(WITH_FAM,,
              [File Alteration Monitor])
fi

# gdbm
if test "$with_gdbm" != "no"; then
    AC_CHECK_LIB(gdbm, gdbm_open,, [with_gdbm=no])
    AC_CHECK_HEADERS(gdbm.h,, [with_gdbm=no])
fi
if test "$with_gdbm" != "no"; then
    AC_DEFINE(WITH_GDBM,,
              [GDBM Hash])
fi

# ogg
if test "$with_ogg" != "no"; then
    AC_CHECK_LIB(ogg, ogg_stream_init,, [with_ogg=no])
    AC_CHECK_HEADERS(ogg/ogg.h,, [with_ogg=no])
fi
if test "$with_ogg" != "no"; then
    # vorbis
    if test "$with_vorbis" != "no"; then
        AC_CHECK_LIB(vorbis, vorbis_info_init,, [with_vorbis=no])
        AC_CHECK_LIB(vorbisenc, vorbis_encode_init,, [with_vorbis=no])
        AC_CHECK_HEADERS(vorbis/codec.h vorbis/vorbisenc.h,, [with_vorbis=no])
    fi
    # speex
    if test "$with_speex" != "no"; then
        AC_CHECK_LIB(speex, speex_encoder_init,, [with_speex=no])
        AC_CHECK_HEADERS(speex.h speex_header.h,, [with_speex=no])
    fi
else
    with_vorbis="no"
    with_speex="no"
fi

# fftw
if test "$with_fftw" != "no"; then
AC_CHECK_LIB(fftw, fftw,, [with_fftw=no])
AC_CHECK_LIB(rfftw, rfftw,, [with_fftw=no])
AC_CHECK_HEADERS(rfftw.h,, [with_fftw=no])
fi

# pam
if test "$with_pam" != "no"; then
AC_CHECK_LIB([pam], [pam_start], [], [with_pam=no])
AC_CHECK_HEADERS([security/pam_appl.h], [], [with_pam=no])
fi

# glib
# FIXME: accept system library path
if test "${with_glib}" != "no" ; then
  AC_CHECK_LIB([glib], [g_main_run], 
    [
     GLIBLIB="-lglib"
     AC_DEFINE([HAVE_LIBGLIB], [1], [Do we have Glib library?])
    ], 
    [with_glib="no"])
fi

case "${with_glib}" in
  ("no")
    ;;
  ("yes" | "")
    GLIBINC="glib"
    test -d "/usr/include/${GLIBINC}" || GLIBINC="glib-1.2"
    GLIBINCLUDE="-I/usr/include/${GLIBINC} -I/usr/lib/glib/include"
    ;;
  (*)
    GLIBINCLUDE="-I${with_glib}/include"
    ;;
esac

if test "${GLIBINCLUDE}" != "" ; then
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="${CPPFLAGS} ${GLIBINCLUDE}"
  AC_CHECK_HEADERS([glib.h], [], [with_glib="no"])
  AC_CHECK_HEADERS([glibconfig.h], [], [with_glib="no"])
  CPPFLAGS="$save_CPPFLAGS"
fi


#gtk
if test "$with_gtk" != "no"; then

  AC_CHECK_LIB(gdk, gdk_init,, [with_gtk=no])
  AC_CHECK_LIB(gtk, gtk_main,, [with_gtk=no])

  if test "$with_gtk" != "yes"; then
    GTKDIR=$with_gtk
  fi
  if test -n "$GTKDIR" -a -d "$GTKDIR/include"; then
    GTKINCLUDE="-I$GTKDIR/include"
  else
    GTKINCLUDE="-I/usr/include/gtk-1.2"
  fi
  
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $GTKINCLUDE $GLIBINCLUDE"
  AC_CHECK_HEADERS(gtk/gtk.h, [], [with_gtk=no])
  CPPFLAGS="$save_CPPFLAGS"
fi


# xplc
case "${with_xplc}" in
  ("yes" | "")
    with_xplc="${WVPACKAGES}/xplc"
    ;;
esac

if test "$with_xplc" != "no" ; then
  LDFLAGS="$LDFLAGS -L${with_xplc}"
  CPPFLAGS="$CPPFLAGS -I${with_xplc}/include"
fi

if test "$with_xplc" != "no"; then
AC_CHECK_LIB(xplc, XPLC_getServiceManager,, [with_xplc=no], -lxplc-cxx)
AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no])
fi


# Qt 
# FIXME: accept system library path
case "${with_qt}" in
  ("no")
    ;;
  ("yes" | "")
    QTINC="qt3"
    test -d /usr/include/${QTINC} || QTINC="qt"
    QTINCLUDE="-I/usr/include/${QTINC}"
    ;;
  (*)
    QTINCLUDE="-I${with_qt}/include"
    ;;
esac

if test "${QTINCLUDE}" != "" ; then
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="${CPPFLAGS} ${QTINCLUDE}"
  AC_CHECK_HEADERS([qobject.h], [], [with_qt="no"])
  CPPFLAGS="$save_CPPFLAGS"
fi

if test "${with_qt}" != "no" ; then
  AC_MSG_CHECKING([for Qt library and its ABI compatibility with $CXX])

  function qt_test() {
	AC_LINK_IFELSE([
	    AC_LANG_SOURCE([[
		#include <qapplication.h>

		int main( int argc, char **argv )
		{
		    QApplication a(argc,argv);
		    return a.exec();
		}
		]])
	    ],
	    [qt_found="yes"],
	    [qt_found="no"])
  }

  save_CPPFLAGS="$CPPFLAGS"
  save_LIBS="$LIBS"
  
  CPPFLAGS="${CPPFLAGS} ${QTINCLUDE}"
  QTLIB="-lqt-mt"
  LIBS="${save_LIBS} ${QTLIB}"

  qt_test
  if test "${qt_found}" = "yes" ; then
	AC_MSG_RESULT([ok])
  else
	QTLIB="-lqt"
	LIBS="${save_LIBS} ${QTLIB}"
	qt_test

	if test "${qt_found}" = "yes" ; then
  		AC_MSG_RESULT([ok])
	else
		AC_MSG_RESULT([no])
	fi
  fi

  if test "${qt_found}" = "yes" ; then
  	AC_DEFINE([HAVE_LIBQT], [1], [Do we have Qt library?])
  else
  	QTLIB=""
	QTINCLUDE=""
	with_qt="no"
  fi

  LIBS="$save_LIBS"
  CPPFLAGS="$save_CPPFLAGS"
fi

# resolver fork
if test "$enable_resolver_fork" == no; then
AC_DEFINE(WVRESOLVER_SKIP_FORK,,
          [Define to disable WvResolver forking for debugging with gdb.])
fi

if test "$with_openssl" == "no"; then
    AC_MSG_WARN([OpenSSL is missing.])
    missing_devel=yes
fi
if test "$with_zlib" == "no"; then
    AC_MSG_WARN([zlib is missing.])
    missing_devel=yes
fi
if test "$with_ogg" == "no"; then
    AC_MSG_WARN([Ogg is missing.])
    missing_devel=yes
fi
if test "$with_vorbis" == "no"; then
    AC_MSG_WARN([Vorbis is missing.])
    missing_devel=yes
fi
if test "$with_speex" == "no"; then
    AC_MSG_WARN([Speex is missing.])
    missing_devel=yes
fi
if test "$with_fftw" == "no"; then
    AC_MSG_WARN([FFTW is missing.])
    missing_devel=yes
fi
if test "$with_pam" == "no"; then
    AC_MSG_WARN([PAM is missing.])
    missing_devel=yes
fi
if test "$with_qt" == "no"; then
    AC_MSG_WARN([Qt is missing.])
    missing_devel=yes
fi
if test "$with_gtk" == "no"; then
    AC_MSG_WARN([Gtk is missing.])
    missing_devel=yes
fi
if test "$with_xplc" == "no"; then
    AC_MSG_WARN([XPLC is missing.])
#    missing_devel=yes
fi

if test "$enable_devel" == "yes"; then
    enable_debug=yes
    enable_fatal_warnings=yes
    enable_warnings=yes
    if test "$missing_devel" == "yes"; then
        AC_MSG_ERROR([Some packages required in developer mode are missing.])
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
AC_SUBST(with_pam)

AC_SUBST(with_glib)
AC_SUBST(GLIBINCLUDE)
AC_SUBST(GLIBLIB)

AC_SUBST(with_gtk)
AC_SUBST(with_xplc)
AC_SUBST(with_fam)
AC_SUBST(with_gdbm)

AC_SUBST(with_qt)
AC_SUBST(QTINCLUDE)
AC_SUBST(QTLIB)

AC_SUBST(CXX)
AC_SUBST(CC)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

touch include/wvautoconf.h

