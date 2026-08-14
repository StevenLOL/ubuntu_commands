# Makefile (C/C++ build flags)

## 1. What is it?

A Makefile tells `make` how to build a project. This note is a sample `CFLAGS`/`LDFLAGS` line used to compile a C/C++ project (e.g. a CUDA/MXNet-style project) with optimizations and CUDA includes.

## 2. What is it for?

- Defining compiler flags: warnings, optimization level, include/lib paths.
- Building native code that links CUDA and custom headers (e.g. `mshadow`).

## 3. How to download / install

No install — `make` comes with `build-essential`. This is the flags snippet:

```make
-I    # include headers
-L    # link libraries
export CFLAGS = -Wall -g -O3 -msse3 -Wno-unknown-pragmas -funroll-loops -I./mshadow/ -I/usr/local/cuda-6.0/include -L/usr/local/cuda-6.0/lib64
```

## 4. How to use

Put the `CFLAGS`/`LDFLAGS` line at the top of your Makefile (or `export` them in the shell before `make`). Adjust the CUDA paths to your installed version.

## 5. Pitfalls

- **`-msse3` is dated** — modern CPUs support SSE4/AVX; use `-march=native` for portable speed (or match your target).
- **CUDA 6.0 paths are old** — point `-I`/`-L` at your real CUDA install (`/usr/local/cuda` symlink is safer than a versioned path).
- **`-O3` can expose undefined-behavior bugs** that `-O0` hid; if a build misbehaves, try `-O2`.
- `-I`/`-L` are compiler/linker flags, not `make` variables by themselves — ensure they're passed to `gcc`/`g++` (e.g. `$(CC) $(CFLAGS)`).
