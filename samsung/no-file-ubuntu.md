# Access Samsung Galaxy files on Ubuntu (MTP)

## 1. What is it?

On Ubuntu, a Samsung Galaxy phone shows up as an MTP device, not a plain USB drive. This note enables MTP access via the `gvfs-mtp` backend.

## 2. What is it for?

- Browsing/copying files to and from a Samsung Galaxy (S2, etc.) over USB.

## 3. How to download / install

```bash
sudo add-apt-repository ppa:langdalepl/gvfs-mtp
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install gvfs
```

Ref: http://askubuntu.com/questions/88552/can-not-access-files-in-any-folder-on-samsung-galaxy-sii

## 4. How to use

Connect the phone via USB, set it to "Media device (MTP)" mode, and it should appear in the file manager.

## 5. Pitfalls

- **Old PPA**: `langdalepl/gvfs-mtp` was for ancient Ubuntu; modern GNOME ships MTP support via `gvfs-backends` (`sudo apt install gvfs-backends`) — use that instead.
- **Phone must be in MTP mode**, not "Charge only" or "Camera (PTP)" — otherwise it won't mount.
- **Unlock the phone** and accept the USB connection prompt; locked phones won't expose storage.
