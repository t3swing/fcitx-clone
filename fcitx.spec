%define name		fcitx
%define version		2.0.1
%define release		3

%define prefix		/usr
%define fcitxbindir	/usr/bin

Name:		%{name}
Version:	%{version}
Release:	%{release}
Summary:	Free Chinese Input Toy for X (XIM)
Packager:	Xie Yanbo <xyb76@sina.com>
URL:		http://www.fcitx.org/
Group:		User Interface/X
Group(zh_CN):	�û�����/����
License:	GPL
Source:		%{name}-%{version}.tar.gz
#BuildRequires:	XFree86, fontconfig
BuildRequires:	XFree86
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}

%description
FCITX is a simplified Chinese input server. It supports WuBi,
Pinyin and QuWei input method. It's small and fast.

%description -l zh_CN
Fcitx����С������뷨��Free Chinese Input Toy for X������һ����GPL��ʽ
�����ġ�����XIM�ļ����������뷨(��ԭ����G���)��������ʡ�ȫƴƴ����˫ƴ
ƴ�����뷨,����������Linux��������UNIXƽ̨�ϡ�
Designed by Yuking <yuking_net@suho.com>

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
mkdir -p %{buildroot}%{_datadir}/fcitx
install -m 644 data/*.mb %{buildroot}%{_datadir}/fcitx
install -m 644 data/*.dat %{buildroot}%{_datadir}/fcitx

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL README
%doc doc/*.txt doc/*.htm
%{_bindir}/*
%{_datadir}/fcitx/*.mb
%{_datadir}/fcitx/*.dat

%changelog
* Mon Feb 2 2004 xyb <xyb76@sina.com>
- Fix spec bug(patch by hamigua <hamigua@linuxsir.org>).

* Thu Jan 15 2004 xyb <xyb76@sina.com>
- skeleton RPM
