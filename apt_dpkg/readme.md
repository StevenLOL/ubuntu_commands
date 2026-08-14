# apt / dpkg Troubleshooting

## 1. What is it?

`apt` is the high-level package manager for Debian/Ubuntu; `dpkg` is the low-level backend that actually installs `.deb` files. This note collects common breakages and their fixes.

## 2. What is it for?

- Fixing broken package installs, half-configured packages, and dependency hell.
- Recovering from a corrupted `source.list` or a deleted core binary (e.g. `ldconfig`).
- Installing software offline (without Internet).

## 3. How to download / install

apt/dpkg are part of the base system — nothing to install. Inspect the active sources:

```bash
cat /etc/apt/sources.list
# also check drop-in files:
ls /etc/apt/sources.list.d/
```

A working mirror example (Tsinghua):

```
http://mirrors.tuna.tsinghua.edu.cn/ubuntu/
```

## 4. How to use (common fixes)

### `ldconfig: not found in PATH or not executable`
`ldconfig` lives in `/sbin`; it may have been removed when `libc6-bin` was purged. Restore it from a package:

```bash
apt-get download libc-bin
dpkg -x libc-bin*.deb libdir/
sudo cp libdir/sbin/ldconfig /sbin/
sudo apt-get install --reinstall libc-bin
sudo apt-get install -f
```

### Offline install
[How can I install software or packages without Internet (offline)?](http://askubuntu.com/questions/974/how-can-i-install-software-or-packages-without-internet-offline)

### `Depends: libc6 (= 2.19-...) but 2.23-... is to be installed`
Caused by a wrong `source.list` (mismatched release). Point the mirror at the correct Ubuntu codename and re-run `apt-get update`.

### dpkg overwrite / diversion error
```
Adding 'diversion of /usr/sbin/update-icon-caches ...' dpkg-divert: error: rename involves overwriting ...
```
Delete the conflicting file, then force the install:

```bash
sudo dpkg -i --force-overwrite /var/cache/apt/archives/smplayer_*.deb
sudo apt-get install -f
```
Ref: http://www.webupd8.org/2011/02/fix-dpkg-error-trying-to-overwrite-x.html

### `Can't exec "locale": No such file or directory`
```bash
apt-get download libc-bin
dpkg -x libc-bin*.deb libdir/
sudo cp libdir/sbin/ldconfig /sbin/
sudo apt-get install --reinstall libc-bin
sudo apt-get install -f
```

### General recovery commands
```bash
dpkg --configure -a
dpkg --configure -a --force-depends
apt-get -f install
```

## 5. Pitfalls

- **`--force-overwrite` can clobber files** from other packages; use it only as a last resort.
- **Wrong mirror codename** (e.g. `bionic` vs `focal`) is the #1 cause of dependency errors — match the mirror to `lsb_release -cs`.
- **Never delete `libc6-bin`** unless you know exactly what you're doing; it removes `ldconfig`.
- **`apt-get -f install`** may remove packages to satisfy dependencies — review the proposed changes first.
