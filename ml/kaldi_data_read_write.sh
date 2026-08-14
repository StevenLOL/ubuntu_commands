# Kaldi data IO (ark / scp / iVector)

## 1. What is it / What is it for?

A cheat-sheet of Kaldi commands for reading and writing **features** (MFCC, stored as `ark`/`scp`) and **i-vectors** (stored as vectors in `ark`). These are the building blocks for inspecting/transforming Kaldi data.


- Dumping Kaldi binary features to text for debugging.
- Converting between `ark` and `scp`, and between text and binary.
- Reading/writing i-vectors the same way.

## 2. How to download / install

No install — these are Kaldi commands (available after building Kaldi, see `kaldi.md`). Ensure `src/{featbin,bin}` are on `PATH`.

## 3. How to use

### Read features from ark (print to terminal)
```bash
copy-feats ark:./abc.ark ark,t:
```
### Dump features to text / binary
```bash
copy-feats ark:./abc.ark ark,t: > a.txt
copy-feats ark:./abc.ark ark,t:a.txt
copy-feats ark:./abc.ark ark:a.bin
```

### Via scp (same content)
```bash
copy-feats scp:./abc.scp ark,t:
copy-feats ark:./abc.ark ark,t:
```

### Write features + generate scp
```bash
copy-feats ark:./abc.ark ark,scp:b.ark,b.scp
```

### i-vectors (use copy-vector)
```bash
copy-vector ark:./ivector.1.ark ark,t:
copy-vector ark:./ivector.1.ark ark,t:YOUR_TEXT_FILE
# text -> kaldi ark (also emits scp)
copy-vector ark,t:./your.ark ark,t:
```

### Other helpers
```bash
copy-vector ark:vad_NIST_SRE08_3summed.model.8.ark ark,t:- > vad.ark.t.text
copy-vector ark:vad.ark.t.text ark:- > 001.ark
copy-feats scp:./data/ngmm/feats.scp ark,t:- | head
feats-to-dim ark:./input.ark ark,t:- | head -n 1
copy-vector ark:./ivector.1.ark ark,t: | sed -e 's/\[\|\]//g'
```

### Data layout
- `mfcc/` holds `.ark` (raw matrix, hundreds of MB) + `.scp` (text index `UtteranceID arkLocation:offset`).
- `feats.scp` and `vad.scp` in the data folder are feature descriptors.

