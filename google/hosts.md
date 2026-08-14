# Accessing GitHub/Google on restricted networks (hosts & reCAPTCHA)

## 1. What is it / What is it for?

Notes for reaching GitHub (and Google services like reCAPTCHA) when DNS is poisoned or the sites are intermittently blocked. Two approaches: maintain a `hosts` file with current GitHub IPs, or redirect blocked Google asset URLs.


- Restoring access to github.com and its asset CDNs (raw, avatars, codeload, ...).
- Making reCAPTCHA load by pointing it at the alternate `recaptcha.net` domain.

## 2. How to download / install

**Option A — SwitchHosts (recommended)**
1. Download SwitchHosts: https://github.com/oldj/SwitchHosts/releases
2. Add a *remote* profile with URL: `https://cdn.jsdelivr.net/gh/521xueweihan/GitHub520@main/hosts` — it auto-pulls fresh GitHub IPs.

**Option B — edit hosts directly**
- Linux/macOS: `/etc/hosts` (with `sudo`)
- Windows: `C:\Windows\System32\drivers\etc\hosts` (as Administrator)

**Option C — browser-only (no system rights)**
Use the Firefox *Live Hosts* / *Gooreplacer* extension to inject the entries per-session.

## 3. How to use

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

