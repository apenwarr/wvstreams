Name: libwvstreams
Version: 3.80
Release: 1
Summary: WvStreams is a network programming library written in C++.
Source: http://open.nit.ca/download/wvstreams-%{version}.tar.gz
URL: http://open.nit.ca/wvstreams
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/libwvstreams-root
BuildRequires: openssl-devel
License: LGPL

%description
WvStreams aims to be an efficient, secure, and easy-to-use library for
doing network applications development.

%package devel
Summary: Development files for WvStreams.
Group: Development/Libraries

%description devel
WvStreams aims to be an efficient, secure, and easy-to-use library for
doing network applications development.  This package contains the files
needed for developing applications which use WvStreams.

%prep
%setup -q -n wvstreams-%{version}

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install PREFIX=$RPM_BUILD_ROOT/usr
chmod 755 $RPM_BUILD_ROOT/usr/lib/*.so.*

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING.LIB src/README
/usr/lib/*.so.*

%files devel
%defattr(-,root,root)
/usr/include/wvstreams
/usr/lib/*.a
/usr/lib/*.so

%changelog
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
