# 文件与磁盘 / Files & Disk

## 编辑 / 创建 / 复制 / 修改 文件目录
```
mkdir somedir
#delete directory
#rm -rf *   (dangerous)
rm -rf ./train_clean/*8k
nano filename
echo somthing > newfile
rm somefile                     #del a file
tar -czvf yourzip.tar.gz directory_to_zip/
sudo chmod 777 ./dir -R
sudo chmod 777 ./somescript.sh
sudo chown -hR root /u          #Change the owner of /u and subfiles to "root".
cp -r /your/fodler/* target/
cp -avr allsour totarget
#remote copy
rcp
scp or
scp -r <HOSTNAME>@<PRIVATE_IP>:/home/<HOSTNAME>/evil_plans/  ./
# http://www.comentum.com/rsync.html
rsync -r localfolder -v -e ssh <USER>@<SERVER_IP>
sudo ln -s /usr/local/nginx/conf/ /etc/nginx   #link a folder(create a shortcut) target should not exists first
ln -s /data2/malay1307/s5/ <YOUR_HOME>/training/linkedMalay1307
```

## 重命名 / rename
```
rename s/"sd_under_classter"/"change to wat?"/g *
/g = all matched
*=all files
eg rename s/"_16k.wav"/".wav"/g *
eg rename s/"Suria"/"suria"/g *
#all to lower case
find . -depth -exec rename 's/(.*)\/([^\/]*)/$1\/\L$2/' {} \;
```

## 分割文件 / split a file
```
split -l 500 a.list   #by lines
split -d -l 500 a.list
split -d -n l/3 a.list # 3 parts without spliting the line;
split -d -n l/3 a.list myprefix
```

## 图片缩放 / image resize
```
for f in ./s5/*.jpg ;do echo $f[1024x] ./resized/$f done;
```

## 文本编码转换 / text encoding change
```
opencc -i ~/Downloads/data/wiki_corpus/corpus_120520.txt -o ./120520.txt -c zht2zhs.ini
```

## 挂载 ISO / mount cd/dvd image (ISO)
```
sudo mount -o loop xxx.iso /media/xxxx
```

## 挂载 USB / mount USB drive
```
lsusb                            # 查看 USB 设备
mount usb
sudo fdisk -l
sudo mkdir /media/external
sudo mount /dev/sdb1 /media/external
sudo mount -t ntfs-3g /dev/sdb1 /media/usb3tb
sudo mount -t ntfs-3g /dev/sdc1 /media/usb3tb
sudo mount -t vfat /dev/sdb1 /media/external -o uid=1000,gid=1000,utf8,dmask=027,fmask=137
```

## 挂载网络盘 / mount network drive
```
sftp://<USER>@<SERVER_IP>
gvfs-mount sftp://<USER>@<SERVER_IP>     #mount with terminal or on the exploer
Enter password for ssh as <USER> on <SERVER_IP>
smb://databackup@<NAS>/databackup
smb://databackup@<SERVER_IP>/databackup
smb://workspace@<NAS>/workspace
smb://workspace@<SERVER_IP>/workspace
ftp://cclahadmin@www.cclah.com
```

## 文件夹放入内存 / mount folder to ram
```
# preload programs into memory via ramfs, then just copy/del file to that folder
mkdir -p /mnt/tmp
mount -t tmpfs -o size=20m tmpfs /mnt/tmp
mkdir -p /mnt/ram
mount -t ramfs -o size=20m ramfs /mnt/ram
```

## 改变桌面壁纸下载路径 / change desktop pictures download home path
```
# http://www.howtogeek.com/howto/17752/use-any-folder-for-your-ubuntu-desktop-even-a-dropbox-folder/
gksu gedit .config/user-dirs.dirs
# not work if change to other place that not under home path?
# just ln some where to home also works
```

## 以 root 打开文件管理器 / open explorer as root
```
gksu nautilus /var/www
```
