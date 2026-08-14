# Ubuntu Commands

个人 Linux / Ubuntu 命令与工具速查笔记。按主题组织，每个子目录含一个 `readme.md`（或等价索引文件）。

## 速查入口 / Quick Start

- [终端命令总入口 commands.md](commands.md) — 高频命令 + 分类索引
- [cheatsheet/](cheatsheet/) — 系统 / 文件磁盘 / 网络SSH / 文本处理 / tmux-cron-docker / 开发配置

## 分类目录 / Categories

### 版本控制 / Version Control
- [git](git/readme.md) · [github](github/readme.md) · [gitlab](gitlab/readme.md)

### 容器 / 虚拟化 / Container & Virtualization
- [docker](docker/readme.md) · [lxc](lxc/readme.txt) · [vbox](vbox/README.md) · [wsl](wsl/windows_wsl.md) · [cygwin](cygwin/readme.md) · [remote-desktop](remote-desktop/readme.md) · [sharefolder](sharefolder/sharefolder.txt) · [RamDisk](RamDisk/README.md)

### 网络 / 安全 / Network & Security
- [ssh](ssh/ssh-no-pswd.md) · [ssh 反向隧道](ssh/ssh_reverse_tunnel.md) · [vpn](vpn/openvpn.md) · [shadowsocks](shadowsocks/readme.md) · [wireless](wireless/readme.md) · [wazuh](wazuh/Wazuh%20Day%201.md) · [uniscan](uniscan/readme.md) · [Decompilation](Decompilation/) *(TODO)*

### 系统管理 / System
- [apt_dpkg](apt_dpkg/readme.md) · [APT](APT/readme.md) · [release_update](release_update/readme.md) · [linux4OldComputers](linux4OldComputers/readme.md) · [swap](swap/add_swap_ubuntu_14.04.txt) · [recovery](recovery/recovery.sh) · [tmux_tmate](tmux_tmate/readme.md) · [threads](threads/readme.md) · [desktop](desktop/readme.md) · [desktop-kde-plasma](desktop/kde-plasma-desktop.txt) · [markdown](markdown/readme.md) · [makefile](makefile/readme.txt) · [binwalk](binwalk/binwalk.md)

### 机器学习 / 语音 / 视觉 / ML / Speech / Vision
- [tensorflow](tensorflow/readme.md) · [tensorflow 安装](tensorflow/install_tensorflow.md) · [skflow](tensorflow/skflow.md) · [torch](torch/readme.md) · [torch_lua](torch_lua/README.md) · [caffe](caffe/readme.md) · [ml/xgboost](ml/xgboost.md) · [ml/bob](ml/bob.txt) · [Alink](ml/alink.md) · [shogun](ml/shogun_install.sh) · [theano](theano/readme.md) · [opencv](opencv/readme.md) · [scipy](scipy/readme.txt) · [kaldi](kaldi/kaldi.md) · [lightGBM](lightGBM/readme.md) · [RAPIDSAI](RAPIDSAI/readme.md) · [crf++](crf++/readme.txt) · [huggingface](huggingface/readme.md) · [LLM/ollama](LLM/) *(TODO)* · [alize](alize/readme.md) · [sailalign](sailalign/sailalign.txt) · [hts](hts/hts.sh)

### 大数据 / Big Data
- [spark](spark/readme.md) · [sge](sge/start.sge.sh) · [elasticip](ElasticIP/readme.md)
- Hadoop：源码/配置已从仓库移除（体积过大），请从官方获取 — https://hadoop.apache.org/releases.html

### 编程语言 / Languages
- [python](python/README.md) · [python/ipython](python/ipython.md) · [conda](conda/conda_readme.md) · [perl](perl/readme.md) · [java](java/install-java.md) · [mono](mono/readme.md) · [latex](latex/readme.md)

### 办公 / 多媒体 / Office & Media
- [pdf](pdf/readme.md) · [office_wine](office_wine/freeoffice.md) · [wine](wine/usage.sh) · [sox](sox/install_sox.md) · [email](email/reinstall_thunder_bird.sh) · [evernote](evernote/readme.txt) · [format_convert](format_convert/audio_format_mp3_wav.md) · [ultraedit](ultraedit/Config) · [youtube](youtube/readme.md) · [android](android/readme.md) · [adb](adb/readme.md) · [google](google/googler.md)

### 其他 / Misc
- [nvidia](nvidia/readme.md) · [sqlite](sqlite/readme.md) · [kaggle](kaggle/readme.md) · [chinese](chinese/readme.md) · [natilus](natilus/readme.md) · [samsung](samsung/no-file-ubuntu.txt) · [elan](elan/elan.txt) · [eclipse](eclipse/installpackage.sh) · [unity3d](unity3d)

## 隐私说明 / Privacy

文档中的 `<USER>`、`<SERVER_IP>`、`<YOUR_HOME>`、`<YOUR_EMAIL>`、`<YOUR_GITHUB>`、`<YOUR_DOCKERHUB>`、`<HOSTNAME>`、`<NAS>`、`<PRIVATE_IP>` 等为占位符，原私有信息（内网 IP、邮箱、用户名、私有路径）已脱敏。

## 维护 / Maintenance

- 新主题请在根目录新建小写文件夹（如 `lower_snake`），内放 `readme.md`。
- 通用高频命令维护在 `commands.md`，分类细节放到 `cheatsheet/`。
- 编辑器备份文件（`*~`、`*.swp`、`*.bak`）已被 `.gitignore` 忽略，请勿提交。

Contact:

<YOUR_EMAIL>
