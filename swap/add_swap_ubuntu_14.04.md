# Add a swap file (Ubuntu 14.04)

## 1. What is it / What is it for?

Swap is disk space the OS uses as overflow when RAM fills up. This note creates a swap **file** (no repartitioning) and makes it permanent.

> **Note (2026):** the commands below are distro-agnostic and still correct on current Ubuntu (22.04/24.04/26.04 LTS). Only the referenced DigitalOcean URL is 14.04-specific; the `fallocate`/`mkswap`/`swapon` steps are identical today.

Ref: https://www.digitalocean.com/community/tutorials/how-to-add-swap-on-ubuntu-14-04


- Preventing OOM (out-of-memory) crashes when RAM is tight.
- Giving a low-RAM server breathing room for bursty workloads.

## 2. How to download / install

No software — uses `fallocate`/`dd`, `mkswap`, `swapon`. Check current state first:
```bash
sudo swapon -s
free -m
df -h          # ensure disk has space
```

## 3. How to use

**Create the file (fast method):**
```bash
sudo fallocate -l 4G /swapfile
# or the slow, portable way:
# sudo dd if=/dev/zero of=/swapfile bs=1G count=4
```

**Secure, format, enable:**
```bash
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

**Make it permanent** — add to `/etc/fstab`:
```text
/swapfile   none    swap    sw    0   0
```

**Tune swappiness / cache pressure:**
```bash
sudo sysctl vm.swappiness=10          # default 60; lower = swap less
sudo sysctl vm.vfs_cache_pressure=50  # default 100
# persist in /etc/sysctl.conf:
# vm.swappiness=10
# vm.vfs_cache_pressure=50
```

**To remove a swap file:**
```bash
cat /etc/fstab                      # confirm which swap to remove
sudo swapoff /swapfile             # deactivate
sudo rm /swapfile                  # delete the file
```

