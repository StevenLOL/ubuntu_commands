# virtualenv (isolated Python environments)

## 1. What is it?

`virtualenv` creates isolated Python environments so each project gets its own packages, separate from the system Python.

## 2. What is it for?

- Avoiding dependency conflicts between projects.
- Reproducible, self-contained Python setups.

## 3. How to download / install

```bash
sudo pip install virtualenv
virtualenv --version
```

(Modern Python also has the built-in `python -m venv`.)

## 4. How to use

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

## 5. Pitfalls

- **Activate is shell-specific**: `source bin/activate` (bash/zsh); Windows uses `Scripts\activate.bat`.
- **`virtualenv` vs `venv`**: on Python 3.3+, prefer `python3 -m venv` (no install needed). `virtualenv` adds features like `--system-site-packages`.
- **Don't `sudo pip install` inside an env** — leave the env active and use plain `pip` to keep packages local.
- **Path must point at a real interpreter** in `-p /usr/bin/python3`.
