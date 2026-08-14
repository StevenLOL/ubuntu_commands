# CRF++ (conditional random field toolkit)

## 1. What is it?

CRF++ is a simple, open-source implementation of **Conditional Random Fields** (CRF) for sequence labelling, widely used for NLP tasks like Chinese word segmentation and NER.

## 2. What is it for?

- Training and applying CRF models on labelled sequences (e.g. tag tokens).
- Command-line tools: `crf_learn` (train), `crf_test` (predict).

## 3. How to download / install

Download:
```
http://code.google.com/p/crfpp/downloads/detail?name=CRF%2B%2B-0.58.tar.gz&can=2&q=
```
(Prefer a maintained mirror — Google Code is long dead.)

Build & install:
```bash
./configure
make
sudo make install
```

## 4. How to use

```bash
crf_learn template train.txt model      # train
crf_test  -m model test.txt             # predict
```

## 5. Pitfalls

- **`libcrfpp.so.0: cannot open shared object file`**: the library isn't on the loader path. Fix:
  ```bash
  echo '/usr/local/lib' | sudo tee /etc/ld.so.conf.d/crfpp.conf
  sudo ldconfig
  ```
- **Google Code is gone** — find CRF++ on a current mirror (e.g. archived copies) since the original link 404s.
- **32/64-bit**: ensure you build on the same arch you run on.
