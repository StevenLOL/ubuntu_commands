# Kaggle API (CLI)

## 1. What is it?

The Kaggle CLI is a Python tool to interact with kaggle.com — download datasets/competition files, submit predictions, and manage your account from the terminal.

## 2. What is it for?

- Downloading datasets and competition data without clicking through the website.
- Scripting bulk data pulls and submissions.

## 3. How to download / install

```bash
pip install kaggle
```

Get your API token:
1. Log in at https://www.kaggle.com/account
2. Click **"Create New API Token"** → downloads `kaggle.json`

Place it where the CLI looks:
```bash
mkdir -p ~/.kaggle
mv ~/Downloads/kaggle.json ~/.kaggle/
chmod 600 ~/.kaggle/kaggle.json     # must not be world-readable
```

## 4. How to use

```bash
kaggle datasets list
kaggle datasets download -d <owner/dataset> -p ./data
kaggle competitions download -c <competition> -p ./data
kaggle competitions submit -c <competition> -f submission.csv -m "my message"
```

## 5. Pitfalls

- **`kaggle.json` permissions**: if it's group/other-readable, the CLI refuses with a 403/"API token permissions". `chmod 600` it.
- **Competition rules**: some competitions require you to accept the rules on the website before `download`/`submit` works.
- **`~/.kaggle` path** is hard-coded; putting the file elsewhere won't be found.
- **Network**: kaggle.com may be blocked on restricted networks — use a mirror/proxy.
