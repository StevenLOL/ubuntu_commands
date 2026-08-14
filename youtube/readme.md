# Download YouTube videos

## 1. What is it?

Two tools: **youtube-dl** (command-line downloader) and **MediaHuman YouTube to MP3/Downloader** (GUI). This note covers installing both on Ubuntu.

## 2. What is it for?

- Downloading YouTube videos/audio from the command line or a GUI.
- Converting to MP3 via MediaHuman's tools.

## 3. How to download / install

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

## 4. How to use

```bash
youtube-dl "<video_url>"                # download best quality
youtube-dl -x --audio-format mp3 "<url>" # extract MP3
```

## 5. Pitfalls

- **youtube-dl breaks often** as YouTube changes; `pip install --upgrade` frequently, or use `yt-dlp` (actively maintained fork) instead.
- **The MediaHuman apt repo key** (`apt-key adv`) is deprecated; modern apt uses signed-by keyrings. The repo may also be outdated — verify before adding.
- **Copyright**: only download content you're allowed to.
- Note: the original note's "search for youtube-dl python lib" is just `pip` above.
