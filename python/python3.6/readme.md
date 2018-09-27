Compile and install python 3.6 on Ubuntu 16.04

Install the necessary dependencies, download the python 3.6 source code, and build the environment and install
```
sudo apt install build-essential checkinstall
sudo apt install libreadline-gplv2-dev libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev
wget https://www.python.org/ftp/python/3.6.0/Python-3.6.0.tar.xz
tar xvf Python-3.6.0.tar.xz
cd Python-3.6.0/
./configure
sudo make altinstall
```
 Install python 3.6 on Ubuntu 16.04 from PPA

You can install Python 3.6  from PPA using the commands below
```
sudo add-apt-repository ppa:jonathonf/python-3.6
sudo apt update
sudo apt install python3.6
```


REF:https://askubuntu.com/questions/865554/how-do-i-install-python-3-6-using-apt-get
