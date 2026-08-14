# WSL2 (Windows Subsystem for Linux)

## 1. What is it?

WSL2 runs a real Linux kernel inside Windows 10/11. This note covers installing Ubuntu on WSL2 and putting it in **mirrored network mode** so it shares the Windows IP (acts like a bridged adapter).

## 2. What is it for?

- Running Ubuntu (bash, apt, Docker, dev tools) directly on Windows.
- Making WSL2 services reachable from the LAN (same IP segment as Windows).

## 3. How to download / install

In PowerShell (Admin):
```powershell
wsl --list --online                 # see available distros
wsl --install -d Ubuntu-22.04       # install Ubuntu
wsl -d Ubuntu-22.04                 # run it
wsl -d Ubuntu-22.04 -u <USER>       # run as a specific user
wsl --shutdown                      # stop WSL
```

## 4. How to use

**Mirrored networking (recommended, Win 11 22H2+):** WSL2 defaults to NAT; mirrored mode makes WSL share Windows's IP so LAN devices reach WSL services directly.

1. Edit/create `.wslconfig` in your Windows user profile:
   ```powershell
   notepad "$env:USERPROFILE\.wslconfig"
   ```
2. Write:
   ```ini
   [wsl2]
   networkingMode=mirrored
   dnsTunneling=true
   firewall=true
   autoProxy=true
   ```
3. Restart:
   ```powershell
   wsl --shutdown
   wsl
   ```
4. Verify inside WSL: `ip addr` should show an IP on the same segment as Windows.

## 5. Pitfalls

- **Mirrored mode needs Windows 11 22H2+**; on older builds use `networkingMode=bridged` with a vEthernet bridge, or accept NAT.
- **`.wslconfig` lives in the Windows profile**, not in WSL's filesystem.
- **`wsl-d` typo**: the correct flag is `wsl -d` (space), not `wsl-d`.
- After changing `.wslconfig` you must `wsl --shutdown` (not just restart) for it to apply.
