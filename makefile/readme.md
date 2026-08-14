# Makefile (C/C++ build flags)

## 1. What is it / What is it for?

A Makefile tells `make` how to build a project. This note is a sample `CFLAGS`/`LDFLAGS` line used to compile a C/C++ project (e.g. a CUDA/MXNet-style project) with optimizations and CUDA includes.


- Defining compiler flags: warnings, optimization level, include/lib paths.
- Building native code that links CUDA and custom headers (e.g. `mshadow`).

## 2. How to download / install

No install — `make` comes with `build-essential`. This is the flags snippet:

```make
-I    # include headers
-L    # link libraries
export CFLAGS = -Wall -g -O3 -msse3 -Wno-unknown-pragmas -funroll-loops -I./mshadow/ -I/usr/local/cuda-6.0/include -L/usr/local/cuda-6.0/lib64
```

## 3. How to use

Put the `CFLAGS`/`LDFLAGS` line at the top of your Makefile (or `export` them in the shell before `make`). Adjust the CUDA paths to your installed version.

