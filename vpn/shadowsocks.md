# Shadowsocks (SOCKS5 proxy)

## 1. What is it / What is it for?

Shadowsocks is a lightweight SOCKS5 proxy (written in Python/C). This note covers running the client (`sslocal`) on Ubuntu and pointing a browser at it via SwitchyOmega, plus the server side.

> Note: this content summarizes the ArchWiki Shadowsocks page; the original note had a large pasted Chinese wiki dump, condensed here.


- Routing browser traffic through a remote SOCKS5 proxy.
- A simple personal proxy (client + your own server).

## 2. How to download / install

```bash
sudo apt install shadowsocks
```

Browser helper: [SwitchyOmega](https://github.com/FelisCatus/SwitchyOmega/releases) (Chrome/Chromium) for switching proxies.

## 3. How to use

**Client config** `/etc/shadowsocks/config.json`:
```json
{
  "server":"remote-shadowsocks-server-ip-addr",
  "server_port":443,
  "local_address":"127.0.0.1",
  "local_port":1080,
  "password":"your-passwd",
  "timeout":300,
  "method":"aes-256-cfb"
}
```
Run:
```bash
sslocal -c /etc/shadowsocks/config.json
# or inline:
sslocal -s <server> -p <server_port> -l 1080 -k <password> -m aes-256-cfb
# background: nohup sslocal -c config.json &
```

Point SwitchyOmega at `127.0.0.1:1080` (SOCKS5) and enable the proxy.

**Server side** (only if you run your own):
```bash
ssserver -c /etc/shadowsocks/config.json      # or: nohup ssserver > log &
```

**systemd:**
```bash
systemctl start shadowsocks@foo     # client, using /etc/shadowsocks/foo.json
systemctl enable shadowsocks@foo    # autostart
journalctl -u shadowsocks@foo       # logs
```

**SOCKS→HTTP** (so apps without SOCKS support work), via privoxy in `/etc/privoxy/config`:
```text
forward-socks5 / 127.0.0.1:1080 .
listen-address 127.0.0.1:8118
```
then `chromium --proxy-server=127.0.0.1:8118`.

