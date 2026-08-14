# Convert DOC / DOCX to text (unoconv)

## 1. What is it?

`unoconv` converts between LibreOffice-supported document formats (doc, docx, odt, pdf, txt, ...) by driving a headless LibreOffice instance.

## 2. What is it for?

- Batch converting `.doc`/`.docx` files to plain text or other formats from the command line.
- Scripting document conversion without opening a GUI.

## 3. How to download / install

```bash
sudo apt-get install unoconv
# (pulls LibreOffice as a dependency)
```

## 4. How to use

```bash
# convert all .doc in ~/Documents to .txt
unoconv -d document --format=txt ~/Documents/*.doc
```

Ref: http://softwarerecs.stackexchange.com/questions/11687/library-for-converting-microsoft-doc-to-docx-python

## 5. Pitfalls

- **LibreOffice must be installed** (unoconv is just a front-end).
- **Headless conflict**: a running LibreOffice GUI can lock the conversion socket (`/tmp/OSL_PIPE_*`); close LibreOffice first.
- **Wildcards**: `~/Documents/*.doc` only expands for existing files; use exact paths in scripts.
- **Format name**: it's `--format=txt`, not `--format=text`.
