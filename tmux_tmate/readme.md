
#Tmux
```
      tmux
      tmux list-sessions
      tmux attach-session -t #
      #enable mouse model: in ~/.tmux.conf
      setw -g mode-mouse on
      or setw -g mouse on
      then tmux source-file ~/.tmux.conf
      #when in tmux
      #switch and list tmux session
      ctrl-b s
      #detach session
      ctrl-b d
```      
      
      
#Tmate
```
sudo apt-get install software-properties-common && \
sudo add-apt-repository ppa:tmate.io/archive    && \
sudo apt-get update                             && \
sudo apt-get install tmate
```
show log/message again:

press prefix + ~ to list all previous tmux/tmate messages
press prefix + : to get tmux prompt. Then type show-messages command to get the same results as in 1).

prefix+b then pagedown/pageup


##Attach tmate session from loacal:
```
tmate -S /tmp/tmate-1002/x9fOaz attach
```

#Build your own tmate server

##[Server](https://tmate.io/)
```

#clone from source and build
git clone https://github.com/tmate-io/tmate-slave.git && cd tmate-slave
./create_keys.sh # This will generate SSH keys, remember the keys fingerprints,in the ./keys folder
./autogen.sh && ./configure && make
sudo ./tmate-slave    #this load the keys from ./keys folder
#if on 14.04 one need to install libssh-0.7.x via:
sudo add-apt-repository ppa:kedazo/libssh-0.7.x
sudo apt-get update
sudo apt-get install libssh-gcrypt-dev libmsgpack-dev

sudo  ./tmate-slave -b 105.159.14.17 -h My.name.shown.on.client  -k ./keys/  -p 30013 

```
##Client
One have to configure ~/.tmat.conf
```
#default setting 
set -g tmate-server-host "ssh.tmate.io"
set -g tmate-server-port 22
set -g tmate-server-rsa-fingerprint   "af:2d:81:c1:fe:49:70:2d:7f:09:a9:d7:4b:32:e3:be"
set -g tmate-server-ecdsa-fingerprint "c7:a1:51:36:d2:bb:35:4b:0a:1a:c0:43:97:74:ea:42"
set -g tmate-identity ""              # Can be specified to use a different SSH key.

```
###[How do I find my RSA key fingerprint?](http://stackoverflow.com/questions/9607295/how-do-i-find-my-rsa-key-fingerprint)
```
ssh-keygen -lf ./keys/ssh_host_rsa_key.pub
```

###[tmate] Error connecting: kex error : no match for method server host key algo: server [ssh-dss,ssh-rsa], client [ecdsa-sha2-nistp256] 

Then [cleanup your known_hosts file](https://hackweek.suse.com/14/projects/1054)




