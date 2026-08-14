# Ubuntu release upgrade

## 1. What is it / What is it for?

`do-release-upgrade` upgrades Ubuntu to the next LTS/release in place (e.g. 18.04 → 20.04).


- Moving to a newer Ubuntu without a fresh install.
- Getting newer kernels/packages on an existing box.

## 2. How to download / install

Ensure the system is up to date first, then run the upgrader:
```bash
sudo apt update && sudo apt upgrade -y
sudo do-release-upgrade
```
(LTS-to-LTS upgrades appear after the first point release; use `-d` for the development release.)

## 3. How to use

Follow the interactive prompts. After the reboot, if your login shell is broken, append `/bin/bash` after your username in `/etc/passwd`:
```
youruser:x:1000:1000:...:/home/youruser:/bin/bash
```

