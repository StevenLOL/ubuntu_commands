# Visual Studio Code (Python setup)

## 1. What is it?

VS Code is Microsoft's free, extensible code editor. This note covers using it with a Conda interpreter and packaging scripts with PyInstaller.

## 2. What is it for?

- Editing/debugging Python (and many other languages) with a lightweight IDE.
- Packaging a Python script into a standalone executable via PyInstaller.

## 3. How to download / install

Download: https://code.visualstudio.com/
Python extension: install "Python" by Microsoft from the Extensions panel.

## 4. How to use

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

## 5. Pitfalls

- **Select the right interpreter** or imports/debugging won't match your env.
- **`-w` hides the console** — use `-c` while debugging crashes.
- **`<YOUR_GITHUB>` placeholder** in the image link must be replaced with the real repo owner.
- The original's Chinese external-tools article link is supplementary; the PyInstaller commands above are canonical.
