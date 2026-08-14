# Cygwin (and MobaXterm) notes

## 1. What is it?

Cygwin is a large collection of GNU and open-source tools that provide a Linux-like environment and command-line interface on Windows. MobaXterm is a Windows terminal that bundles Cygwin plus an X11 server.

## 2. What is it for?

- Running Linux-style commands (`bash`, `grep`, `ssh`, `rsync`, ...) on a Windows machine.
- Lightweight package installs via `apt-cyg` (a community Cygwin package manager).

## 3. How to download / install

- Cygwin: https://www.cygwin.com/ — run `setup-x86_64.exe`.
- MobaXterm: https://mobaxterm.mobatek.net/ (portable edition available).
- `apt-cyg` (Cygwin package installer):
  ```bash
  # inside Cygwin
  wget https://raw.githubusercontent.com/transcode-open/apt-cyg/master/apt-cyg
  chmod +x apt-cyg
  mv apt-cyg /usr/local/bin/
  ```

## 4. How to use

```bash
apt-cyg install wget vim git     # install packages
echo $PATH                        # Cygwin PATH; note it differs from Windows PATH
```

## 5. Pitfalls

- **PATH differs from Windows**: Cygwin has its own `/bin`, `/usr/bin`; Windows `.exe` paths aren't on it by default.
- **`apt-cyg` is third-party** and occasionally unmaintained; prefer the official `setup-x86_64.exe` for critical packages.
- **Line endings**: files edited in Windows Notepad get CRLF; Cygwin tools expect LF — use `dos2unix`.
- **Symlinks** in Cygwin require special handling and don't behave like Linux symlinks on NTFS.
