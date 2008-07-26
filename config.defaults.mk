COMPILER_STANDARD=posix
EXEEXT=
INSTALL=/usr/bin/install -c
INSTALL_DATA=${INSTALL} -m 644
INSTALL_PROGRAM=${INSTALL}
INSTALL_SCRIPT=${INSTALL}
LN_S=ln -s
LN=ln
MOC=/usr/bin/moc

LIBS_XPLC=-lxplc-cxx -lxplc
LIBS_DBUS=-ldbus-1
LIBS_QT=-lqt-mt
LIBS_PAM=-lpam
LIBS_TCL=

prefix=/usr/local
datadir=${prefix}/share
includedir=${prefix}/include
infodir=${prefix}/share/info
localstatedir=${prefix}/var
mandir=${prefix}/share/man
sharedstatedir=${prefix}/com
sysconfdir=${prefix}/etc

exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
libexecdir=${exec_prefix}/libexec
sbindir=${exec_prefix}/sbin

enable_debug=yes
enable_optimization=no
enable_resolver_fork=
enable_warnings=
enable_testgui=

with_dbus=
with_openssl=../wvports/openssl/build/openssl
with_pam=
with_tcl=no
with_readline=no
with_qt=/usr
with_xplc=../wvports/xplc/build/xplc
with_zlib=
