# Kaldi (speech recognition toolkit)

## 1. What is it?

Kaldi is the leading open-source toolkit for **automatic speech recognition (ASR)** and speaker recognition. It compiles to many small command-line binaries (`src/bin`, `src/featbin`, `src/nnet2bin`, `ivectorbin`, ...).

## 2. What is it for?

- Building ASR / speaker-ID pipelines (feature extraction → monophone → triphone → DNN).
- Research on acoustic modelling (GMM-HMM, nnet2/nnet3, i-vectors).

## 3. How to download / install

Check out the trunk (SVN) and build:
```bash
svn cleanup kaldi-trunk
svn co https://svn.code.sf.net/p/kaldi/code/trunk kaldi-trunk
cd kaldi-trunk/tools
make -j 4
cd ../src
sudo apt-get install libatlas-dev libatlas-base-dev
./configure
make depend -j 10
make -j 10
```

## 4. How to use

Add the built binaries to your `PATH` (in `~/.profile` or `~/.bashrc`):
```bash
PATH=<YOUR_HOME>/kaldi-trunk/src/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/bin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/fgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/gmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/kwsbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/nnet2bin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/onlinebin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/featbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/fstbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/ivectorbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/latbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/nnetbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmm2bin:$PATH
```

If you hit `fatal error: clapack.h`:
```bash
sudo apt-get install libatlas-base-dev
sudo apt-get --purge remove liblapack-dev liblapack3 liblapack3gf
export CPLUS_INCLUDE_PATH=/usr/include/atlas
```

## 5. Pitfalls

- **Replace `<YOUR_HOME>`** with your actual path; the snippet above is a template.
- **ATLAS/LAPACK conflict**: the `clapack.h` error is fixed by installing `libatlas-base-dev` and removing the conflicting `liblapack-dev`.
- **Old SVN trunk**: Kaldi moved to GitHub; prefer `git clone https://github.com/kaldi-asr/kaldi.git` for current code.
- **`make -j` with too many jobs** can OOM on small machines; lower the `-j` count.
- **`onlinebin`** requires PortAudio; build it via `tools/install_portaudio.sh` first (see `kaldi_install_LVCSR_Demo`).
