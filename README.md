# Ubuntu Commands

个人 Linux / Ubuntu 命令与工具速查笔记。按主题组织，每个子目录含一个 `readme.md`。

## 速查入口 / Quick Start

- [终端命令总入口 commands.md](commands.md) — 高频命令 + 分类索引
- [cheatsheet/](cheatsheet/) — 系统 / 文件磁盘 / 网络SSH / 文本处理 / tmux-cron-docker / 开发配置

## 分类目录 / Categories

### 版本控制 / Version Control
- [git](git/readme.md) · [github](github/readme.md) · [gitlab](gitlab/readme.md)

### 容器 / 虚拟化 / Container & Virtualization
- [docker](docker/readme.md) · [lxc](lxc/) · [vbox](vbox/README.md) · [wsl](wsl/windows_wsl.md) · [cygwin](cygwin/readme.md) · [windows](windows/cygwin/readme.md) · [remote-desktop](remote-desktop/readme.md) · [sharefolder](sharefolder/) · [RamDisk](RamDisk/README.md)

### 网络 / 安全 / Network & Security
- [ssh](ssh/ssh-no-pswd.md) · [ssh 反向隧道](ssh/ssh_reverse_tunnel.md) · [vpn](vpn/) · [shadowsocks](shadowsocks/readme.md) · [wireless](wireless/readme.md) · [wazuh](wazuh/) · [kali](kali/uniscan.md) · [uniscan](uniscan/readme.md) · [Decompilation](Decompilation/)

### 系统管理 / System
- [apt_dpkg](apt_dpkg/readme.md) · [APT](APT/readme.md) · [release_update](release_update/readme.md) · [linux4OldComputers](linux4OldComputers/readme.md) · [swap](swap/) · [recovery](recovery/) · [tmux_tmate](tmux_tmate/readme.md) · [threads](threads/readme.md) · [desktop](desktop/readme.md) · [desktop-kde-plasma](desktop-kde-plasma/) · [markdown](markdown/readme.md) · [makefile](makefile/) · [binwalk](binwalk/)

### 机器学习 / 语音 / 视觉 / ML / Speech / Vision
- [tensorflow](tensorflow/readme.md) · [google_tensorflow](google_tensorflow/readme.md) · [torch](torch/readme.md) · [torch_lua](torch_lua/README.md) · [caffe](caffe/readme.md) · [keras](ml/) · [theano](theano/readme.md) · [opencv](opencv/readme.md) · [scipy](scipy/) · [kaldi](kaldi/kaldi.md) · [Alink](Alink/readme.md) · [lightGBM](lightGBM/readme.md) · [RAPIDSAI](RAPIDSAI/readme.md) · [crf++](crf++/) · [huggingface](huggingface/readme.md) · [LLM/ollama](LLM/ollama/readme.md) · [shogun](shogun/) · [alize](alize/readme.md) · [sailalign](sailalign/) · [hts](hts/hts.sh)

### 大数据 / Big Data
- [spark](spark/readme.md) · [sge](sge/) · [elasticip](ElasticIP/readme.md)
- Hadoop：源码/配置已从仓库移除（体积过大），请从官方获取 — https://hadoop.apache.org/releases.html

### 编程语言 / Languages
- [python](python/README.md) · [python3.6](python/python3.6/readme.md) · [pip](python/pip/README.md) · [virtualenv](python/virtualenv/readme.md) · [jupyter_lab](python/jupyter_lab/readme.md) · [ipython_notebook](python/ipython_notebook/readme.md) · [pyqt5](python/pyqt5/readme.md) · [VisualStudioCode](python/VisualStudioCode/readme.md) · [conda](conda/) · [perl](perl/readme.md) · [java](java/install-java.md) · [mono](mono/readme.md) · [latex](latex/readme.md)

### 办公 / 多媒体 / Office & Media
- [pdf](pdf/readme.md) · [office_wine](office_wine/) · [wine](wine/usage.sh) · [sox](sox/) · [email](email/) · [evernote](evernote/) · [format_convert](format_convert/) · [ultraedit](ultraedit/) · [youtube](youtube/readme.md) · [android](android/readme.md) · [adb](adb/readme.md) · [google](google/) · [chrome](google/)

### 其他 / Misc
- [nvidia](nvidia/readme.md) · [sqlite](sqlite/readme.md) · [kaggle](kaggle/readme.md) · [chinese](chinese/readme.md) · [natilus](natilus/readme.md) · [samsung](samsung/) · [elan](elan/) · [eclipse](eclipse/installpackage.sh) · [unity3d](unity3d/)

## 隐私说明 / Privacy

文档中的 `<USER>`、`<SERVER_IP>`、`<YOUR_HOME>`、`<YOUR_EMAIL>`、`<YOUR_GITHUB>`、`<YOUR_DOCKERHUB>`、`<HOSTNAME>`、`<NAS>`、`<PRIVATE_IP>` 等为占位符，原私有信息（内网 IP、邮箱、用户名、私有路径）已脱敏。

## 维护 / Maintenance

- 新主题请在根目录新建小写文件夹（如 `lower_snake`），内放 `readme.md`。
- 通用高频命令维护在 `commands.md`，分类细节放到 `cheatsheet/`。
- 编辑器备份文件（`*~`、`*.swp`）已被 `.gitignore` 忽略，请勿提交。

Contact:

<YOUR_EMAIL>
