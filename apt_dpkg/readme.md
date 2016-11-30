
#source.list

cat /etc/apt/source.list

http://mirrors.tuna.tsinghua.edu.cn/ubuntu/

#FAQ
##ldconfig' not found in PATH or not executable
ldconfig is not in the /sbin , may deleted during remove libc6-bin package
To fix: cp from other source
##offline
[How can I install software or packages without Internet (offline)?](http://askubuntu.com/questions/974/how-can-i-install-software-or-packages-without-internet-offline)

##Depends: libc6 (= 2.19-0ubuntu6.9) but 2.23-0ubuntu3 is to be installed
Cause by wrong source.list


##Adding 'diversion of /usr/sbin/update-icon-caches to /usr/sbin/update-icon-caches.gtk2 by libgtk-3-bin' dpkg-divert: error: rename involves overwriting `/usr/sbin/update-icon-caches.gtk2' with

Just deleted that file, and retry

http://www.webupd8.org/2011/02/fix-dpkg-error-trying-to-overwrite-x.html
```
sudo dpkg -i --force-overwrite /var/cache/apt/archives/smplayer_0.6.9+svn3595-1ppa1~maverick1_i386.deb

sudo apt-get install -f
```

##Can't exec “locale”: No such file or directory at /usr/share/perl5/Debconf/Encoding.pm line 16
http://askubuntu.com/posts/640581/
```
apt-get download libc-bin

dpkg -x libc-bin*.deb libdir/

and then:

sudo cp libdir/sbin/ldconfig /sbin/

sudo apt-get install --reinstall libc-bin

sudo apt-get install -f

Taken from [old Ubuntu forum archives][1].


  [1]: http://ubuntuforums.org/archive/index.php/t-1266104.html
```

dpkg --configure -a

dpkg --configure -a --force-depends

apt-get -f install



