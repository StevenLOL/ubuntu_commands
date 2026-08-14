# SoX (Sound eXchange)

## 1. What is it / What is it for?

SoX is the command-line Swiss-army knife for audio: convert formats, resample, change channels, and apply basic effects.


- Converting audio to the format ML/ASR toolkits expect (e.g. 16 kHz mono WAV).
- Batch transcoding with shell loops.

## 2. How to download / install

```bash
sudo apt-get install lame
sudo apt-get install libsox-fmt-mp3      # mp3 read/write
sudo apt-get install sox
# full format support:
sudo apt-get install libasound2-plugins libasound2-python libsox-fmt-all
```
Ref: http://quadloops.com/installing-sox-in-ubuntu-12-04-and-working-with-ogg-and-vorbis-formats/

## 3. How to use

```bash
# mp3 -> 16kHz mono wav
sox my-audio.mp3 -r 16000 -c 1 mywav.wav
```

Batch loop over a folder:
```bash
ext=mp3
for file in *.$ext; do
   sox "$file" -r 8000 -c 1 -b 16 "$(basename "$file" .$ext).wav"
done
```

Extract audio from a video container (`.mp4`) to `.wav`:
```bash
sox ./clip.mp4 -o extracted.wav
```

Convert existing WAV to mono WAV:
```bash
sox ./test01.wav -c 1 test02.wav
```

