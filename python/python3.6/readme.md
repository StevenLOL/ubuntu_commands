# Install Python 3.6 on Ubuntu 16.04

## 1. What is it?

Two ways to get Python 3.6 on Ubuntu 16.04 (which ships 3.5): build from source, or install from a PPA.

## 2. What is it for?

- Running code that requires Python 3.6 specifically.
- Getting a newer Python without upgrading the whole OS.

## 3. How to download / install

**Option A — build from source**
```bash
sudo apt install build-essential checkinstall
sudo apt install libreadline-gplv2-dev libncursesw5-dev libssl-dev \
  libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev
wget https://www.python.org/ftp/python/3.6.0/Python-3.6.0.tar.xz
tar xvf Python-3.6.0.tar.xz
cd Python-3.6.0/
./configure
sudo make altinstall
```

**Option B — PPA (simpler)**
```bash
sudo add-apt-repository ppa:jonathonf/python-3.6
sudo apt update
sudo apt install python3.6
```

Ref: https://askubuntu.com/questions/865554/how-do-i-install-python-3-6-using-apt-get

## 4. How to use

```bash
python3.6 --version
```

## 5. Pitfalls

- **Use `make altinstall`**, not `make install` — the latter can shadow the system `python3`.
- **The `jonathonf/python-3.6` PPA is deprecated**; for newer Ubuntu just use the default `python3` or `deadsnakes` PPA (`ppa:deadsnakes/ppa`).
- **`libreadline-gplv2-dev`** is a 16.04 package name; on 18.04+ use `libreadline-dev`.
