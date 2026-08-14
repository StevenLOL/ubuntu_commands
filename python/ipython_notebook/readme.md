# Jupyter Notebook (IPython Notebook)

## 1. What is it / What is it for?

Jupyter Notebook is the web-based interactive Python environment (formerly "IPython Notebook"). This note covers install, plotting, remote access, kernels, and extensions.


- Interactive data-science / ML workflows in the browser.
- Sharing/running notebooks remotely and converting them to scripts.

## 2. How to download / install

```bash
sudo apt install python-dev     # needed for "Python.h" build errors
sudo pip install ipython -U
sudo pip install jupyter -U
sudo pip install notebook -U
# for Python 2.7 use ipython<6:  pip install "ipython==5.5.0" jupyter notebook -U
```

If you get "no extension" errors: `sudo apt-get install --reinstall python-setuptools`.

## 3. How to use

### Start
```bash
ipython notebook --ip='0.0.0.0' --pylab=inline --port=7777
# or
ipython notebook yournotebook.ipynb
```

### Plot inline
```python
#coding=utf-8
%matplotlib inline
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.clf(); plt.plot(YOURDATA); plt.show()
```

### Cell timing / autoreload
```python
%%time
%load_ext autoreload
%autoreload 2
```

### Remote access via password
```bash
jupyter notebook --generate-config
python -c "from notebook.auth import passwd; print(passwd())"
# paste the sha1 into ~/.jupyter/jupyter_notebook_config.py:
# c.NotebookApp.password = u'sha1:...'
```

### Remote access via SSH tunnel (safer)
```bash
ssh -L 8000:localhost:8888 user@server_ip
# then open http://localhost:8000 on your laptop
```

### Add Python 2 / 3 kernels
```bash
python3 -m pip install jupyterhub notebook ipykernel
python3 -m ipykernel install
python2 -m pip install ipykernel
python2 -m ipykernel install
```

### Extensions & SSL
```bash
pip install jupyter_nbextensions_configurator jupyter_contrib_nbextensions -i https://pypi.douban.com/simple
jupyter contrib nbextension install --user --skip-running-check
jupyter nbextensions_configurator enable --user
openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
```

### Convert notebook to script
```bash
jupyter nbconvert --to script ./0001.ipynb
```

