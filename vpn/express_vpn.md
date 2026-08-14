# ExpressVPN (client)

## 1. What is it?

ExpressVPN is a commercial VPN service. This note covers installing its Linux client from the official `.deb` and activating it.

## 2. What is it for?

- One-command VPN connection to ExpressVPN's servers from a Linux box.

## 3. How to download / install

Download the `.deb` for your architecture from your ExpressVPN account dashboard, then:
```bash
sudo dpkg -i ./yourdeb.deb
```

## 4. How to use

```bash
expressvpn activate          # paste the activation code from your account
expressvpn connect smart     # connect to the fastest server
expressvpn autoconnect 1     # auto-connect on boot
```

Other helpers: `expressvpn disconnect`, `expressvpn list` (servers), `expressvpn status`.

## 5. Pitfalls

- **Activation code is personal** — don't share it; it's tied to your subscription.
- **`autoconnect 1`** starts the tunnel at boot, which can block a headless server's direct access if the VPN drops — use with care.
- The `.deb` depends on `resolvconf`; if `dpkg -i` complains, run `sudo apt-get install -f`.
