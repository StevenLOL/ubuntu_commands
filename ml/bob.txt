# Bob (signal-processing / biometric toolkit)

## 1. What is it?

Bob is a Python/C++ signal-processing and machine-learning toolkit from Idiap, used for biometrics (face, speaker, vein recognition) and speech/signal research.

## 2. What is it for?

- Building biometric recognition pipelines (feature extraction, scoring).
- Speech/signal processing helpers (e.g. `xbob.sox` wraps SoX for audio IO).

## 3. How to download / install

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

## 4. How to use

Import Bob modules in Python (e.g. `import bob.io.audio`) after install.

## 5. Pitfalls

- **Bob 1.x is very old** — modern Bob is split into separate packages (`bob.io`, `bob.learn`, ...). Source builds need a matching numpy/C++ toolchain.
- **`xbob.sox`** depends on SoX being installed system-wide.
- Build failures usually trace back to a mismatched numpy/C++ ABI — build against the numpy you'll run with.
