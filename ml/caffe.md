# Caffe (deep learning framework)

## 1. What is it / What is it for?

Caffe is a classic, C++/CUDA deep-learning framework with Python and MATLAB bindings, popular for image classification before the PyTorch/TensorFlow era. This note points to a ready-made **nvidia-docker** Caffe image.


- Training/inference of CNN image models (the original "model zoo" framework).
- Quick starts via a prebuilt GPU Docker image instead of compiling from scratch.

## 2. How to download / install

See [nvidia docker](https://github.com/<YOUR_GITHUB>/ubuntu_commands/tree/master/docker). After installing `nvidia-docker` + the NVIDIA driver, find the prepared Caffe image:

```bash
sudo nvidia-docker search <YOUR_DOCKERHUB>
```

Then `nvidia-docker pull <YOUR_DOCKERHUB>/caffe:<tag>`.

For a from-source build, see `caffe_install_docker.md` and `caffe_ubuntu_16.04_15.10.txt` in this folder.

## 3. How to use

```bash
# run the image with GPUs + a data mount
sudo nvidia-docker run -ti --device /dev/nvidia0 --device /dev/nvidiactl \
  --device /dev/nvidia-uvm -v /data:/data <YOUR_DOCKERHUB>/caffe /bin/bash
```

