# install remote desktop for UBUNTU >14.04
```
sudo apt-get install xrdp
sudo apt-get install xfce4
## sudo apt remove xscreensaver  #remove screensaver
echo xfce4-session >~/.xsession
sudo service xrdp restart
```

To fix the TAB problem:
```
In short, you can either edit:
~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml
find the line
<property name="&lt;Super&gt;Tab" type="string" value="switch_window_key"/>
and change it to
<property name="&lt;Super&gt;Tab" type="empty"/>
Or go to "Settings->Window Manager", in the "Keyboard" tab, clear the binding of "Switch wndow for same application -> <Super> Tab"
```





# others:
```
sudo apt-add-repository ppa:freenx-team
sudo apt-get update
sudo apt-get install freenx-server

wget https://bugs.launchpad.net/freenx-server/+bug/576359/+attachment/1378450/+files/nxsetup.tar.gz

tar -xvf nxsetup.tar.gz

sudo cp nxsetup /usr/lib/nx/nxsetup

sudo /usr/lib/nx/nxsetup --install 

sudo apt-get install gnome-session-fallback


sudo apt-get install qtnx

```


#Install mate 

sudo apt-add-repository ppa:ubuntu-mate-dev/ppa
sudo apt-update
sudo apt-get install mate-desktop-environment
echo mate-session >> ~/.xsession


# For 18.04 19.04 ?
```
http://www.c-nergy.be/products.html
```
