
#Git basic
sudo apt-get install git

git clone ...
git rm
git add .
git reset --hard
git remote show ..
git checkout -b "0.1.2"


#Using SSH with bitbucket or github

github and bitbucket support SSH , if upload your SSH key to your account
for bitbucket
https://confluence.atlassian.com/bitbucket/how-to-install-a-public-key-on-your-bitbucket-cloud-account-276628835.html

add ssh
ssh-gen
ssh-agent
ssh-add .....your rsa

#There are two script in this folder:

one for easy push , or as its name imply git save my files!

###gitsave
or
###gitsave "some text please"



To say you have already have a project one your local disk, let's push it to git...

Go to project directory then git-new-ssh "my_git_project_000"




#[Git, how can I untrack files according to .gitignore? ](http://stackoverflow.com/questions/20840866/git-how-can-i-untrack-files-according-to-gitignore)

You need to remove the files from the index.

git rm -r --cached . 

and then add

git add .

Finally commit:

git commit -a -m "Untrack ignored files!"

