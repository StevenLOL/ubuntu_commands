# Share a folder across users (Samba)

## 1. What is it?

Enabling and controlling folder sharing via **Samba** (`smb.conf`) so multiple users — and optionally guests — can access a shared directory on a Linux box.

## 2. What is it for?

- Sharing a folder with other local users or over the network (SMB/CIFS).
- Locking down guest access and per-folder permissions.

## 3. How to download / install

```bash
sudo apt-get install samba
sudo nano /etc/samba/smb.conf
```

## 4. How to use

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

## 5. Pitfalls

- **`usershare owner only = false`** lets any user create shares — a security relaxation; only enable if you trust local users.
- **Samba must be restarted** after editing `smb.conf`: `sudo systemctl restart smbd nmbd`.
- **Guest account is a LightDM concept**; on GDM it's disabled differently.
- **Firewall**: SMB uses ports 139/445 — open them for network access.
- This is a minimal Samba note, not a full file-server setup.
