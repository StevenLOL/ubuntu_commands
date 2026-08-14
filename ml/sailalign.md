# SAILAlign (forced speech alignment)

## 1. What is it?

SAILAlign is a forced-alignment toolkit that aligns a text transcript to an audio recording (word/phone-level timing), useful for building speech corpora.

## 2. What is it for?

- Generating time-aligned transcripts from audio + text.
- Preparing labelled data for ASR/TTS training.

## 3. How to download / install

```bash
git clone https://github.com/nassosoassos/sail_align.git
```

Then build per its README (typically CMake + dependencies like HTK/Kaldi-style tooling).

## 4. How to use

After building, run alignment with an audio file + its transcript to get word/phone timings (see the cloned repo's docs).

## 5. Pitfalls

- **Old/unmaintained repo** — may need patching for modern compilers.
- **Heavy deps**: forced alignment usually requires a working ASR backend (e.g. HTK) already installed.
- Verify the upstream README for the exact build steps; this note only captures the clone command.
