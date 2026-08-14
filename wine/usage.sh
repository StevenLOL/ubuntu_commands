
windows visual folder is in home/.wine

is 64bits but most win program need 32bits

calling a program in v driver:

env WINEPREFIX="<YOUR_HOME>/.wine" wine C:\\windows\\command\\start.exe /Unix <YOUR_HOME>/.wine/dosdevices/c:/users/Public/Desktop/Cool\ Edit\ Pro\ 2.1.lnk


create a v drive :

export WINEPREFIX=fodlername

export WINEARCH=win32

winecfg
