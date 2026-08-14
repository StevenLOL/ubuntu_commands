# Nautilus "Open Terminal" extension

## 1. What is it?

`nautilus-open-terminal` is a Nautilus (GNOME Files) plugin that adds an **"Open in Terminal"** context-menu entry, so you can launch a shell at the current folder.

## 2. What is it for?

- Opening a terminal in the directory you're browsing — saves `cd`-ing manually.

## 3. How to download / install

```bash
sudo apt-get install nautilus-open-terminal
# then restart Nautilus:
nautilus -q
```

## 4. How to use

Right-click any folder in Files → **Open in Terminal**.

## 5. Pitfalls

- **Package renamed**: on modern GNOME it's bundled into `nautilus-extension-gnome-terminal` (GNOME Terminal) or `nautilus-open-terminal` may be unavailable — install the equivalent extension for your file manager.
- **Restart needed**: the entry won't appear until Nautilus reloads (`nautilus -q`).
- The original note's `gedit ~/.nautilus-terminal` path is outdated; the plugin no longer uses that config file.
