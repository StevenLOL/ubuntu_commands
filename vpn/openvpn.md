# OpenVPN (client)

## 1. What is it / What is it for?

OpenVPN is an open-source VPN protocol/client. This note covers installing the OpenVPN client on Ubuntu and connecting with a provider-supplied `.ovpn` profile.


- Connecting to an OpenVPN server (work VPN, privacy provider) from Ubuntu.
- Tunneling all traffic through the VPN.

## 2. How to download / install

```bash
sudo apt-get install openvpn
```

Obtain a `.ovpn` connection profile from your VPN provider's web dashboard.

## 3. How to use

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

