# Theano (GPU math / deep learning, Ubuntu 14.04)

## 1. What is it?

Theano is an older Python library that lets you define, optimize, and evaluate mathematical expressions (incl. deep nets) on CPU or NVIDIA GPU via CUDA. This note is a battle-tested setup for Ubuntu 14.04 + GTX 860M (CUDA 6.5).

> Try at your own risk — driver/CUDA versions are dated; treat as reference.

## 2. What is it for?

- Symbolically building and GPU-accelerating numeric/ML computations (the predecessor of modern autodiff frameworks).
- Useful historically; for new work prefer PyTorch/TensorFlow.

## 3. How to download / install

1. Confirm the GPU is visible: `lspci | grep -i NVIDIA`
2. Install driver **NVIDIA 340.76** and **CUDA 6.5 toolkit**. Do **NOT** install `bumblebee`.
3. Switch to the NVIDIA card: `prime-switch nvidia`
4. Blacklist conflicting drivers in `/etc/modprobe.d/blacklist-file-drivers.conf`:
   ```text
   blacklist nvidia-349
   blacklist nvidia-346
   blacklist xserver-xorg-video-nouveau
   ```
   (Keep `nvidia-340` un-blacklisted.) List drivers with `ubuntu-drivers devices`.
5. Verify: `nvidia-modprobe`, `nvidia-settings`, `nvidia-smi` work.
6. Build `deviceQuery`:
   ```bash
   cd /usr/local/cuda-6.5/samples/1_Utilities/deviceQuery/
   sudo make && ./deviceQuery    # expect Result = PASS
   ```
7. Install Theano + deps:
   ```bash
   sudo apt-get install python-numpy python-scipy python-dev python-pip python-nose g++ libopenblas-dev git
   sudo pip uninstall theano
   sudo pip install git+git://github.com/Theano/Theano.git
   ```
8. CUDA 6.5 needs gcc-4.8: `sudo ln -s /usr/bin/gcc-4.8 /usr/local/cuda/bin/gcc` (same for g++).

## 4. How to use

Create `~/.theanorc`:
```ini
[global]
floatX = float32
device = gpu

[nvcc]
fastmath = True

[cuda]
root = /usr/local/cuda-6.5/
```

Test (`test.py`):
```python
from theano import function, config, shared, sandbox
import theano.tensor as T
import numpy, time
vlen = 10 * 30 * 768
iters = 1000
rng = numpy.random.RandomState(22)
x = shared(numpy.asarray(rng.rand(vlen), config.floatX))
f = function([], T.exp(x))
print f.maker.fgraph.toposort()
t0 = time.time()
for i in xrange(iters):
    r = f()
t1 = time.time()
print 'Looping %d times took' % iters, t1 - t0, 'seconds'
print 'Result is', r
if numpy.any([isinstance(x.op, T.Elemwise) for x in f.maker.fgraph.toposort()]):
    print 'Used the cpu'
else:
    print 'Used the gpu'
```
Run `sudo python test.py`. If you see `libcublas.so.6.5: cannot open shared object file`, add to `~/.bashrc`:
```bash
sudo ldconfig /usr/local/cuda-6.5/lib64/
sudo ldconfig /usr/local/cuda-6.5/targets/x86_64-linux/lib/
export LD_LIBRARY_PATH=/usr/local/cuda-6.5/targets/x86_64-linux/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/cuda-6.5/bin:$PATH
export PATH=/usr/local/cuda-6.5/targets/x86_64-linux/lib:$PATH
```

Keras backend switch: `~/.keras/keras.json` → `{"epsilon":1e-07,"floatx":"float32","backend":"theano"}`.
CPU-only run: `THEANO_FLAGS=mode=FAST_RUN,device=cpu python ./harpc_label_server.py`

## 5. Pitfalls

- **Black screen / login loop** after driver tweaks: `Ctrl+Alt+F1`, `sudo rm /etc/X11/xorg.conf`, then `sudo service lightdm stop && sudo service lightdm start`. The author also added `nvidia-modprobe` / `prime-switch intel` to `~/.bashrc` to escape a login loop.
- **gcc version**: CUDA 6.5 only supports gcc-4.8; newer gcc breaks nvcc.
- **`libcublas.so` not found** → run the `ldconfig` + `LD_LIBRARY_PATH` steps above.
- **Dated stack**: CUDA 6.5 / driver 340 / Ubuntu 14.04 are ancient; on modern systems use PyTorch/TensorFlow. This is a historical reference, not a current install guide.
- **Python 2 syntax** (`print` statements, `xrange`) — Theano examples here target Py2; modernise for Py3 if reusing.
