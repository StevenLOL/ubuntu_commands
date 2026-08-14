# Markdown tips

## 1. What is it / What is it for?

Markdown is a lightweight markup language for formatted text. This note covers basic usage and **Typora's sequence-diagram** extension.


- Writing formatted docs (README, notes) in plain text.
- Rendering UML-style sequence diagrams inside Typora.

## 2. How to download / install

Markdown needs no install (it's a syntax). For sequence diagrams, use **Typora** (https://typora.io/) with its built-in support. Basic usage ref: https://www.jianshu.com/p/307a13c79fe4

## 3. How to use

```sequence
title: MarkDown sequence
participant finefine as ff
participant kunkun as kk
ff-->kk: this is kunkun?
kk-->ff: yes!
```

Typora renders the `sequence` block as a diagram.

