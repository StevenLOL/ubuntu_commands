# Elastic IP (cloud public IP binding)

## 1. What is it / What is it for?

An Elastic IP is a static public IPv4 address you can attach to a cloud instance (AWS calls it "Elastic IP"; on UCloud it's "弹性IP"). The public IP is assigned by the cloud platform, not configured inside the guest OS.


- Reaching a VM from the internet via a fixed public IP.
- Binding services (e.g. a JSON-RPC server) to a stable public address that survives reboots/reassignments.

## 2. How to download / install

No software to install — it's a cloud control-panel feature. Allocate an Elastic IP in the console and bind it to your instance.

## 3. How to use

```bash
ifconfig        # you will NOT see the public IP here — only the private/internal IP
```

The public IP is mapped by the cloud platform's NAT. To expose a service:
- Bind your service to the **internal** IP (or `0.0.0.0`), and make sure the cloud **security group / firewall** opens the port.

```bash
# example: a JSON-RPC service bound to the internal IP that the Elastic IP forwards to
# just ensure the listening port is allowed in the firewall
```

