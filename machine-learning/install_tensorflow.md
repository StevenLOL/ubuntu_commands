# Build TensorFlow with Bazel

## 1. What is it / What is it for?

Instructions for building TensorFlow from source on Ubuntu 14.04/15.10 using **Bazel** and Java 8. (Historical — **as of 2026 current TF ships prebuilt wheels (2.x); Bazel builds are only needed for custom ops/a specific version.** A source build today targets CUDA 12.x/13.x and a recent Bazel; see the official "Build from source" guide rather than the PPA/Java-8 steps below.)


- Compiling a TensorFlow wheel tuned to your CUDA/cuDNN/CPU.
- Building TF inside a Docker image (the original used a `tensorflow/tensorflow:latest-devel-gpu` container).

## 2. How to download / install

**Java 8 (Oracle):**
```bash
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer
```

**Bazel (apt repo):**
```bash
echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
sudo apt-get update && sudo apt-get install bazel
```

**Inside the TF devel Docker image** (alternative):
```bash
docker pull tensorflow/tensorflow:latest-devel-gpu
docker run -it -p 8888:8888 tensorflow/tensorflow:latest-devel-gpu bash
# then: git clone --recurse-submodules https://github.com/tensorflow/tensorflow
#       ./configure
#       bazel build --config=opt --config=cuda //tensorflow/tools/pip_package:build_pip_package
```

## 3. How to use

After `bazel build`, produce and install the wheel:
```bash
bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tf_pkg
pip install /tmp/tf_pkg/*.whl
```

