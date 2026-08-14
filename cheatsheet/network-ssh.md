# 网络与 SSH / Network & SSH

## SSH
```
ssh <USER>@<SERVER_IP>
ssh -l 23 <USER>@<SERVER_IP>
```

## Ping / 扫描网内主机
```
for x in {11..255};do ssh <SERVER_IP_PREFIX>.$x; done;

# scan host in the network:
arp
sudo nmap -sn 10.99.23.1/24 > readme.txt
avahi-browse -a -v -r -t -d local   #sudo apt-get install avahi-discover
```

## Apache ab 压力测试
```
apt-get install apache2-utils
ab -n 10000 -c 100 https://www.baidu.com/
```

## 多线程 / 同时跑两条命令
```
some_commands &
with wait
a&
b&
wait

lspci | grep Intel    # two comands at same time |
```

## 队列 / work with queue
```
check queue state
qstat -u "*"
qdel job-ID
qsub hello_world.sh &
```

## 打开网页 / open webpage
```
#open webpage from ternimal
links www.zaobao.com
lynx
w3m
# As far as I know, these browsers do not support programmed reloading, however it can easily be accomplished by using a terminal multiplexer like tmux. For example if you start the browser in one terminal like this:
tmux new-session -s browse 'w3m google.com'
# Then you can send commands to it from another terminal with the send-keys command. So to make w3m reload the current page do this:
tmux send-keys -t browse R
```
