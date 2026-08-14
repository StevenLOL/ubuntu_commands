# VirtualBox (desktop virtualization)

## 1. What is it / What is it for?

VirtualBox is Oracle's free, cross-platform desktop hypervisor for running guest OSes (Windows, Linux, Android-x86, ...) inside Ubuntu.


- Running another OS alongside Ubuntu without dual-booting.
- A fast Android-x86 emulator for app development/testing (much quicker than the ARM emulator).

## 2. How to download / install

Download the installer + **Extension Pack** from:
https://www.virtualbox.org/wiki/Downloads

Then add your user to the `vboxusers` group (so USB/devices work):
```bash
sudo usermod -aG vboxusers yourusername     # then log out/in
```

(Optionally rebuild kernel modules after a kernel upgrade: `sudo /etc/init.d/vboxdrv setup`.)

## 3. How to use

**Android-x86 as a fast emulator:**
1. Download an Android-x86 ISO (e.g. `android-x86-4.0-RC1-eeepc.iso`) and install VirtualBox.
2. New VM: Linux 2.6, 512 MB RAM, 2 GB disk, Network = `PCnet-FAST III` (Bridged Adapter).
3. Install Android and start it.
4. In the VM: `Alt-F1` → `dhcpcd eth0` → `setprop net.dns1 <ROUTER_IP>` → `netcfg` (note the IP). `Alt-F7` to return to the UI.
5. On the host: `adb connect <VM_IP>` to talk to the Android VM.
6. In Eclipse, set the Run target to "Manual" and pick the device.

**Networking a guest (manual):**
```bash
su
netcfg eth0 dhcp
echo nameserver <ip> > /etc/resolv.conf
dnsmasq
setprop net.dns1 8.8.8.8
```

