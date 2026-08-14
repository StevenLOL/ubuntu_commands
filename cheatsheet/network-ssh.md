# Network & SSH cheatsheet

Quick reference for SSH, network scanning, load testing, parallel commands, job queues, and text-mode browsers.

## SSH
```
ssh <USER>@<SERVER_IP>
ssh -l 23 <USER>@<SERVER_IP>      # login via port 23
```

## Ping / scan hosts on the LAN
```
for x in {11..255};do ssh <SERVER_IP_PREFIX>.$x; done;
# scan hosts:
arp
sudo nmap -sn <SERVER_IP>/24 > readme.txt
avahi-browse -a -v -r -t -d local   # sudo apt-get install avahi-discover
```

## Apache ab load test
```
apt-get install apache2-utils
ab -n 10000 -c 100 https://www.baidu.com/
```

## Run commands in parallel
```
some_commands &
# with wait:
a&
b&
wait

lspci | grep Intel    # pipe two commands at once
```

## Work with a job queue (SGE)
```
qstat -u "*"          # check queue state
qdel job-ID
qsub hello_world.sh &
```

## Open a webpage from the terminal
```
links www.zaobao.com
lynx
w3m
# These text browsers don't support programmed reloading by themselves; use a multiplexer like tmux:
tmux new-session -s browse 'w3m google.com'
# then send keys from another terminal, e.g. reload:
tmux send-keys -t browse R
```
