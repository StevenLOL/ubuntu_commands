

#install
```
sudo apt install curl openssh-server ca-certificates 
#install postfix in internet mode
sudo apt-get install postfix

#check the gitlab for you os
curl -sS https://packages.gitlab.com/install/repositories/gitlab/gitlab-ce/script.deb.sh | sudo bash
#install gitlab, it will take some time to complete
sudo apt-get install gitlab-ce

```

#init or reconfigure if you had made some changes
```
sudo gitlab-ctl reconfigure
```
then using the ip to access your machine from browser and reset your password


#configure dirs/path of repository

```
/etc/gitlab/gitlab.rb.

git_data_dirs({
  "default" => "/var/opt/gitlab/git-data",
  "alternative" => "/mnt/nas/git-data"
})
Note that the target directories and any of its subpaths must not be a symlink.
Run sudo gitlab-ctl reconfigure for the changes to take effect.
```

#where is the setting pannel ???
First go to admin area, then at the top right corner, click a gear button, the last one is setting pannel.

REF: 

https://gitlab.com/gitlab-org/omnibus-gitlab/blob/master/doc/settings/configuration.md#storing-git-data-in-an-alternative-directory

https://about.gitlab.com/downloads/#ubuntu1404
