#http://www.linuxscrew.com/2010/03/24/fastest-way-to-create-ramdisk-in-ubuntulinux/
sudo -s
#how many free memory left?


mkdir /tmp/ramdisk; chmod 777 /tmp/ramdisk
mount -t tmpfs -o size=8192M tmpfs /tmp/ramdisk/
#or 
mount -t ramfs -o size=8192M ramfs /tmp/ramdisk/
#what has been mounted? 
dh -h
#how to use it by move files to /tmp/ramdisk folder
free -m
#free -m will shows an increasing use of memory




umount ramfs -f      
umount tmpfs -f


