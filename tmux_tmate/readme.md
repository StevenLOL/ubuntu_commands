# tmux & tmate (terminal multiplexers)

## 1. What is it?

- **tmux**: a terminal multiplexer — keep multiple sessions/windows alive after you disconnect (great for long server jobs).
- **tmate**: a fork of tmux that shares your session over SSH so others can join remotely (pairing / remote support).

## 2. What is it for?

- Persistent shell sessions on a server (detach, reconnect later).
- Sharing a terminal session with a collaborator via a public tmate server (or your own).

## 3. How to download / install

**tmux:**
```bash
sudo apt-get install tmux
```

**tmate:**
```bash
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:tmate.io/archive
sudo apt-get update
sudo apt-get install tmate
```

## 4. How to use

### tmux basics
```bash
tmux                                  # new session
tmux list-sessions                   # list
tmux attach-session -t <id>           # re-attach
# inside tmux:
#   Ctrl-b s   -> switch/list sessions
#   Ctrl-b d   -> detach
```

Enable mouse mode in `~/.tmux.conf`:
```text
setw -g mouse on
```
then `tmux source-file ~/.tmux.conf`.

View past messages: prefix (`Ctrl-b`) + `~` to list messages, or `Ctrl-b :` then `show-messages`. Scroll: prefix + `PageUp`/`PageDown`.

### tmate
```bash
tmate                                 # prints a shareable SSH/Web URL
tmate -S /tmp/tmate-1002/x9fOaz attach   # attach an existing tmate session
```

### Build your own tmate server
```bash
git clone https://github.com/tmate-io/tmate-slave.git && cd tmate-slave
./create_keys.sh                      # generates keys in ./keys; note the fingerprints
./autogen.sh && ./configure && make
sudo ./tmate-slave -b <SERVER_IP> -h "MyServer" -k ./keys/ -p 30013
```
Client `~/.tmat.conf`:
```text
set -g tmate-server-host "ssh.tmate.io"
set -g tmate-server-port 22
set -g tmate-server-rsa-fingerprint   "af:2d:81:c1:fe:49:70:2d:7f:09:a9:d7:4b:32:e3:be"
set -g tmate-server-ecdsa-fingerprint "c7:a1:51:36:d2:bb:35:4b:0a:1a:c0:43:97:74:ea:42"
```

Find your RSA fingerprint: `ssh-keygen -lf ./keys/ssh_host_rsa_key.pub`.

## 5. Pitfalls

- **Typo fixes vs original**: it's `tmux`, not `tmux_tmate`; `setw -g mouse on` (not the deprecated `mode-mouse`).
- **`tmate.io/archive` PPA may be stale**; on modern Ubuntu, install `tmate` from the main repos if available.
- **`tmate-server-rsa-fingerprint` mismatch** → "kex error: no match for method" — clean `~/.ssh/known_hosts` and re-check the fingerprint with `ssh-keygen -lf`.
- **Old libssh (0.7.x) on 14.04**: the `kedazo/libssh-0.7.x` PPA is dead; the current tmate builds against modern libssh.
- **tmate shares your terminal** — only share with people you trust; the public server exposes a live shell URL.
- Refs: tmate.io · stackoverflow RSA fingerprint question.
