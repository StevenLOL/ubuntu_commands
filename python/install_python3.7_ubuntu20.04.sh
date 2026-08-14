# Build Python 3.7 from source on Ubuntu 20.04

## 1. What is it / What is it for?

A scripted recipe to compile and install Python 3.7.8 from source on Ubuntu 20.04 (useful when the distro only ships 3.8+).


- Getting a specific Python version not available via apt.
- Building a self-contained `python3.7` alongside the system Python.

## 2. How to download / install

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

## 3. How to use

```bash
python3.7 --version
```

