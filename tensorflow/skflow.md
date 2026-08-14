# skflow (TensorFlow + scikit-learn API)

## 1. What is it?

`skflow` was a high-level scikit-learn-style wrapper for TensorFlow (the precursor to `tf.estimator`/`tf.keras`). It lets you train TF models with the familiar `fit`/`predict` interface.

## 2. What is it for?

- Quick ML prototyping with a sklearn-like API on top of TensorFlow.

## 3. How to download / install

```bash
pip install git+git://github.com/tensorflow/skflow.git
```

## 4. How to use

```python
import skflow
from sklearn import datasets, metrics

iris = datasets.load_iris()
classifier = skflow.TensorFlowLinearClassifier(n_classes=3)
classifier.fit(iris.data, iris.target)
score = metrics.accuracy_score(iris.target, classifier.predict(iris.data))
print("Accuracy: %f" % score)
```

## 5. Pitfalls

- **skflow is deprecated/abandoned** — it was folded into TensorFlow as `tf.contrib.learn` (now `tf.estimator`). Use `tf.keras` for new code.
- **Python 2 / old TF**: the `git+git://` install targets TF 0.x; it will not work with modern TF2.
- For current work, prefer `from tensorflow import keras` and `tf.keras.Sequential`.
- Ref: github.com/tensorflow/skflow
