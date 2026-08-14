# Ubuntu release upgrade

## 1. What is it?

`do-release-upgrade` upgrades Ubuntu to the next LTS/release in place (e.g. 18.04 → 20.04).

## 2. What is it for?

- Moving to a newer Ubuntu without a fresh install.
- Getting newer kernels/packages on an existing box.

## 3. How to download / install

Ensure the system is up to date first, then run the upgrader:
```bash
sudo apt update && sudo apt upgrade -y
sudo do-release-upgrade
```
(LTS-to-LTS upgrades appear after the first point release; use `-d` for the development release.)

## 4. How to use

Follow the interactive prompts. After the reboot, if your login shell is broken, append `/bin/bash` after your username in `/etc/passwd`:
```
youruser:x:1000:1000:...:/home/youruser:/bin/bash
```

## 5. Pitfalls

- **Third-party PPAs break upgrades** — disable them (`sudo add-apt-repository --remove ppa:...`) before upgrading, or the process aborts.
- **`/etc/passwd` edits are dangerous** — a syntax error can lock you out; edit with `vipw`, not a plain editor.
- **Interrupting the upgrade = broken system**; ensure power/network stability.
- **Third-party repos / custom kernels** may not survive; verify afterward.
