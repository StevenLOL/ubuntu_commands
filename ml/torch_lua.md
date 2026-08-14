# PyTorch (Python ML framework)

## 1. What is it?

PyTorch is the modern Python deep-learning framework (Tensor/autograd/`torch.nn`), succeeding the Lua Torch. This note shows how to install a pinned CPU/GPU build and check CUDA/Python compatibility.

## 2. What is it for?

- Building and training neural networks with dynamic (define-by-run) graphs.
- GPU training via CUDA (`torch.cuda`).

## 3. How to download / install

Example pinned install (CUDA 10.2):
```bash
conda install pytorch==1.9.1 torchvision==0.10.1 torchaudio==0.9.1 cudatoolkit=10.2 -c pytorch
```
Find your version: https://pytorch.org/get-started/previous-versions/

## 4. How to use

```python
import torch
print(torch.__version__, torch.cuda.is_available())
```

The note also references a CUDA-vs-Python compatibility table (image):
`https://github.com/<YOUR_GITHUB>/ubuntu_commands/assets/.../513c26b4-...png`

## 5. Pitfalls

- **Match CUDA to the wheel**: install the `cudatoolkit` version your driver supports; a mismatched build can't use the GPU.
- **Replace `<YOUR_GITHUB>`** — the asset link is a template.
- **conda vs pip**: if you use pip, use the `pip` command from the PyTorch site (not the conda one).
- Pin versions for reproducibility; unpinned `conda install pytorch` may pull an unexpected CUDA build.
