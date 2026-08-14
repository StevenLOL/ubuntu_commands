# Bob (signal-processing / biometric toolkit)

## 1. What is it / What is it for?

Bob is a Python/C++ signal-processing and machine-learning toolkit from Idiap, used for biometrics (face, speaker, vein recognition) and speech/signal research.


- Building biometric recognition pipelines (feature extraction, scoring).
- Speech/signal processing helpers (e.g. `xbob.sox` wraps SoX for audio IO).

## 2. How to download / install

Build Bob 1.2 from source:
```bash
# in the bob source tree
cd python
python setup.py install
# audio support add-on:
pip install xbob.sox
```

Also install the scientific stack:
```bash
pip install scipy numpy
```

## 3. How to use

Import Bob modules in Python (e.g. `import bob.io.audio`) after install.

