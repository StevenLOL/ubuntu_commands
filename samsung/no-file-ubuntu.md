# Access Samsung Galaxy files on Ubuntu (MTP)

## 1. What is it / What is it for?

On Ubuntu, a Samsung Galaxy phone shows up as an MTP device, not a plain USB drive. This note enables MTP access via the `gvfs-mtp` backend.


- Browsing/copying files to and from a Samsung Galaxy (S2, etc.) over USB.

## 2. How to download / install

```bash
sudo add-apt-repository ppa:langdalepl/gvfs-mtp
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install gvfs
```

Ref: http://askubuntu.com/questions/88552/can-not-access-files-in-any-folder-on-samsung-galaxy-sii

## 3. How to use

Connect the phone via USB, set it to "Media device (MTP)" mode, and it should appear in the file manager.

