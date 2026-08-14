# Chinese Input Method (ibus / fcitx / Sogou)

## 1. What is it?

A Chinese (Pinyin) input method framework for Linux. The two common stacks are **ibus** (GTK-friendly, default on many Ubuntu GNOME installs) and **fcitx** (used by Sogou Pinyin for Linux).

## 2. What is it for?

- Typing Chinese characters on an Ubuntu desktop.
- This note covers installing `ibus` and restarting the `fcitx` / Sogou panel after it freezes.

## 3. How to download / install (ibus)

```bash
sudo apt-get install ibus ibus-clutter ibus-gtk ibus-gtk3 ibus-qt4
im-switch -s ibus
ibus-setup
```

Then log out / log back in, and add your input method under `Settings → Region & Language → Input Sources`.

## 4. How to use

Switch input with `Super (Win) + Space`. For Sogou (fcitx-based):

```bash
# restart a frozen Sogou / fcitx panel
killall fcitx
killall sogou-qimpanel
# they auto-respawn; if not, restart fcitx manually:
fcitx &
```

## 5. Pitfalls

- **ibus vs fcitx conflict**: don't run both. If Sogou is installed, use `fcitx` as the active framework (`im-switch -s fcitx`).
- **Panel won't show**: kill both `fcitx` and `sogou-qimpanel`, then relaunch `fcitx`.
- **`im-switch` may be missing** on newer Ubuntu — use `im-config` instead.
- **Need to re-login** after installing an input method for it to appear in settings.
