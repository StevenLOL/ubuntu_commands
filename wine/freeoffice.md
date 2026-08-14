# FreeOffice (SoftMaker) on Linux

## 1. What is it / What is it for?

FreeOffice is SoftMaker's free, MS-Office-compatible office suite (TextMaker, PlanMaker, Presentations) for Windows/macOS/Linux.


- Reading/writing Word/Excel/PowerPoint files with better MS-format fidelity than LibreOffice in some cases.
- A lightweight MS-Office-compatible alternative.

## 2. How to download / install

Download the `.deb` (e.g. `softmaker-freeoffice-2016_763-01_i386.deb`) from https://www.freeoffice.com/ , then:
```bash
sudo dpkg -i softmaker-freeoffice-2016_763-01_i386.deb
sudo apt-get install -f
```

Tested on Ubuntu 14.04 / 15.10 (32- and 64-bit).

> **As of 2026:** FreeOffice is now on the **2024 / NX** generation (64-bit only; the old `i386` package above is obsolete). Download the current `.deb` from https://www.freeoffice.com/ — the `dpkg -i` + `apt-get install -f` steps are unchanged.

## 3. How to use

Launch TextMaker/PlanMaker/Presentations from the application menu.

