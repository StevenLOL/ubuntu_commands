# RamDisk (tmpfs) on Ubuntu

## 1. What is it?

A RamDisk maps a folder onto RAM (via the Linux `tmpfs` filesystem) so files written there live in memory — extremely fast, but volatile.

## 2. What is it for?

- Speeding up I/O for temporary/cache data you don't need to persist (build artifacts, scratch files).
- Avoiding SSD wear for high-write temporary files.

## 3. How to download / install

No install — it's a kernel feature. Create and mount a tmpfs (see `ramdisk.sh`):
```bash
sudo -s
mkdir /tmp/ramdisk && chmod 777 /tmp/ramdisk
mount -t tmpfs -o size=8192M tmpfs /tmp/ramdisk/
# alternative: ramfs (no size limit, riskier)
# mount -t ramfs -o size=8192M ramfs /tmp/ramdisk/
```

## 4. How to use

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

## 5. Pitfalls

- **Volatile**: everything in a RamDisk is lost on reboot/power loss. Don't store anything you can't regenerate.
- **`size=` is a limit for `tmpfs`**; `ramfs` ignores it and can eat all RAM — prefer `tmpfs` with an explicit size.
- **Size in MB**: `size=8192M` = 8 GB; make sure you have that much free RAM.
- **`chmod 777`** makes it world-writable; tighten permissions if security matters.
- Ref: http://www.linuxscrew.com/2010/03/24/fastest-way-to-create-ramdisk-in-ubuntulinux/
