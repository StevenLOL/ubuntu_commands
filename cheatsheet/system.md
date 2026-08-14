# 系统 / System

## 系统信息 / system vars & info
```
lsb_release -a    #show ubuntu version
echo $PATH
which ls
ulimit -a                         #show system limits
sudo netstat -tupn                #network states
sudo lshw                         #list hardware
lspci
cat /proc/cpuinfo                 #get cpu info
top iftop iotop glances          sudo iftop -i eth1
ps aux
ps -aef
ps -auwe | grep <USER> | grep server | grep 28166 | grep PWD
pwdx #pid                         # get process working directoy of #pid
w                                 #list users and pts and what they are doing
ps -t pts/1 | awk '/[0-9]/ {print $1}' | xargs sudo kill   #kill a user's pts
lsof -i :8080                     #list binded port
sudo netstat -peanut
pkill process
kill process_id
pkill -u username                 #kill all process of a user
free -m                           #memory usage
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches   #drop cache memory
stress -c 2 -t 10000              #stress test
```

## 磁盘 / disk
```
df -h                             #list harddisk size
du -hs                            #list folder size
ls -al /dev/disk/by-uuid/
sudo ntfsfix /dev/sda5            #fixed NTFS disk error
dd if=/dev/sda of=/dev/sdd        #clone harddisk sda to sdd, you must use sudo
sudo mkdir /media/mountpoint
sudo lsblk -o NAME,FSTYPE,SIZE,MOUNTPOINT,LABEL   #list show disks labels
sudo mkfs.ext4 /dev/sdd           #format disk
sudo mount /dev/sdb1/ /media/mountpoint
# related error: mount: you must specify the filesystem type if mout /dev/sdb/ ...
# /dev/sdb/ is the device  /dev/sdb1/ is the voloum of that disk
sudo chown -R <USER> /media/mountpoint
#auto mount on startup
1. create a mount folder
sudo mkdir /media/DATA
2. find the disk you want to mount via df
3. change the /etc/fstab file
```

## 触摸板 / touchpad
```
xinput list
xinput set-prop 14 "Device Enabled" 0
xinput set-prop 14 "Device Enabled" 1
sudo shutdown -r now # reboot now
```

## 系统日志 / 历史 / system log & history
```
/var/log/auth.log
~/.bash_history
#set history size in ~/.bashrc
#save history one command is entered:
export PROMPT_COMMAND='history -a;history -c;history -r'
#history append in tmux  (in ~/.bashrc)
shopt -s histappend
shopt -s histreedit
shopt -s histverify
HISTCONTROL='ignoreboth'
PROMPT_COMMAND="history -a;history -c;history -r; $PROMPT_COMMAND"
# 查看内核日志
dmesg | tail -30
```

## Fail2ban
```
sudo apt install fail2ban
sudo cat /var/log/fail2ban.log
sudo cat /etc/fail2ban/jail.conf     #check jial rules
sudo service fail2ban restart
sudo fail2ban-client status ssh      #check status
sudo fail2ban-client set ssh unbanip <SERVER_IP>  #unban ip address
```

## 给用户发消息 / Send message to other users
```
who
write username tty
#control+d  to stop
```

## 新增用户 / add user account
```
sudo adduser <USER>
sudo adduser <USER> sudo
sudo useradd -d /home/testuser -m testuser
sudo passwd testuser
gksudo gnome-control-center user-accounts
sudo passwd  #change passwd of root user
sudo passwd -l root #To disable the root login
sudo passwd -S root #check if locked or not (will have L in output instead of P)
who    #check users
# Expire Account
sudo usermod --expiredate 1 peter          # disallow peter from logging in
sudo usermod --expiredate "" peter         # set expiration date of peter to Never
sudo passwd -l peter                        # take away peters password
sudo passwd -u peter                        # give peter back his password
```

## 修改机器名 / change machine name
```
sudo apt-get install winbind
# You probably want to add 'wins' where it says "hosts:" in /etc/nsswitch.conf
sudo gedit /etc/hostname
sudo gedit /etc/hosts
sudo apt-get install avahi-daemon
```

## 禁用鼠标滚轮 / disable mouse scroll
```
xinput list
xinput get-button-map 8
xinput set-button-map 8 1 2 3 0 0
# 4,5 means scoring up/download ; and sudo save to ~/.profile
```

## 缩短命令行提示符 / shorten command prompt
```
# To change it for the current terminal instance only:
PS1='\u:\W\$ '

# To change it "permanently" (in ~/.bashrc):
# find the section:
if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
# Remove the @\h, and replace the \w with an uppercase \W:
if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u\[\033[00m\]:\[\033[01;34m\]\W\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u:\W\$ '
fi
```

## 其它 / OTHERS
```
sudo ufw disable
gksu gedit /etc/bumblebee/bumblebee.conf     #edit in sudo in gui mode
sudo service lightdm stop                    #stop gui

# loop dir
for ifile in <YOUR_HOME>/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts/*.wav
do
    show=`basename $ifile .wav`
    echo $show
    /usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
        --fInputMask=$ifile --sOutputMask=<YOUR_HOME>/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts_gender/$show.seg ZHIZHENGTEST &
done

if [ -d "/path/to/dir" ]
then
    echo "Directory /path/to/dir exists."
else
    echo "Error: Directory /path/to/dir does not exists."
fi

for D in *; do echo $D; find $D -type f| wc -l; done   #get files count in sub folder

#disable auto logout
set LOCK_SCREEN=false
sudo nano /etc/default/acpi-support
```
