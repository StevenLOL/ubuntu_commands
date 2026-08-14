# Ubuntu Desktop (GUI on a server install)

## 1. What is it / What is it for?

Ubuntu offers a full GNOME desktop environment (`ubuntu-desktop`). A *server* install ships only the command line; this note shows how to add the GUI afterward.


- Turning a headless Ubuntu Server into a desktop you can use with a monitor or via remote desktop.
- Useful when you need a browser / GUI tool on a box that started life as a server.

## 2. How to download / install

```bash
# Full GNOME desktop (large download)
sudo apt-get install ubuntu-desktop

# Or a lighter option:
# sudo apt-get install xubuntu-desktop      # XFCE
# sudo apt-get install lubuntu-desktop      # LXDE
```

After install, reboot; the GDM login screen appears.

## 3. How to use

Log in at the GUI. To start the desktop from a console-only boot:

```bash
sudo systemctl start gdm3      # or lightdm depending on what was installed
```

For remote access, install a VNC/XRDP server (see `remote-desktop/`).

