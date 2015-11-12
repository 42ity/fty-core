
#MVY: note that non opensuse version is stripped down to libbiosapi.so et all
#     need to package nut.rpm first!!

%global core_ver 0.1.1447316673~504b4d1
Name:           core
Version:        0.1.1447316673~504b4d1
Release:        0
License:        GPL-3.0
Summary:        BIOS project
Url:            https://github.com/eaton-bob
Group:          Development/Libraries/C and C++
Source0:        %{name}-%{version}.tar.gz
Source1:        configure.ac
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  pkg-config
BuildRequires:  malamute-devel
%if %{defined opensuse_version}
BuildRequires:  nut-devel
BuildRequires:  libcidr-devel
BuildRequires:  gcc-c++
BuildRequires:  libtool
BuildRequires:  file-devel
BuildRequires:  cyrus-sasl-devel
BuildRequires:  tntdb-devel
BuildRequires:  tntnet-devel tntnet
# documentation
BuildRequires:  asciidoc
BuildRequires:  xmlto
# % if %{defined opensuse_version}
BuildRequires:  systemd-rpm-macros
Requires(pre):  systemd-rpm-macros
Requires(preun):systemd-rpm-macros
Requires(post): systemd-rpm-macros
Requires(postun):systemd-rpm-macros
%endif

%description
The BIOS project

%package devel
Summary:    Devel files for %{name}
Group:      Development/Languages/C and C++
Requires:   %{name} = %{version}

%description devel
Development files (headers, pkgconfig, cmake) for %{name}.

%prep
%setup -q
# Ugly hack as we don't have fresh enough nut but we don't need ti for development
sed -i 's|libnutscan >= 2.7.2|libnutscan >= 2.7.1|' configure.ac

%build
%if %{defined opensuse_version}

export SUSE_ASNEEDED=0
autoreconf -fiv

%configure \
%if %{defined opensuse_version}
  --with-saslauthd-mux=/var/run/sasl2/mux
%endif
 --disable-static

make %{?_smp_mflags}

%else

cp %{SOURCE1} .
autoreconf -fiv
%{configure}
make libbiosapi.la

%endif

%install
%if %{defined opensuse_version}
%make_install

find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

# to keep compatibility with core-dev.deb
mkdir -p %{buildroot}%{_includedir}/bios/
mv %{buildroot}%{_includedir}/* \
   %{buildroot}%{_includedir}/bios/* || :

%else

mkdir -p %{buildroot}/%{_includedir}/bios/
cp include/*.h %{buildroot}/%{_includedir}/bios

mkdir -p %{buildroot}/%{_libdir}/
cp .libs/libbiosapi.so* %{buildroot}/%{_libdir}/

mkdir -p %{buildroot}/%{_libdir}/pkgconfig/

sed -e 's#@prefix@#%{_prefix}#' \
    -e 's#@exex_prefix@#%{_exec_prefix}#' \
    -e 's#@libdir@#%{_libdir}#' \
    -e 's#@includedir@#%{_includedir}#' \
    -e 's#@VERSION@#0.1.0#' \
    ./src/api/libbiosapi.pc.in > \
    %{buildroot}/%{_libdir}/pkgconfig/libbiosapi.pc

%endif

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc COPYING
%{_libdir}/*.so.*
%if %{defined opensuse_version}
%{_bindir}/*
%{_libdir}/bios/
%{_libexecdir}
%{_datadir}
#MVY: prevents file listed twice error - most likely libexecdir above
# % {_unitdir}/*
%endif

%files devel
%defattr(-,root,root)
%{_libdir}/*.so
%{_includedir}/bios
%{_libdir}/pkgconfig/libbiosapi.pc

%changelog

