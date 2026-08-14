# 隐私占位符批量替换脚本（精准模式）
# 策略：仅替换"用户/他人真实私有环境信息"，保留公开的官方教程示例（如 Atlassian 的 emmap1）。
# 替换前对每个受影响文件做 .bak 备份，可回滚。

$ErrorActionPreference = 'Stop'
$root = 'e:/Dev/ubuntu_commands'

# 备份函数
function Backup($f) {
    $bak = "$f.bak"
    if (-not (Test-Path $bak)) { Copy-Item $f $bak }
}

# 替换规则：正则 -> 占位符
# 注意：使用 [regex]::Escape 处理字面量；对 IP 用正则。
$rules = @(
    # docker/readme.md: 真实用户名 steven 的挂载路径
    @{ file = "$root/docker/readme.md"; pattern = '/mnt/steven/'; replace = '<YOUR_HOME>/' },
    @{ file = "$root/docker/readme.md"; pattern = ':/steven/'; replace = ':<YOUR_HOME>/' },

    # sge: 真实集群内网 IP
    @{ file = "$root/sge/start.sge.sh"; pattern = '192\.168\.2\.10'; replace = '<SERVER_IP_1>' },
    @{ file = "$root/sge/start.sge.sh"; pattern = '192\.168\.2\.11'; replace = '<SERVER_IP_2>' },
    @{ file = "$root/sge/start.sge.sh"; pattern = '192\.168\.2\.12'; replace = '<SERVER_IP_3>' },

    # wazuh: 具体目标 IP（标注"替换成你自己的"）
    @{ file = "$root/wazuh/Wazuh Day 1.md"; pattern = '192\.168\.1\.11'; replace = '<TARGET_IP>' },

    # vbox: 路由器 IP（标注 your router IP）
    @{ file = "$root/vbox/README.md"; pattern = '192\.168\.1\.1'; replace = '<ROUTER_IP>' },

    # lxc: 容器示例 IP
    @{ file = "$root/lxc/readme.txt"; pattern = '10\.0\.3\.55'; replace = '<CONTAINER_IP>' },

    # kali/uniscan: 目标网段 IP
    @{ file = "$root/kali/uniscan.md"; pattern = '192\.168\.0\.185'; replace = '<TARGET_IP>' },

    # cheatsheet/network-ssh: 企业内网段
    @{ file = "$root/cheatsheet/network-ssh.md"; pattern = '10\.99\.23\.1/24'; replace = '<PRIVATE_SUBNET>' },

    # 第三方他人真实用户名路径（非用户本人，但属他人隐私，统一占位）
    @{ file = "$root/caffe/install_docker.md"; pattern = '/home/crw/'; replace = '<YOUR_HOME>/' },
    @{ file = "$root/opencv/opencv_ocr.md"; pattern = '/home/jose/'; replace = '<YOUR_HOME>/' },
    @{ file = "$root/python/matplotlib/matplotlib_chinese.md"; pattern = '/home/liyang/'; replace = '<YOUR_HOME>/' },
    @{ file = "$root/python/ipython_notebook/readme.md"; pattern = '/home/zhenyu/'; replace = '<YOUR_HOME>/' },
    @{ file = "$root/office_wine/Installing Microsoft Office.txt"; pattern = '/home/cgomez/'; replace = '<YOUR_HOME>/' }
)

foreach ($r in $rules) {
    if (Test-Path $r.file) {
        Backup $r.file
        $content = Get-Content -Raw -Path $r.file
        $new = [regex]::Replace($content, $r.pattern, $r.replace)
        if ($new -ne $content) {
            Set-Content -NoNewline -Path $r.file -Value $new
            Write-Output "替换完成: $($r.file)  [$($r.pattern) -> $($r.replace)]"
        } else {
            Write-Output "无命中(跳过): $($r.file)  [$($r.pattern)]"
        }
    } else {
        Write-Output "文件不存在: $($r.file)"
    }
}

Write-Output "=== 隐私占位符替换完成 ==="
