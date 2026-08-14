# Python tips (interactive shell, pip wheels)

## 1. What is it / What is it for?

Miscellaneous Python notes: enabling tab-completion in the interactive shell, and a tip that `.whl` files are just zip archives.


- Making the plain `python` REPL usable (tab completion).
- Understanding that you can inspect/unpack a `.whl` like any zip.

## 2. How to download / install

For tab completion, install the readline helpers:
```bash
sudo pip install readline rlcompleter3
```
Create `~/.pythonrc`:
```python
# ~/.pythonrc — enable tab completion
try:
    import readline
except ImportError:
    print("Module readline not available.")
else:
    import rlcompleter
    readline.parse_and_bind("tab: complete")
```
Then in `~/.bashrc`:
```bash
export PYTHONSTARTUP=~/.pythonrc
```

## 3. How to use

Start `python` and press Tab to complete names. For Windows prebuilt wheels, see https://www.lfd.uci.edu/~gohlke/pythonlibs/ . The `.whl` format is a zip — `unzip package.whl` works.

