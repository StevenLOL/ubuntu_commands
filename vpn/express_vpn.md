# ExpressVPN (client)

## 1. What is it / What is it for?

ExpressVPN is a commercial VPN service. This note covers installing its Linux client from the official `.deb` and activating it.


- One-command VPN connection to ExpressVPN's servers from a Linux box.

## 2. How to download / install

Download the `.deb` for your architecture from your ExpressVPN account dashboard, then:
```bash
sudo dpkg -i ./yourdeb.deb
```

## 3. How to use

```bash
expressvpn activate          # paste the activation code from your account
expressvpn connect smart     # connect to the fastest server
expressvpn autoconnect 1     # auto-connect on boot
```

Other helpers: `expressvpn disconnect`, `expressvpn list` (servers), `expressvpn status`.

