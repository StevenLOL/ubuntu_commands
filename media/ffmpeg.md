# FFmpeg — 音频 / 视频处理瑞士军刀

## What

FFmpeg 是一套跨平台的音视频录制、转换、流化处理工具集（含 `ffmpeg`、`ffprobe`、`ffplay`）。
本笔记侧重**音频处理**常用命令：格式转换、提取音轨、剪辑、音量调节、合并等。

## How to install

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install -y ffmpeg

# 验证
ffmpeg -version
ffprobe -version
```

`ffprobe` 用于查看媒体文件信息，很多时候比 `ffmpeg` 更顺手。

## How to use

基本语法：

```bash
ffmpeg [全局参数] -i 输入文件 [输出参数] 输出文件
```

不加 `-c` 参数时，FFmpeg 会按输出扩展名自动选择编码器（重新编码）。
加 `-c copy` 表示**流复制**（不重新编码，速度快、无损，但只能用于同容器/兼容流）。

## Cheat-sheet

### 信息查询

```bash
ffprobe input.mp3                       # 查看文件信息（时长、码率、编码）
ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 input.mp3
                                       # 只打印时长（秒）
```

### 格式转换

```bash
ffmpeg -i input.wav output.mp3          # WAV -> MP3（重新编码）
ffmpeg -i input.mp3 -c copy out.m4a     # MP3 -> M4A 容器（流复制，不重编码）
```

### 提取音轨（从视频中分离音频）

```bash
ffmpeg -i video.mp4 -vn -acodec copy audio.aac   # 提取音轨，不重编码
ffmpeg -i video.mp4 -vn -acodec libmp3lame audio.mp3  # 提取并转 MP3
```

### 剪辑 / 拼接

```bash
ffmpeg -i input.mp3 -ss 00:00:10 -to 00:00:30 out.mp3   # 截取 10s~30s
ffmpeg -i input.mp3 -ss 00:00:10 -t 00:00:20 out.mp3    # 从 10s 起取 20s
ffmpeg -i "concat:1.mp3|2.mp3|3.mp3" -acodec copy merged.mp3  # 拼接（文件需同参数）
```

### 音量 / 采样率 / 声道

```bash
ffmpeg -i input.mp3 -af "volume=1.5" louder.mp3        # 音量 ×1.5
ffmpeg -i input.mp3 -ar 44100 out.mp3                   # 重采样到 44.1kHz
ffmpeg -i input.mp3 -ac 1 mono.mp3                      # 转单声道
```

### 录音（从默认设备）

```bash
ffmpeg -f pulse -i default -t 10 recording.mp3         # 用 PulseAudio 录 10 秒
```

### RTSP 流（拉流 / 录制 / 低延迟预览）

RTSP 常用于 IP 摄像头等设备的实时视频。用 `-rtsp_transport` 选 UDP（低延迟）或 TCP（稳）。

```bash
# 保存为文件（流复制，不重编码）
ffmpeg -rtsp_transport tcp -i rtsp://HOST:554/stream -c copy out.mp4

# 低延迟实时预览
ffplay -rtsp_transport udp -fflags nobuffer -flags low_delay rtsp://HOST:554/stream

# 低延迟拉流：裸帧输出到 stdout（供程序逐帧读取，丢弃音频）
ffmpeg -rtsp_transport udp \
  -probesize 500000 -analyzeduration 1000000 \
  -fflags +discardcorrupt+nobuffer -max_delay 0 -reorder_queue_size 0 \
  -i rtsp://HOST:554/stream \
  -f rawvideo -pix_fmt bgr24 -s 1920x1080 -an -

# 转推为新 RTSP 流（relay）
ffmpeg -rtsp_transport tcp -i rtsp://HOST:554/stream \
  -c:v copy -f rtsp rtsp://localhost:8554/relay
```

| 参数 | 作用 |
| --- | --- |
| `-rtsp_transport tcp/udp` | 传输方式（UDP 延迟低，TCP 更稳） |
| `-probesize 500000` | 限制探测字节数，加速起流 |
| `-analyzeduration 1000000` | 限制分析时长（微秒），加速起流 |
| `-fflags +discardcorrupt+nobuffer` | 丢弃损坏帧、不缓冲，降低延迟 |
| `-max_delay 0` / `-reorder_queue_size 0` | 关闭重排序延迟 |
| `-f rawvideo -pix_fmt bgr24` | 输出裸帧（BGR24），供 OpenCV 等逐帧读 |
