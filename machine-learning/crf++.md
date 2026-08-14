# CRF++ (conditional random field toolkit)

## 1. What is it / What is it for?

CRF++ is a simple, open-source implementation of **Conditional Random Fields** (CRF) for sequence labelling, widely used for NLP tasks like Chinese word segmentation and NER.


- Training and applying CRF models on labelled sequences (e.g. tag tokens).
- Command-line tools: `crf_learn` (train), `crf_test` (predict).

## 2. How to download / install

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

## 3. How to use

```bash
crf_learn template train.txt model      # train
crf_test  -m model test.txt             # predict
```

