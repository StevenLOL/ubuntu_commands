# ubuntu_commands
Commonly used UBUNTU commands and scripts

[WIP]


#Commonly used UBUNTU commands

## navigation & browsing
      cd ~/         #go to home directory of current account ~/=home
      cd ..         #go to parent directory
      cat /data2/malay1307/s5/irun.sh   #display file content
      head somefile                     #display only the head part of a files
      tral somfile                     #display .... trail
      wordcount
      wc /data/groundtrueE/text
      list file properties
      ls -l data/groundtrue
      ls -d */   # list all directory
      ls -d *bin # list all directories end with bin
      tree -d    # list all directory

##system vars/info
      echo $PATH
      which ls
      #list hardware
      sudo lshw
      lspci
      #get cpu info
      cat /proc/cpuinfo
      top iftop iotop glances     sudo iftop -i eth1
      ps aux
      pkill process_id
      #kill all process of a user
      pkill -u username
      #memory usage
      free -m
      #disk usage
      df -h  #list harddisk size
      du -hs #list folder size
      ls -al /dev/disk/by-uuid/
      sudo mkdir /media/mountpoint
      #list show disks labels
      sudo lsblk -o NAME,FSTYPE,SIZE,MOUNTPOINT,LABEL
      #format disk
      sudo mkfs.ext4 /dev/sdd
      #mount 
      sudo mount /dev/sdb1/ /media/mountpoint
      #related error: mount: you must specify the filesystem type if mout /dev/sdb/ ...
      #/dev/sdb/ is the device  /dev/sdb1/ is the voloum of that disk, so one must volume voloum but not a disk
      sudo chown -R steven /media/mountpoint
      for D in *; do echo $D; find $D -type f| wc -l; done    #get files count in sub folder
      ##touchpad
      xinput list
			xinput set-prop 14 "Device Enabled" 0
			xinput set-prop 14 "Device Enabled" 1			
			sudo shutdown -r now # reboot now

##editing,create,copy modify files directories
      mkdir somedir
      #delete directory 
      #rm -rf *
      rm -rf ./train_clean/*8k
      nano filename
      echo somthing > newfile
      rm somefile                     #del a file
      sudo chmod 777 ./dir -R
      sudo chmod 777 ./somescript.sh
      #change ownership of a folder and sub
      sudo chown -hR root /u
      #Change the owner of /u and subfiles to "root".
      #copy folder wiht sub folders
      cp -r /your/fodler/* target/
      cp -avr allsour totarget
      #remote copy
      rcp
      scp or
      scp -r catbert@192.168.1.103:/home/catbert/evil_plans/  ./
      http://www.comentum.com/rsync.html
      rsync -r localfolder -v -e ssh steven@155.69.149.184
      sudo ln -s /usr/local/nginx/conf/ /etc/nginx   #link a folder(create a shortcut) target should not exists first
      ln -s /data2/malay1307/s5/ /home/sjd601/training/linkedMalay1307
##add user
    sudo useradd -d /home/testuser -m testuser
    sudo passwd testuser
    gksudo gnome-control-center user-accounts 
    sudo adduser steven
    sudo adduser steven sudo
    sudo usermod -g root username
##Rename files
      rename s/"sd_under_classter"/"change to wat?"/g *
      /g = all matched
      *=all files
      eg rename s/"_16k.wav"/".wav"/g *
      eg rename s/"Suria"/"suria"/g *
      #all to lower case
      find . -depth -exec rename 's/(.*)\/([^\/]*)/$1\/\L$2/' {} \;
##String operation
			#substring
			stringZ=abcABC123ABCabc
			echo ${stringZ:7}                            # 23ABCabc
			echo ${stringZ:7:3}                          # 23A
                                             # Three characters of substring
                        #display first column of something
                        cat feats.filtered.ark | awk '{print $1}'    #$0 is the whole line
                        cat feats.filtered.ark | awk '{print $1,$1}'

##Firefox flash plugin
	sudo apt-get install flashplugin-nonfree

#Folders in this project

There are many folders here, each contains guide to install or use that package.













#The best way to learn is to get a book : [Advanced Bash-Scripting Guide](./abs-guide-new.pdf)
by Mendel Cooper
