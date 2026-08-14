# Limit CPU threads for a Python program

## 1. What is it / What is it for?

Two ways to cap how many CPUs/threads a program (e.g. a Python script using numpy/OpenMP/MKL) may use.

Ref: http://stackoverflow.com/questions/39381974/


- Preventing a job from saturating all cores (so the box stays usable for others).
- Reproducible runs by limiting parallelism.

## 2. How to download / install

No install — uses `taskset` (util-linux, preinstalled) and env vars.

## 3. How to use

**Pin to specific CPUs (cores 1–3):**
```bash
taskset -c 1-3 python yourProgram.py
```

**Limit OpenMP threads:**
```bash
OMP_NUM_THREADS=4 python yourProgram.py
```

(For MKL/numpy, also consider `MKL_NUM_THREADS` / `NUMEXPR_NUM_THREADS`.)

