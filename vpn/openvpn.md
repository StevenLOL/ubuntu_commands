# OpenVPN (client)

## 1. What is it?

OpenVPN is an open-source VPN protocol/client. This note covers installing the OpenVPN client on Ubuntu and connecting with a provider-supplied `.ovpn` profile.

## 2. What is it for?

- Connecting to an OpenVPN server (work VPN, privacy provider) from Ubuntu.
- Tunneling all traffic through the VPN.

## 3. How to download / install

```bash
sudo apt-get install openvpn
```

Obtain a `.ovpn` connection profile from your VPN provider's web dashboard.

## 4. How to use

Connect (foreground, to see logs):
```bash
sudo openvpn ~/client.ovpn
```

Run in the background and auto-start at boot:
```bash
openvpn /etc/openvpn/client.ovpn > /dev/null &
# add the line to /etc/rc.local to start on boot
```

Verify your new public IP at https://www.whatismyip.com/

## 5. Pitfalls

- **`sudo` required** — OpenVPN needs root to create the tunnel interface.
- **`/etc/rc.local` is deprecated** on systemd Ubuntu; for boot-start, use a systemd service or `systemctl enable openvpn-client@<name>`.
- **Profile auth**: some `.ovpn` files need a username/password; embed creds carefully or use `--auth-user-pass`.
- Ref: linuxidc.com OpenVPN tutorial.
