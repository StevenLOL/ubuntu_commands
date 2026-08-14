# GitLab CE (self-hosted Git server)

## 1. What is it?

GitLab Community Edition (CE) is a self-hosted Git platform (like a private GitHub) with repos, CI, issues, and a web UI. Installed via the Omnibus package.

## 2. What is it for?

- Hosting private Git repositories on your own server.
- Team collaboration with merge requests, CI runners, and container registry.

## 3. How to download / install

```bash
sudo apt install curl openssh-server ca-certificates postfix   # postfix in "Internet" mode
curl -sS https://packages.gitlab.com/install/repositories/gitlab/gitlab-ce/script.deb.sh | sudo bash
sudo apt-get install gitlab-ce
```

Then apply config and start:
```bash
sudo gitlab-ctl reconfigure
```
Open the server IP in a browser and reset the root password.

Refs:
- https://gitlab.com/gitlab-org/omnibus-gitlab/blob/master/doc/settings/configuration.md
- https://about.gitlab.com/downloads/#ubuntu1404

## 4. How to use

### Move repository data to another disk
Edit `/etc/gitlab/gitlab.rb`:
```ruby
git_data_dirs({
  "default" => "/var/opt/gitlab/git-data",
  "alternative" => "/mnt/nas/git-data"
})
# target dirs must NOT be symlinks
```
Then `sudo gitlab-ctl reconfigure`.

### Set the external URL
```ruby
external_url "http://gitlab.example.com"
```
Then `sudo gitlab-ctl reconfigure`.

### Settings panel
Admin Area → top-right gear icon → the last item is the settings panel.

### Unprotect master for developers
Project → Settings → "Protected Branches" → disable protection so developers can push.

## 5. Pitfalls

- **`reconfigure` is required** after every `gitlab.rb` change — edits won't apply until then.
- **No symlinks** in `git_data_dirs` paths; Omnibus rejects them.
- **Postfix prompts** during install — choose "Internet" mode or skip and configure mail later.
- **First reconfigure is slow** (runs migrations); wait for it to finish.
- **External URL mismatch** makes clone links / password-reset emails point at the wrong host — set it before inviting users.
- **Resource heavy**: GitLab CE wants ≥4 GB RAM; on small VMs it may OOM.
