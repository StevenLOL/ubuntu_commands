# GitHub access from restricted networks (HOSTS fix)

## 1. What is it / What is it for?

When `github.com` is unreachable or images/assets fail to load (common on restricted networks), mapping GitHub's hostnames to current IPs in the system `hosts` file often restores access.


- Fixing "connection reset" / "image not showing up" issues for github.com and its asset CDNs.
- A stop-gap when DNS resolution to GitHub is blocked or poisoned.

## 2. How to download / install

No software — you edit a text file.

- **Linux/macOS**: `/etc/hosts` (edit with `sudo`).
- **Windows**: `C:\Windows\System32\drivers\etc\hosts` (edit as Administrator).
- **Windows GUI tool**: *UsbEAm Hosts Editor* can auto-fill these entries.

## 3. How to use

Append entries like the following (IPs change over time — verify current ones before use):

```text
185.199.109.153 assets-cdn.github.com
185.199.109.154 github.githubassets.com
20.205.243.165 codeload.github.com
151.101.76.133 github-releases.githubusercontent.com
151.101.76.133 objects.githubusercontent.com
20.27.177.116 api.github.com
20.207.73.82 gist.github.com
151.101.76.133 raw.githubusercontent.com
151.101.76.133 avatars.githubusercontent.com
20.205.243.166 github.com
```

Save and flush DNS (`ipconfig /flushdns` on Windows, or just reopen the browser).

Minimal version (from `github_image_not_show_up.md`):
```text
140.82.114.4    github.com
140.82.114.3    github.com
140.82.112.3    github.com
199.232.96.133 raw.githubusercontent.com
```

