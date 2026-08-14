# Audacity

## What

Audacity is a free, open-source, cross-platform audio recorder and editor. It supports multi-track editing, noise reduction, cutting, and effects, and is commonly used for recording, podcasts, and simple audio post-production.

## How to install

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install -y audacity
```

Via Flatpak (usually a newer version):

```bash
flatpak install flathub org.audacityteam.Audacity
```

## How to use

Launch:

```bash
audacity            # launch the GUI directly
flatpak run org.audacityteam.Audacity
```

Common operations (in the GUI):

- Record: click the red record button, choose the input device (microphone).
- Import: `File → Import → Audio`, supports wav / mp3 / ogg, etc.
- Export: `File → Export → Export as WAV / MP3`.
- Noise reduction: select a silent segment → `Effect → Noise Reduction → Get Noise Profile` → select all → `Effect → Noise Reduction`.
- Edit: `Ctrl + I` to split, `Delete` to remove, drag to move clips.

Batch conversion via CLI (Audacity has no full CLI; use `ffmpeg` for conversion):

```bash
# transcode after recording
ffmpeg -i input.wav -ar 44100 -ac 1 output.mp3
```

## Cheat-sheet

| Action | Shortcut |
| --- | --- |
| Play / Stop | `Space` |
| Record | `R` |
| Split clip | `Ctrl + I` |
| Delete | `Delete` |
| Select all | `Ctrl + A` |
| Zoom in / out | `Ctrl + 1` / `Ctrl + 2` |
