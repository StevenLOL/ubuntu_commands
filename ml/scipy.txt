# SciPy (scientific Python stack)

## 1. What is it?

SciPy is the core scientific-computing library for Python (numerical integration, optimization, signal processing, stats) built on NumPy. This note just points to Windows prebuilt wheels.

## 2. What is it for?

- Numerical/scientific computing in Python (the foundation for scikit-learn, pandas, etc.).

## 3. How to download / install

```bash
pip install numpy scipy
```

On Windows, if pip builds fail, use prebuilt wheels:
https://www.lfd.uci.edu/~gohlke/pythonlibs/

## 4. How to use

```python
import scipy
from scipy import optimize, signal
```

## 5. Pitfalls

- **Build from source needs a Fortran compiler** (gfortran); on Windows prefer the Gohlke wheels or `conda install scipy`.
- **`scipy` requires `numpy`** of a compatible version; let pip resolve both.
- The Gohlke page is unofficial — prefer `conda`/`pip` wheels for current Python.
