# Share a folder across users (Samba)

## 1. What is it / What is it for?

Enabling and controlling folder sharing via **Samba** (`smb.conf`) so multiple users — and optionally guests — can access a shared directory on a Linux box.


- Sharing a folder with other local users or over the network (SMB/CIFS).
- Locking down guest access and per-folder permissions.

## 2. How to download / install

```bash
sudo apt-get install samba
sudo nano /etc/samba/smb.conf
```

## 3. How to use

**Allow non-owner users to create usershares** — in the `[global]` section of `smb.conf`:
```ini
usershare owner only = false
```

**Disable the guest account** (LightDM):
```bash
sudo nano /etc/lightdm/lightdm.conf
# add:
allow-guest=false
```

**Prevent standard users from seeing your folder**: right-click the folder → Properties → Permissions, and set "Others" to "None".

