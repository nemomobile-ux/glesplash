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
%config %{_sysconfdir}/powervr.d/glesplash-fb.ini
/lib/systemd/system/glesplash-fb.service
/lib/systemd/system/basic.target.wants/glesplash-fb.service
/lib/systemd/system/multi-user.target.wants/glesplash-fb.service
%{_datadir}/%{name}/*.png

%prep
%setup -q


%build
qmake
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
mkdir -p %{buildroot}/lib/systemd/system/basic.target.wants
mkdir -p %{buildroot}/lib/systemd/system/multi-user.target.wants
ln -sf ../glesplash-fb.service %{buildroot}/lib/systemd/system/basic.target.wants
# basic.target is to get splash screen as early as possible
# multi-user.target is to get splash started when we change from actdead to user mode
ln -sf ../glesplash-fb.service %{buildroot}/lib/systemd/system/multi-user.target.wants
