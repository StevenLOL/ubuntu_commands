get pip.py

https://bootstrap.pypa.io/get-pip.py

How to cache downloaded PIP packages? 

printf '[global]\ndownload_cache = ~/.cache/pip\n' >> ~/.pip/pip.conf

Ref : http://stackoverflow.com/questions/10336308/how-to-cache-downloaded-pip-packages


In case line to pypi.python.org is not stable, using mirrors as listed in:

https://pypi-mirrors.org/

in ~/.pip/pip.conf

```
[global]
download_cache = /data/.cache/pip
index-url = https://pypi.douban.com/simple

timeout = 9000
#index-url = https://mirrors.aliyun.com/pypi/simple
#or https://mirrors.zte.com.cn/pypi/simple/
#trusted-host = mirrors.aliyun.com


#index-url = https://pypi.tuna.tsinghua.edu.cn/simple

```


Download the package only
```
pip download package -d /path/to/downloaded/file
```



To preserve environment

```
sudo -E pip install pydoop
REF:https://github.com/crs4/pydoop/issues/170
```

OR
```
[global]

index-url = http://mirrors.aliyun.com/pypi/simple
#or https://mirrors.zte.com.cn/pypi/simple/
trusted-host = mirrors.aliyun.com
```

OR

```
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple

pip config list

```

Install a known version:

```
pip install 'tensorflow>=0.8,<1.0'

```


#install tensorflow 0.12 on ubuntu 14.04

```

apt-get install software-properties-common
sudo add-apt-repository ppa:fkrull/deadsnakes-python2.7
sudo apt-get update
sudo apt-get upgrade

pip uninstall pip
wget https://bootstrap.pypa.io/get-pip.py
sudo python ./get-pip.py

pip install 'tensorflow<1.0'
```

