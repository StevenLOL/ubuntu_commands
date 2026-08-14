# FreeOffice (SoftMaker) on Linux

## 1. What is it?

FreeOffice is SoftMaker's free, MS-Office-compatible office suite (TextMaker, PlanMaker, Presentations) for Windows/macOS/Linux.

## 2. What is it for?

- Reading/writing Word/Excel/PowerPoint files with better MS-format fidelity than LibreOffice in some cases.
- A lightweight MS-Office-compatible alternative.

## 3. How to download / install

Download the `.deb` (e.g. `softmaker-freeoffice-2016_763-01_i386.deb`) from https://www.freeoffice.com/ , then:
```bash
sudo dpkg -i softmaker-freeoffice-2016_763-01_i386.deb
sudo apt-get install -f
```

Tested on Ubuntu 14.04 / 15.10 (32- and 64-bit).

## 4. How to use

Launch TextMaker/PlanMaker/Presentations from the application menu.

## 5. Pitfalls

- **Old `.deb` version** (`2016`) in the note — grab a current FreeOffice build from the site.
- **`i386` deb on 64-bit**: the `apt-get install -f` step pulls 32-bit libs; on a pure 64-bit system prefer the amd64 package if offered.
- FreeOffice is free but not open-source; for fully open software use LibreOffice.
