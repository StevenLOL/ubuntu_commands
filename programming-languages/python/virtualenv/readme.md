# virtualenv (isolated Python environments)

## 1. What is it / What is it for?

`virtualenv` creates isolated Python environments so each project gets its own packages, separate from the system Python.


- Avoiding dependency conflicts between projects.
- Reproducible, self-contained Python setups.

## 2. How to download / install

```bash
sudo pip install virtualenv
virtualenv --version
```

(Modern Python also has the built-in `python -m venv`.)

## 3. How to use

```bash
# env for default python
virtualenv justtest

# env for a specific python3
virtualenv -p /usr/bin/python3 ~/python3_env

# activate
source ./justtest/bin/activate

# deactivate
deactivate
```

