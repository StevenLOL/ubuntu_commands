# FFmpeg — the audio / video Swiss Army knife

## What

FFmpeg is a cross-platform suite for recording, converting, and streaming audio and video (includes `ffmpeg`, `ffprobe`, `ffplay`). This note focuses on common **audio** commands: format conversion, audio extraction, trimming, volume, and merging.

## How to install

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install -y ffmpeg

# verify
ffmpeg -version
ffprobe -version
```

`ffprobe` shows media file info and is often more handy than `ffmpeg` for inspection.

## How to use

Basic syntax:

```bash
ffmpeg [global options] -i input [output options] output
```

Without `-c`, FFmpeg auto-selects an encoder by the output extension (re-encodes). Adding `-c copy` means **stream copy** (no re-encode, fast and lossless, only valid for compatible streams/containers).

## Cheat-sheet

### Inspect media info

```bash
ffprobe input.mp3                       # show file info (duration, bitrate, codec)
ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 input.mp3
                                        # print duration in seconds only
```

### Format conversion

```bash
ffmpeg -i input.wav output.mp3          # WAV -> MP3 (re-encode)
ffmpeg -i input.mp3 -c copy out.m4a     # MP3 -> M4A container (stream copy, no re-encode)
```

### Extract audio (from a video)

```bash
ffmpeg -i video.mp4 -vn -acodec copy audio.aac   # extract audio track, no re-encode
ffmpeg -i video.mp4 -vn -acodec libmp3lame audio.mp3  # extract and encode to MP3
```

### Trim / concatenate

```bash
ffmpeg -i input.mp3 -ss 00:00:10 -to 00:00:30 out.mp3   # cut 10s~30s
ffmpeg -i input.mp3 -ss 00:00:10 -t 00:00:20 out.mp3    # from 10s, take 20s
ffmpeg -i "concat:1.mp3|2.mp3|3.mp3" -acodec copy merged.mp3  # concat (same params required)
```

### Volume / sample rate / channels

```bash
ffmpeg -i input.mp3 -af "volume=1.5" louder.mp3        # volume x1.5
ffmpeg -i input.mp3 -ar 44100 out.mp3                   # resample to 44.1kHz
ffmpeg -i input.mp3 -ac 1 mono.mp3                      # to mono
```

### Record from default device

```bash
ffmpeg -f pulse -i default -t 10 recording.mp3         # record 10s via PulseAudio
```

### RTSP stream (pull / record / low-latency preview)

RTSP is commonly used for live video from IP cameras and similar devices. Use `-rtsp_transport` to choose UDP (low latency) or TCP (stable).

```bash
# save to file (stream copy, no re-encode)
ffmpeg -rtsp_transport tcp -i rtsp://HOST:554/stream -c copy out.mp4

# low-latency live preview
ffplay -rtsp_transport udp -fflags nobuffer -flags low_delay rtsp://HOST:554/stream

# low-latency pull: raw frames to stdout (for per-frame reading by a program, audio dropped)
ffmpeg -rtsp_transport udp \
  -probesize 500000 -analyzeduration 1000000 \
  -fflags +discardcorrupt+nobuffer -max_delay 0 -reorder_queue_size 0 \
  -i rtsp://HOST:554/stream \
  -f rawvideo -pix_fmt bgr24 -s 1920x1080 -an -

# re-push as a new RTSP stream (relay)
ffmpeg -rtsp_transport tcp -i rtsp://HOST:554/stream \
  -c:v copy -f rtsp rtsp://localhost:8554/relay
```

| Param | Effect |
| --- | --- |
| `-rtsp_transport tcp/udp` | transport mode (UDP low latency, TCP more stable) |
| `-probesize 500000` | limit probe bytes, speed up stream start |
| `-analyzeduration 1000000` | limit analysis duration (microseconds), speed up start |
| `-fflags +discardcorrupt+nobuffer` | drop corrupt frames, no buffering, lower latency |
| `-max_delay 0` / `-reorder_queue_size 0` | disable reorder delay |
| `-f rawvideo -pix_fmt bgr24` | output raw frames (BGR24) for per-frame reading (e.g. OpenCV) |
