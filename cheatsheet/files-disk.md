# Files & Disk cheatsheet

Quick reference for file/dir operations, rename, split, mount (ISO/USB/network/RAM), encoding conversion.

## Create / edit / copy / modify files & dirs
```
mkdir somedir
# delete a directory (dangerous): rm -rf *
rm -rf ./train_clean/*8k
nano filename
echo something > newfile
rm somefile                     # delete a file
tar -czvf yourzip.tar.gz directory_to_zip/
sudo chmod 777 ./dir -R
sudo chmod 777 ./somescript.sh
sudo chown -hR root /u          # change owner of /u and subfiles to "root"
cp -r /your/folder/* target/
cp -avr allsrc totarget
# remote copy:
rcp
scp -r <HOSTNAME>@<PRIVATE_IP>:/home/<HOSTNAME>/evil_plans/  ./
# http://www.comentum.com/rsync.html
rsync -r localfolder -v -e ssh <USER>@<SERVER_IP>
sudo ln -s /usr/local/nginx/conf/ /etc/nginx   # symlink a folder (target must NOT exist first)
ln -s /data2/malay1307/s5/ <YOUR_HOME>/training/linkedMalay1307
```

## Rename
```
rename s/"sd_under_classter"/"change to what?"/g *
# /g = all matches, * = all files
rename s/"_16k.wav"/".wav"/g *
rename s/"Suria"/"suria"/g *
# all filenames to lowercase:
find . -depth -exec rename 's/(.*)\/([^\/]*)/$1\/\L$2/' {} \;
```

## Split a file
```
split -l 500 a.list            # by lines
split -d -l 500 a.list
split -d -n l/3 a.list         # 3 parts without splitting lines
split -d -n l/3 a.list myprefix
```

## Image resize
```
for f in ./s5/*.jpg ;do echo $f[1024x] ./resized/$f; done;
```

## Text encoding change
```
opencc -i ~/Downloads/data/wiki_corpus/corpus_120520.txt -o ./120520.txt -c zht2zhs.ini
```

## Mount ISO
```
sudo mount -o loop xxx.iso /media/xxxx
```

## Mount USB drive
```
lsusb                            # list USB devices
sudo fdisk -l
sudo mkdir /media/external
sudo mount /dev/sdb1 /media/external
sudo mount -t ntfs-3g /dev/sdb1 /media/usb3tb
sudo mount -t vfat /dev/sdb1 /media/external -o uid=1000,gid=1000,utf8,dmask=027,fmask=137
```

## Mount network drive
```
sftp://<USER>@<SERVER_IP>
gvfs-mount sftp://<USER>@<SERVER_IP>     # mount via terminal or file manager
smb://databackup@<NAS>/databackup
smb://workspace@<SERVER_IP>/workspace
ftp://cclahadmin@www.cclah.com
```

## Mount a folder into RAM
```
mkdir -p /mnt/tmp
mount -t tmpfs -o size=20m tmpfs /mnt/tmp
mkdir -p /mnt/ram
mount -t ramfs -o size=20m ramfs /mnt/ram
```

## Change desktop pictures download path
```
# http://www.howtogeek.com/howto/17752/use-any-folder-for-your-ubuntu-desktop-even-a-dropbox-folder/
gksu gedit .config/user-dirs.dirs
# symlinking a folder into home also works
```

## Open file manager as root
```
gksu nautilus /var/www
```
