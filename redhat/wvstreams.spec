Summary: C++ network libraries for rapid application development.
Name: wvstreams4.0
Version: 4.0
Release: 1
Source: http://open.nit.ca/download/wvstreams-%{version}.tar.gz
URL: http://open.nit.ca/wvstreams
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: fam-devel, fftw-devel, libogg-devel, libvorbis-devel, openssl-devel, openslp-devel, pam-devel, qdbm-devel, speex-devel, xplc-devel >= 0.3.7
Group: None
License: LGPL

%description
None

%package -n libwvstreams4.0-base
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries

%description -n libwvstreams4.0-base
C++ network libraries for rapid application development.

WvStreams is a library suite that is comprised of several parts.  Included
in the base package are:
 * WvString: a convenient and efficient C++ string class
 * WvList: an easy-to-use linked list
 * WvHashTable: an efficient and easy-to-use hash table
 * WvFile: a WvStream wrapper for handling files
 * WvStreamClone: a base class which makes writing your own WvStreams easy
 * WvLog: a log files handler
 * UniIniGen: a tiny version of UniConf for simple configuration systems

%package -n libwvstreams4.0-extras
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, fam, openssl, openslp, pam, qdbm

%description -n libwvstreams4.0-extras
C++ network libraries for rapid application development.

WvStreams is a library suite that is comprised of several parts.  Included
in the extras package are:
 * WvIPStreams: which includes WvTCPStream and WvUDPStream
 * WvCrypto streams: a REALLY easy way to add SSL support to applications

These are the base classes used to build programs such as the ever popular
WvDial, TunnelVision, FastForward, KWvDial, retchmail, and many more yet
to come. ;)

%package -n libwvstreams4.0-fft
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, libwvstreams4.0-extras, fftw

%description -n libwvstreams4.0-fft
This package contains the library necessary for the WvStreams to libfftw
wrapper. It enables WvStreams programs to easily handle Fast-Fourier
transforms, instead of forcing the programmer to use the much harder to use
libfftw interface.

%package -n libwvstreams4.0-qt
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, libwvstreams4.0-extras, qt

%description -n libwvstreams4.0-qt
This package contains the library necessary to tie WvStreams and Qt program
event loops together to enable WvStreams to act as the I/O and configuration
back end for Qt and KDE.

%package -n libwvstreams4.0-speex
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, libwvstreams4.0-extras, libogg, speex

%description -n libwvstreams4.0-speex
This library contains the WvSpeexEncoder and WvSpexxDecoder to enable quick 
and painless creation of audio streams using the Speex Voice over IP CODEC

%package -n libwvstreams4.0-vorbis
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, libwvstreams4.0-extras, libogg, libvorbis

%description -n libwvstreams4.0-vorbis
This library contains the WvOggEncoder and WvOggDecoder to enable quick and
painless creation of audio streams using the OggVorbis CODEC

%package -n libuniconf4.0
Summary: C++ network libraries for rapid application development.
Group: System Environment/Libraries
Requires: libwvstreams4.0-base, libwvstreams4.0-extras

%description -n libuniconf4.0
C++ network libraries for rapid application development.

UniConf is a configuration system that can serve as the centrepiece among
many other, existing configuration systems, such as:
 * GConf
 * KConfig
 * Windows registry
 * Mutt ;)

UniConf can also be accessed over the network, with authentication, allowing
easy replication of configuration data via the UniReplicateGen.

%package -n libwvstreams4.0-devel
Summary: Headers, development libraries and documentation for the WvStreams library
Group: Development/Libraries
Requires: libuniconf4.0, libwvstreams4.0-base, libwvstreams4.0-extras, libwvstreams4.0-fft, libwvstreams4.0-qt, libwvstreams4.0-speex, libwvstreams4.0-vorbis

%description -n libwvstreams4.0-devel
C++ network libraries for rapid application development.

This package contains header files and development libraries needed to
develop programs using the WvStreams networking library.

%package -n uniconfd
Summary: Server that manages UniConf elements
Group: System Environment/Libraries
Requires: libuniconf4.0

%description -n uniconfd
UniConf is a configuration system that can serve as the centrepiece among
many other, existing configuration systems.

UniConf can also be accessed over the network, with authentication, allowing
easy replication of configuration data via the UniReplicateGen.

This package contains the server that accepts incoming TCP or Unix
connections, and gets or sets UniConf elements at the request of a
UniConf client.

%post -n uniconfd
/sbin/chkconfig --add uniconfd

%preun
if [ $1 = 0 ]; then
    service uniconfd stop > /dev/null 2>&1
    /sbin/chkconfig --del uniconfd
fi

%prep
%setup -q -n wvstreams-%{version}

%build
export CXXFLAGS=-I/usr/kerberos/include # stupid redhat 9 kerberos..
%configure \
    --disable-debug --disable-verbose \
    --with-qt --with-vorbis --with-speex --with-openslp \
    --with-fam --with-fftw \
    --with-xplc=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
install -d 644 $RPM_BUILD_ROOT/etc/rc.d/init.d/
install -m 755 redhat/uniconfd.init $RPM_BUILD_ROOT/etc/rc.d/init.d/uniconfd
chmod 755 $RPM_BUILD_ROOT/usr/lib/*.so.*
install -d 644 $RPM_BUILD_ROOT/usr/share/man/man8
gzip $RPM_BUILD_ROOT/usr/share/man/uniconfd.8
mv $RPM_BUILD_ROOT/usr/share/man/uniconfd.8.gz $RPM_BUILD_ROOT/usr/share/man/man8

%clean
rm -rf $RPM_BUILD_ROOT

%files -n libwvstreams4.0-base
%defattr(-,root,root,-)
%doc COPYING.LIB README
/usr/lib/libwvbase.so.4.0

%files -n libwvstreams4.0-extras
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libwvstreams.so.4.0
/usr/lib/libwvutils.so.4.0

%files -n libwvstreams4.0-fft
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libwvfft.so.4.0

%files -n libwvstreams4.0-qt
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libwvqt.so.4.0

%files -n libwvstreams4.0-speex
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libwvoggspeex.so.4.0

%files -n libwvstreams4.0-vorbis
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libwvoggvorbis.so.4.0

%files -n libuniconf4.0
%defattr(-,root,root,-)
%doc COPYING.LIB
/usr/lib/libuniconf.so.4.0

%files -n libwvstreams4.0-devel
%defattr(-,root,root)
/usr/include/wvstreams
/usr/lib/*.a
/usr/lib/*.so
/usr/lib/pkgconfig/*pc

%files -n uniconfd
%defattr(-,root,root)
%doc COPYING.LIB
%doc uniconf/daemon/sample.ini
/etc/uniconf.conf
%config /etc/rc.d/init.d/uniconfd
/usr/sbin/uniconfd
/usr/share/man/man8/uniconfd.8.gz
/var/lib/uniconf/uniconfd.ini

%changelog
* Tue Sep  7 2004 William Lachance <wlach@nit.ca>
- New upstream release, split into seperate packages.

* Mon Jul 29 2002 Patrick Patterson <ppatters@nit.ca>
- Synchronise with Upstream

* Wed Feb 27 2002 Nalin Dahyabhai <nalin@redhat.com>
- merge the main and -devel packages into one .spec file
- use globbing to shorten the file lists
- don't define name, version, and release as macros (RPM does this by default)
- use the License: tag instead of Copyright: (equivalent at the package level,
  but License: reflects the intent of the tag better)
- use a URL to point to the source of the source tarball
- add BuildRequires: openssl-devel (libwvcrypto uses libcrypto)
- move the buildroot to be under %%{_tmppath}, so that it can be moved by
  altering RPM's configuration

* Tue Jan 29 2002 Patrick Patterson <ppatters@nit.ca>
- Initial Release of WvStreams
