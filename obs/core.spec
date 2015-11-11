Name:           core
Version:        %{core_ver}
Release:        0
License:        GPL-3.0
Summary:        BIOS project
Url:            https://github.com/eaton-bob
Group:          Development/Libraries/C and C++
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  zeromq-devel >= 4.2
BuildRequires:  libsodium-devel
BuildRequires:  czmq-devel >= 3.0
BuildRequires:  pkg-config
BuildRequires:  nut-devel
BuildRequires:  libcidr-devel
BuildRequires:  gcc-c++
BuildRequires:  libtool
BuildRequires:  file-devel
BuildRequires:  cyrus-sasl-devel
BuildRequires:  tntdb-devel
BuildRequires:  tntnet-devel tntnet
BuildRequires:  malamute-devel
BuildRequires:  autoconf
BuildRequires:  automake
# documentation
BuildRequires:  asciidoc
BuildRequires:  xmlto
%if %{defined opensuse_version}
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
autoreconf -fiv
%configure --disable-static
make %{?_smp_mflags}

%install
%make_install

%files
%defattr(-,root,root)
%doc LICENSE
%{_bindir}/*
%{_libdir}/*.so.*
%{_libexecdir}
%{_datadir}
%{_unitdir}/*.service

%files devel
%defattr(-,root,root)
%{_libdir}/*.so
%{_includedir}

%changelog

