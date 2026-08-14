# GitLab CE (self-hosted Git server)

## 1. What is it / What is it for?

GitLab Community Edition (CE) is a self-hosted Git platform (like a private GitHub) with repos, CI, issues, and a web UI. Installed via the Omnibus package.


- Hosting private Git repositories on your own server.
- Team collaboration with merge requests, CI runners, and container registry.

## 2. How to download / install

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

## 3. How to use

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

