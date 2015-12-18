

#!/bin/sh
# Makes a non git directory to a new private bitbucket repository with all files added to initial commit
# Usage = enter your bitbucket username and password in the script
USERNAME="YOUR USERNAME"
PASSWORD="YOUR PASSWORD"

echo "Creating new remote repo on bitbucket"
curl --user $USERNAME:$PASSWORD https://api.bitbucket.org/1.0/repositories/ --data name=$1 --data is_private='true'

git init
git add -A .
git commit -m "Initial commit"

git remote add origin ssh://git@bitbucket.org/$USERNAME/$1.git
git remote set-url origin ssh://git@bitbucket.org/$USERNAME/$1.git
git push origin master


#git push -u origin --all
#git push -u origin --tags

