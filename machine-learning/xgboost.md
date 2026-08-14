# XGBoost (gradient-boosted trees)

## 1. What is it / What is it for?

XGBoost is a scalable, distributed gradient-boosting library (GBDT) — a standard for tabular-data ML. This note covers pip install and building the GPU plugin from source.


- High-performance classification/regression on tabular data (Kaggle favourite).
- GPU-accelerated tree training via the `updater_gpu` plugin.

## 2. How to download / install

**CPU (pip):**
```bash
sudo pip install xgboost
```

**GPU (from source):**
Ref: https://github.com/dmlc/xgboost/tree/master/plugin/updater_gpu
```bash
# install CUDA + driver first
# download & unzip CUB from https://nvilabs.github.io/cub
git clone --recursive https://github.com/dmlc/xgboost
cd xgboost
git submodule init
git submodule update
cp ./make/config.mk ./
# edit config.mk: set DPLUGIN_UPDATER_GPU = ON and point CUB_PATH to your cub folder
mkdir build && cd build
cmake .. -DPLUGIN_UPDATER_GPU=ON
make -j
cd ../python-package
sudo python ./setup.py install
```

## 3. How to use

```python
import xgboost as xgb
# standard sklearn-style API: xgb.XGBClassifier / XGBRegressor
```

Test the GPU plugin:
```bash
cd plugin/updater_gpu
python -m nose test/python
```

