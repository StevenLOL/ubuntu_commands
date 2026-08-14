# System / disk / user cheatsheet

A grab-bag of everyday Ubuntu command-line recipes: system info, disks, users, hostname, logs. Quick reference — copy the line you need.

## System info & processes
```
lsb_release -a    # show Ubuntu version
echo $PATH
which ls
ulimit -a                         # show system limits
sudo netstat -tupn                # network states
sudo lshw                         # list hardware
lspci
cat /proc/cpuinfo                 # get cpu info
top iftop iotop glances          sudo iftop -i eth1
ps aux
ps -aef
ps -auwe | grep <USER> | grep server | grep 28166 | grep PWD
pwdx #pid                         # get process working directory of #pid
w                                 # list users/pts and what they're doing
ps -t pts/1 | awk '/[0-9]/ {print $1}' | xargs sudo kill   # kill a user's pts
lsof -i :8080                     # list process bound to a port
sudo netstat -peanut
pkill process
kill process_id
pkill -u username                 # kill all processes of a user
free -m                           # memory usage
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches   # drop cache memory
stress -c 2 -t 10000              # stress test
```

## Disk
```
df -h                             # list disk sizes
du -hs                            # list folder size
ls -al /dev/disk/by-uuid/
sudo ntfsfix /dev/sda5            # fix NTFS disk error
dd if=/dev/sda of=/dev/sdd        # clone disk sda -> sdd (use sudo)
sudo mkdir /media/mountpoint
sudo lsblk -o NAME,FSTYPE,SIZE,MOUNTPOINT,LABEL   # list disks/labels
sudo mkfs.ext4 /dev/sdd           # format disk
sudo mount /dev/sdb1/ /media/mountpoint
# error: "you must specify the filesystem type" -> /dev/sdb/ is the device, /dev/sdb1/ is the partition
sudo chown -R <USER> /media/mountpoint
# auto-mount on boot:
# 1. sudo mkdir /media/DATA
# 2. find the disk with df
# 3. add a line to /etc/fstab
```

## Touchpad
```
xinput list
xinput set-prop 14 "Device Enabled" 0     # disable
xinput set-prop 14 "Device Enabled" 1     # enable
sudo shutdown -r now                       # reboot now
```

## System log & history
```
/var/log/auth.log
~/.bash_history
# save history as soon as a command is entered (in ~/.bashrc):
export PROMPT_COMMAND='history -a;history -c;history -r'
# append history inside tmux (in ~/.bashrc):
shopt -s histappend
shopt -s histreedit
shopt -s histverify
HISTCONTROL='ignoreboth'
PROMPT_COMMAND="history -a;history -c;history -r; $PROMPT_COMMAND"
dmesg | tail -30                    # kernel log
```

## Fail2ban
```
sudo apt install fail2ban
sudo cat /var/log/fail2ban.log
sudo cat /etc/fail2ban/jail.conf     # check jail rules
sudo service fail2ban restart
sudo fail2ban-client status ssh      # check status
sudo fail2ban-client set ssh unbanip <SERVER_IP>  # unban an IP
```

## Send a message to another user
```
who
write username tty
# Ctrl+D to stop
```

## Add a user account
```
sudo adduser <USER>
sudo adduser <USER> sudo
sudo useradd -d /home/testuser -m testuser
sudo passwd testuser
gksudo gnome-control-center user-accounts
sudo passwd  # change root password
sudo passwd -l root # disable root login
sudo passwd -S root # check if locked (L = locked, P = usable)
who    # check users
sudo usermod --expiredate 1 peter          # disallow peter from logging in
sudo usermod --expiredate "" peter         # set peter's expiration to Never
sudo passwd -l peter                        # take away peter's password
sudo passwd -u peter                        # restore peter's password
```

## Change machine name
```
sudo apt-get install winbind
# add 'wins' to the "hosts:" line in /etc/nsswitch.conf
sudo gedit /etc/hostname
sudo gedit /etc/hosts
sudo apt-get install avahi-daemon
```

## Disable mouse scroll
```
xinput list
xinput get-button-map 8
xinput set-button-map 8 1 2 3 0 0     # 4,5 = scroll up/down; save to ~/.profile with sudo
```

## Shorten the command prompt
```
# current terminal only:
PS1='\u:\W\$ '
# permanently (in ~/.bashrc): replace \w with uppercase \W and drop @\h, e.g.
PS1='${debian_chroot:+($debian_chroot)}\u:\W\$ '
```

## Others
```
sudo ufw disable
gksu gedit /etc/bumblebee/bumblebee.conf     # edit as root in GUI
sudo service lightdm stop                    # stop GUI

# loop over files in a dir
for ifile in <YOUR_HOME>/Dropbox/.../utts/*.wav
do
    show=`basename $ifile .wav`
    /usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
        --fInputMask=$ifile --sOutputMask=<YOUR_HOME>/.../$show.seg ZHIZHENGTEST &
done

if [ -d "/path/to/dir" ]; then echo "exists"; else echo "missing"; fi

for D in *; do echo $D; find $D -type f | wc -l; done   # file count per subfolder

# disable auto-logout
set LOCK_SCREEN=false
sudo nano /etc/default/acpi-support
```
