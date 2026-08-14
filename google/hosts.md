# Accessing GitHub/Google on restricted networks (hosts & reCAPTCHA)

## 1. What is it?

Notes for reaching GitHub (and Google services like reCAPTCHA) when DNS is poisoned or the sites are intermittently blocked. Two approaches: maintain a `hosts` file with current GitHub IPs, or redirect blocked Google asset URLs.

## 2. What is it for?

- Restoring access to github.com and its asset CDNs (raw, avatars, codeload, ...).
- Making reCAPTCHA load by pointing it at the alternate `recaptcha.net` domain.

## 3. How to download / install

**Option A — SwitchHosts (recommended)**
1. Download SwitchHosts: https://github.com/oldj/SwitchHosts/releases
2. Add a *remote* profile with URL: `https://cdn.jsdelivr.net/gh/521xueweihan/GitHub520@main/hosts` — it auto-pulls fresh GitHub IPs.

**Option B — edit hosts directly**
- Linux/macOS: `/etc/hosts` (with `sudo`)
- Windows: `C:\Windows\System32\drivers\etc\hosts` (as Administrator)

**Option C — browser-only (no system rights)**
Use the Firefox *Live Hosts* / *Gooreplacer* extension to inject the entries per-session.

## 4. How to use

Append GitHub entries (example snapshot; IPs change — prefer the SwitchHosts remote feed):
```text
# GitHub520 Host Start
140.82.112.3    github.com
140.82.113.4    gist.github.com
185.199.108.133 raw.githubusercontent.com
185.199.108.153 assets-cdn.github.com
185.199.108.133 avatars.githubusercontent.com
140.82.112.9    codeload.github.com
# ... (full list from the GitHub520 feed)
# GitHub520 Host End
```

**reCAPTCHA fix**: the validation script fails to load from `https://www.google.com/recaptcha/api.js`. With the Gooreplacer Firefox extension, redirect:
```
https://www.google.com/recaptcha/api.js  ->  https://recaptcha.net/recaptcha/api.js
```

## 5. Pitfalls

- **IPs rotate frequently** — a hand-edited hosts block goes stale in weeks. Use the SwitchHosts remote feed to stay current.
- **Wrong entries break everything** — bad mappings can block access entirely; comment lines out to debug.
- **hosts ≠ proxy**: it won't help if the IP itself is firewalled; then you need a proxy/VPN.
- **reCAPTCHA redirect** is a per-browser fix only; it doesn't restore other Google services.
- Refs: https://www.jianshu.com/p/7dce995dda9e · https://www.cnblogs.com/mouseleo/p/11896578.html
