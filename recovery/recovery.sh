# Recover a broken graphics stack on Ubuntu

## 1. What is it?

A recovery recipe for when the X server / NVIDIA drivers get tangled (black screen, wrong resolution). It reinstalls `xserver-xorg`, rebuilds the modeline, and removes `nouveau` if needed.

## 2. What is it for?

- Fixing "no GUI" / wrong-resolution problems after a driver mismatch or upgrade.
- Restoring a usable desktop and display resolution.

## 3. How to download / install

No download — this is a sequence of apt/`xrandr` commands. Run as root.

## 4. How to use

```bash
sudo add-apt-repository ppa:xorg-edgers/ppa
sudo apt-get update && sudo apt-get upgrade
sudo apt-get purge nvidia* xserver-xorg xinit
sudo apt-get install xserver-xorg xinit
sudo apt-get install mesa-utils

# build a 1920x1080 modeline and apply it
gtf 1920 1080 59.9
xrandr --newmode "1920x1080_59.90"  172.51  1920 2040 2248 2576  1080 1081 1084 1118  -HSync +Vsync
xrandr --addmode eDP1 1920x1080_59.90
startx
# then open the GUI resolution settings and "Apply"

# if nouveau interferes:
sudo apt-get --purge remove xserver-xorg-video-nouveau
sudo modprobe -r nouveau
```

## 5. Pitfalls

- **Typo in original**: it's `sudo apt-get install xserver-xorg xinit` (not `instll`).
- **`purge nvidia*`** removes the proprietary driver; you'll fall back to nouveau/Mesa until you reinstall a driver.
- **`xorg-edgers` PPA is abandoned**; on modern Ubuntu avoid it — use the official NVIDIA `.run`/CUDA repo instead.
- **`eDP1`** is the eDP panel name on laptops; on a desktop the output is usually `HDMI-0`/`DP-1` — adjust the `xrandr --addmode` target.
- **`startx` as root** can create root-owned config files in your home; fix ownership afterwards.
