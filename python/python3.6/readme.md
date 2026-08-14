# Install Python 3.6 on Ubuntu 16.04

## 1. What is it / What is it for?

Two ways to get Python 3.6 on Ubuntu 16.04 (which ships 3.5): build from source, or install from a PPA.

> **Historical note (2026):** Python 3.6 reached **end-of-life in Dec 2021**, and Ubuntu 16.04 is also EOL. For a new machine just use the system Python: `sudo apt install python3` on Ubuntu 22.04/24.04/26.04 (ships 3.10/3.12/3.14). Only follow the steps below if you must reproduce a 3.6 environment.


- Running code that requires Python 3.6 specifically.
- Getting a newer Python without upgrading the whole OS.

## 2. How to download / install

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

## 3. How to use

```bash
python3.6 --version
```

