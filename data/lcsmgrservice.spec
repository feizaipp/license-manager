Name:		lcsmgrservice
Version:	1.0
Release:	1%{?dist}
Summary:	license manager dbus service

License:      GPL-2.0
Source0:     %{name}-%{version}.tar.gz
#Source1 :    %{name}.pp.bz2
#Source2 :    %{name}.if

BuildRequires: glib2-devel
BuildRequires: libudisks2-devel
BuildRequires: openssl-devel

Requires: openssl

%description
license manager dbus service

%package        tools
Summary:        license manager  tools
Group:          Applications/Databases
Requires:       lcsmgrservice

%description  tools
license manager  tools

%prep
%setup -q

%build
./autogen.sh
./configure --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin \
--sbindir=/usr/sbin --sysconfdir=/etc --datadir=/usr/share \
--includedir=/usr/include --libdir=/usr/lib64 --libexecdir=/usr/libexec \
--localstatedir=/var --sharedstatedir=/var/lib --mandir=/usr/share/man \
--infodir=/usr/share/info
make

#pushd selinux
#make -f Makefile
#popd

%install
[ "${RPM_BUILD_ROOT}" != "/" ] && [ -d ${RPM_BUILD_ROOT} ] && rm -rf ${RPM_BUILD_ROOT};
make install DESTDIR=$RPM_BUILD_ROOT
install -d -m 700 $RPM_BUILD_ROOT/var/lib/lcsmgrservice
# install -D -p -m 644 %{SOURCE1} $RPM_BUILD_ROOT/usr/share/selinux/packages/%{name}.pp.bz2
# install -D -p -m 644 %{SOURCE2} $RPM_BUILD_ROOT/usr/share/selinux/devel/include/contrib/%{name}.if

#pushd selinux
#make -f Makefile install DESTDIR=$RPM_BUILD_ROOT
#popd

%clean
[ "${RPM_BUILD_ROOT}" != "/" ] && [ -d ${RPM_BUILD_ROOT} ] && rm -rf ${RPM_BUILD_ROOT};

%post
%systemd_post lcsmgrservice.service
#semodule -i /usr/share/selinux/packages/%{name}.pp.bz2
systemctl enable lcsmgrservice.service >/dev/null 2>&1

%files
%dir %{_libexecdir}/lcsmgrservice
%{_libexecdir}/lcsmgrservice/lcsmgrserviced

%{_sysconfdir}/dbus-1/system.d/org.freedesktop.LcsMgrService.conf
%{_unitdir}/lcsmgrservice.service
%{_datadir}/dbus-1/system-services/org.freedesktop.LcsMgrService.service
/usr/bin/lsmgr
%dir /var/lib/lcsmgrservice
#/usr/share/selinux/devel/include/contrib/lcsmgrservice.if
#/usr/share/selinux/packages/lcsmgrservice.pp.bz2

%files tools
/usr/bin/lsmgrsign

%changelog
* Tue Jul 30 2019 zpehome <zpehome@yeah.net> - 1.0-1
- Initial version of the package
