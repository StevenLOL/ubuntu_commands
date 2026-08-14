# ALIZE (speaker verification toolkit)

## 1. What is it?

ALIZE is an open-source speaker-verification system. It ships two parts: **ALIZE** (the core) and **LIA_RAL** (a set of helper scripts/algorithms from LIA).

## 2. What is it for?

- Building and testing speaker-recognition / verification systems from audio features.
- Research on Gaussian-mixture speaker models.

## 3. How to download / install

```bash
svn co http://alize.univ-avignon.fr/svn/ALIZE/trunk/ ALIZE &
svn co http://alize.univ-avignon.fr/svn/LIA_RAL/trunk/ LIA_RAL &
```

Build (autotools-based):
```bash
aclocal
autoconf
automake
./configure
make
```

## 4. How to use

After building, the ALIZE libraries/executables are used by speaker-verification pipelines (often driven by LIA_RAL Perl/Shell scripts). Link your project against the built `libalize`.

## 5. Pitfalls

- **Note the original had `./make`** — that's a typo; the command is `make` (no dot-slash prefix; `./make` will fail unless such a file exists).
- **autotools order matters**: `aclocal` → `autoconf` → `automake` before `./configure`.
- **Old SVN repo**: the project is largely unmaintained; expect to patch for modern compilers/gcc.
