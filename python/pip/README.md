# pip (Python package manager)

## 1. What is it / What is it for?

`pip` installs Python packages. This note covers getting pip, caching, mirrors, version pinning, offline download, and (historically) a TensorFlow 0.12 install example.

> **As of 2026:** pip itself is unchanged and all commands below still work. `get-pip.py` still bootstraps pip. The TensorFlow 0.12 example at the bottom is **historical** — current TF is 2.x (`pip install tensorflow`). The Tsinghua mirror (`pypi.tuna.tsinghua.edu.cn`) is still live; the old `douban` pip mirror is dead.


- Installing/managing Python packages from PyPI or a mirror.
- Caching downloads, pinning versions, and fetching wheels offline.

## 2. How to download / install

```bash
# bootstrap pip
wget https://bootstrap.pypa.io/get-pip.py
python get-pip.py
```

### Configure a China mirror (in `~/.pip/pip.conf`)
```ini
[global]
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
trusted-host = pypi.tuna.tsinghua.edu.cn
download_cache = ~/.cache/pip
timeout = 9000
```
Or set it directly:
```bash
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
pip config list
```

## 3. How to use

```bash
pip install 'tensorflow>=0.8,<1.0'     # version range
pip install 'tensorflow<1.0'
pip download package -d /path/to/dl    # download only (offline use)
pip install -E pydoop                     # -E preserves environment (rarely used)
```

### TensorFlow 0.12 on Ubuntu 14.04 (legacy)
```bash
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:fkrull/deadsnakes-python2.7
sudo apt-get update && sudo apt-get upgrade
pip uninstall pip
wget https://bootstrap.pypa.io/get-pip.py
sudo python ./get-pip.py
pip install 'tensorflow<1.0'
```

