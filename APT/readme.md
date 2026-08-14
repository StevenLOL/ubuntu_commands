# APT sources.list (Aliyun mirror, Ubuntu 18.04)

## 1. What is it?

`/etc/apt/sources.list` tells `apt` where to fetch packages from. This note provides a ready-to-use Aliyun mirror config for Ubuntu 18.04 (bionic).

## 2. What is it for?

- Speeding up `apt update` / `apt install` by using a nearby mirror instead of `archive.ubuntu.com`.
- Replacing a broken or slow default source.

## 3. How to download / install

Find your Ubuntu version codename first:

```bash
. /etc/os-release; echo $ID$VERSION_ID
# e.g. ubuntu18.04 -> codename "bionic"
```

Back up and overwrite the sources file:

```bash
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
sudo nano /etc/apt/sources.list
```

Paste the Aliyun config below (for **18.04 / bionic**), then:

```bash
sudo apt update
```

## 4. How to use

`/etc/apt/sources.list` (bionic):

```text
deb http://mirrors.aliyun.com/ubuntu/ bionic main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ bionic main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ bionic-security main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ bionic-security main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ bionic-updates main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ bionic-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ bionic-backports main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ bionic-backports main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ bionic-proposed main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ bionic-proposed main restricted universe multiverse
```

## 5. Pitfalls

- **Codename must match your release.** The block above is for `bionic` (18.04). For 20.04 use `focal`, 22.04 `jammy`, 24.04 `noble`. A mismatch causes `404` / dependency errors.
- **`deb-src` is optional** — only needed if you build from source.
- **`proposed` can be unstable**; many leave it commented out.
- Always run `sudo apt update` after editing and watch for `404` lines.
