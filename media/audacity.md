# Audacity

## What

Audacity 是一款免费开源的跨平台音频录制与编辑软件，支持多轨编辑、降噪、剪辑、效果处理等，常用于录音、播客制作与简单音频后期。

## How to install

Ubuntu / Debian：

```bash
sudo apt update
sudo apt install -y audacity
```

通过 Flatpak 安装（版本通常更新）：

```bash
flatpak install flathub org.audacityteam.Audacity
```

## How to use

启动：

```bash
audacity            # 直接启动 GUI
flatpak run org.audacityteam.Audacity
```

常用操作（GUI 内）：

- 录制：点击红色录音键，选择输入设备（麦克风）。
- 导入：`文件 → 导入 → 音频`，支持 wav / mp3 / ogg 等。
- 导出：`文件 → 导出 → 导出为 WAV / MP3`。
- 降噪：选中静音片段 → `效果 → 降噪 → 取得噪声样本` → 全选 → `效果 → 降噪`。
- 剪辑：`Ctrl + I` 拆分，`Delete` 删除，拖拽移动片段。

命令行批量转换（需 `ffmpeg` 配合，Audacity 本身无完整 CLI）：

```bash
# 录制后用 ffmpeg 转码
ffmpeg -i input.wav -ar 44100 -ac 1 output.mp3
```

## Cheat-sheet

| 操作 | 快捷键 |
| --- | --- |
| 播放 / 停止 | `Space` |
| 录制 | `R` |
| 拆分剪辑 | `Ctrl + I` |
| 删除 | `Delete` |
| 全选 | `Ctrl + A` |
| 放大 / 缩小 | `Ctrl + 1` / `Ctrl + 2` |
