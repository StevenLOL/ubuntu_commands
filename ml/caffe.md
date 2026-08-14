# Caffe (deep learning framework)

## 1. What is it?

Caffe is a classic, C++/CUDA deep-learning framework with Python and MATLAB bindings, popular for image classification before the PyTorch/TensorFlow era. This note points to a ready-made **nvidia-docker** Caffe image.

## 2. What is it for?

- Training/inference of CNN image models (the original "model zoo" framework).
- Quick starts via a prebuilt GPU Docker image instead of compiling from scratch.

## 3. How to download / install

See [nvidia docker](https://github.com/<YOUR_GITHUB>/ubuntu_commands/tree/master/docker). After installing `nvidia-docker` + the NVIDIA driver, find the prepared Caffe image:

```bash
sudo nvidia-docker search <YOUR_DOCKERHUB>
```

Then `nvidia-docker pull <YOUR_DOCKERHUB>/caffe:<tag>`.

For a from-source build, see `caffe_install_docker.md` and `caffe_ubuntu_16.04_15.10.txt` in this folder.

## 4. How to use

```bash
# run the image with GPUs + a data mount
sudo nvidia-docker run -ti --device /dev/nvidia0 --device /dev/nvidiactl \
  --device /dev/nvidia-uvm -v /data:/data <YOUR_DOCKERHUB>/caffe /bin/bash
```

## 5. Pitfalls

- **Deprecated**: Caffe is largely unmaintained; prefer PyTorch/TensorFlow for new work.
- **CUDA version lock**: old Caffe needs specific CUDA/cuDNN; mismatches cause build/runtime errors (see `caffe_ubuntu_16.04_15.10.txt` for the classic GCC-5/ABI hacks).
- **`<YOUR_DOCKERHUB>` / `<YOUR_GITHUB>` placeholders** must be replaced with your real namespace.
