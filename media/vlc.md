# VLC media player

## What

VLC 是 VideoLAN 出品的开源跨平台多媒体播放器与框架，支持几乎所有音视频格式与流媒体协议，无需额外解码器，也可用作简单的音视频转码、串流与录制工具。

## How to install

Ubuntu / Debian：

```bash
sudo apt update
sudo apt install -y vlc
```

通过 Snap 安装：

```bash
sudo snap install vlc
```

## How to use

GUI 启动：

```bash
vlc
```

命令行播放：

```bash
vlc path/to/video.mp4
vlc http://example.com/stream.m3u8      # 播放网络流
```

命令行转码（实为调用 ffmpeg 后端，VLC 也自带转码能力）：

```bash
# 提取音频为 mp3
vlc input.mp4 --no-sout-video \
  --sout-audio --sout '#transcode{acodec=mp3,ab=192}:std{access=file,mux=raw,dst=out.mp3}' vlc://quit
```

串流（将本地视频推为 HTTP 流）：

```bash
vlc input.mp4 --sout '#standard{access=http,mux=ts,dst=:8080/stream}' --sout-keep
```

截图（从视频抓取一帧）：

```bash
vlc video.mp4 --video-filter=scene \
  --scene-ratio=24 --scene-path=./shots --scene-prefix=frame vlc://quit
```

## Cheat-sheet

| 用途 | 命令要点 |
| --- | --- |
| 播放文件 | `vlc file.mp4` |
| 播放流 | `vlc http://.../stream.m3u8` |
| 转码音频 | `--sout '#transcode{acodec=mp3}:std{access=file,...}'` |
| 推流 | `--sout '#standard{access=http,mux=ts,dst=:8080/stream}'` |
| 静音退出 | 末尾加 `vlc://quit` |
