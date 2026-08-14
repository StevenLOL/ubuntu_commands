# Ubuntu Commands

Personal Linux / Ubuntu command and tool cheat-sheet notes. Organized by topic; each subdirectory contains a `readme.md` (or an equivalent index file).

## Quick Start

- [commands.md](commands.md) — most-used commands + category index
- [cheatsheet/](cheatsheet/) — system / files & disk / network & SSH / text processing / tmux-cron-docker / dev config
- [rufus](rufus/readme.md) — make a bootable Linux install USB (the first step before installing Linux, on Windows)

## Categories

### Install Media / Bootable USB
- [rufus](rufus/readme.md) — flash a Linux `.iso` onto a USB stick (Windows; portable, no install). On Linux you need no extra tool — `dd` / `gnome-disks` already do this.

### Version Control
- [git](git/readme.md) · [github](github/readme.md) · [gitlab](gitlab/readme.md)

### Container & Virtualization
- [docker](docker/readme.md) · [lxc](lxc/readme.md) · [vbox](vbox/README.md) · [wsl](wsl/windows_wsl.md) · [cygwin](cygwin/readme.md) · [remote-desktop](remote-desktop/readme.md) · [sharefolder](sharefolder/sharefolder.md) · [RamDisk](RamDisk/README.md)

### Network & Security
- [ssh](ssh/ssh-no-pswd.md) · [ssh reverse tunnel](ssh/ssh_reverse_tunnel.md) · [vpn](vpn/openvpn.md) · [shadowsocks](vpn/shadowsocks.md) · [wireless](vpn/wireless.md) · [wazuh](wazuh/Wazuh%20Day%201.md) · [uniscan](uniscan/readme.md)

### System
- [apt_dpkg](apt_dpkg/readme.md) · [APT](APT/readme.md) · [release_update](release_update/readme.md) · [linux4OldComputers](linux4OldComputers/readme.md) · [swap](swap/add_swap_ubuntu_14.04.md) · [recovery](recovery/recovery.sh) · [tmux_tmate](tmux_tmate/readme.md) · [threads](threads/readme.md) · [desktop](desktop/readme.md) (incl. [KDE Plasma](desktop/kde-plasma-desktop.md)) · [markdown](markdown/readme.md) · [makefile](makefile/readme.md) · [binwalk](binwalk/binwalk.md)

### Machine Learning / Speech / Vision
- [tensorflow](tensorflow/readme.md) · [tensorflow install](tensorflow/install_tensorflow.md) · [skflow](tensorflow/skflow.md) · [ml index](ml/xgboost.md) · [ml/bob](ml/bob.md) · [Alink](ml/alink.md) · [shogun](ml/shogun_install.sh) · [torch](ml/torch.md) · [torch_lua](ml/torch_lua.md) · [caffe](ml/caffe.md) · [theano](ml/theano.md) · [lightgbm](ml/lightgbm.md) · [RAPIDSAI](ml/rapidsai.md) · [opencv](ml/opencv.md) · [kaldi](ml/kaldi.md) · [huggingface](ml/huggingface.md) · [crf++](ml/crf++.md) · [alize](ml/alize.md) · [sailalign](ml/sailalign.md) · [hts](ml/hts.sh) · [scipy](ml/scipy.md)

### Big Data
- [spark](spark/readme.md) · [sge](sge/start.sge.sh) · [elasticip](ElasticIP/readme.md)
- Hadoop: source/config removed from repo (too large). Get it from the official site — https://hadoop.apache.org/releases.html

### Languages
- [python](python/README.md) · [python/ipython](python/ipython.md) · [conda](conda/conda_readme.md) · [perl](perl/readme.md) · [java](java/install-java.md) · [mono](mono/readme.md) · [latex](latex/readme.md)

### Office & Media
- [format_convert](format_convert/audio_format_mp3_wav.md) · [pdf](format_convert/pdf.md) · [wine](wine/usage.sh) · [wine/freeoffice](wine/freeoffice.md) · [sox](sox/install_sox.md) · [email](email/reinstall_thunder_bird.sh) · [evernote](evernote/readme.md) · [ultraedit](ultraedit/Config) · [youtube](youtube/readme.md) · [android](android/readme.md) · [android/adb](android/adb.md) · [google](google/googler.md)

### Misc
- [nvidia](nvidia/readme.md) · [sqlite](sqlite/readme.md) · [kaggle](kaggle/readme.md) · [chinese](chinese/readme.md) · [natilus](natilus/readme.md) · [samsung](samsung/no-file-ubuntu.md) · [elan](elan/elan.md) · [eclipse](eclipse/installpackage.sh) · [unity3d](unity3d)

## Privacy

Placeholders such as `<USER>`, `<SERVER_IP>`, `<YOUR_HOME>`, `<YOUR_EMAIL>`, `<YOUR_GITHUB>`, `<YOUR_DOCKERHUB>`, `<HOSTNAME>`, `<NAS>`, `<PRIVATE_IP>` are redacted. The original private data (intranet IPs, emails, usernames, private paths) has been scrubbed.

## Maintenance

- To add a new topic, create a lowercase folder at the repo root (e.g. `lower_snake`) with a `readme.md` inside.
- General high-frequency commands live in `commands.md`; topic details go under `cheatsheet/`.
- Editor backup files (`*~`, `*.swp`, `*.bak`) are ignored by `.gitignore`; do not commit them.
- **No-download rule:** many tasks in this repo use tools that are already on the system (`tar`, `ssh`, `git`, `curl`, `dd`, `fallocate`, `gnome-disks`, …). Install a package or app only when it is genuinely missing — do not assume you must download something first. Each note states whether a download is actually needed.

Contact:

<YOUR_EMAIL>
