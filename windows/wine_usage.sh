# Wine usage snippets

## 1. What is it / What is it for?

Two shell snippets for working with Wine: installing Wine 1.3 (old) and configuring a 32-bit prefix.


- Quick reference for the old `wine1.3` install and setting Windows 7 mode.

## 2. How to download / install

```bash
# deprecated old install:
# sudo add-apt-repository ppa:ubuntu-wine/ppa
# sudo apt-get update
# sudo apt-get install wine1.3
# modern:
sudo apt-get install wine
```

## 3. How to use

After install, open **Wine → Configure Wine** and set the Windows version to **Windows 7**.

Create/use a 32-bit prefix:
```bash
WINEARCH=win32 WINEPREFIX=~/.wine winecfg
```

