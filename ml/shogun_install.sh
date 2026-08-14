# SHOGUN (machine learning toolbox) — install

## 1. What is it?

SHOGUN is a large open-source ML toolbox offering unified interfaces in many languages (Python, R, Java, Lua, Octave, C#, Ruby, command-line). This note lists the Ubuntu packages for each language binding.

## 2. What is it for?

- Large-scale ML (SVMs, multiple kernel learning, dimensionality reduction) from your language of choice.

## 3. How to download / install

```bash
sudo apt-get install libshogun-dev          # C++ dev headers (build extensions)
sudo apt-get install shogun-csharp-modular  # C# interface
sudo apt-get install shogun-lua-modular     # Lua interface
sudo apt-get install shogun-java-modular    # Java interface
sudo apt-get install shogun-octave-static    # static Octave interface
sudo apt-get install shogun-octave-modular  # modular Octave interface
sudo apt-get install shogun-python-static   # static Python interface
sudo apt-get install shogun-python-modular  # modular Python interface
sudo apt-get install shogun-r-static        # static R interface
sudo apt-get install shogun-ruby-modular    # Ruby interface
sudo apt-get install shogun-cmdline-static  # command-line interface
sudo apt-get install shogun-elwms-static    # "eierlegende wollmilchsau": one interface to R/Python/Octave
sudo apt-get install shogun-doc             # documentation
```

## 4. How to use

Install the binding(s) you need, then `import shogun` (Python modular) or call the relevant language API.

## 5. Pitfalls

- **Old packages**: SHOGUN's Debian/Ubuntu packages are stale; for current versions build from source or use `conda`/pip where available.
- **static vs modular**: `static` bundles everything; `modular` is the cleaner per-class API — prefer `modular`.
- Only install the bindings you'll use to avoid pulling huge dependency trees.
