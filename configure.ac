# Process this file with autoconf to produce a configure script.
AC_INIT(wvstreams, 3.71, wvstreams-dev@nit.ca)

AC_REVISION($Id: configure.ac,v 1.22 2003/03/28 23:55:35 drheld Exp $)

echo $0 "$*" > reconfigure
chmod +x reconfigure

AC_CONFIG_SRCDIR(streams/wvstream.cc)

AC_ARG_ENABLE(devel,
              AC_HELP_STRING([--enable-devel],
                             [developer options]))

AC_ARG_ENABLE(fatal-warnings,
              AC_HELP_STRING([--enable-fatal-warnings],
                             [turn warnings into errors]))

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

AC_ARG_WITH(fam,
            AC_HELP_STRING([--with-fam],
                           [FAM]))

AC_ARG_WITH(fftw,
            AC_HELP_STRING([--with-fftw],
                           [FFTW]))

AC_ARG_WITH(pam,
            AC_HELP_STRING([--with-pam],
                           [PAM]))

AC_ARG_WITH(gtk,
            AC_HELP_STRING([--with-gtk],
                           [GTK]))

AC_ARG_WITH(glib,
            AC_HELP_STRING([--with-glib],
                           [Location of Glib headers]))

AC_ARG_WITH(xplc,
            AC_HELP_STRING([--with-xplc],
                           [XPLC]),
            LDFLAGS="$CXXFLAGS -L$withval" CPPFLAGS="$CPPFLAGS -I$withval/include")

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--disable-debug],
                             [strip debug information]),
              [],
              [enable_debug="yes"])
AC_SUBST(enable_debug)
AC_MSG_CHECKING([for --enable-debug])
AC_MSG_RESULT([$enable_debug])


AC_ARG_ENABLE(efence,
              AC_HELP_STRING([--enable-efence],
                             [link against Electric Fence]),
              [],
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

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

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

# qt
AC_ARG_WITH(qt,
            AC_HELP_STRING([--with-qt],
                           [Qt]),
	    [test "$withval" = "yes" -o "$withval" = "" || {
	    	QTDIR="$withval"
		with_qt="yes"
		}],
	    [])

if test "$with_qt" != "no" ; then
  AC_CHECK_LIB(qt-mt, main, 
    [QTLIB="-lqt-mt"], 
    [
      AC_CHECK_LIB(qt, main, 
        [QTLIB="-lqt"], 
	[with_qt="no"])
    ])
fi

if test "$with_qt" != "no" ; then
  if test -n "${QTDIR}"; then
    QTINCLUDE="-I${QTDIR}/include"
  else
    QTINC="qt3"
    test -d /usr/include/${QTINC} || QTINC="qt"
    QTINCLUDE="-I/usr/include/${QTINC}"
  fi
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="${CPPFLAGS} ${QTINCLUDE}"
  AC_CHECK_HEADERS([qobject.h], [], [with_qt="no"])
  CPPFLAGS="$save_CPPFLAGS"
fi


if test "$with_qt" != "no" ; then
  AC_MSG_CHECKING([for Qt vs. $CXX ABI compatibility])

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="${CPPFLAGS} ${QTINCLUDE}"
  save_LIBS="$LIBS"
  LIBS="${LIBS} ${QTLIB}"

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
	    [qt_compatible="yes"],
	    [qt_compatible="no"])
  }

  qt_test
  if [[ "$qt_compatible" = "yes" ]] ; then
	QTCXX="$CXX"
	AC_MSG_RESULT([ok])
  else
	AC_MSG_RESULT([no])
	this_compiler=`type -p $CXX`
	filter="\(^$CXX$"
	this_compiler_deref=`stat --format "%N" "$this_compiler" \
		| awk '{print $3}' | sed -e "s/\\\`\(.*\)'/\1/"`
	if [[ -n "$this_compiler_deref" ]] ; then
		filter="${filter}\|${this_compiler_deref}$"
	else
		gpp_compiler=`type -p g++`
		gpp_compiler_deref=`stat --format "%N" "$gpp_compiler" \
			| awk '{print $3}' | sed -e "s/\\\`\(.*\)'/\1/"`
		if [[ -n "$gpp_compiler_deref" ]] ; then
			filter="${filter}\|g++$"
		fi
	fi
	filter="${filter}\)"
	other_compilers=`find /usr/bin -name "g++*" -printf "%f\n" | \
		sort | \
		grep -v "$filter"`
	other_compilers_list=`echo $other_compilers`
		
	QTCXX=""
	test -n "$other_compilers" && {
		for comp in $other_compilers ; do
			save_CXX="$CXX"
			CXX="$comp"
			AC_MSG_CHECKING([for Qt vs. $comp ABI compatibility])
			qt_test
			CXX="$save_CXX"
			if test "$qt_compatible" = "yes" ; then
				QTCXX="$comp"
				break
			else
				AC_MSG_RESULT([no])
			fi
		done
	}
	if [[ -n "$QTCXX" ]] ; then
		AC_MSG_RESULT([ok])
	else
		AC_MSG_ERROR([The Qt library $QTLIB doesn't match any C++ compiler])
	fi
  fi

  LIBS="$save_LIBS"
  CPPFLAGS="$save_CPPFLAGS"
  CXX="$QTCXX"
  CC=`echo "$QTCXX" | tr \'+\' \'c\'`

  dnl AC_MSG_NOTICE([with_qt=$with_qt QTINCLUDE=$QTINCLUDE QTLIB=$QTLIB CXX=$CXX CC=$CC])
fi

AC_SUBST(with_qt)
AC_SUBST(QTINCLUDE)
AC_SUBST(QTLIB)
AC_SUBST(CXX)
AC_SUBST(CC)

#gtk
if test "$with_gtk" != "no"; then

AC_CHECK_LIB(gdk, gdk_init,, [with_gtk=no])
AC_CHECK_LIB(gtk, gtk_main,, [with_gtk=no])

if test "$with_glib" != "yes"; then
    GLIBDIR=$with_glib
fi
if test -n "$GLIBDIR"; then
    GLIBINCLUDE="$GLIBDIR/include"
else
    GLIBINCLUDE="/usr/include/glib-1.2"
fi

if test "$with_gtk" != "yes"; then
    GTKDIR=$with_gtk
fi
if test -n "$GTKDIR"; then
    GTKINCLUDE="$GTKDIR/include"
else
    GTKINCLUDE="/usr/include/gtk-1.2"
fi
CPPFLAGS="$CPPFLAGS -I$GTKINCLUDE -I$GLIBINCLUDE -I/usr/lib/glib/include"
AC_CHECK_HEADERS(gtk/gtk.h,, [with_gtk=no])
fi

# xplc
if test "$with_xplc" != "no"; then
AC_CHECK_LIB(xplc, XPLC_getServiceManager,, [with_xplc=no], -lxplc-cxx)
AC_CHECK_HEADERS(xplc/core.h,, [with_xplc=no])
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
AC_SUBST(with_qt)
AC_SUBST(QTINCLUDE)
AC_SUBST(QTLIB)
AC_SUBST(with_glib)
AC_SUBST(with_gtk)
AC_SUBST(with_xplc)
AC_SUBST(with_fam)

AC_CONFIG_FILES(config.mk)
AC_CONFIG_HEADERS(include/wvautoconf.h)
AC_OUTPUT

touch include/wvautoconf.h

