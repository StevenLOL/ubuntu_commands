# Git (version control)

## 1. What is it / What is it for?

Git is a distributed version-control system. This folder also holds helper scripts (`git-new-ssh.sh`, `gitsave.sh`) that automate "create remote repo from a local folder" and "save & push" workflows.


- Tracking source changes, branching, and collaborating via GitHub/Bitbucket.
- One-command publishing of a local folder to a new remote repo (`git-new-ssh.sh`).
- Quick "save everything and push" (`gitsave.sh`).

## 2. How to download / install

```bash
sudo apt-get install git
git config --global user.name  "Your Name"
git config --global user.email "you@example.com"
```

SSH setup (recommended for GitHub/Bitbucket):
```bash
ssh-keygen -t rsa -b 4096      # creates ~/.ssh/id_rsa(.pub)
ssh-agent bash
ssh-add ~/.ssh/id_rsa
# upload ~/.ssh/id_rsa.pub to your account settings
```

## 3. How to use

### Basics
```bash
git clone <url>
git add .
git rm <file>
git reset --hard
git remote show origin
git checkout -b "0.1.2"        # new branch
git commit --amend             # change last commit message
```

### Submodules
```bash
git clone <url> --recursive
git submodule update --recursive --remote
git submodule update --init
```
Ref: http://stackoverflow.com/questions/1030169/

### Untrack files that are now in .gitignore
```bash
git rm -r --cached .
git add .
git commit -a -m "Untrack ignored files!"
```

### Shallow / partial clone
```bash
git clone --depth 1 <url>
git clone --depth 1 https://github.com/keras-team/keras.git -b 2.2.0

# clone a single folder (sparse, via archive)
git archive --format tar --remote ssh://server.org/path/to/git HEAD docs/usage > /tmp/usage_docs.tgz
# or via svn:
svn checkout https://github.com/USER/repo/trunk/some/folder
```

### Pull all branches
```bash
git branch -r | grep -v '\->' | while read remote; do git branch --track "${remote#origin/}" "$remote"; done
git fetch --all; git pull --all
```

### Helper scripts
- `git-new-ssh.sh "my_project"` — creates a new **private** Bitbucket repo and pushes the current folder. Edit `USERNAME`/`PASSWORD` in the script first.
- `gitsave.sh ["message"]` — `git add .` + commit (default "save") + `git push origin master`.

### If github.com is unreachable
Use a mirror such as `gitclone.com`:
```bash
git clone https://gitclone.com/github.com/wazuh/wazuh-docker.git
```

## 4. Related notes (in this repo)

- [GitHub access from restricted networks (HOSTS fix)](github-hosts.md) — fix "connection reset" / images not loading by mapping GitHub hostnames to IPs in `/etc/hosts`.
- [GitLab CE (self-hosted)](gitlab-ce.md) — install and manage a self-hosted GitLab server (omnibus, data dir, external URL).

