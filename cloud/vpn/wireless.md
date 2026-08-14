# Fix a stuck Wi-Fi interface (managed mode)

## 1. What is it / What is it for?

A recipe to force a Linux Wi-Fi card (`wlan0`) back into **managed** mode when it got stuck in monitor/ad-hoc mode and NetworkManager can't use it.


- Restoring normal Wi-Fi after the interface was left in a non-managed state (e.g. by aircrack/monitor tools).
- Re-attaching the card to NetworkManager.

## 2. How to download / install

No install — uses `ip`, `iw`, and `NetworkManager` (preinstalled on Ubuntu desktop).

## 3. How to use

```bash
# 1. bring the interface down
sudo ip link set wlan0 down
# 2. force managed mode
sudo iw dev wlan0 set type managed
# 3. bring it back up
sudo ip link set wlan0 up
# 4. restart NetworkManager
sudo systemctl restart NetworkManager
```

