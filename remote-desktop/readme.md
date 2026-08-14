# Remote Desktop on Ubuntu (xrdp / FreeNX / MATE)

## 1. What is it / What is it for?

Ways to reach an Ubuntu desktop remotely: **xrdp** (RDP server, works with Windows Remote Desktop), **FreeNX** (NX protocol), and **MATE** desktop over xrdp. This note focuses on xrdp + XFCE/MATE and fixing the `<Super>Tab` conflict.


- Remotely controlling an Ubuntu GUI from Windows (RDP) or an NX client.
- Running a lightweight desktop (XFCE/MATE) over a remote session.

## 2. How to download / install

**xrdp + XFCE (Ubuntu > 14.04):**
```bash
sudo apt-get install xrdp
sudo apt-get install xfce4
# sudo apt-get remove xscreensaver   # optional
echo xfce4-session >~/.xsession
sudo service xrdp restart
```

**MATE over xrdp:**
```bash
sudo add-apt-repository ppa:ubuntu-mate-dev/ppa
sudo apt update
sudo apt-get install mate-desktop-environment
echo mate-session >> ~/.xsession
```

**FreeNX (alternative):**
```bash
sudo apt-add-repository ppa:freenx-team
sudo apt-get update
sudo apt-get install freenx-server
wget https://bugs.launchpad.net/freenx-server/+bug/576359/+attachment/1378450/+files/nxsetup.tar.gz
tar -xvf nxsetup.tar.gz
sudo cp nxsetup /usr/lib/nx/nxsetup
sudo /usr/lib/nx/nxsetup --install
sudo apt-get install gnome-session-fallback qtnx
```

Ref for 18.04/19.04: http://www.c-nergy.be/products.html

## 3. How to use

Connect with a Windows "Remote Desktop Connection" client to the server's IP; log in and you get the XFCE/MATE session.

### Fix the TAB problem (Super+Tab steals focus)
Edit `~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml`, find:
```xml
<property name="&lt;Super&gt;Tab" type="string" value="switch_window_key"/>
```
and change it to:
```xml
<property name="&lt;Super&gt;Tab" type="empty"/>
```
Or in *Settings → Window Manager → Keyboard*, clear the "Switch window for same application → <Super>Tab" binding.

