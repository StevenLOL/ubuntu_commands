# JupyterLab

## 1. What is it / What is it for?

JupyterLab is the next-generation web UI for Jupyter notebooks (notebooks, terminals, and file browser in one IDE-like interface).


- Running notebooks with a richer IDE experience than classic Notebook.
- Remote notebook servers with a password.

## 2. How to download / install

```bash
pip install jupyterlab
```
Refs: https://jupyter.org/install.html · https://www.cnblogs.com/jscs/p/13724589.html

## 3. How to use

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

