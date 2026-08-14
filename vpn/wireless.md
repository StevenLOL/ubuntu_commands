# 1. 彻底关闭网卡
sudo ip link set wlan0 down
# 2. 强制重置为托管模式
sudo iw dev wlan0 set type managed
# 3. 重新拉起网卡
sudo ip link set wlan0 up
# 4. 重启网络服务
sudo systemctl restart NetworkManager