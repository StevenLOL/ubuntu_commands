# LaTeX (Chinese support) on Ubuntu

## 1. What is it / What is it for?

LaTeX is a document-preparation system; this note covers installing a TeX distribution with **Chinese language support** and the LaTeXStudio editor on Ubuntu.


- Writing papers/reports (including Chinese text) with TeX/LaTeX.
- A GUI editor (LaTeXStudio) for compiling and previewing `.tex` files.

## 2. How to download / install

```bash
sudo apt-get install texlive-lang-chinese     # CJK fonts + packages
sudo apt-get install latexstudio              # editor (if available in your release)
# broader install if needed:
# sudo apt-get install texlive-full
```

## 3. How to use

Write a `.tex` file and compile:
```bash
xelatex mydoc.tex        # best for Chinese (uses system fonts via fontspec)
# or: pdflatex mydoc.tex
```
Open the project in LaTeXStudio to build and preview.

