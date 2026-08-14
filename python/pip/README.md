# pip (Python package manager)

## 1. What is it?

`pip` installs Python packages. This note covers getting pip, caching, mirrors, version pinning, offline download, and a TensorFlow 0.12 install example.

## 2. What is it for?

- Installing/managing Python packages from PyPI or a mirror.
- Caching downloads, pinning versions, and fetching wheels offline.

## 3. How to download / install

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

## 4. How to use

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

## 5. Pitfalls

- **The Douban/Alibaba/ZTE mirrors** in the original note are retired or unreliable — prefer Tsinghua (`pypi.tuna.tsinghua.edu.cn`) or the official index.
- **`pip uninstall pip`** is dangerous — it can remove pip itself; reinstall via get-pip.py.
- **Version pins**: `'tensorflow<1.0'` is ancient; use current releases for new work.
- **`download_cache` directive is deprecated** in modern pip (it uses `~/.cache/pip` automatically); the `pip.conf` key may be ignored.
- `sudo -E pip` preserves env vars but runs pip as root — prefer a virtualenv.
