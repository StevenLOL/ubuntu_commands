# Everpad (Evernote client for Linux)

## 1. What is it?

Everpad is an unofficial Evernote desktop client for Linux, integrating notes into the Unity indicator and syncing with Evernote.

## 2. What is it for?

- Taking and syncing Evernote notes from a Linux desktop without a browser.

## 3. How to download / install

```bash
sudo add-apt-repository ppa:nvbn-rm/ppa
sudo apt-get update && sudo apt-get install everpad
```

## 4. How to use

Launch Everpad from the application menu / indicator; sign in with your Evernote account to sync.

## 5. Pitfalls

- **Unmaintained**: the PPA (`nvbn-rm`) is old and may not build on recent Ubuntu releases.
- **Evernote API limits** can throttle sync; large notebooks may be slow.
- On modern Ubuntu the PPA may be 404 — consider the web client instead.
