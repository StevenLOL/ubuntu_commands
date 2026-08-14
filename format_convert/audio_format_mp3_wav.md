# Audio format conversion (mp3 / wav) with sox

## 1. What is it / What is it for?

`sox` (Sound eXchange) is the Swiss-army knife for audio file conversion and basic editing on the command line. This note covers converting mp3/video to wav at a target sample rate / channel count.


- Converting speech/audio into the 16 kHz mono WAV that ASR / ML toolkits expect.
- Extracting audio from a video container (`.mp4`) to `.wav`.

## 2. How to download / install

```bash
sudo apt-get install sox libsox-fmt-all    # libsox-fmt-all adds mp3/flac support
```

## 3. How to use

```bash
# mp3 -> 16kHz mono wav
sox ./002.mp3 -r 16000 002.16000.wav -c 1

# extract audio from mp4 -> wav
sox ./1322500_10151667049358163_56401_n.mp4 -o test01.wav

# wav -> mono wav
sox ./test01.wav -c 1 test02.wav
```

