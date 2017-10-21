
REF:

http://www.cnblogs.com/makefile/p/5410722.html

LocalPC <<=================>> Public Server(ServerB)


From LocalPC:
```
ssh -f -N -R 30012:localhost:22 steven@ServerB
```
To login to LocalPC,

1. From ServerB:
```
ssh -p 30015 localhost
```
2. Through public network
```
ssh -p 30015 ServerB
```
## FAQ

If "Connection refused", add following to /et/ssh/sshd_config @ ServerB

```
GatewayPorts yes
```
To check if a port is open:
```
nmap -p 30015 ServerB
```

Use autossh to build a stable tunnel
```
autossh -M 9000 -f -N -R 30012:localhost:22 steven@ServerB
```
