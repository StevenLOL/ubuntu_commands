# PDF tools (QPDF & PyPDF2)

## 1. What is it?

- **QPDF**: a command-line tool to transform/inspect PDFs, including removing print/edit restrictions (decrypt).
- **PyPDF2**: a pure-Python library to read, write, split, merge, and encrypt PDFs programmatically.

## 2. What is it for?

- Stripping password/permission restrictions from a PDF you own.
- Programmatic PDF manipulation (encrypt, merge, rotate) in Python.

## 3. How to download / install

**QPDF**
```powershell
# Windows
winget install QPDF.QPDF
# restart terminal afterwards to refresh PATH
```
```bash
# Ubuntu / Debian
sudo apt update && sudo apt install -y qpdf
```

**PyPDF2**
```bash
pip install pypdf2
```
Docs: https://pythonhosted.org/PyPDF2/

## 4. How to use

**QPDF — remove restrictions**
```bash
# no document password
qpdf --decrypt input.pdf output_unlocked.pdf
# with a known password
qpdf --password=your_password --decrypt input.pdf output_unlocked.pdf
```

**PyPDF2 — add encryption**
```python
from PyPDF2 import PdfFileWriter, PdfFileReader

def add_encryption(input_pdf, output_pdf, password):
    pdf_writer = PdfFileWriter()
    pdf_reader = PdfFileReader(input_pdf)
    for page in range(pdf_reader.getNumPages()):
        pdf_writer.addPage(pdf_reader.getPage(page))
    pdf_writer.encrypt(user_pwd=password, owner_pwd=None, use_128bit=True)
    with open(output_pdf, 'wb') as fh:
        pdf_writer.write(fh)

if __name__ == '__main__':
    add_encryption(input_pdf='badao.pdf',
                   output_pdf='badadoencrypted.pdf',
                   password='badao')
```

## 5. Pitfalls

- **`--decrypt` only removes restrictions you can already open**; it cannot crack an unknown owner password on a properly encrypted PDF.
- **PyPDF2 old API**: `PdfFileWriter`/`getNumPages` are from PyPDF2 ≤1.x; newer `pypdf` (v3+) renamed these to `PdfWriter`/`len(reader.pages)`. Adjust imports on modern installs.
- **Bug: default encoding not UTF-8** — an old PyPDF2 had a non-UTF-8 default at line 238 of `PyPDF2/utils.py` (Python 3.8 era). Upgrade PyPDF2/`pypdf` to avoid it.
- **QPDF vs qpdf**: command is lowercase `qpdf`.
