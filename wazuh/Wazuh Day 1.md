# Wazuh in 7 days

# Day 1 

**Day 1 核心目标**：搭建 Wazuh SIEM\+EDR 平台、接入 Kali 终端、**成功可视化看到 SSH 暴力破解告警**、跑通完整安全检测链路。

**说明**：本版本修复全网99%教程的遗漏坑：rsyslog缺失、Agent不回溯历史日志。

---

## 前置检查：进入正确工作目录

下载项目文件
```
git clone https://github.com/wazuh/wazuh-docker.git -b v4.12.0
# 如果在中国使用如下替代方案:
git clone https://gitclone.com/github.com/wazuh/wazuh-docker.git -b v4.12.0
```

所有 docker 命令必须在 single\-node 目录执行：

```bash
cd ~/wazuh-docker/single-node
docker compose -f generate-indexer-certs.yml run --rm generator
docker compose run -d
```

---

## 步骤1：确认三容器全部正常 Up

```bash
docker compose ps
```

必须全部状态为 Up：wazuh\.manager、wazuh\.indexer、wazuh\.dashboard

---

## 步骤2：登录 Wazuh Web 控制台

访问：https://Kali内网IP
账号：admin  密码：SecretPassword

---

## 步骤3：安装 Wazuh Agent 并补全日志采集（关键必做）

### 3\.1 部署 Agent

Endpoints → Deploy new agent → Linux DEB amd64
服务器地址填写：**本机真实内网IP（禁止127\.0\.0\.1）**
Agent 命名：kali\-test

### 3\.2 补全 SSH 日志采集（新版Kali默认缺失）

```bash
sudo nano /var/ossec/etc/ossec.conf
```

添加如下配置（采集 SSH 认证日志）：

```xml
<localfile>
  <log_format>syslog</log_format>
  <location>/var/log/auth.log</location>
</localfile>
```

### 3\.3 重启 Agent 生效

```bash
sudo systemctl restart wazuh-agent
```

### 3\.4 关键原理（必懂）

**Wazuh Agent 只采集「重启/配置生效之后的新日志」，不回溯历史日志**，之前的爆破记录全部无效，必须重启后重新爆破。

---

## 步骤4：补全系统日志环境（全网最大坑）

新版 Kali 默认无 rsyslog、无 auth\.log，导致无 SSH 日志、无告警。

```bash
sudo apt install -y rsyslog
sudo systemctl enable --now rsyslog
```

验证：ls /var/log/auth\.log 必须存在

### 4\.1 新建测试用户

```bash
sudo useradd testuser
echo "testuser:abc123456" | sudo chpasswd
```

---

## 步骤5：模拟 SSH 暴力破解 \+ 可视化告警查看

### 5\.1 使用内网IP


先查看本机IP：

```bash
ip a
```

假设IP：192\.168\.1\.11（替换成你自己的）

### 5\.2 执行高强度爆破（满足60秒≥6次失败阈值）

```bash
hydra -l testuser -P /usr/share/wordlists/rockyou.txt.gz ssh://192.168.1.11 -t 16 -w 1
```

运行10秒即可停止，此时系统会产生大量 Failed password 日志。

### 5\.3 验证本地日志已生成（源头确认）

```bash
sudo grep "Failed password" /var/log/auth.log | wc -l
```

输出 ≥10 即为达标

---

## 步骤6：警查看方式

### 6\.1 Endpoints

❌ Endpoints → 终端详情页：**只有趋势图，没有告警列表、没有搜索框，永远看不到明细**

### 6\.2 正确入口

✅ 页面顶部导航栏最左侧：**Threat Hunting**（真正的安全事件告警列表页）

### 6\.3 必改时间范围（解决8小时时差看不到告警）

问题根因：Docker 容器默认 UTC 时区，比北京时间慢8小时
你现在的告警时间是「凌晨」，如果浏览器筛选「最近15分钟」直接过滤掉

操作：页面右上角时间选择器 → 改为 **Last 24 hours**

### 6\.4 搜索框精准定位（位置明确）

搜索框位置：**页面顶部、时间选择器左侧的长条输入框**

输入任意一条即可检索出所有 SSH 告警：

- sshd（模糊匹配所有SSH日志）

- 5760（单次登录失败告警）

- 5712（SSH暴力破解聚合告警）

输入后回车，告警立刻全部出来。

---

## 步骤7：区分两类告警（面试/验收必懂）

1. **规则5760/5710：单次 SSH 登录失败**
只要输错密码就触发，必现

2. **规则5712：SSH 暴力破解（高危告警）**
触发条件：**同一IP、60秒内 ≥6次失败**
高强度 hydra 爆破后自动触发

---

## 步骤8：彻底解决时区时差问题（永久修复）

修改 docker\-compose\.yml，给三个容器统一亚洲上海时区，后续告警时间完全和电脑一致。

```bash
nano docker-compose.yml
```

在 wazuh\.manager、wazuh\.indexer、wazuh\.dashboard 三个服务内添加：

```yaml
environment:
  - TZ=Asia/Shanghai
volumes:
  - /etc/localtime:/etc/localtime:ro
```

重启服务生效：

```bash
sudo docker compose down
sudo docker compose up -d
```

---

## 终极排查链路（卡死必看）

1. 本地 auth\.log 有失败日志 ✅

2. Agent 配置采集 auth\.log ✅

3. Agent 重启加载新配置 ✅

4. 使用真实内网IP爆破（非127\.0\.0\.1）✅

5. Threat Hunting 页面选24小时范围 ✅

6. 搜索 sshd 查看告警明细 ✅

---

## 关键实操：Wazuh 手工强制扫描全套方案（解决不上报、不刷新、无日志）

**核心必考原理**：Wazuh Agent 日志采集采用 **尾部跟随模式**，**不回溯读取历史日志**。修改采集配置、新增日志文件、服务重启后，必须手动触发扫描重读日志，否则永久无法生成新告警，这是90%告警失效的核心原因。

### 方式1：日志采集强制重启（排查SSH告警失效首选）

适用场景：修改ossec\.conf日志配置、新增auth\.log、爆破后无新告警、日志不上报

```bash
sudo systemctl restart wazuh-agent
```

核心作用：重新加载所有采集规则、重新跟随日志尾部、重建与Manager 1514通信连接、重置采集状态，仅捕获**重启后全新产生的日志**。

### 方式2：全盘资产强制扫描（刷新终端硬件信息）

适用场景：终端Inventory data页面空白、看不到CPU/内存/进程/端口信息、资产数据长期不更新

```bash
sudo /var/ossec/bin/wazuh-control restart
```

核心作用：跳过默认1小时定时扫描周期，手动触发一次完整Syscollector资产扫描，立即上报终端所有硬件、进程、网络资产信息。

### 方式3：在线日志规则调试（精准排查规则不生效）

适用场景：怀疑日志格式异常、规则不匹配、有日志但无告警，精准定位问题

```bash
sudo /var/ossec/bin/wazuh-logtest
```

使用方法：执行命令后，粘贴一条SSH失败日志，回车即可实时查看日志解码结果、命中规则ID、风险级别，快速区分是日志问题还是平台规则问题。

### 标准告警排查固定流程（通用万能）

爆破后无告警，严格按以下步骤执行，100%解决常规告警失效问题：

1. 执行 **sudo systemctl restart wazuh\-agent** 重置采集状态

2. 重新执行hydra高强度爆破，生成全新可被采集的SSH失败日志

3. 等待30秒，预留日志上报\+规则匹配耗时

4. 进入Threat Hunting页面，时间范围选择Last 24h

5. 搜索sshd、5710、5712，查看对应告警明细

Wazuh Agent 默认**不会主动重读旧日志、不会实时刷新资产**，以下三条命令为 **人工强制扫描/重读日志/上报** 万能指令，卡顿时直接执行。

### 方式1：强制重启Agent（最万能 = 重读日志 \+ 重连服务端 \+ 重新资产扫描）

每次改配置、补日志、看不到告警，优先执行这条：

```bash
sudo systemctl restart wazuh-agent
```

作用：

- 重新加载 ossec\.conf 所有采集规则

- 重新跟随 auth\.log 尾部新日志

- 立刻上报终端资产（CPU/内存/进程）

- 重建与 Manager 1514 连接

### 方式2：手工单独触发 Syscollector 资产扫描（强制刷新硬件/进程/网络信息）

Web 终端详情页看不到 CPU、内存、进程时用这条：

```bash
sudo /var/ossec/bin/wazuh-control restart
```

作用：强制立即执行一次全盘资产扫描，无需等待1小时定时任务。

### 方式3：手动测试日志规则（精准调试，看日志能不能被识别）

怀疑日志格式不被识别、规则不生效时使用：

```bash
sudo /var/ossec/bin/wazuh-logtest
```

粘贴一条 SSH 失败日志，可实时看到：是否解码成功、命中哪条规则、级别多少，排查0告警神器。

每次爆破后没告警，按顺序执行：

1. sudo systemctl restart wazuh\-agent （强制重读日志）

2. 重新 hydra 爆破一次（产生**全新日志**）

3. Threat Hunting 选 Last24h \+ 搜索 sshd

1. 本地 auth\.log 有失败日志 ✅

2. Agent 配置采集 auth\.log ✅

3. Agent 重启加载新配置 ✅

4. 使用真实内网IP爆破（非127\.0\.0\.1）✅

5. Threat Hunting 页面选24小时范围 ✅

6. 搜索 sshd 查看告警明细 ✅ 


临时停止 Wazuh Agent
```
# 1. 先停止服务
sudo systemctl stop wazuh-agent

# 2. 强制清理所有残留的 Wazuh 子进程
sudo pkill -9 -f 'wazuh-'



```

```
# 进入 docker-compose.yml 所在目录
cd ~/wazuh-docker/
# 停止所有服务
docker compose down
```