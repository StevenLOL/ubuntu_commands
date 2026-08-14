# binwalk (firmware analysis)

## 1. What is it?

`binwalk` is a tool for analyzing, reverse-engineering, and extracting firmware images and arbitrary binary blobs. It scans a file for known magic signatures (filesystems, compressed streams, bootloaders, etc.) and can recursively extract embedded content.

## 2. What is it for?

- Identifying what is inside a firmware dump (e.g. `uImage`, `squashfs`, `jffs2`, `crypto` blocks).
- Extracting the filesystem / compressed payloads from a router or IoT device image.
- Forensic analysis of unknown binaries.

## 3. How to download / install

```bash
sudo apt install -y binwalk p7zip-full cpio mtd-utils \
    python3 python3-pip python3-venv build-essential git wget curl \
    liblzma-dev liblzo2-dev zlib1g-dev sleuthkit lzop

# Python-based extractors binwalk relies on:
sudo pip3 install ubi_reader jefferson
```

## 4. How to use

### Scan signatures only
```bash
binwalk YourSysFW.bin
```

### Extract (recursive) as root
```bash
sudo binwalk -e -M --run-as=root YourSysFW.bin
```
- `-e` : extract known signatures
- `-M` : recursively scan extracted files
- `--run-as=root` : needed because some extractors (squashfs/jffs2) require root

Output lands in `_YourSysFW.bin.extracted/`.

## 5. Pitfalls

- **Run as root.** Several extractors (`squashfs`, `jffs2`, `ubi`) fail or hang without root.
- **Missing extractor = silent skip.** If `ubi_reader` / `jefferson` aren't installed, those filesystems won't be extracted. Install them via pip.
- **`-M` can explode** on pathological inputs (huge recursion). Review output size.
- **Encrypted blocks** won't extract — binwalk only finds signatures, it does not decrypt.
- On newer Ubuntu, prefer installing binwalk via `pipx`/`pip` (the apt version may lag).
