# Rufus (make a bootable Linux install USB on Windows)

## 1. What is it / What is it for?

Rufus is a small, free, **portable** Windows utility that writes an `.iso` (or `.img`) image onto a USB stick to create a **bootable installer** — e.g. an Ubuntu install drive. It is the typical **first step before installing Linux**: you download the distro's ISO, flash it to a USB stick with Rufus, then boot the target machine from that stick and install Linux.

Homepage: https://rufus.ie/

## 2. How to download / install

Rufus ships as a **single portable `.exe`** — no installer, no admin rights required.

1. Go to https://rufus.ie/ and grab `rufus-<ver>.exe` (the "Portable" build).
2. Run it directly (double-click). That's it — nothing to install.

(There is also an `.exe` that downloads a one-time helper on first run; the portable build avoids even that.)

## 3. How to use

Make an Ubuntu (or any Linux) install USB:

1. Plug in a **blank/empty USB stick** (Rufus will erase it — back up first!).
2. In Rufus:
   - **Device**: pick your USB stick.
   - **Boot selection**: click *SELECT* and choose the downloaded `.iso` (e.g. `ubuntu-24.04.1-desktop-amd64.iso`).
   - **Partition scheme**: `GPT` for UEFI machines (modern default); `MBR` for old BIOS-only.
   - **Target system**: `UEFI (non CSM)` for GPT.
   - Leave *File system* as `FAT32` for UEFI boot.
3. Click **START** → choose *Write in ISO image mode* (default) → confirm.
4. When done, **safely eject** the stick, plug it into the target PC, boot from USB (often `F12`/`F2`/`Del` at power-on), and install Linux.
