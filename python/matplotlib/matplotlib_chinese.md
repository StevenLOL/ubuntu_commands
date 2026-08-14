# Matplotlib — display Chinese characters

## 1. What is it?

By default Matplotlib renders Chinese text as boxes (■■) because it ships only Latin fonts. This note shows how to make Chinese labels/titles render correctly.

## 2. What is it for?

- Plotting charts with Chinese axis labels, titles, and legends.

## 3. How to download / install

1. Download a Chinese TrueType font (e.g. `simhei.ttf`).
2. Copy it into Matplotlib's font dir:
   ```python
   python -c "import matplotlib; print(matplotlib.__file__)"
   # copy simhei.ttf into <that path>/mpl-data/fonts/ttf/
   ```
3. Clear the font cache:
   ```bash
   rm -rf ~/.matplotlib/*.cache
   rm -rf ~/.cache/matplotlib/*.cache
   ```

## 4. How to use

**Method A — set the global font family:**
```python
import matplotlib.pyplot as plt
plt.rcParams['font.sans-serif'] = ['SimHei']
plt.subplots(figsize=(18, 10))
plt.title('中文')
plt.show()
```

**Method B — per-element font (explicit path):**
```python
from matplotlib.font_manager import FontProperties
font = FontProperties(fname='/home/liyang/simhei.ttf', size=10)
plt.subplots(figsize=(18, 10))
plt.title('中文', fontproperties=font)
plt.show()
```

## 5. Pitfalls

- **Cache must be cleared** after adding the font, or Matplotlib still shows boxes.
- **Font filename must match** `rcParams['font.sans-serif']` (e.g. `'SimHei'`) — a typo silently falls back to a Latin font.
- **Method B path is hardcoded** (`/home/liyang/...`) — change it to your real font location.
- Ref: https://www.zhihu.com/question/25404709
