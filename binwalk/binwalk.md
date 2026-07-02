```
sudo apt install -y binwalk p7zip-full cpio mtd-utils     python3 python3-pip python3-venv     build-essential git wget curl     liblzma-dev liblzo2-dev zlib1g-dev     sleuthkit lzop
sudo pip3 install ubi_reader jefferson
sudo binwalk -e -M --run-as=root YourSysFW.bin
```

