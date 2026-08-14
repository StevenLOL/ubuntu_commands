# skflow (TensorFlow + scikit-learn API)

## 1. What is it / What is it for?

`skflow` was a high-level scikit-learn-style wrapper for TensorFlow (the precursor to `tf.estimator`/`tf.keras`). It lets you train TF models with the familiar `fit`/`predict` interface.


- Quick ML prototyping with a sklearn-like API on top of TensorFlow.

## 2. How to download / install

```bash
pip install git+git://github.com/tensorflow/skflow.git
```

## 3. How to use

```python
import skflow
from sklearn import datasets, metrics

iris = datasets.load_iris()
classifier = skflow.TensorFlowLinearClassifier(n_classes=3)
classifier.fit(iris.data, iris.target)
score = metrics.accuracy_score(iris.target, classifier.predict(iris.data))
print("Accuracy: %f" % score)
```

