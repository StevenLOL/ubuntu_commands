# SSH reverse tunnel (reach a NAT'd machine)

## 1. What is it / What is it for?

An SSH **reverse tunnel** (`-R`) lets a machine behind NAT/firewall be reached from outside by punching a tunnel through a public server you control.


- Accessing a home/office PC (LocalPC) that has no public IP, via a public server (ServerB).
- Stable tunnels via `autossh` that survive disconnects.

## 2. How to download / install

SSH is built in. For stable tunnels install `autossh`:
```bash
sudo apt-get install autossh
```

## 3. How to use

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

