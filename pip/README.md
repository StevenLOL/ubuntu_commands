get pip.py

https://bootstrap.pypa.io/get-pip.py

How to cache downloaded PIP packages? 

printf '[global]\ndownload_cache = ~/.cache/pip\n' >> ~/.pip/pip.conf

Ref : http://stackoverflow.com/questions/10336308/how-to-cache-downloaded-pip-packages


In case line to pypi.python.org is not stable, using mirrors as listed in:

https://pypi-mirrors.org/

in pip.conf

```
[global]
download_cache = /data/.cache/pip
index-url = https://pypi.douban.com/simple
```


Download the package only
```
pip install package -d /path/to/downloaded/file
```



To preserve environment

```
sudo -E pip install pydoop
REF:https://github.com/crs4/pydoop/issues/170
```
