# Visual Studio Code (Python setup)

## 1. What is it / What is it for?

VS Code is Microsoft's free, extensible code editor. This note covers using it with a Conda interpreter and packaging scripts with PyInstaller.


- Editing/debugging Python (and many other languages) with a lightweight IDE.
- Packaging a Python script into a standalone executable via PyInstaller.

## 2. How to download / install

Download: https://code.visualstudio.com/
Python extension: install "Python" by Microsoft from the Extensions panel.

## 3. How to use

### Work with Conda
- Add Interpreter → locate the Conda executable → choose an environment.
  (See the screenshot referenced in the original note: `https://github.com/<YOUR_GITHUB>/ubuntu_commands/assets/5893788/fa662a29-...`)

### Release / package a script
```bash
pyinstaller -F xxx.py          # single-file exe
pyinstaller -F -w xxx.py       # windowed (no console)
pyinstaller -F -c xxx.py       # with console
pyinstaller -F -i xxx.ico xxx.py   # with icon
```

