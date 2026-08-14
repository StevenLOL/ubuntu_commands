# TensorFlow (install & build)

## 1. What is it / What is it for?

TensorFlow is Google's open-source ML framework. This folder covers installing the GPU build via pip, building it from source with **Bazel**, the CUDA/cuDNN prerequisites, troubleshooting, and building the Android demo. (The original notes target TF 0.7–0.12 — treat as historical. **As of 2026 the current stable line is TensorFlow 2.x** — e.g. `pip install tensorflow` installs 2.16+; for GPU add `tensorflow[and-cuda]` which pulls a compatible CUDA 12.x.)


- Training/inference of neural nets on CPU or GPU.
- Building the Android TF demo app.

## 2. How to download / install

**Modern (recommended):**
```bash
pip install tensorflow        # CPU
pip install tensorflow[and-cuda]   # or pip install tensorflow-gpu  (older)
```

**From the old notes (GPU, TF 0.7.1, Python 2.7):**
```bash
sudo pip install --upgrade https://storage.googleapis.com/tensorflow/linux/gpu/tensorflow-0.7.1-cp27-none-linux_x86_64.whl
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda/lib64"
export CUDA_HOME=/usr/local/cuda
sudo apt-get install build-essential gfortran
```

**Build with Bazel** (needs Java 8 + Bazel): see `install_tensorflow.md`.

**CUDA + cuDNN:** download the CUDA `.deb` from developer.nvidia.com and `cudnn` from developer.nvidia.com; copy cuDNN includes/libs into your CUDA dir (see `install_tensorflow.md`).

## 3. How to use

Test the install:
```python
import tensorflow as tf
hello = tf.constant('Hello, TensorFlow!')
sess = tf.Session()
print(sess.run(hello))     # b'Hello, TensorFlow!'
a, b = tf.constant(10), tf.constant(32)
print(sess.run(a + b))     # 42
```

**Build the Android demo** (in the TF source root, edit `WORKSPACE` to point at your SDK/NDK paths, then):
```bash
bazel build //tensorflow/examples/android:tensorflow_demo
<YOUR_HOME>/Android/Sdk/platform-tools/adb install -r -g ./tensorflow_demo.apk
```

