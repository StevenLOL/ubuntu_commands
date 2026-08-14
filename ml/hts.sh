# HTS (HMM-based Text-to-Speech toolkit)

## 1. What is it / What is it for?

HTS (Hidden Markov Model based Text-to-Speech) is a toolkit that trains statistical speech-synthesis models on top of **HTK**. This script installs HTS patched into HTK, plus its demo.

Refs: HTS tool http://hts.sp.nitech.ac.jp/archives/2.2/HTS-2.2_for_HTK-3.4.1.tar.bz2 · demo http://hts.sp.nitech.ac.jp/archives/2.2/HTS-demo_CMU-ARCTIC-ADAPT_STRAIGHT.tar.bz2 · STRAIGHT https://www.dropbox.com/s/6v3xj08qk77y64x/STRAIGHTV40_007d.zip · SPTK http://sp-tk.sourceforge.net/


- Building a parametric TTS voice from aligned speech data.
- Research on HMM/GMM speech synthesis.

## 2. How to download / install

Prereqs (32-bit compatibility libs on 64-bit hosts):
```bash
sudo apt-get install build-essential libc6-i386 libc6-dev-i386 xorg-dev libx11-xcb-dev ia32-libs
```

Patch HTK with HTS and build:
```bash
patch -p1 -d . < HTS-2.2_for_HTK-3.4.1.patch
./configure
make all
sudo make install
make hdecode
make hlmtools
sudo make install-hdecode
sudo make install-hlmtools
```
HTS installs to `/usr/local/HTS-2.2beta/bin`; expose it:
```bash
sudo cp /usr/local/HTS-2.2beta/bin/* /usr/local/bin
```

Also install SPTK (from sourceforge) and STRAIGHT (download above).

## 3. How to use

Download the demo, extract, and run its training scripts:
```bash
tar xf HTS-demo_CMU-ARCTIC-ADAPT_STRAIGHT.tar.bz2
cd HTS-demo_CMU-ARCTIC-ADAPT_STRAIGHT
# when running, remove the "\n" in data/makefile first
```

