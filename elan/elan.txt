# ELAN (linguistic annotation tool)

## 1. What is it?

ELAN is a professional tool for the creation of annotations (time-aligned tiers) on video and audio recordings, used in linguistic and gesture research.

## 2. What is it for?

- Annotating speech, gesture, and sign-language data with multiple synchronized tiers.
- Creating `.eaf` annotation files linked to media.

## 3. How to download / install

Download from https://archive.mpi.nl/tla/elan . On Linux it ships as a self-contained launcher:

```bash
./runELAN_4.6.2        # run the bundled launcher
```

Requires a JRE; install one if missing:
```bash
sudo apt install default-jre
```

## 4. How to use

Run `./runELAN_4.6.2`. Open a media file, create tiers, and add time-aligned annotations; save as `.eaf`.

## 5. Pitfalls

- **Must run as the user who owns the media path**; running from root can break file permissions.
- **Launcher name changes per version** (`runELAN_4.6.2`); check the actual filename in the download.
- **Large media files** need enough RAM; ELAN loads the whole file into memory.
