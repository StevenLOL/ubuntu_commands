# RamDisk (tmpfs) on Ubuntu

## 1. What is it / What is it for?

A RamDisk maps a folder onto RAM (via the Linux `tmpfs` filesystem) so files written there live in memory — extremely fast, but volatile.


- Speeding up I/O for temporary/cache data you don't need to persist (build artifacts, scratch files).
- Avoiding SSD wear for high-write temporary files.

## 2. How to download / install

No install — it's a kernel feature. Create and mount a tmpfs (see `ramdisk.sh`):
```bash
sudo -s
mkdir /tmp/ramdisk && chmod 777 /tmp/ramdisk
mount -t tmpfs -o size=8192M tmpfs /tmp/ramdisk/
# alternative: ramfs (no size limit, riskier)
# mount -t ramfs -o size=8192M ramfs /tmp/ramdisk/
```

## 3. How to use

```bash
dh -h          # what's mounted / sizes
free -m        # RAM usage grows as you fill the ramdisk
# move files in:
mv bigfile /tmp/ramdisk/
```

Unmount when done:
```bash
umount tmpfs -f
# or: umount ramfs -f
```

