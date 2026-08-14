# SSH: passwordless login & key setup

## 1. What is it?

SSH key-based authentication lets you log in (and use Git over SSH) without typing a password each time. This note covers generating keys, copying them to a server, restricting by IP, and using them with GitHub/Bitbucket.

## 2. What is it for?

- Passwordless, more-secure SSH login and Git push/pull.
- Restricting SSH access to a subnet via TCP-wrappers.

## 3. How to download / install

SSH is built into Linux/macOS. Generate a key pair:
```bash
ssh-keygen -t rsa        # creates ~/.ssh/id_rsa (private) + id_rsa.pub (public)
```

## 4. How to use

**Copy your key to a server (passwordless login):**
```bash
ssh <USER>@<SERVER_IP> mkdir -p .ssh
cat .ssh/id_rsa.pub | ssh <USER>@<SERVER_IP> 'cat >> .ssh/authorized_keys'
ssh <USER>@<SERVER_IP>          # now no password
```

**Restrict SSH to a local subnet:**
```text
# /etc/hosts.deny
sshd:ALL
# /etc/hosts.allow
sshd:192.168.0.0/16
```

**For GitHub / Bitbucket:**
1. `ssh-keygen` (creates the key pair).
2. Copy the contents of `id_rsa.pub` and paste into your account's SSH keys page.
3. Load the private key locally:
   ```bash
   ssh-agent bash
   eval "$(ssh-agent -s)"
   ssh-add PATH_TO_YOUR_PRIVATE_KEY
   ```

**Keep-alive for long sessions:**
```bash
ssh -o StrictHostKeyChecking=no -o ServerAliveInterval=15 -o ServerAliveCountMax=3 \
    -N -f -R 2001:localhost:22 -l username ipaddress "uptime"
```

Ref: https://confluence.atlassian.com/bitbucket/set-up-ssh-for-git-728138079.html

## 5. Pitfalls

- **Never share the private key (`id_rsa`)** — only the `.pub` goes to servers/accounts.
- **`.ssh` perms**: `~/.ssh` should be `700` and `authorized_keys` `600`, or SSH refuses them.
- **`ssh-agent bash`** starts a new shell; run the rest inside it, or the key isn't loaded for subsequent commands.
- **`hosts.allow/deny` TCP-wrappers** are ignored by some modern SSH builds (use `sshd_config` `AllowUsers`/`Match Address` instead).
- **Passphrase**: a key with a passphrase is safer; use an agent so you type it once.
