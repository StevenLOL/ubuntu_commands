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
- [git](git/readme.md) · [git/github](git/github.md) · [git/gitlab](git/gitlab.md)

### Container & Virtualization
- [docker](docker/readme.md) · [lxc](lxc/readme.md) · [vbox](vbox/README.md) (incl. [sharefolder](vbox/sharefolder.md)) · [windows](windows/wine.md) (Wine, [WSL2](windows/windows_wsl.md), [cygwin](windows/cygwin/readme.md), [MS Office](windows/installing_microsoft_office.md), [FreeOffice](windows/freeoffice.md)) · [remote-desktop](ssh/remote-desktop.md) · [RamDisk](RamDisk/README.md)

### Network & Security
- [ssh](ssh/ssh-no-pswd.md) · [ssh reverse tunnel](ssh/ssh_reverse_tunnel.md) · [vpn](vpn/openvpn.md) · [shadowsocks](vpn/shadowsocks.md) · [wireless](vpn/wireless.md)
- [network-security](network-security/) — [monitoring/wazuh](network-security/monitoring/wazuh/Wazuh%20Day%201.md) · [uniscan](network-security/uniscan/readme.md) · [reverse-engineering/Decompilation](network-security/reverse-engineering/Decompilation/README.md)

### System
- [apt_dpkg](apt_dpkg/readme.md) (incl. [sources.list mirror](apt_dpkg/sources_list.md)) · [release_update](release_update/readme.md) · [linux4OldComputers](linux4OldComputers/readme.md) · [swap](swap/add_swap_ubuntu_14.04.md) · [recovery](recovery/recovery.sh) · [tmux/tmate](ssh/tmux-tmate.md) · [threads](threads/readme.md) · [desktop](desktop/readme.md) (incl. [KDE Plasma](desktop/kde-plasma-desktop.md)) · [binwalk](binwalk/binwalk.md)

### Machine Learning / Speech / Vision
- [tensorflow](machine-learning/readme_tensorflow.md) · [tensorflow install](machine-learning/install_tensorflow.md) · [skflow](machine-learning/skflow.md) · [ml index](machine-learning/xgboost.md) · [ml/bob](machine-learning/bob.md) · [Alink](machine-learning/alink.md) · [shogun](machine-learning/shogun_install.sh) · [torch](machine-learning/torch.md) · [torch_lua](machine-learning/torch_lua.md) · [caffe](machine-learning/caffe.md) · [theano](machine-learning/theano.md) · [lightgbm](machine-learning/lightgbm.md) · [RAPIDSAI](machine-learning/rapidsai.md) · [opencv](machine-learning/opencv.md) · [kaldi](machine-learning/kaldi.md) · [huggingface](machine-learning/huggingface.md) · [crf++](machine-learning/crf++.md) · [alize](machine-learning/alize.md) · [sailalign](machine-learning/sailalign.md) · [hts](machine-learning/hts.sh) · [scipy](machine-learning/scipy.md)

### Big Data
- [spark](spark/readme.md) · [sge](sge/start.sge.sh) · [elasticip](ElasticIP/readme.md)
- Hadoop: source/config removed from repo (too large). Get it from the official site — https://hadoop.apache.org/releases.html

### Programming Languages
- [python](programming-languages/python/README.md) · [python/ipython](programming-languages/python/ipython.md) · [conda](programming-languages/conda/conda_readme.md) · [perl](programming-languages/perl/readme.md) · [java](programming-languages/java/install-java.md) · [mono](programming-languages/mono/readme.md) · [latex](programming-languages/latex/readme.md) · [makefile](programming-languages/makefile/readme.md) · [markdown](programming-languages/markdown/readme.md) · [LLM](programming-languages/LLM/readme.md) · [chinese](programming-languages/chinese/readme.md)

### Office & Media
- [format_convert](format_convert/audio_format_mp3_wav.md) · [pdf](format_convert/pdf.md) · [wine](windows/wine.md) · [wine/freeoffice](windows/freeoffice.md) · [sox](sox/install_sox.md) · [email](email/reinstall_thunder_bird.sh) · [evernote](evernote/readme.md) · [ultraedit](ultraedit/Config) · [youtube](youtube/readme.md) · [android](android/readme.md) · [android/adb](android/adb.md) · [google](google/googler.md)

### Misc
- [nvidia](nvidia/readme.md) · [sqlite](sqlite/readme.md) · [kaggle](kaggle/readme.md) · [natilus](natilus/readme.md) · [samsung](samsung/no-file-ubuntu.md) · [elan](elan/elan.md) · [eclipse](eclipse/installpackage.sh) · [unity3d](unity3d)

## Privacy

Placeholders such as `<USER>`, `<SERVER_IP>`, `<YOUR_HOME>`, `<YOUR_EMAIL>`, `<YOUR_GITHUB>`, `<YOUR_DOCKERHUB>`, `<HOSTNAME>`, `<NAS>`, `<PRIVATE_IP>` are redacted. The original private data (intranet IPs, emails, usernames, private paths) has been scrubbed.

## Maintenance

- To add a new topic, create a lowercase folder at the repo root (e.g. `lower_snake`) with a `readme.md` inside.
- General high-frequency commands live in `commands.md`; topic details go under `cheatsheet/`.
- Editor backup files (`*~`, `*.swp`, `*.bak`) are ignored by `.gitignore`; do not commit them.

Contact:

<YOUR_EMAIL>
