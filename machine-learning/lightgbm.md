# LightGBM (gradient boosting framework)

## 1. What is it / What is it for?

LightGBM is Microsoft's fast, distributed gradient-boosting framework (GBDT) with both CPU and GPU support. This note covers CPU install, GPU build on CUDA, and training with the GPU.


- Training high-performance gradient-boosted tree models for classification/regression.
- Speeding up training on a GPU via OpenCL.

## 2. How to download / install

**CPU (pip):**
```bash
pip install lightgbm
```

**GPU (from source):**
```bash
git clone --recursive https://github.com/Microsoft/LightGBM
sudo apt install libboost-dev libboost-system-dev libboost-filesystem-dev libboost-all-dev
cd LightGBM
mkdir build && cd build
cmake -DUSE_GPU=1 -DOpenCL_LIBRARY=/usr/local/cuda-8.0/lib64/libOpenCL.so \
  -DOpenCL_INCLUDE_DIR=/usr/local/cuda-8.0/include/ ../
make -j 4
make install
cd ../python-package
python setup.py install --precompile
```
Ref: https://github.com/Microsoft/LightGBM/issues/1097

## 3. How to use

GPU training in Python:
```python
model = lgb.LGBMRegressor(num_leaves=31, n_jobs=-1,
                          learning_rate=0.05, n_estimators=1000,
                          device='gpu',
                          gpu_platform_id=0, gpu_device_id=0)
model.fit(trainx, trainy,
          eval_set=[(devx, devy)], eval_metric='mse',
          early_stopping_rounds=5)
```

