# JupyterLab

## 1. What is it?

JupyterLab is the next-generation web UI for Jupyter notebooks (notebooks, terminals, and file browser in one IDE-like interface).

## 2. What is it for?

- Running notebooks with a richer IDE experience than classic Notebook.
- Remote notebook servers with a password.

## 3. How to download / install

```bash
pip install jupyterlab
```
Refs: https://jupyter.org/install.html · https://www.cnblogs.com/jscs/p/13724589.html

## 4. How to use

```bash
# start, listening on all interfaces, as root
jupyter lab --ip 0.0.0.0 --allow-root
```

### Progress bars (tqdm) not showing?
```bash
jupyter nbextension enable --py widgetsnbextension
jupyter labextension install @jupyter-widgets/jupyterlab-manager
```

### Set a password
```bash
jupyter server --generate-config
jupyter server password
```

## 5. Pitfalls

- **`--allow-root`** is required when running as root; otherwise JupyterLab refuses to start.
- **`--ip 0.0.0.0` without a password** exposes your server — set a password (`jupyter server password`) or use an SSH tunnel.
- **tqdm widgets** need the `jupyter-widgets` extension; without it you see the raw `HBox(...)` object instead of a bar.
- Labextension install needs Node.js; on a server without it, skip the widget extension.
