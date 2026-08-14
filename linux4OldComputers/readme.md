# Lightweight Linux distros for old computers

## 1. What is it?

Two Debian-based, systemd-free distros built to run well on aging / low-RAM hardware:

- **MX Linux** — stable, polished Xfce desktop with a rich "MX Tools" suite; good balance of performance and usability.
  Site: https://mxlinux.org/
- **antiX Linux** — extremely lightweight (runs on ~256 MB RAM), no systemd, uses IceWM/JWM window managers; revives very old machines.
  Site: https://antixlinux.com/

## 2. What is it for?

- Giving old laptops/desktops a second life with a fast, responsive OS.
- A low-overhead environment when you don't need a heavy desktop (GNOME/KDE).

## 3. How to download / install

Download the ISO from the official sites above, flash to a USB stick (e.g. `dd` or BalenaEtcher), and boot the installer. Both offer a live-USB that you can try before installing.

## 4. How to use

Boot the live medium, then run the included installer (MX has "MX Live USB Maker" / antiX has its installer). Use the MX Tools / control-center applets for updates, backups, and package management.

## 5. Pitfalls

- **32-bit support**: antiX still ships 32-bit images; MX's 32-bit edition was discontinued for newer releases — check before putting it on very old hardware.
- **No systemd**: some tutorials assume `systemctl`; use the distro's `sysVinit`/service tools instead.
- **Hardware quirks**: very old Wi-Fi/Broadcom chips may need extra firmware packages.
