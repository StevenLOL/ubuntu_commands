# Install Eclipse + PyDev (script)

## 1. What is it / What is it for?

This `installpackage.sh` is a helper that downloads and installs **Eclipse** (the Java-based IDE) together with **PyDev** (Python development plugin) and the JDK.


- One-shot setup of a Python-capable Eclipse IDE on a fresh Ubuntu box.
- Useful when you prefer Eclipse over PyCharm for Python editing.

## 2. How to download / install

The script pulls the Eclipse tarball, the JDK, and PyDev. Inspect it, then run:

```bash
chmod +x installpackage.sh
./installpackage.sh
```

Prereq: a working internet connection and `wget`/`tar` available.

## 3. How to use

After the script finishes, launch Eclipse from its install directory and enable the PyDev interpreter under `Window → Preferences → PyDev → Interpreters → Python Interpreter`.

