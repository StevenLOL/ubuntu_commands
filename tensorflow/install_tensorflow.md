# Build TensorFlow with Bazel

## 1. What is it?

Instructions for building TensorFlow from source on Ubuntu 14.04/15.10 using **Bazel** and Java 8. (Historical — current TF provides prebuilt wheels; build only if you need custom ops/a specific version.)

## 2. What is it for?

- Compiling a TensorFlow wheel tuned to your CUDA/cuDNN/CPU.
- Building TF inside a Docker image (the original used a `tensorflow/tensorflow:latest-devel-gpu` container).

## 3. How to download / install

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

## 4. How to use

After `bazel build`, produce and install the wheel:
```bash
bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tf_pkg
pip install /tmp/tf_pkg/*.whl
```

## 5. Pitfalls

- **`webupd8team/java` PPA is dead** — use OpenJDK 8 (`apt install openjdk-8-jdk`) instead.
- **Bazel version vs TF**: each TF release requires a specific Bazel version; mismatch → configure/build errors. Check TF's `configure.py` for the required Bazel.
- **`--config=cuda`** needs CUDA/cuDNN installed and `./configure` pointed at them.
- **Long build**: TF from source takes a very long time; prefer wheels unless you must build.
