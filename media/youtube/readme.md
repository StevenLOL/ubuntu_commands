# Download YouTube videos

## 1. What is it / What is it for?

Two tools: **youtube-dl** (command-line downloader) and **MediaHuman YouTube to MP3/Downloader** (GUI). This note covers installing both on Ubuntu.


- Downloading YouTube videos/audio from the command line or a GUI.
- Converting to MP3 via MediaHuman's tools.

## 2. How to download / install

**youtube-dl:**
```bash
sudo pip install --upgrade youtube_dl
```
(Or the static binary from http://rg3.github.io/youtube-dl/ .)

**MediaHuman (GUI):**
```bash
sudo add-apt-repository 'http://www.mediahuman.com/packages/ubuntu'
sudo apt-key adv --keyserver pgp.mit.edu --recv-keys D808832C7D19F1F3
sudo apt-get update
sudo apt-get install youtube-to-mp3      # YouTube to MP3 Converter
sudo apt-get install youtube-downloader  # YouTube Downloader
```

## 3. How to use

```bash
youtube-dl "<video_url>"                # download best quality
youtube-dl -x --audio-format mp3 "<url>" # extract MP3
```

