# Fix a stuck Wi-Fi interface (managed mode)

## 1. What is it?

A recipe to force a Linux Wi-Fi card (`wlan0`) back into **managed** mode when it got stuck in monitor/ad-hoc mode and NetworkManager can't use it.

## 2. What is it for?

- Restoring normal Wi-Fi after the interface was left in a non-managed state (e.g. by aircrack/monitor tools).
- Re-attaching the card to NetworkManager.

## 3. How to download / install

No install — uses `ip`, `iw`, and `NetworkManager` (preinstalled on Ubuntu desktop).

## 4. How to use

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

## 5. Pitfalls

- **Interface name may differ**: modern systems use `wlo1`/`wlpXsY`, not `wlan0` — check with `ip link`.
- **`iw set type managed` fails if the card is busy** (e.g. still in monitor mode by another process); kill the offending tool first.
- **NetworkManager restart drops the current connection** briefly; reconnect afterward.
