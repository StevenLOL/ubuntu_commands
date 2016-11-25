#There are two types of script in this folder:



##A) git-new-*

Create a git repository in github/bitbuck from a local folder.

To say local foler "project_abc". Go to project directory then 
```
git-new-ssh "my_git_project_000"
```
This will create a git repository in github/bitbuck called my_git_project_000 with all files in folder project_abc.

One need to configure account username and password in git-new-*

##B) gitsave
gitsave for save all my files and publish to remote repository
###usage:
```
gitsave
#or
gitsave "some text please"
```

#Using SSH with bitbucket or github

github and bitbucket support SSH , if upload your SSH key to your account.
for [bitbucket](https://confluence.atlassian.com/bitbucket/how-to-install-a-public-key-on-your-bitbucket-cloud-account-276628835.html)

add ssh

ssh-gen

ssh-agent

ssh-add .....your rsa



#Git basic
sudo apt-get install git

git clone ...

git rm

git add .

git reset --hard

git remote show ..

git checkout -b "0.1.2"


git commit --amend #change commit message


#[Git, how can I untrack files according to .gitignore? ](http://stackoverflow.com/questions/20840866/git-how-can-i-untrack-files-according-to-gitignore)

You need to remove the files from the index.

git rm -r --cached . 

and then add

git add .

Finally commit:

git commit -a -m "Untrack ignored files!"


#Faster clone by set depth =1
```
git clone --depth 1 ...
```
