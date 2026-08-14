# Hugging Face (huggingface_hub CLI + mirror)

## 1. What is it?

Hugging Face hosts ML models, datasets, and spaces. The `huggingface_hub` Python package provides `huggingface-cli` to download/upload from the Hub, and supports a mirror for users behind restricted networks.

## 2. What is it for?

- Downloading pretrained models/datasets (e.g. `gpt2`) from the Hub.
- Pushing your own models/datasets to the Hub.

## 3. How to download / install

```bash
pip install -U huggingface_hub
```

## 4. How to use

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

## 5. Pitfalls

- **`huggingface.co` blocked** on some networks → set `HF_ENDPOINT` to `https://hf-mirror.com`.
- **Large downloads**: use `--resume-download` so an interrupted pull can continue.
- **Auth**: to push, run `huggingface-cli login` with a token from your HF account settings.
- The env var only affects `huggingface_hub`/transformers; set it before running the command.
