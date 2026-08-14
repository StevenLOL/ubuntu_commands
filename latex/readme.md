# LaTeX (Chinese support) on Ubuntu

## 1. What is it?

LaTeX is a document-preparation system; this note covers installing a TeX distribution with **Chinese language support** and the LaTeXStudio editor on Ubuntu.

## 2. What is it for?

- Writing papers/reports (including Chinese text) with TeX/LaTeX.
- A GUI editor (LaTeXStudio) for compiling and previewing `.tex` files.

## 3. How to download / install

```bash
sudo apt-get install texlive-lang-chinese     # CJK fonts + packages
sudo apt-get install latexstudio              # editor (if available in your release)
# broader install if needed:
# sudo apt-get install texlive-full
```

## 4. How to use

Write a `.tex` file and compile:
```bash
xelatex mydoc.tex        # best for Chinese (uses system fonts via fontspec)
# or: pdflatex mydoc.tex
```
Open the project in LaTeXStudio to build and preview.

## 5. Pitfalls

- **`latexstudio` may not be in default repos** — it's often installed via a `.deb` from the project site; `apt` may 404.
- **Chinese needs XeLaTeX/LuaLaTeX + a CJK font**; bare `pdflatex` without CJK packages renders gibberish.
- **`texlive-full` is huge** (several GB); install `texlive-lang-chinese` + the specific packages you need to save space.
- Use `\usepackage{ctex}` (or `ctexart` document class) for Chinese.
