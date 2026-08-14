# OpenCV (computer vision library)

## 1. What is it?

OpenCV is the most widely used open-source computer-vision library (C++ core with Python/Java bindings). This note covers quick pip/apt installs and links to a from-source build script.

## 2. What is it for?

- Image processing, video capture, feature extraction, and ML-based vision.
- Prototyping CV pipelines in Python (`cv2`).

## 3. How to download / install

```bash
# OpenCV 2.4 (old) via apt
sudo apt install python-opencv

# OpenCV 3.2+ via pip (recommended)
sudo pip install opencv-python
# headless (no GUI) server build:
sudo pip install opencv-python-headless
```

To build from source (full control), use the script collection:
https://github.com/jayrambhia/Install-OpenCV

## 4. How to use

```python
import cv2
img = cv2.imread('photo.jpg')
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
cv2.imwrite('gray.jpg', gray)
```

## 5. Pitfalls

- **`python-opencv` (apt) is ancient** (2.4) — prefer `opencv-python` from pip for anything modern.
- **`cv2` not found after pip install**: install into the interpreter you actually run (use `python -m pip install`).
- **GUI error on headless servers**: `cv2.imshow` fails without X11; use `opencv-python-headless` and avoid `imshow`/`waitKey` in scripts.
- **Version vs CUDA**: GPU-accelerated `cv2.cuda` requires a custom build; pip wheels are CPU-only.
