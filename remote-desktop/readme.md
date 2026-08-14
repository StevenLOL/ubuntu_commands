# Remote Desktop on Ubuntu (xrdp / FreeNX / MATE)

## 1. What is it?

Ways to reach an Ubuntu desktop remotely: **xrdp** (RDP server, works with Windows Remote Desktop), **FreeNX** (NX protocol), and **MATE** desktop over xrdp. This note focuses on xrdp + XFCE/MATE and fixing the `<Super>Tab` conflict.

## 2. What is it for?

- Remotely controlling an Ubuntu GUI from Windows (RDP) or an NX client.
- Running a lightweight desktop (XFCE/MATE) over a remote session.

## 3. How to download / install

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

## 4. How to use

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

## 5. Pitfalls

- **Typo in original**: `sudo apt-update` should be `sudo apt update`.
- **`~/.xsession` must match the installed desktop** (xfce4-session vs mate-session) or you get a blank session.
- **FreeNX PPA is deprecated** — it may not build on current Ubuntu; prefer xrdp.
- **xrdp vs Wayland**: modern Ubuntu uses Wayland; xrdp needs X11 — install/select an X11 session (XFCE/MATE) for xrdp to work.
- **Firewall**: open port 3389 (RDP) / 22 (NX over SSH) on the server.
