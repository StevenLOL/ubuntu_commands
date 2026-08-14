# SciPy (scientific Python stack)

## 1. What is it / What is it for?

SciPy is the core scientific-computing library for Python (numerical integration, optimization, signal processing, stats) built on NumPy. This note just points to Windows prebuilt wheels.


- Numerical/scientific computing in Python (the foundation for scikit-learn, pandas, etc.).

## 2. How to download / install

```bash
pip install numpy scipy
```

On Windows, if pip builds fail, use prebuilt wheels:
https://www.lfd.uci.edu/~gohlke/pythonlibs/

## 3. How to use

```python
import scipy
from scipy import optimize, signal
```

