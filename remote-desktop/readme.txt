#install remote desktop for UBUNTU >14.04

sudo apt-get install xrdp
sudo apt-get install xfce-4
sudo apt-get install xfce4
echo xfce4-session >~/.xsession
sudo service xrdp restart








#others:

sudo apt-add-repository ppa:freenx-team
sudo apt-get update
sudo apt-get install freenx-server

wget https://bugs.launchpad.net/freenx-server/+bug/576359/+attachment/1378450/+files/nxsetup.tar.gz

tar -xvf nxsetup.tar.gz

sudo cp nxsetup /usr/lib/nx/nxsetup

sudo /usr/lib/nx/nxsetup --install 

sudo apt-get install gnome-session-fallback


sudo apt-get install qtnx

