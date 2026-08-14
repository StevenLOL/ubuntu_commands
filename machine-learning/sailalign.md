# SAILAlign (forced speech alignment)

## 1. What is it / What is it for?

SAILAlign is a forced-alignment toolkit that aligns a text transcript to an audio recording (word/phone-level timing), useful for building speech corpora.


- Generating time-aligned transcripts from audio + text.
- Preparing labelled data for ASR/TTS training.

## 2. How to download / install

```bash
git clone https://github.com/nassosoassos/sail_align.git
```

Then build per its README (typically CMake + dependencies like HTK/Kaldi-style tooling).

## 3. How to use

After building, run alignment with an audio file + its transcript to get word/phone timings (see the cloned repo's docs).

