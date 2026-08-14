# Wine (run Windows apps on Linux)

## 1. What is it / What is it for?

Wine is a compatibility layer that runs Windows `.exe` applications on Linux without a Windows license. This note covers installing Wine, the 32-bit vs 64-bit prefix gotcha, and launching a Windows program from a `.lnk` shortcut.


- Running Windows-only software (e.g. old editors, games) on Ubuntu.
- Isolating apps in separate "WINEPREFIX" environments.

## 2. How to download / install

```bash
# old PPA approach (deprecated):
# sudo add-apt-repository ppa:ubuntu-wine/ppa && sudo apt-get update
# sudo apt-get install wine1.3
# modern:
sudo apt-get install wine
```

Set the Windows version to 7 in `winecfg` (Applications → Windows Version → Windows 7).

## 3. How to use

Windows files live under `~/.wine` (a 64-bit prefix by default, but most Windows programs need 32-bit):
```bash
# create a 32-bit prefix
export WINEPREFIX=<foldername>
export WINEARCH=win32
winecfg
```

Launch a program via its `.lnk` shortcut:
```bash
env WINEPREFIX="<YOUR_HOME>/.wine" wine C:\\windows\\command\\start.exe \
  /Unix <YOUR_HOME>/.wine/dosdevices/c:/users/Public/Desktop/Cool\ Edit\ Pro\ 2.1.lnk
```

