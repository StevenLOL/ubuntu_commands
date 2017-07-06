# xgboost


# install 
```
sudo pip install xgboost
```

# install with gpu support

[REF](https://github.com/dmlc/xgboost/tree/master/plugin/updater_gpu) 

```
# install from source
Dependenceies

Install cuda and driver

Dowload and unzip cub from https://nvilabs.github.io/cub


git clone --recursive https://github.com/dmlc/xgboost
git submodule init
git submodule update

cd xgboost

cp ./make/config.mk ./
set DPLUGIN_UPDATER_GPU = ON
and point CUB_PATH to your cub folder

mkdir build
cd build
cmake .. -DPLUGIN_UPDATER_GPU=ON
make -j

cd ..

cd python-package
sudo python ./setup.py install

cd ..

```

## test

```
cd plugin/updater_gpu
python -m nose test/python

```
