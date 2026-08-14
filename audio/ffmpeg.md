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

## Pitfalls

- **`-c copy` 不能改编码**：只复制流，无法做格式转换或剪辑（剪辑精度受关键帧限制）。需要真正改格式/剪辑时必须重新编码。
- **覆盖不提示**：输出文件已存在时 FFmpeg 会直接覆盖。加 `-n` 跳过已存在文件，或 `-y` 显式确认覆盖。
- **MP3 拼接要求一致**：用 `concat:` 协议拼接时，各文件编码参数（码率、采样率、声道）必须完全相同，否则用 `concat` demuxer（写个 `list.txt` + `-f concat -i list.txt`）。
- **无损 ≠ 流复制**：`-c copy` 是"不重编码"，但源若是_lossy_（如 MP3），复制后仍是_lossy_，不会变清晰。
- **大文件注意磁盘**：重新编码是 CPU 密集操作，长文件建议先用 `-ss`/`-t` 小段测试。
