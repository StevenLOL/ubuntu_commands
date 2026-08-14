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
- [git](git/readme.md) · [git/github-hosts](git/github-hosts.md) · [git/gitlab-ce](git/gitlab-ce.md)

### Container & Virtualization
- [docker](docker/readme.md) · [lxc](lxc/readme.md) · [vbox](vbox/README.md) (incl. [sharefolder](vbox/sharefolder.md)) · [windows](windows/wine.md) (Wine, [WSL2](windows/windows_wsl.md), [cygwin](windows/cygwin/readme.md), [MS Office](windows/installing_microsoft_office.md), [FreeOffice](windows/freeoffice.md)) · [remote-desktop](ssh/remote-desktop.md) · [RamDisk](system/RamDisk/README.md)

### Security (信息安全)
- [ssh](ssh/ssh-no-pswd.md) · [ssh reverse tunnel](ssh/ssh_reverse_tunnel.md) · [vpn](vpn/openvpn.md) · [shadowsocks](vpn/shadowsocks.md) · [wireless](vpn/wireless.md)
- [security](security/) — [monitoring/wazuh](security/monitoring/wazuh/Wazuh Day 1.md) · [uniscan](security/uniscan/readme.md) · [reverse-engineering/Decompilation](security/reverse-engineering/Decompilation/apktool/) · [binwalk](security/binwalk/binwalk.md)

### System (系统组件 / 桌面 / 输入法)
- [apt_dpkg](system/apt_dpkg/readme.md) (incl. [sources.list mirror](system/apt_dpkg/sources_list.md)) · [release_update](system/release_update/readme.md) · [linux4OldComputers](system/linux4OldComputers/readme.md) · [swap](system/swap/add_swap_ubuntu_14.04.md) · [recovery](system/recovery/recovery.sh) · [threads](system/threads/readme.md) · [desktop](system/desktop/readme.md) (incl. [KDE Plasma](system/desktop/kde-plasma-desktop.md)) · [natilus](system/natilus/readme.md) · [input-method](system/input-method/chinese.md)

### System
- [tmux/tmate](ssh/tmux-tmate.md)

### Machine Learning / Speech / Vision
- [tensorflow](machine-learning/readme_tensorflow.md) · [tensorflow install](machine-learning/install_tensorflow.md) · [skflow](machine-learning/skflow.md) · [ml index](machine-learning/xgboost.md) · [ml/bob](machine-learning/bob.md) · [Alink](machine-learning/alink.md) · [shogun](machine-learning/shogun_install.sh) · [torch](machine-learning/torch.md) · [torch_lua](machine-learning/torch_lua.md) · [caffe](machine-learning/caffe.md) · [theano](machine-learning/theano.md) · [lightgbm](machine-learning/lightgbm.md) · [RAPIDSAI](machine-learning/rapidsai.md) · [opencv](machine-learning/opencv.md) · [kaldi](machine-learning/kaldi.md) · [huggingface](machine-learning/huggingface.md) · [crf++](machine-learning/crf++.md) · [alize](machine-learning/alize.md) · [sailalign](machine-learning/sailalign.md) · [hts](machine-learning/hts.sh) · [scipy](machine-learning/scipy.md)

### Big Data
- [spark](spark/readme.md) · [sge](sge/start.sge.sh) · [elasticip](ElasticIP/readme.md)
- Hadoop: source/config removed from repo (too large). Get it from the official site — https://hadoop.apache.org/releases.html

### Software Development
- [python](software-dev/python/README.md) · [python/ipython](software-dev/python/ipython.md) · [conda](software-dev/conda/conda_readme.md) · [perl](software-dev/perl/readme.md) · [java](software-dev/java/install-java.md) · [mono](software-dev/mono/readme.md) · [latex](software-dev/latex/readme.md) · [makefile](software-dev/makefile/readme.md) · [markdown](software-dev/markdown/readme.md) · [LLM](software-dev/LLM/ollama/readme.md) · [android](software-dev/android/readme.md) · [android/adb](software-dev/android/adb.md) · [eclipse](software-dev/eclipse/installpackage.sh)

### Office & Media
- [format_convert](format_convert/) · [pdf](format_convert/pdf.md) · [wine](windows/wine.md) · [wine/freeoffice](windows/freeoffice.md) · [email](email/reinstall_thunder_bird.sh) · [evernote](evernote/readme.md) · [ultraedit](ultraedit/Config) · [youtube](youtube/readme.md) · [google](google/googler.md)

### Audio (音频处理)
- [ffmpeg](audio/ffmpeg.md) · [sox](audio/sox/install_sox.md) · [elan](audio/elan/elan.md)

### Misc
- [nvidia](nvidia/readme.md) · [sqlite](sqlite/readme.md) · [kaggle](kaggle/readme.md) · [samsung](samsung/no-file-ubuntu.md) · [unity3d](unity3d)

## Privacy

Placeholders such as `<USER>`, `<SERVER_IP>`, `<YOUR_HOME>`, `<YOUR_EMAIL>`, `<YOUR_GITHUB>`, `<YOUR_DOCKERHUB>`, `<HOSTNAME>`, `<NAS>`, `<PRIVATE_IP>` are redacted. The original private data (intranet IPs, emails, usernames, private paths) has been scrubbed.

## Maintenance

- To add a new topic, create a lowercase folder at the repo root (e.g. `lower_snake`) with a `readme.md` inside.
- General high-frequency commands live in `commands.md`; topic details go under `cheatsheet/`.
- Editor backup files (`*~`, `*.swp`, `*.bak`) are ignored by `.gitignore`; do not commit them.

Contact:

<YOUR_EMAIL>
