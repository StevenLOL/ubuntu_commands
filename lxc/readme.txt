# LXC (Linux Containers)

## 1. What is it?

LXC is OS-level virtualization — lightweight "containers" that run a full Linux distro sharing the host kernel, managed via `libvirt` tools (virsh, virt-manager) just like VMs. Ref: http://xmodulo.com/lxc-containers-ubuntu.html

## 2. What is it for?

- Running another distro (e.g. Ubuntu) instantly without dual-boot or a heavy VM.
- Isolated environments you can manage with the same libvirt tooling as KVM.

## 3. How to download / install

```bash
sudo apt-get install lxc
lxc-checkconfig          # verify kernel supports LXC
```
After install, the default bridge `lxcbr0` is created automatically (`/etc/lxc/default.conf`).

Available templates live in `/usr/share/lxc/templates`.

## 4. How to use

```bash
sudo lxc-create -n test-lxc -t ubuntu                 # default Ubuntu (host's release/arch)
sudo lxc-create -n test-lxc -t ubuntu -- --release utopic   # specific release
sudo lxc-ls --fancy                                    # list containers

sudo lxc-start -n test-lxc -d                         # start as daemon
sudo lxc-console -n test-lxc                          # attach console (Ctrl+a q to exit)
sudo lxc-stop -n test-lxc
sudo lxc-destroy -n test-lxc
sudo lxc-stop -n test-lxc && sudo lxc-clone -o test-lxc -n test-lxc2

brctl show lxcbr0      # verify the container veth is bridged
```
Container rootfs: `/var/lib/lxc/<name>/rootfs`; package cache: `/var/cache/lxc`.

## 5. Pitfalls

- **`liblxc.so.1: undefined symbol: cgmanager_get_pid_cgroup_abs_sync`** → your `libcgmanager` is too old for the LXC build. Fix: `sudo apt-get install libcgmanager0`.
- **Template download takes minutes** (it fetches/validates packages) — be patient.
- **The `lxc-ls –fancy` dashes** are em-dashes in the original; use `lxc-ls --fancy` (ASCII) on the command line.
- LXC is lower-level than Docker; for app packaging prefer Docker.
- Refs: 51cto LXC tutorial · xmodulo.
