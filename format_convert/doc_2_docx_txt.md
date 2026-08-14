# Convert DOC / DOCX to text (unoconv)

## 1. What is it / What is it for?

`unoconv` converts between LibreOffice-supported document formats (doc, docx, odt, pdf, txt, ...) by driving a headless LibreOffice instance.


- Batch converting `.doc`/`.docx` files to plain text or other formats from the command line.
- Scripting document conversion without opening a GUI.

## 2. How to download / install

```bash
sudo apt-get install unoconv
# (pulls LibreOffice as a dependency)
```

## 3. How to use

```bash
# convert all .doc in ~/Documents to .txt
unoconv -d document --format=txt ~/Documents/*.doc
```

Ref: http://softwarerecs.stackexchange.com/questions/11687/library-for-converting-microsoft-doc-to-docx-python

