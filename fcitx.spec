%define name		fcitx
%define version		2.0.1
%define release		1

%define prefix		/usr
%define fcitxbindir	/usr/bin

Name:		%{name}
Version:	%{version}
Release:	%{release}
Summary:	Free Chinese Input Toy for X (XIM)
Packager:	Xie Yanbo <xyb76@sina.com>
Group:		System/I18n/Chinese
Copyright:	GPL
Source:		%{name}-%{version}.tar.gz
URL:		http://www.fcitx.org/download/%{name}-%{version}.tar.bz2
BuildRequires:	XFree86-devel
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}

%description
FCITX is a simplified Chinese input server. It supports WuBi,
Pinyin and QuWei input method. It's small and fast.

%prep
%setup -q

%build
%ifarch i386 i486 i586 i686
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS"		./configure --prefix=%{prefix} 	--host=i386-pc-linux-gnu
%else
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS"		./configure --prefix=%{prefix}
%endif

make

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL README
%doc doc/*.txt doc/*.htm
%{_bindir}/*
%{_datadir}/fcitx/data/*.mb
%{_datadir}/fcitx/data/*.dat

%changelog
* Thu Jan 15 2004 xyb <xyb76@sina.com>
- skeleton RPM
