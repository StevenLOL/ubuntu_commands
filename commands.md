# Terminal Commands Cheatsheet

> This is the main entry point. Full categorized notes live in `cheatsheet/`; tool-specific topics are in each subdirectory's `readme.md`.
> Press `Tab` for shell completion. Topic index: see [README.md](README.md).

## Categories

- [System](cheatsheet/system.md) — system info, disk, users, logs, Fail2ban, hostname, prompt
- [Files & Disk](cheatsheet/files-disk.md) — edit/copy/rename, split, ISO/USB/network mount, RamFS
- [Network & SSH](cheatsheet/network-ssh.md) — ssh, LAN scan, ab test, queue, open web page
- [Text Processing](cheatsheet/text-processing.md) — strings, search & sort, line batch, random sample
- [tmux / cron / docker](cheatsheet/tmux-cron-docker.md) — tmux, cron, startup, Docker examples
- [Dev Config](cheatsheet/dev-config.md) — .bashrc / /etc/profile / .theano.rc / pip.conf / tmux.conf

## Most-used

### navigation & browsing
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

### wordcount
```
wc /data/groundtrueE/text
```

### install update software
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

### list installed module package
```
lsmod | grep nouveau
sudo dpkg -L packagename  #find installed file location
```

### change repositories location with replace txt
```
/etc/apt/sources.list
sudo cat /etc/apt/sources.list | sed 's/sg.archive.ubuntu.com/download.nus.edu.sg\/mirror/'  > ./sources.list
sudo cat /etc/apt/sources.list | sed 's/archive.ubuntu.com/mirrors.aliyun.com\/mirror/'  > ./sources.list
sudo cp ./sources.list /etc/apt/sources.list
```

### build software
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

### play audio
```
play a.wav
sox suria102_262_1904.wav  -p trim 300 5 |play -p
sox a.mp3 -c 1 -r 16000  -b 16 a.wav  #change mp3 to wav, 16khz 16bits mono
```

### wait
```
sleep 60
```
