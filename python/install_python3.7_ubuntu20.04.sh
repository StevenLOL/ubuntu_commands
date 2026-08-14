# Build Python 3.7 from source on Ubuntu 20.04

## 1. What is it?

A scripted recipe to compile and install Python 3.7.8 from source on Ubuntu 20.04 (useful when the distro only ships 3.8+).

## 2. What is it for?

- Getting a specific Python version not available via apt.
- Building a self-contained `python3.7` alongside the system Python.

## 3. How to download / install

```bash
# 1. update
sudo apt update && sudo apt upgrade -y

# 2. build dependencies
sudo apt-get install -y gcc make build-essential libssl-dev zlib1g-dev \
  libbz2-dev libreadline-dev libsqlite3-dev wget curl llvm \
  libncurses5-dev libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev

# 3. fetch source
wget https://www.python.org/ftp/python/3.7.8/Python-3.7.8.tgz
# (or download the Linux tarball from python.org directly)

# 4. extract
tar -xzvf Python-3.7.8.tgz

# 5. configure
cd Python-3.7.8
./configure --enable-optimizations

# 6. build & install
sudo make
sudo make install
```

## 4. How to use

```bash
python3.7 --version
```

## 5. Pitfalls

- **`make install` can overwrite `python3`** if you don't use `--enable-optimizations` + altinstall; safer is `sudo make altinstall` to keep `python3` pointing at the system version.
- **`--enable-optimizations` is slow** (runs PGO); omit for a faster build.
- **Missing `libssl-dev`** → no `ssl`/`pip` over HTTPS; install it before configure.
- The original note had Chinese inline comments; this version keeps only the working commands.
