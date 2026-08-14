# RAPIDS (GPU data science)

## 1. What is it?

RAPIDS is NVIDIA's suite of GPU-accelerated Python data-science libraries (cuDF, cuML, cuGraph) — pandas/scikit-learn-like APIs that run on CUDA.

## 2. What is it for?

- Accelerating ETL, dataframe ops, and ML on the GPU instead of pandas on CPU.
- Drop-in-ish replacements (`import cudf as pd`) for GPU boxes.

## 3. How to download / install

The easiest path is the official Docker image (pin the CUDA/Ubuntu/Python tags you need):
```bash
sudo docker pull rapidsai/rapidsai:cuda10.2-runtime-ubuntu18.04-py3.7
```
Ref: https://rapids.ai/start.html

Or `conda install -c rapidsai -c nvidia rapids` for a local env (match your CUDA version).

## 4. How to use

```bash
sudo docker run --gpus all -it rapidsai/rapidsai:cuda10.2-runtime-ubuntu18.04-py3.7
```
Then in Python: `import cudf, cuml`.

## 5. Pitfalls

- **CUDA version must match**: `cuda10.2` image needs a CUDA 10.2 driver/host; pick the tag for your stack.
- **GPU required**: none of this runs on CPU; `--gpus all` (or `nvidia-container-toolkit`) is mandatory.
- **Pin the tag**: `rapidsai/rapidsai` without a tag pulls latest, which may not match your CUDA/driver — always pin.
- **Old tags rot**: the `cuda10.2-ubuntu18.04-py3.7` example is dated; use a current release from rapids.ai/start.
