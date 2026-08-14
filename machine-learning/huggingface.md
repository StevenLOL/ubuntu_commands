# Hugging Face (huggingface_hub CLI + mirror)

## 1. What is it / What is it for?

Hugging Face hosts ML models, datasets, and spaces. The `huggingface_hub` Python package provides `huggingface-cli` to download/upload from the Hub, and supports a mirror for users behind restricted networks.


- Downloading pretrained models/datasets (e.g. `gpt2`) from the Hub.
- Pushing your own models/datasets to the Hub.

## 2. How to download / install

```bash
pip install -U huggingface_hub
```

## 3. How to use

**China mirror** (avoids blocked access to huggingface.co):
```bash
# Linux
export HF_ENDPOINT=https://hf-mirror.com
# Windows PowerShell
$env:HF_ENDPOINT = "https://hf-mirror.com"
```

**Test download:**
```bash
huggingface-cli download --resume-download gpt2 --local-dir gpt2
```

