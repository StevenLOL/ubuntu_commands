# 1. 查看可用发行版
wsl --list --online

# 2. 安装 Ubuntu（推荐）
wsl --install -d Ubuntu-22.04

# 3. 运行 Ubuntu
wsl-d Ubuntu-22.04
## 以指定定用户名
wsl-d Ubuntu-22.04 -u steven 
# 4. 关闭 Ubuntu
wsl --shutdown
# 5. 设置网络

WSL2 默认是 NAT 模式，和 Windows 不在同一网段。要实现"桥接"效果（同一局域网、同网段 IP），可以用镜像模式（最推荐，Windows 11 22H2+）这是微软现在官方推荐的方式，效果等同于桥接——WSL2 和 Windows 共享同一个 IP，局域网其他设备可以直接访问 WSL 里的服务。

## Step 1：创建/编辑 .wslconfig
powershell
```
notepad "$env:USERPROFILE\.wslconfig"
```
## Step 2：写入以下内容：
```
ini
[wsl2]
networkingMode=mirrored
dnsTunneling=true
firewall=true
autoProxy=true
```
## Step 3：重启 WSL
powershell
```
wsl --shutdown
wsl
```
## 验证：
bash
```
# 在 WSL 里查看 IP，应该和 Windows 主机同一个网段
ip addr
```