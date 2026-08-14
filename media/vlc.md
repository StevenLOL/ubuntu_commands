# VLC media player

## What

VLC is an open-source, cross-platform media player and framework from VideoLAN. It plays almost every audio/video format and streaming protocol without extra codecs, and also works as a simple tool for audio/video transcoding, streaming, and recording.

## How to install

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install -y vlc
```

Via Snap:

```bash
sudo snap install vlc
```

## How to use

Launch the GUI:

```bash
vlc
```

Play from CLI:

```bash
vlc path/to/video.mp4
vlc http://example.com/stream.m3u8      # play a network stream
```

Transcode via CLI (VLC uses its own transcode pipeline; `ffmpeg` can also be used):

```bash
# extract audio to mp3
vlc input.mp4 --no-sout-video \
  --sout-audio --sout '#transcode{acodec=mp3,ab=192}:std{access=file,mux=raw,dst=out.mp3}' vlc://quit
```

Stream (push a local video as an HTTP stream):

```bash
vlc input.mp4 --sout '#standard{access=http,mux=ts,dst=:8080/stream}' --sout-keep
```

Capture a frame (grab one frame from the video):

```bash
vlc video.mp4 --video-filter=scene \
  --scene-ratio=24 --scene-path=./shots --scene-prefix=frame vlc://quit
```

## Cheat-sheet

| Purpose | Command |
| --- | --- |
| Play file | `vlc file.mp4` |
| Play stream | `vlc http://.../stream.m3u8` |
| Transcode audio | `--sout '#transcode{acodec=mp3}:std{access=file,...}'` |
| Push stream | `--sout '#standard{access=http,mux=ts,dst=:8080/stream}'` |
| Quit silently | append `vlc://quit` |
