# Add a swap file (Ubuntu 14.04)

## 1. What is it?

Swap is disk space the OS uses as overflow when RAM fills up. This note creates a swap **file** (no repartitioning) and makes it permanent.

Ref: https://www.digitalocean.com/community/tutorials/how-to-add-swap-on-ubuntu-14-04

## 2. What is it for?

- Preventing OOM (out-of-memory) crashes when RAM is tight.
- Giving a low-RAM server breathing room for bursty workloads.

## 3. How to download / install

No software — uses `fallocate`/`dd`, `mkswap`, `swapon`. Check current state first:
```bash
sudo swapon -s
free -m
df -h          # ensure disk has space
```

## 4. How to use

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

## 5. Pitfalls

- **Typo in original**: it's `vm.swappiness` (not `vm.swapiness`).
- **fstab option**: use `sw` (not `defaults`) for a swap entry.
- **`dd` is slow** compared to `fallocate`; prefer `fallocate` (works on most filesystems; on some like XFS you may need `dd`).
- **Size**: a common rule is 1–2× RAM; on SSD, swap is slow but fine as a safety net.
- **Swappiness=10** is good for servers; desktops often keep 60.
- Working from the DigitalOcean tutorial by Justin Ellingwood.
