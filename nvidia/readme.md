# NVIDIA driver & CUDA setup on Ubuntu

## 1. What is it / What is it for?

Notes for installing the NVIDIA proprietary driver and the **CUDA** toolkit (plus cuDNN) on Ubuntu, configuring env vars, and fixing common GPU/CUDA breakages.

> **As of 2026:** the current CUDA Toolkit is **13.x** (e.g. 13.3.x); the `.deb` "cuda" metapackage method below still works and is the recommended path. Driver + CUDA now install together via `sudo apt install cuda` (or `nvidia-driver-XXX`). The older recipes in this folder (`cuda8.0.txt`, `cuda.txt`) are historical.


- Enabling GPU compute (Deep Learning, CUDA apps) by installing the driver + CUDA + cuDNN.
- Fixing "library not found" and multi-GPU/display issues.

## 2. How to download / install

**Recommended: the CUDA `.deb` (runfile also works)**
1. Download the runfile/`.deb` from https://developer.nvidia.com/cuda-downloads/
2. `sudo dpkg -i <package>` (adds the CUDA apt repo), then:
   ```bash
   sudo apt-get update
   sudo apt-get install cuda
   ```
3. **cuDNN** (speeds up DNNs) is downloaded separately: https://developer.nvidia.com/cudnn — copy its contents into your CUDA lib folder.

**Older recipes in this folder:**
- `readme.md` — the `.deb` method + `nvidia-smi -l 1` to watch the GPU.
- `cuda8.0.txt` — CUDA 8.0 + GTX 1080: remove the bundled `361` driver, install `367.27` from the `.run`.
- `cuda.txt` — env vars to source.

**Environment (add to `~/.bashrc`)** — see `cuda.txt`:
```bash
export CUDA_HOME=/usr/local/cuda
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/cuda/lib64
```

## 3. How to use

```bash
nvidia-smi -l 1        # live GPU stats
nvidia-settings        # display config (run as root for 2-GPU setups)
# verify a CUDA build:
cd /usr/local/cuda/samples/1_Utilities/deviceQuery && sudo make && ./deviceQuery
```

Multi-display / multi-GPU: save X config to `/etc/X11/xorg.conf` (see `nvidiaxorgsetting/Xorg.config`).

