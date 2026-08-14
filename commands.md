# 终端命令速查 / Terminal Commands Cheatsheet

> 这是总入口。完整分类速查见 `cheatsheet/` 目录；各工具专题见对应子目录的 `readme.md`。
> 按 `tab` 获取命令补全建议：`command` ([code](https://github.com/<YOUR_GITHUB>/ubuntu_commands/tree/master/python)) completion。

## 分类索引 / Categories

- [系统 / System](cheatsheet/system.md) — 系统信息、磁盘、用户、日志、Fail2ban、机器名、提示符
- [文件与磁盘 / Files & Disk](cheatsheet/files-disk.md) — 编辑/复制/重命名、分割、ISO/USB/网络挂载、RamFS
- [网络与 SSH / Network & SSH](cheatsheet/network-ssh.md) — ssh、网内扫描、ab 测试、队列、打开网页
- [文本处理 / Text Processing](cheatsheet/text-processing.md) — 字符串、搜索排序、逐行批处理、随机抽取
- [终端复用/定时/容器 / tmux-cron-docker](cheatsheet/tmux-cron-docker.md) — tmux、cron、开机启动、Docker 示例
- [开发环境配置 / Dev Config](cheatsheet/dev-config.md) — .bashrc / /etc/profile / .theano.rc / pip.conf / tmux.conf

## 高频命令 / Most-used

### 导航与浏览 / navigation & browsing
```
cd ~/         #go to home directory of current account ~/=home
cd ..         #go to parent directory
cat somefile  #display file content
head somefile                     #display only the head part of a files
tail somefile                     #display .... tail
tail -n +2 somefile               #skip the first 2 lines
tail -f somefile                  #tail and follow, update upon file change
less somefile                     #display file by parts
ls -l data/groundtrue             #list file properties
ls -d */      # list all directory
ls -d *bin    # list all directories end with bin
tree -d       # list all directory
find ./path/ -type f -exec ls -dl \{\} \; | awk '{print $9}'   #list files recurisively
find . -exec ls -dl \{\} \; | awk '{print $3, $5, $9}'
find ./ -iname "train_lm.sh"
```

### 字数统计 / wordcount
```
wc /data/groundtrueE/text
```

### 安装与更新软件 / install update software
```
sudo apt-get update
sudo apt-get install xx
sudo apt-get install xx --reinstall
sudo apt-get remove somepackage
sudo apt-get purge somepackage
sudo apt-get -f install  #fix some missing package
sudo nano /etc/apt/sources.list            #chagne software source
sudo dpkg -i ./wps-office_9.1.0.4751~a15_i386.deb
sudo dpkg -P ./wps-office_9.1.0.4751~a15   #purge a installed deb
sudo apt-get dist-upgrade                  #force update
sudo do-release-upgrade                    #update from 12.04-12.10
sudo pip install some_python_package
sudo pip install -r your_file
sudo pip -U                                #install and update
sudo pip uninstall pycuda
sudo python setup.py install               #if there is a setup.py
```

### 列出已安装模块 / list installed module package
```
lsmod | grep nouveau
sudo dpkg -L packagename  #find installed file location
```

### 切换软件源 / change repositories location with replace txt
```
/etc/apt/sources.list
sudo cat /etc/apt/sources.list | sed 's/sg.archive.ubuntu.com/download.nus.edu.sg\/mirror/'  > ./sources.list
sudo cat /etc/apt/sources.list | sed 's/archive.ubuntu.com/mirrors.aliyun.com\/mirror/'  > ./sources.list
sudo cp ./sources.list /etc/apt/sources.list
```

### 编译软件 / build software
```
./configure
make -j 4
sudo make install
# or
mkdir build
cd build
cmake ..
make all
sudo make install
autoconf
```

### 播放音频 / play audio
```
play a.wav
sox suria102_262_1904.wav  -p trim 300 5 |play -p
sox a.mp3 -c 1 -r 16000  -b 16 a.wav  #change mp3 to wav, 16khz 16bits mono
```

### 等待 / wait
```
sleep 60
```
