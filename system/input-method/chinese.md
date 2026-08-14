# Chinese Input Method (ibus / fcitx / Sogou)

## 1. What is it / What is it for?

A Chinese (Pinyin) input method framework for Linux. The two common stacks are **ibus** (GTK-friendly, default on many Ubuntu GNOME installs) and **fcitx** (used by Sogou Pinyin for Linux). It lets you type Chinese characters on an Ubuntu desktop via pinyin composition, and switch between multiple language layouts.

## 2. How to download / install (ibus)

```bash
sudo apt-get install ibus ibus-clutter ibus-gtk ibus-gtk3 ibus-qt4
im-switch -s ibus
ibus-setup
```

Then log out / log back in, and add your input method under `Settings → Region & Language → Input Sources`.

For Fcitx + Sogou / Rime:

```bash
sudo apt-get install fcitx fcitx-pinyin fcitx-rime
# set Fcitx as the active framework:
im-config -n fcitx
```

> IBus core (alternative minimal set):
> ```bash
> sudo apt-get install ibus ibus-pinyin        # + ibus-sunpinyin / ibus-googlepinyin as needed
> ibus restart
> ```

## 3. How to use

- Open **Settings → Region & Language → Input Sources**, click **+**, add your language (e.g. "Chinese (Intelligent Pinyin)").
- Switch input methods with `Super (Win) + Space`.
- Right-click the tray icon to configure pinyin settings.

For Sogou (fcitx-based), restart a frozen panel:

```bash
# restart a frozen Sogou / fcitx panel
killall fcitx
killall sogou-qimpanel
# they auto-respawn; if not, restart fcitx manually:
fcitx &
```

## 4. Notes

- `im-config` (or `im-switch`) picks the active framework (IBus vs Fcitx) — only one should be active.
- After install, log out / log back in so the input method autostarts.

## 5. Pitfalls

- Two frameworks active at once (IBus + Fcitx) → input box flickers / no candidate window. Choose one.
- Candidate window invisible on some DEs → set `IBUS_GTK_VERSION=3` or restart the desktop session.
- Pinyin engine missing → install `ibus-pinyin` (not just `ibus`).
- Sogou panel freezes → `killall fcitx && killall sogou-qimpanel`, then let it auto-respawn (or `fcitx &`).
