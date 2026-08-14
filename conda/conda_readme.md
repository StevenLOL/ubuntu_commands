# Conda (package & environment manager)

> Ref: https://blog.csdn.net/lx_ros/article/details/123597208

## 1. What is it?

Conda is an open-source package and environment manager, shipped with Anaconda/Miniconda. It runs from the command line (or the Anaconda Navigator GUI) and creates isolated environments. Originally for Python, it now manages packages for R, Ruby, Lua, Scala, Java, JavaScript, C/C++, FORTRAN, etc.

## 2. What is it for?

- Isolating project dependencies in separate environments.
- Installing, upgrading, and removing packages without touching the system Python.
- Reproducing environments via `environment.yml`.

## 3. How to download / install

```bash
# Miniconda (recommended, lightweight)
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh

# or full Anaconda
wget https://repo.anaconda.com/archive/Anaconda3-latest-Linux-x86_64.sh
bash Anaconda3-latest-Linux-x86_64.sh
```

Restart the shell, then verify:

```bash
conda --version
```

## 4. How to use

### Manage Conda itself
```bash
conda --version
conda update conda
conda update anaconda
```

### Environment management
```bash
conda create --name your-env                  # create empty env
conda create --name snakes python=3.5         # create with a package + version
conda activate your-env                       # activate
conda deactivate                              # deactivate
conda info --envs                             # list environments
conda remove --name ENVNAME --all             # delete an env
conda create --clone ENVNAME --name NEWENV    # clone

# export / recreate
conda env export --name ENVNAME > envname.yml
conda env create -f envname.yml -n your-env-name

# revisions
conda list --revisions
conda list --name ENVNAME --revisions
conda install --name ENVNAME --revision REV_NUMBER
```

### Package management
```bash
conda search pkg-name
conda install pkg-name
conda list                                     # list installed (current env)
conda list --name ENVNAME
conda create --name NEWENV --file pkgs.txt     # create env from a pkg list
conda update --all --name ENVNAME
conda uninstall PKGNAME --name ENVNAME
conda install --yes PKG1 PKG2

# version pinning
conda install PKGNAME==3.1.4
conda install "PKGNAME[version='3.1.2|3.1.4']"
conda install "PKGNAME>2.5,<3.2"
conda install conda-forge::PKGNAME             # install from a channel
```

### Configuration (channels / mirrors)
```bash
conda config --show channels
conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/
conda config --remove channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/
conda clean -i                                 # clear index cache
```

A common Tsinghua `.condarc`:
```yaml
channels:
  - defaults
show_channel_urls: true
default_channels:
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/r
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/msys2
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/bioconda/
custom_channels:
  conda-forge: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  pytorch: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
```

`.condarc` search order (Linux): `~/.condarc`, `$CONDA_PREFIX/.condarc`, `$CONDARC`, `/etc/conda/.condarc`, etc.

## 5. Pitfalls

- **`conda activate` fails**: run `source <conda_root>/etc/profile.d/conda.sh` first, or open a new login shell.
- **Mixing pip and conda**: prefer `conda install`; if you must `pip install`, do it inside an activated env and expect possible conflicts.
- **Channel priority**: too many mirrored channels can cause solver slowdown; keep the list short.
- **`base` auto-activate**: disable with `conda config --set auto_activate_base false` if annoying.
- **Version spec syntax**: `PKGNAME==3.1.4` (exact) vs `PKGNAME=3.1` (search/build string) are different.
