# Ubuntu Input Method (输入法)

## 1. What is it / What is it for?

An input method framework lets you type languages that need composition (Chinese, Japanese, Korean, etc.) on Ubuntu. The default modern stack is **IBus**; **Fcitx** is a popular alternative.

- Typing CJK characters via pinyin / anthy / hangul composition.
- Switching between multiple language layouts.

## 2. How to download / install (IBus + Chinese pinyin)

```bash
# IBus core + pinyin engine
sudo apt-get install ibus ibus-pinyin

# (Optional) SunPinyin / Google Pinyin engines
# sudo apt-get install ibus-sunpinyin
# sudo apt-get install ibus-googlepinyin

# Restart IBus
ibus restart
```

For Fcitx + Sogou / Rime:

```bash
sudo apt-get install fcitx fcitx-pinyin fcitx-rime
# set Fcitx as the active framework:
im-config -n fcitx
```

## 3. How to use

- Open **Settings → Region & Language → Input Sources**, click **+**, add your language (e.g. "Chinese (Intelligent Pinyin)").
- Switch input methods with `Super (Win) + Space`.
- Right-click the tray icon to configure pinyin settings.

## 4. Notes

- `im-config` picks the active framework (IBus vs Fcitx) — only one should be active.
- After install, log out / log back in so the input method autostarts.
- This note is a placeholder; expand with your own tested commands.

## 5. Pitfalls

- Two frameworks active at once (IBus + Fcitx) → input box flickers / no candidate window. Choose one.
- Candidate window invisible on some DEs → set `IBUS_GTK_VERSION=3` or restart the desktop session.
- Pinyin engine missing → install `ibus-pinyin` (not just `ibus`).
