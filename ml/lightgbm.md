
To install and use GPU for lightGBM training

If you are looking for CPU based solution:

```
pip install lightGBM
```

# GPU

## download the source from git:

```

git clone --recursive https://github.com/Microsoft/LightGBM
```
## Build and install lightGDM GPU
### Install libs
```
apt install libboost-dev libboost-system-dev libboost-filesystem-dev libboost-all-dev
cd LightGBM
mkdir build
cd build
cmake -DUSE_GPU=1 -DOpenCL_LIBRARY=/usr/local/cuda-8.0/lib64/libOpenCL.so -DOpenCL_INCLUDE_DIR=/usr/local/cuda-8.0/include/ ../
make -j 4
make install
cd ..
```

### Install Python-package
```
cd python-package
python setup.py install --precompile
```
[REF](https://github.com/Microsoft/LightGBM/issues/1097)

## Model training
```
        model=lgb.LGBMRegressor(num_leaves=31,n_jobs=-1,
                                learning_rate=0.05,n_estimators=1000,
                               device='gpu',
                                gpu_platform_id=0,
                               gpu_device_id=0)
         model.fit(trainx,trainy,
                 eval_set=[(devx,devy)],
                 eval_metric='mse',
                 early_stopping_rounds=5)                         
```
# FAQ
## Exception: Please install CMAKE first
## libstdc++.so.6: version `GLIBCXX_3.4.20' not found
```
conda install libgcc
```
[REF](https://github.com/dmlc/xgboost/issues/1043)

## No Opencl device found
```
mkdir -p /etc/OpenCL/vendors  
echo "libnvidia-opencl.so.1" > /etc/OpenCL/vendors/nvidia.ic
```
[REF](https://github.com/Microsoft/LightGBM/issues/586)
