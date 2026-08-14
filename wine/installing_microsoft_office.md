# Install Microsoft Office 2010 under Wine

## 1. What is it?

A recipe for installing **Microsoft Office 2010 (32-bit)** on Ubuntu via Wine 1.5 + Winetricks. (Historical — modern Wine + Office 2010 is better supported, but the 32-bit prefix requirement still applies.)

Ref: http://craigacgomez.blogspot.sg/2012/09/installing-microsoft-office-2010-in.html

## 2. What is it for?

- Running Word/Excel/PowerPoint 2010 on Linux when MS-format fidelity matters.

## 3. How to download / install

**32-bit OpenGL libs (64-bit Ubuntu only):**
```bash
sudo apt-get install mesa-utils mesa-utils-extra libgl1-mesa-glx:i386 libgl1-mesa-dev
sudo ln -s /usr/lib/i386-linux-gnu/mesa/libGL.so.1 /usr/lib/i386-linux-gnu/mesa/libGL.so
sudo ln -s /usr/lib/i386-linux-gnu/mesa/libGL.so /usr/lib/i386-linux-gnu/libGL.so
```

**Wine:**
```bash
sudo add-apt-repository ppa:ubuntu-wine/ppa
sudo apt-get update && sudo apt-get install wine
```

**32-bit prefix + Winetricks:**
```bash
export WINEPREFIX="<YOUR_HOME>/.wineprefixes/office2010/"
export WINEARCH="win32"
winetricks
# install: dotnet20, msxml6, corefonts
```

## 4. How to use

```bash
wine setup.exe        # from the Office 2010 install media
```
Then in `winecfg` (Libraries tab), set `msxml6`, `riched20`, `gdiplus` to **Native (Windows)**.

(Alternative one-liner: `WINEARCH=win32 WINEPREFIX=~/.wine winecfg`.)

## 5. Pitfalls

- **32-bit only**: Office 2010 32-bit is what works on Wine; use a `win32` prefix.
- **`ubuntu-wine` PPA is dead** — install current Wine from the Ubuntu repos / WineHQ.
- **Product key in the original note** (`WWW-VQ7F...`) is a personal/example key — keep keys out of shared docs.
