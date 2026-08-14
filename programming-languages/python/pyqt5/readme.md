# PyQt5 GUI development

## 1. What is it / What is it for?

PyQt5 is the Python binding for the Qt 5 GUI toolkit. This note collects tips for using it inside PyCharm and packaging a script with PyInstaller.


- Building desktop GUI applications in Python.
- Packaging a Python script into a standalone executable (`.exe` on Windows).

## 2. How to download / install

```bash
pip install pyqt5
pip install pyinstaller
```

In PyCharm, point the project interpreter at a Conda/venv environment that has PyQt5 installed.

## 3. How to use

### PyCharm + Conda
- Add interpreter → locate the Conda executable → choose an environment that has PyQt5.

### Package with PyInstaller
```bash
pyinstaller -F xxx.py        # single-file exe
pyinstaller -F -w xxx.py     # windowed (no console)
pyinstaller -F -c xxx.py     # with console
pyinstaller -F -i xxx.ico xxx.py   # with an icon
```

