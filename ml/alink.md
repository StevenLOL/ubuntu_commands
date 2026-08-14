# PyAlink (Alibaba's Alink ML library for Flink)

## 1. What is it / What is it for?

PyAlink is the Python interface to **Alink**, Alibaba's machine-learning library built on top of **Apache Flink**. It provides batch/streaming ML algorithms (classification, regression, clustering, feature engineering) that run on a Flink cluster or locally.

Ref: https://github.com/alibaba/Alink


- Running scalable ML pipelines (training + prediction) on Flink.
- Using Alink algorithms from Python (pandas/sklearn-like API).

## 2. How to download / install

Requirements:
- Python 3.6 or 3.7
- Java 8

```bash
pip install pyalink            # latest Flink version supported by Alink
# or a pinned older Flink:
pip install pyalink-flink-1.9
```

If pip is slow, switch the pip index or download the wheel directly:
- Flink 1.10 wheel (MD5 `6bf3a50a4437116793149ead57d9793c`)
- Flink 1.9  wheel (MD5 `e6d2a0ba3549662d77b51a4a37483479`)

## 3. How to use

```python
from pyalink.alink import *
# start a local Flink session, then build a pipeline
```

With Anaconda, run `pip install` from the **Anaconda prompt**, and use the matching `pip`/`pip3` if you have several Pythons.

