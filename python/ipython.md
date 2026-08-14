# IPython tips

## 1. What is it?

IPython is an enhanced interactive Python shell. This note captures two handy magic commands.

## 2. What is it for?

- Saving your REPL session to a script for later reuse.

## 3. How to download / install

```bash
pip install ipython
```

## 4. How to use

```python
# save lines 1-48 of the current session to a file
%save filename 1-48

# save the command history to a python file
%hist -f my_history.py
```

## 5. Pitfalls

- `%save`/`%hist` are IPython magics — they don't work in the plain `python` REPL.
- Line ranges are 1-based and refer to the session's input history.
