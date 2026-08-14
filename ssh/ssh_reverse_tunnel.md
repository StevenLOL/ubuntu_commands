# SSH reverse tunnel (reach a NAT'd machine)

## 1. What is it?

An SSH **reverse tunnel** (`-R`) lets a machine behind NAT/firewall be reached from outside by punching a tunnel through a public server you control.

## 2. What is it for?

- Accessing a home/office PC (LocalPC) that has no public IP, via a public server (ServerB).
- Stable tunnels via `autossh` that survive disconnects.

## 3. How to download / install

SSH is built in. For stable tunnels install `autossh`:
```bash
sudo apt-get install autossh
```

## 4. How to use

From LocalPC, open a reverse tunnel to ServerB:
```bash
ssh -f -N -R 30012:localhost:22 <USER>@<HOSTNAME>
```
Then to log into LocalPC:
```bash
# from ServerB:
ssh -p 30012 localhost
# or from anywhere through ServerB:
ssh -p 30012 ServerB
```

Make it stable with autossh:
```bash
autossh -M 9000 -f -N -R 30012:localhost:22 <USER>@<HOSTNAME>
```

## 5. Pitfalls

- **`Connection refused` on the forwarded port** → add `GatewayPorts yes` to `/etc/ssh/sshd_config` on ServerB and restart sshd.
- **Check the port is open**: `nmap -p 30012 ServerB`.
- **`GatewayPorts` typo**: the original note wrote `/et/ssh/sshd_config` — it's `/etc/ssh/sshd_config`.
- **`-R` binds to localhost by default**; `GatewayPorts yes` is what lets external clients use the tunnel.
- **Reverse tunnel ≠ VPN**: it only forwards the one chosen port.
