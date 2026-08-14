# Wine usage snippets

## 1. What is it?

Two shell snippets for working with Wine: installing Wine 1.3 (old) and configuring a 32-bit prefix.

## 2. What is it for?

- Quick reference for the old `wine1.3` install and setting Windows 7 mode.

## 3. How to download / install

```bash
# deprecated old install:
# sudo add-apt-repository ppa:ubuntu-wine/ppa
# sudo apt-get update
# sudo apt-get install wine1.3
# modern:
sudo apt-get install wine
```

## 4. How to use

After install, open **Wine → Configure Wine** and set the Windows version to **Windows 7**.

Create/use a 32-bit prefix:
```bash
WINEARCH=win32 WINEPREFIX=~/.wine winecfg
```

## 5. Pitfalls

- **`wine1.3` / `ubuntu-wine` PPA is dead** — use the current `wine` package.
- **Windows 7 mode** is a good default for older apps; some need XP mode instead.
- The original note's first line had a garbled `sudo` (`udo`) — it's `sudo add-apt-repository`.
