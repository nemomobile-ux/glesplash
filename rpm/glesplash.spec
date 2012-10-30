Name: glesplash
Version: 0.1
Release: 1
Summary: GL splash screen
Group: System/Base
License: Proprietary
Source0: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(libpng)
BuildRequires: pkgconfig(egl)
BuildRequires: pkgconfig(glesv2)
BuildRequires: pkgconfig(x11)
BuildRequires: qt-qmake

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/glesplash
%{_bindir}/glesplash-fb


%prep
%setup -q


%build
qmake
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
