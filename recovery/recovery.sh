sudo add-apt-repository ppa:xorg-edgers/ppa
sudo apt-get update
sudo apt-get upgrade
sudo apt-get purge nvidia*
sudo apt-get purge xserver-xorg
sudo apt-get purge xinit
sudo apt-get instll xserver-xorg xinit

#get graphic driver...
sudo apt-get install mesa-utils

gtf 1920 1080 59.9
xrandr --newmode "1920x1080_59.90"  172.51  1920 2040 2248 2576  1080 1081 1084 1118  -HSync +Vsync
xrandr --addmode eDP1 1920x1080_59.90

startx

# open gui resulution and "APPLY"



remove nouveau if needed
sudo apt-get --purge remove xserver-xorg-video-nouveau
sudo modprobe -r nouveau