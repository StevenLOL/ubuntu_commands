# Elastic IP (cloud public IP binding)

## 1. What is it?

An Elastic IP is a static public IPv4 address you can attach to a cloud instance (AWS calls it "Elastic IP"; on UCloud it's "弹性IP"). The public IP is assigned by the cloud platform, not configured inside the guest OS.

## 2. What is it for?

- Reaching a VM from the internet via a fixed public IP.
- Binding services (e.g. a JSON-RPC server) to a stable public address that survives reboots/reassignments.

## 3. How to download / install

No software to install — it's a cloud control-panel feature. Allocate an Elastic IP in the console and bind it to your instance.

## 4. How to use

```bash
ifconfig        # you will NOT see the public IP here — only the private/internal IP
```

The public IP is mapped by the cloud platform's NAT. To expose a service:
- Bind your service to the **internal** IP (or `0.0.0.0`), and make sure the cloud **security group / firewall** opens the port.

```bash
# example: a JSON-RPC service bound to the internal IP that the Elastic IP forwards to
# just ensure the listening port is allowed in the firewall
```

## 5. Pitfalls

- **Public IP is invisible inside the VM** — `ifconfig` only shows the private IP; that's expected.
- **Firewall/security group**: even with an Elastic IP, the port must be opened or the service is unreachable.
- **Binding to the wrong address**: bind to `0.0.0.0` (or the internal IP), not the public IP, which the OS doesn't own.
- **Elastic IPs may cost money** when left unbound to a running instance (AWS charges for unattached ones).
