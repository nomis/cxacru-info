Name: cxacru-info
Version: 0.5
Group: Applications/System
Summary: outputs cxacru status information from sysfs

Release: 1
BuildRoot: %{_tmppath}/rpm-%{name}_%{version}
Source: http://thttpd.lp0.eu/cxacru/%{name}_%{version}.tar.bz2

Vendor: Simon Arlott
License: GPL-2
URL: http://simon.arlott.org/sw/cxacru-info/

%description
Retrives status information of Conexant Accessrunner ADSL
USB modems from sysfs. By default it uses the first cxacru
device it finds, otherwise it uses the device with the
specified device number.

%prep
%setup -q -n %{name}_%{version}

%build
make

%install
rm -rf $RPM_BUILD_ROOT/
make install BINDIR=$RPM_BUILD_ROOT%{_bindir} MANDIR=$RPM_BUILD_ROOT%{_mandir}

%clean
rm -rf $RPM_BUILD_ROOT/

%files
%defattr(-,root,root)
%{_bindir}/cxacru-info
%doc %{_mandir}/man1/cxacru-info.*
