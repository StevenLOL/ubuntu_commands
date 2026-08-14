# IPython tips

## 1. What is it / What is it for?

IPython is an enhanced interactive Python shell. This note captures two handy magic commands.


- Saving your REPL session to a script for later reuse.

## 2. How to download / install

```bash
pip install ipython
```

## 3. How to use

```python
# save lines 1-48 of the current session to a file
%save filename 1-48

# save the command history to a python file
%hist -f my_history.py
```

