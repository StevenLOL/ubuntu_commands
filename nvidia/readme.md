# NVIDIA driver & CUDA setup on Ubuntu

## 1. What is it?

Notes for installing the NVIDIA proprietary driver and the **CUDA** toolkit (plus cuDNN) on Ubuntu, configuring env vars, and fixing common GPU/CUDA breakages.

## 2. What is it for?

- Enabling GPU compute (Deep Learning, CUDA apps) by installing the driver + CUDA + cuDNN.
- Fixing "library not found" and multi-GPU/display issues.

## 3. How to download / install

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

## 4. How to use

```bash
nvidia-smi -l 1        # live GPU stats
nvidia-settings        # display config (run as root for 2-GPU setups)
# verify a CUDA build:
cd /usr/local/cuda/samples/1_Utilities/deviceQuery && sudo make && ./deviceQuery
```

Multi-display / multi-GPU: save X config to `/etc/X11/xorg.conf` (see `nvidiaxorgsetting/Xorg.config`).

## 5. Pitfalls

- **Typo in original**: it's `sudo dpkg -i`, not `dpkag`.
- **`libcublas.so.X not found`**: add the CUDA lib path to a conf and reload:
  ```bash
  echo '/usr/local/cuda-5.5/lib64/' | sudo tee /etc/ld.so.conf.d/cuda_lib.conf
  sudo ldconfig
  ```
  (see `libcublas.so.notfound`.) Match the version to your CUDA install.
- **Driver/CUDA version lock**: the driver version must satisfy the CUDA toolkit; a mismatch breaks `nvidia-smi`. Remove the conflicting bundled driver first if needed (`sudo apt-get remove cuda-drivers nvidia-361`).
- **Secure Boot**: on UEFI systems, the NVIDIA module must be signed or Secure Boot disabled, or the driver won't load.
- **GCC mismatch**: old CUDA (5/6) needs old gcc (4.4/4.8); newer CUDA supports current gcc. The Chinese note at the bottom of `nvidia.txt` shows the gcc-4.4 downgrade trick.
- **`blacklist nouveau`** is required; the installer can add it, but a leftover nouveau module blocks the driver.
- **`nvidia.txt` has a leftover Chinese block** about gcc 4.4 install; it's a historical reference, not part of the English flow.
