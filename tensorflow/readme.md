# TensorFlow (install & build)

## 1. What is it?

TensorFlow is Google's open-source ML framework. This folder covers installing the GPU build via pip, building it from source with **Bazel**, the CUDA/cuDNN prerequisites, troubleshooting, and building the Android demo. (The original notes target TF 0.7–0.12 — treat as historical; for current TF use `pip install tensorflow`.)

## 2. What is it for?

- Training/inference of neural nets on CPU or GPU.
- Building the Android TF demo app.

## 3. How to download / install

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

## 4. How to use

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

## 5. Pitfalls

- **These notes are ancient** (TF 0.7, Py2, CUDA 7.5); modern TF uses `pip install tensorflow` and TF2 eager/keras APIs. The `tf.Session()` example is TF1 style.
- **Typos in original**: `nivdia-smi` should be `nvidia-smi`; and `sudo apt-get install protobuf -I` (`-I` is not a valid pip flag).
- **`protobub` / `extern` import errors**: `sudo apt-get install --reinstall python-setuptools` and reinstall `protobuf`/`tensorflow` until clean.
- **cuDNN version must match CUDA**; copy the right headers/libs into `/usr/local/cuda-7.5/...`.
- **`<YOUR_HOME>` placeholders** in the Android `WORKSPACE` block must be replaced.
- Refs: github.com/tensorflow/tensorflow · SyntaxNet Chinese model blog.
