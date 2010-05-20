%define major_version 10.05.99
%define tar_name php5-midgard2

%global php_zendabiver %((echo 0; php -i 2>/dev/null | sed -n 's/^PHP Extension => //p') | tail -1)
%define php_extdir %(php-config --extension-dir 2>/dev/null)

%if 0%{?suse_version}
%define php_confdir php5/conf.d
%define rpm_name php5-midgard2
%else
%define php_confdir php.d
%define rpm_name php-midgard2
%endif

Name:           %{rpm_name}
Version:        %{major_version}
Release:        OBS
Summary:        PHP extension for Midgard

Group:          Development/Languages
License:        LGPLv2+
URL:            http://www.midgard-project.org/
Source0:        %{url}download/%{tar_name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  php-devel >= 5.2
BuildRequires:  midgard2-core-devel >= %{major_version}

%if 0%{?suse_version}
Provides:       php-midgard2 = %{version}-%{release}
%else
%if %{?php_zend_api}0
Requires:       php(zend-abi) = %{php_zend_api}
Requires:       php(api) = %{php_core_api}
%else
Requires:       php-zend-abi = %{php_zendabiver}
%endif
%endif

%description
The %{name} package contains a dynamic shared object that will add
Midgard support to PHP. Midgard is a persistent storage framework which 
is used e.g. by the Midgard Content Management System. PHP is an 
HTML-embeddable scripting language. If you need Midgard support for PHP 
applications, you will need to install this package in addition to the 
php package.

This module provides also abilities to use D-Bus and GObject event 
handling in PHP applications.


%prep
%setup -q -n %{tar_name}-%{version}


%build
export PHP_RPATH=no
phpize
%configure
make %{?_smp_mflags}


%install
%if 0%{?suse_version} == 0
rm -rf $RPM_BUILD_ROOT
mkdir -p $(dirname $RPM_BUILD_ROOT)
mkdir $RPM_BUILD_ROOT
%endif
install -D -m 755 .libs/midgard2.so $RPM_BUILD_ROOT%{php_extdir}/midgard2.so
install -D -m 644 midgard2.ini $RPM_BUILD_ROOT%{_sysconfdir}/%{php_confdir}/midgard2.ini


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc COPYING
%{php_extdir}/midgard2.so
%config(noreplace) %{_sysconfdir}/%{php_confdir}/midgard2.ini


%changelog
* Wed Oct 28 2009 Jarkko Ala-Louvesniemi <jval@puv.fi> 9.03.99-766.1
- Initial OBS package: merged OBS specifics from 8.09.6-1.1

* Tue Oct 27 2009 Jarkko Ala-Louvesniemi <jval@puv.fi> 9.03.99-1
- Renamed package from php-midgard to php-midgard2
- Changed tar_name from php5-midgard to php5-midgard2
- Build requirement is now for midgard2-core-devel
- Changed midgard to midgard2 in file names
- Updated package description to include event handling support
- Changed minimum PHP version requirement from 5.1 to 5.2

* Thu Jul 16 2009 Jarkko Ala-Louvesniemi <jval@puv.fi> 8.09.5-1
- Initial package using the Fedora spec file template.
