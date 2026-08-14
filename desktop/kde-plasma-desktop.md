# KDE Plasma Desktop

## 1. What is it?

KDE Plasma is an alternative desktop environment for Linux, lighter and more customizable than GNOME. `kde-plasma-desktop` is the minimal Plasma metapackage.

## 2. What is it for?

- Installing a KDE desktop instead of (or alongside) GNOME/Ubuntu desktop.
- A familiar file-manager-centric workflow (Krusader recommended).

## 3. How to download / install

```bash
sudo apt-get install kde-plasma-desktop
# or the netbook variant:
sudo apt-get install kde-plasma-netbook

# Recommended file manager
sudo apt-get install krusader
```

## 4. How to use

Pick "Plasma" at the login screen session menu. Launch Krusader from the menu for a dual-pane file manager.

## 5. Pitfalls

- Installing KDE alongside GNOME gives you a session-choice menu at login; the display manager (SDDM vs GDM) may switch to SDDM.
- The full `kubuntu-desktop` metapackage is much larger; `kde-plasma-desktop` is the minimal set.
