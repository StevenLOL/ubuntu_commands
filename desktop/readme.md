# Ubuntu Desktop (GUI on a server install)

## 1. What is it?

Ubuntu offers a full GNOME desktop environment (`ubuntu-desktop`). A *server* install ships only the command line; this note shows how to add the GUI afterward.

## 2. What is it for?

- Turning a headless Ubuntu Server into a desktop you can use with a monitor or via remote desktop.
- Useful when you need a browser / GUI tool on a box that started life as a server.

## 3. How to download / install

```bash
# Full GNOME desktop (large download)
sudo apt-get install ubuntu-desktop

# Or a lighter option:
# sudo apt-get install xubuntu-desktop      # XFCE
# sudo apt-get install lubuntu-desktop      # LXDE
```

After install, reboot; the GDM login screen appears.

## 4. How to use

Log in at the GUI. To start the desktop from a console-only boot:

```bash
sudo systemctl start gdm3      # or lightdm depending on what was installed
```

For remote access, install a VNC/XRDP server (see `remote-desktop/`).

## 5. Pitfalls

- **Huge download** — `ubuntu-desktop` pulls in hundreds of MB. Use `xubuntu-desktop` if disk/network is tight.
- **On cloud VMs** a desktop still needs a local display or a remote-desktop server; it won't help unless you can reach it.
- **Conflicts**: installing multiple desktop metapackages can leave you with mixed session choices at login — pick one and stick to it.
