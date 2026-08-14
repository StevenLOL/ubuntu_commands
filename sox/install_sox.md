# SoX (Sound eXchange)

## 1. What is it?

SoX is the command-line Swiss-army knife for audio: convert formats, resample, change channels, and apply basic effects.

## 2. What is it for?

- Converting audio to the format ML/ASR toolkits expect (e.g. 16 kHz mono WAV).
- Batch transcoding with shell loops.

## 3. How to download / install

```bash
sudo apt-get install lame
sudo apt-get install libsox-fmt-mp3      # mp3 read/write
sudo apt-get install sox
# full format support:
sudo apt-get install libasound2-plugins libasound2-python libsox-fmt-all
```
Ref: http://quadloops.com/installing-sox-in-ubuntu-12-04-and-working-with-ogg-and-vorbis-formats/

## 4. How to use

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

## 5. Pitfalls

- **No MP3 support without `libsox-fmt-mp3`** — bare `sox` can't read mp3 and errors out.
- **Option order**: put `-r/-c/-b` after the input and before the output.
- **8 kHz / 16-bit**: fine for telephony ASR, but too low for music; choose rates per use case.
- The original note's `libsox-fmt-mp3` is the key package; don't skip it.
