# Cygwin (and MobaXterm) notes

## 1. What is it / What is it for?

Cygwin is a large collection of GNU and open-source tools that provide a Linux-like environment and command-line interface on Windows. MobaXterm is a Windows terminal that bundles Cygwin plus an X11 server.


- Running Linux-style commands (`bash`, `grep`, `ssh`, `rsync`, ...) on a Windows machine.
- Lightweight package installs via `apt-cyg` (a community Cygwin package manager).

## 2. How to download / install

- Cygwin: https://www.cygwin.com/ — run `setup-x86_64.exe`.
- MobaXterm: https://mobaxterm.mobatek.net/ (portable edition available).
- `apt-cyg` (Cygwin package installer):
  ```bash
  # inside Cygwin
  wget https://raw.githubusercontent.com/transcode-open/apt-cyg/master/apt-cyg
  chmod +x apt-cyg
  mv apt-cyg /usr/local/bin/
  ```

## 3. How to use

```bash
apt-cyg install wget vim git     # install packages
echo $PATH                        # Cygwin PATH; note it differs from Windows PATH
```

