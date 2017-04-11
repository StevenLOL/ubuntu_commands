Terminal commands:

press tab for possible suggetions

# navigation & browsing
```
cd ~/         #go to home directory of current account ~/=home
cd ..         #go to parent directory
cat /data2/malay1307/s5/irun.sh   #display file content
head somefile                     #display only the head part of a files
tail somfile                     #display .... tail
less somefile                    #display file by parts
list file properties
ls -l data/groundtrue
ls -d */   # list all directory
ls -d *bin # list all directories end with bin
tree -d    # list all directory
#list files recurisively
find ./path/ -type f -exec ls -dl \{\} \; | awk '{print $9}'
```
# wordcount
```
wc /data/groundtrueE/text
```
# system vars/info
```
lsb_release -a    #show ubuntu version
echo $PATH
which ls
#network states
sudo netstat -tupn
#list hardware
sudo lshw
lspci
#get cpu info
cat /proc/cpuinfo
top iftop iotop glances     sudo iftop -i eth1
ps aux
ps -aef
ps -auwe | grep steven | grep server | grep 28166 | grep PWD
#list binded port
lsof -i :8080
sudo netstat -peanut
pkill process
#kill all process of a user
kill process_id
pkill -u username
#memory usage
free -m
#drop cache memory
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches
#disk usage
df -h  #list harddisk size
du -hs #list folder size
ls -al /dev/disk/by-uuid/
sudo ntsfix /dev/sda5    #fixed NTFS disk error: ...The disk contains an unclean file system (0, 0). Metadata kept in Windows cache, refused to mount.
dd if=/dev/sda of /dev/sdd #clone harddisk sda to sdd, you must use sudo
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
```
## system log / history
```
/var/log/auth.log
~/.bash_history
#set history size in ~/.bashrc
#save history one command is entered:
export PROMPT_COMMAND='history -a;history -c;history -r'
#REF http://superuser.com/questions/555310/bash-save-history-without-exit
```  
# editing,create,copy modify files directories
```
mkdir somedir
#delete directory 
#rm -rf *
rm -rf ./train_clean/*8k
nano filename
echo somthing > newfile
rm somefile                     #del a file
tar -czvf yourzip.tar.gz directory_to_zip/ 
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
```
# add user
```
sudo adduser steven
sudo adduser steven sudo
sudo useradd -d /home/testuser -m testuser
sudo passwd testuser
gksudo gnome-control-center user-accounts 
sudo passwd  #change passwd of root user
sudo passwd -l root #To disable the root login
sudo passwd -S root #check if locked or not (will have L in output instead of P)
```
# rename
```
rename s/"sd_under_classter"/"change to wat?"/g *
/g = all matched
*=all files
eg rename s/"_16k.wav"/".wav"/g *
eg rename s/"Suria"/"suria"/g *
#all to lower case
find . -depth -exec rename 's/(.*)\/([^\/]*)/$1\/\L$2/' {} \;
```
# string 
```
#substring
stringZ=abcABC123ABCabc
echo ${stringZ:7}                            # 23ABCabc
echo ${stringZ:7:3}                          # 23A
                                       # Three characters of substring
#replace '\t' with ' ':
cat a.txt | tr '\t' ' '
or sed 's/old_string/new_string/g'

#get the first row of a csv or tsv file:
cat ./myfile.tsv | awk '{print $1}'
```
# search sort
```
cat ./spk2utt | cut -d ' ' -f 1 > spk2gender #cut the first coloum
sort -nk 12 sort by 12 colloums
sort -r flist.txt > rflist.txt
cat hub-140-f.data | sort -k 5,5 -k 2,2 -k 3,3n > hub-140-f.data.sorted    #sort by 5,2,3 3 in digit format
grep Mean exp/sgmm2_5a/decode_graph_tg222pr_24h.new.crf_fmllr/scoring/*.txt.sys | sort -nk 12
grep Mean exp/sgmm2_5a/decode_graph_tg222pr_24h.new.crf/scoring/*.txt.sys | sort -nk 12 
grep WER exp/tri3b/decode_tgpr_eval24hr.si/wer_* | sort -nk 2
#search 
find . -name "words.txt"
#search text in files
grep -r word *
#find and grep
find ./exp/tri3b/decode_tgpr_eval6hr_rttm14Aug13/ -name "*.sys" | xargs grep Mean | sort -nk 11
#search include sub directory
grep someword -R ./
grep 'pattern1\|pattern2' filename  #or
grep -E 'Tech|Sales' employee.txt   #or
egrep 'pattern1|pattern2' filename  #or
grep -E 'pattern1.*pattern2' filename #and
grep -v 'pattern1' filename           #not
```
# split a file
```
#split text by lines :
split -l 500 a.list   #by lines
split -d -l 500 a.list
split -d -n l/3 a.list # 3 parts without spliting the line;
split -d -n l/3 a.list myprefix                                                         
```
# ssh
```
ssh sjd601@155.69.146.214
ssh -l 23 sjd601@155.69.146.214
```
# multi threads
```
some_commands &
with wait 
a&
b&
wait
```   
# two comands at same time |
```
lspci | grep Intel
```
# wait
sleep 60
# work with queue
```
check queue state
qstat -u "*"
qdel job-ID 
qsub hello_world.sh &
```        
# detach terminal from remote:			
```
#ssh to a server, the connection may fail if task run for hours,and given error : broken pipe, the solution is using, or nohup tmux once login via ssh
aaron@localpc$ ssh root@remoteserver
root@remoteserver# nohup ./run_eval24hr_M_SAD_SD.sh </dev/null &
nohup ./local/run_dnn.sh </dev/null &   
nohup some_command > nohup2.out&
tmux
tmux list-sessions
tmux attach-session -t #
#enable mouse model: in ~/.tmux.conf
setw -g mode-mouse on
or setw -g mouse on
then tmux source-file ~/.tmux.conf
#when in tmux
#switch and list tmux session
ctrl-b s
#detach session
ctrl-b d
```
# run task at time interval, given time
```
crontab -e
01 * * * * python /home/steven/Dropbox/workspace/python_lib/utli/remote_script/ws09loop.py
```
# run task during system startup
```
sudo nano /etc/rc.local
REF [How can I make “rc.local” run on startup?](http://askubuntu.com/questions/9853/how-can-i-make-rc-local-run-on-startup)
```
```
cp myscript.sh /etc/init.d/
sudo update-rc.d myscript.sh defaluts 90
#to remove
suod update-rc.d -f myscript.sh remove
```
# open exploer as root
```
gksu nautilus /var/www      
```
# image resize
```
for f in ./s5/*.jpg ;do echo $f[1024x] ./resized/$f done;
```
# text encoding change
```
opencc -i ~/Downloads/data/wiki_corpus/corpus_120520.txt -o ./120520.txt -c zht2zhs.ini
```
# ping
```
for x in {11..255};do ssh 155.69.151.$x; done;
```    

# install update software
```
sudo apt-get update
sudo apt-get install xx
sudo apt-get install xx --reinstall
sudo apt-get remove somepackage
sudo apt-get purge somepackage
sudo apt-get -f install  #fix some missing package
#chagne software source
sudo nano /etc/apt/sources.list
#install a package
sudo dpkg -i ./wps-office_9.1.0.4751~a15_i386.deb
#purge a installed deb
sudo dpkg -P ./wps-office_9.1.0.4751~a15_i386
#force update
sudo apt-get dist-upgrade
#update from 12.04-12.10
sudo do-release-upgrade
sudo pip some python_package
sudo pip some python_package -I #reinstall it inorge if it is installed
sudo pip install -r your_file   #
sudo pip -U #install and update
sudo pip uninstall pycuda
sudo python setup.py install    #if there is a setup.py in download pyton package
```
# list installed module package
```
lsmod | grep nouveau
sudo dpkg -L packagename  #find installed file location
```
# change repositories location with replace txt 
```
/etc/apt/sources.list
sudo cat /etc/apt/sources.list | sed 's/sg.archive.ubuntu.com/download.nus.edu.sg\/mirror/'  > ./sources.list
sudo cp ./sources.list /etc/apt/sources.list
```
# play audio
```
play a.wav
sox suria102_262_1904.wav  -p trim 300 5 |play -p
#formate change
sox a.mp3 -c 1 -r 16000  -b 16 a.wav  #change mp3 to wav format with sampling rate of 16khz 16bits per sample and mono channel 
```
# build software
```
./configure
make -j 4
sudo make install
or 
mkdir build
cd build
cmake ..
make all
sudo make install 
autoconf
```
# mount cd/dvd iamge (ISO) 
```
sudo mount -o loop xxx.iso /media/xxxx
```
# mount USB drive
```
mount usb
sudo fdisk -l
sudo mkdir /media/external
sudo mount /dev/sdb1 /media/external
sudo mount -t ntfs-3g /dev/sdb1 /media/usb3tb
sudo mount -t ntfs-3g /dev/sdc1 /media/usb3tb
sudo mount -t vfat /dev/sdb1 /media/external -o uid=1000,gid=1000,utf8,dmask=027,fmask=137 
```
# mount network drive
```
sftp://steven@155.69.149.215
gvfs-mount sftp://sjd601@155.69.146.214     #mount with terminal or on the exploer
gvfs-mount sftp://dcl603@155.69.146.214
gvfs-mount sftp://dcl603@155.69.146.212
Enter password for ssh as sjd601 on 155.69.146.212
155.69.149.254
smb://databackup@mybooklive1/databackup
smb://databackup@155.69.149.109/databackup
155.69.149.240
smb://workspace@mybooklive2/workspace
smb://workspace@155.69.149.240/workspace
15569
ftp://cclahadmin@www.cclah.com
```
# change desktop pictures download home path
```
http://www.howtogeek.com/howto/17752/use-any-folder-for-your-ubuntu-desktop-even-a-dropbox-folder/
gksu gedit .config/user-dirs.dirs
not work if change to other place that not under home path?
just ln some where to home also works
```

 
# change machine name
```
sudo apt-get install winbind
You probably want to add 'wins' where it says "hosts:" in /etc/nsswitch.conf
sudo gedit /etc/hostname
sudo gedit /etc/hosts
sudo apt-get install avahi-daemon
```
# open webpage
```
#open webpage from ternimal
links www.zaobao.com
lynx
w3m
#As far as I know, these browsers do not support programmed reloading, however it can easily be accomplished by using a terminal multiplexer like tmux. For example if you start the browser in one terminal like this:
tmux new-session -s browse 'w3m google.com'
#Then you can send commands to it from another terminal with the send-keys command. So to make w3m reload the current page do this:
tmux send-keys -t browse R 
```

# mount folder to ram , put folder in ram
```
#to say you have lots of rams and would like to run program faster by preload them into memory, you can setup ramfs, then just copy/del file to that folder
mkdir -p /mnt/tmp
mount -t tmpfs -o size=20m tmpfs /mnt/tmp
mkdir -p /mnt/ram
mount -t ramfs -o size=20m ramfs /mnt/ram
```
# scripting
```
#passing args to a.sh
a.sh 13 213 323
in a.sh  $1 $2 $3 & will run in background
in a.sh v1=$1 v2=$2
```
# read text file and process line by lines
```
FILE=wav.list
ext=wav
outputfolder=seg_remove_cm
cat $FILE | while read line; do
#cat file hrizationly
paste -d"," *.txt
    
echo "Processing $line"
 nameOnly=$(echo $line | awk -F / '{ print $7 }')
 nameOnly=${nameOnly%.$ext}
 echo "NAME= $nameOnly"
java -Xmx2048m -classpath ./batch.jar \
edu.cmu.sphinx.tools.feature.FeatureFileDumper \
-config ./frontend.config.xml \
-name cepstraFrontEnd \
-i $line \
-o ./mfcc/$nameOnly.mfcc  
# ./wave2mfcc.sh $line $ext
done
```
# read a fodler and process file one by one
```
#loop and find names 
eg1:

for file in ./input/gt_ctm/*.ctm
do
 # do something on "$file"
 echo "Processing $file"
  nameOnly=$(echo $file | awk -F "/" '{print $NF}') 
 echo $nameOnly
 nameOnly=${nameOnly%.ctm}
 echo ${nameOnly}

 #python sort_ctm.py "$file" "./output/gt_rttm/sorted_$nameOnly.ctm"
 #python ctm_2_rttm_from_segments_and_utt2spk.py "./output/gt_rttm/sorted_$nameOnly.ctm" "./input/gt_ctm/segments" "./input/gt_ctm/utt2spk" "./output/gt_rttm/$nameOnly.rttm"
 python my_validation.py "./output/gt_rttm/sorted_$nameOnly.ctm" "./output/gt_rttm/$nameOnly.rttm"
 
#offical validator by NIST, take longer time
# perl rttmValidator.pl -f -i "./output/gt_rttm/$nameOnly.rttm"
 
done

#eg2:
#loop dir

for ifile in /home/steven/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts/*.wav
do
  #echo $ifile
  show=`basename $ifile .wav`
  echo $show
  /usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
 --fInputMask=$ifile --sOutputMask=/home/steven/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts_gender/$show.seg ZHIZHENGTEST &
done
#eg3:
#loop vars

names=( Jennifer Tonya Anna Sadie )

for name in ${names[@]}
do
	echo $name
done
```
# split data and multithreading
```
mkdir -p $outdir

split -d -n l/15 /home/steven/apps/getgender/lium/magor_gt_merge_nodev.test.lst ${tid}

for x in ./${tid}*
do
  echo $x
  ./getgender.sh $x $outdir &
done
```
# pick random files 
```
find ./audio -type f | shuf -n 10
```
# scan host in the network:
```
arp
sudo nmap -sn 10.99.23.1/24 > readme.txt
avahi-browse -a -v -r -t -d local   #sudo apt-get install avahi-discover
```
# disable mouse scroll
```
xinput list
xinput get-button-map 8
xinput set-button-map 8 1 2 3 0 0 
4,5 means scoring up/download
and sudo save to ~/.profile
```

#to make gonme-session-fallback look nice,right click on the bootom tray and group applictions and 4 rows for workspace
 or install kde-plasma

# shorten command promt
```	
To change it for the current terminal instance only

Just enter PS1='\u:\W\$ ' and press enter.
To change it "permanently"

In your ~/.bashrc, find the following section:

if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi

Remove the @\h, and replace the \w with an uppercase \W, so that it becomes:

if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u\[\033[00m\]:\[\033[01;34m\]\W\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u:\W\$ '
```

# OTHERS
```
#=========================================================================================================================================


cp -a /data2/malay1307 /home/sjd601/training/malay_13_07/
sudo rm -rf ./kaldi-trunk-whatever/
sudo cp -a -H -L kaldi-trunk kaldi-trunk-whatever   #replace link file with source 

sudo ufw disable


#edit in sudo in gui mode
gksu gedit /etc/bumblebee/bumblebee.conf

stop gui 
sudo service lightdm stop  

loop dir

for ifile in /home/steven/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts/*.wav
do
	#echo $ifile
	show=`basename $ifile .wav`
	echo $show
	/usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
 --fInputMask=$ifile --sOutputMask=/home/steven/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts_gender/$show.seg ZHIZHENGTEST &

done

if [ -d "/path/to/dir" ]
then
    echo "Directory /path/to/dir exists."
else
    echo "Error: Directory /path/to/dir does not exists."
fi

for D in *; do echo $D; find $D -type f| wc -l; done


)others

#disable auto logout

set LOCK_SCREEN=false
sudo nano /etc/default/acpi-support





#-#

```

# rc and conf files
## .bashrc
```
~/.profile or ~/.bashrc


PATH=/home/steven/apps/getmfcc/utils:$PATH
PATH=/home/steven/kaldi-trunk/src/utils:$PATH
PATH=/home/steven/kaldi-trunk/src/bin:$PATH     
PATH=/home/steven/kaldi-trunk/src/fgmmbin:$PATH  
PATH=/home/steven/kaldi-trunk/src/gmmbin:$PATH      
PATH=/home/steven/kaldi-trunk/src/kwsbin:$PATH  
PATH=/home/steven/kaldi-trunk/src/nnet2bin:$PATH  
PATH=/home/steven/kaldi-trunk/src/onlinebin:$PATH  
PATH=/home/steven/kaldi-trunk/src/sgmmbin:$PATH
PATH=/home/steven/kaldi-trunk/src/featbin:$PATH  
PATH=/home/steven/kaldi-trunk/src/fstbin:$PATH   
PATH=/home/steven/kaldi-trunk/src/ivectorbin:$PATH  
PATH=/home/steven/kaldi-trunk/src/latbin:$PATH  
PATH=/home/steven/kaldi-trunk/src/nnetbin:$PATH   
PATH=/home/steven/kaldi-trunk/src/sgmm2bin:$PATH
xinput set-button-map 8 1 2 3 0 0
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 60
update-alternatives --config g++
sudo update-initramfs -c -k all -u         
export HADOOP_HOME=/usr/local/hadoop      
export PATH=$PATH:$HADOOP_HOME/bin        
export JAVA_HOME=/usr/lib/jvm/java-8-oracl       
export CUDA_HOME=/usr/local/cuda                        
export C_INCLUDE_PATH=/usr/local/cuda-8.0/include/      
export CPLUS_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export SPARK_HOME="/data/apps/spark-2.0.1-bin-hadoop2.7"   


#allow to pipe a python output to a file in 'utf-8' 
#http://stackoverflow.com/questions/13481582/pipe-output-of-python-script

export PYTHONIOENCODING=utf-8  
```

## ~/.theano.rc
```
[global]
floatX = float32
device = gpu0
mode= FAST_RUN

[nvcc]
fastmath = True

[dnn.conv]                                       
algo_fwd = time_once
algo_bwd_data = time_once
algo_bwd_filter = time_once

```
## ~/.pip/pip.conf
``` 
[global]
download_cache = /data/.cache/pip
index-url = https://pypi.douban.com/simple
```
## ~/.tmux.conf 
```
setw -g mouse on
```
## ~/.tmat.conf
```
#more in folder tmux tmat
set -g tmate-server-host "x.x.x.x"
set -g tmate-server-port your port
set -g tmate-server-rsa-fingerprint   "x...xxx.x.x.x"
set -g tmate-server-ecdsa-fingerprint "xxxx.x.x.x.x.x"
#set -g tmate-identity ""              # Can be specified to use a different SSH key.
```
