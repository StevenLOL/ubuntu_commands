download vbox from website
1)vbox
2)extention pack



http://www.oracle.com/technetwork/server-storage/virtualbox/downloads/index.html#vbox



sudo usermod -aG vboxusers yourusername #logout


Intalling the amazingly fast Android x86 emulator
Here's how I installed Android x86 on VirtualBox and use it for Android development:
1. Download an image of Android x86. For example, android-x86-4.0-RC1-eeepc.iso.
2. Download and install VirtualBox.
3. Create a new virtual machine, choose Linux 2.6, 512MB RAM, 2GB HD, Network: PCnet-FAST III, attached to Bridged Adapter.
4. Install Android and start it.
5. On the virtual device, switch to Alt-F1 and set up the network:
$dhcpcd eth0
$setprop net.dns1 192.168.1.1 <--your router IP
$netcfg <--notice the IP address
6. Switch to Alt-F7 and test the network. It should work now.
7. On the host computer, connect to the device:
$adb connect <your device IP address>
7. In Eclipse, go "Run Configurations..." > Target. Choose "Manual" mode, and select the device when you hit run.

Hopefully everything works. Expect a speed boost of 10000% on the deploying and testing phase (so you won't have to wait that long to run a test). Done.

sudo /etc/init.d/vboxdrv setup





su <-goto super user mode
netcfg eth0 dhcp
echo nameserver <ip> > /etc/resolv.conf
dnsmasq 
setprop net.dns1 8.8.8.8
