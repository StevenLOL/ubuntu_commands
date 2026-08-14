# Tesseract OCR (with Leptonica + pytesser)

## 1. What is it / What is it for?

**Tesseract** is an open-source OCR engine; **Leptonica** is its image-I/O backend; **pytesser** is a thin Python wrapper. This note documents building Tesseract from source (the apt version lacked the `-psm` page-segmentation flag), plus common failure modes.


- Recognising printed text from images (scans, screenshots, simple CAPTCHAs).
- Batch OCR via the `tesseract` CLI or `pytesser` in Python.

## 2. How to download / install

The apt version was broken (no `-psm`), so build from source. Prereqs:
```bash
sudo apt-get install libpng12-dev libjpeg62-dev libtiff4-dev gcc g++ automake
```

Build **Leptonica** (Tesseract needs it or configure fails with "leptonica not found"):
- Download: http://www.leptonica.org/download.html
- `./configure && make -j4 && sudo make install` (use **leptonica-1.68**, not 1.69 — see pitfalls)

Build **Tesseract**:
- Download: http://code.google.com/p/tesseract-ocr/ (use a current mirror — Google Code is dead)
- `./configure && make -j4 && sudo make install`

Language data:
- Download the `tessdata` (e.g. `eng`, `chi_sim`) and place it in `/usr/local/share/tessdata/`.
- For Chinese: `chi_sim.traineddata` goes in the tessdata dir.

**pytesser** (Python wrapper):
- Download pytesser, then `from pytesser import *` and `image_to_string(im)`.

## 3. How to use

```bash
# basic OCR to out.txt
tesseract test.tif out
# specify language + page-segmentation mode
tesseract code.jpg result -l chi_sim -psm 7 nobatch
```

`-psm` values: 0=OSD only, 3=auto (default), 6=single block, 7=single line, 8=single word, 10=single char.

Whitelist digits/letters:
```bash
# edit /usr/share/tesseract-ocr/tessdata/configs/digits
tessedit_char_whitelist abcdefghijklmnopqrstuvwxyz0123456789
tesseract 1.tif out nobatch digits
```

If Tesseract only reads TIFF, convert first with ImageMagick:
```bash
sudo apt-get install imagemagick
convert -compress none -depth 8 -alpha off test.jpg test.tif
tesseract test.tif out
```

Improve accuracy (raise contrast):
```python
from PIL import Image, ImageEnhance
from pytesser import *
im = Image.open('fonts_test.png')
im = ImageEnhance.Contrast(im).enhance(4)
print image_to_string(im)
```

