# RAPIDS (GPU data science)

## 1. What is it / What is it for?

RAPIDS is NVIDIA's suite of GPU-accelerated Python data-science libraries (cuDF, cuML, cuGraph) — pandas/scikit-learn-like APIs that run on CUDA.


- Accelerating ETL, dataframe ops, and ML on the GPU instead of pandas on CPU.
- Drop-in-ish replacements (`import cudf as pd`) for GPU boxes.

## 2. How to download / install

The easiest path is the official Docker image (pin the CUDA/Ubuntu/Python tags you need):
```bash
sudo docker pull rapidsai/rapidsai:cuda10.2-runtime-ubuntu18.04-py3.7
```
Ref: https://rapids.ai/start.html

Or `conda install -c rapidsai -c nvidia rapids` for a local env (match your CUDA version).

## 3. How to use

```bash
sudo docker run --gpus all -it rapidsai/rapidsai:cuda10.2-runtime-ubuntu18.04-py3.7
```
Then in Python: `import cudf, cuml`.

