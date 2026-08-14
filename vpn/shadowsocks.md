
#Client:
```
sudo apt install shadowsocks
sslocal -c /etc/shadowsocks/config.json
```

Download and install https://github.com/FelisCatus/SwitchyOmega/releases for chrome

Setup socket proxy point to 127.0.0.1 port 1080

And enable proxy




Your Server IP:xxx.xxx.xxx.xxx

Your Server Port: 8381

Your Password: liubida!!!8381

Your Local IP:127.0.0.1

Your Local Port:1080

Your Encryption Method:aes-256-cfb


REF: https://wiki.archlinux.org/index.php/Shadowsocks_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)




    Home
    Packages
    Forums
    Wiki
    Bugs
    AUR
    Download

Shadowsocks (简体中文)

Shadowsocks是一个轻量级socks5代理，以python写成；
Contents

    1 安装
    2 配置
        2.1 客户端
        2.2 服务端
        2.3 以守护进程形式运行客户端
        2.4 以守护进程形式运行服务端
        2.5 加密
        2.6 Chrome/Chromium
    3 参阅

安装

可自[community]中安装已打包好的shadowsocks。
shadowsocks 	shadowsocks基本包；
libsodium
python2-numpy
python2-salsa20 	Salsa20和Chacha20支持；
配置

shadowsocks以json为配置文件格式，以下是一个样例：

/etc/shadowsocks/config.json

{
	"server":"remote-shadowsocks-server-ip-addr",
	"server_port":443,
	"local_address":"127.0.0.1",
	"local_port":1080,
	"password":"your-passwd",
	"timeout":300,
	"method":"aes-256-cfb",
	"fast_open":false,
	"workers":1
}

提示: 若需同时指定多个服务端ip，可参考"server":["1.1.1.1","2.2.2.2"],
server 	服务端监听地址(IPv4或IPv6)
server_port 	服务端端口，一般为443
local_address 	本地监听地址，缺省为127.0.0.1
local_port 	本地监听端口，一般为1080
password 	用以加密的密匙
timeout 	超时时间（秒）
method 	加密方法，默认的table是一种不安全的加密，此处首推aes-256-cfb
fast_open 	是否启用TCP-Fast-Open
wokers 	worker数量，如果不理解含义请不要改
客户端

在config.json所在目录下运行sslocal即可；若需指定配置文件的位置：

# sslocal -c /etc/shadowsocks/config.json

注意: 有用户报告无法成功在运行时加载config.json
，或可尝试手动运行：

# sslocal -s 服务器地址 -p 服务器端口 -l 本地端端口 -k 密码 -m 加密方法

配合nohup和&可以使之后台运行，关闭终端也不影响：

#nohup sslocal -s 服务器地址 -p 服务器端口 -l 本地端端口 -k 密码 -m 加密方法 &

提示: 当然也有图形化的使用shadowsocks-gui@gitHub,如果不希望自己编译的话，也可以到shadowsocks-gui@sourceforge下载。

由于作者被相关部门请去喝茶，出于某些原因，其项目已经从github移除,但仍然可以从aur中下载安装

对debian用户的友情提示，由于源中没有shadowsocks-qt5，可到unbuntu的源中下载，下载shadowsocks-qt5以及libqtshadowsocks，使用dpkg命令或者借助gdebi点击安装。
服务端
提示: 普通用户无需配置服务端；

在服务器上cd到config.json所在目录：

    运行ssserver；
    如果想在后台一直运行，可改执行：nohup ssserver > log &；

以守护进程形式运行客户端

Shadowsocks的systemd服务可在/etc/shadowsocks/里调用不同的conf-file.json（以conf-file为区分标志），例： 在/etc/shadowsocks/中创建了foo.json配置文件，那么执行以下语句就可以调用该配置：

# systemctl start shadowsocks@foo

若需开机自启动：

# systemctl enable shadowsocks@foo

提示: 可用journalctl -u shadowsocks@foo来查询日志；
以守护进程形式运行服务端

以上只是启动了客户端的守护进程，如果架设的是服务器，则需要：

# systemctl start shadowsocks-server@foo
# systemctl enable shadowsocks-server@foo

提示: 如果使用的服务端端口号小于1024，需要修改usr/lib/systemd/system/shadowsocks-server@.service使得user=root，之后使用systemctl daemon-reload重新载入守护进程配置，即可开启监听。
加密
注意: 默认加密方法table速度很快，但很不安全。推荐使用aes-256-cfb或者bf-cfb，照目前的趋势，ChaCha20是占用最小速度最快的一种方式。请不要使用rc4，它不安全。
提示: 安装M2Crypto可略微提升加密速度，对于Python2来说，安装python2-m2crypto即可。

可选的加密方式：

    aes-256-cfb: 默认加密方式
    aes-128-cfb
    aes-192-cfb
    aes-256-ofb
    aes-128-ofb
    aes-192-ofb
    aes-128-ctr
    aes-192-ctr
    aes-256-ctr
    aes-128-cfb8
    aes-192-cfb8
    aes-256-cfb8
    aes-128-cfb1
    aes-192-cfb1
    aes-256-cfb1
    bf-cfb
    camellia-128-cfb
    camellia-192-cfb
    camellia-256-cfb
    cast5-cfb
    chacha20
    idea-cfb
    rc2-cfb
    rc4-md5
    salsa20
    seed-cfb

注意: 官方软件源的shadowsocks不支持全部加密方式，官方软件源Chacha20以及salsa20的支持可以安装libsodium（For salsa20 and chacha20 support） 。若对非主流加密方式有需求，可尝试aur中的shadowsocks-nodejsAUR[broken link: archived in aur-mirror]
,
Chrome/Chromium

至此，本地监听端口127.0.0.1:1080已配置完毕。现以Chrome/Chromium为例，示范使用代理服务器的方法。

方法一：

请安装 Proxy SwitchyOmega插件（SwitchySharp已停止开发），若商店打不开的话可以直接下载Github上面的crx文件可参考该扩展提供的图解流程。

另外提供老版Proxy SwitchySharp扩展因为它诞生早，经过了更多用户的考验，且基本功能完备。

方法二：

1.直接指定Chromium走socks代理似乎不能远程dns解析，这未必是用户的期望，可使用privoxy等软件转化socks代理为http代理。

编辑privoxy配置文件（不要漏下1080后面的点)

/etc/privoxy/config

forward-socks5   /               127.0.0.1:1080 .
listen-address  127.0.0.1:8118

重启服务应用更改：

# /etc/init.d/privoxy restart

2.假设转化后的http代理为127.0.0.1:8118，则在终端中启动：

$ chromium %U --proxy-server=127.0.0.1:8118

参阅

    shadowsocks announcement
    shadowsocks@gitHub
    shadowsocks使用说明
    About Shadowsocks
    shadowsocks最新配置

Categories:

    简体中文Networking (简体中文)

Navigation menu

    Page Discussion View source History 

    Create account Log in 

Navigation

    Main page
    Categories
    Getting involved
    Wiki news
    Random page

Search
 
interaction

    Help
    Contributing
    Recent changes
    Recent talks
    New pages
    Statistics
    Reports
    Requests

Tools

    What links here
    Related changes
    Special pages
    Printable version
    Permanent link
    Page information

In other languages

    English
    日本語

    This page was last modified on 10 May 2016, at 00:27. Content is available under GNU Free Documentation License 1.3 or later unless otherwise noted. Privacy policy About ArchWiki Disclaimers 

