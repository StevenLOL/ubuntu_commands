REF: http://os.51cto.com/art/201411/457319.htm
REF: http://xmodulo.com/lxc-containers-ubuntu.html
如何在Ubuntu上创建及管理LXC容器？
2014-11-18 00:45 布加迪编译 51CTO 字号：T | T
一键收藏，随时查看，分享好友！
即便没有Docker的种种好处，我喜欢LXC容器的地方在于，LXC可以由libvirt接口来管理，Docker却不是这样。如果你之前一直使用基于libvirt的管理工具（比如virt-manager或virsh），就可以使用同样那些工具来管理LXC容器。
AD：
虽然早在十多年前就引入了容器这个概念，用来安全地管理共享式主机托管环境（比如FreeBSD监狱），但LXC或Docker之类的Linux只是最近因日益需要为云计算部署应用程序而进入主流。虽然这阵子Docker备受媒体的关注，并且得到了各大云服务提供商（比如亚马逊AWS和微软Azure）以及发行版提供商（比如红帽和Ubuntu）的大力支持，但LXC实际上却是针对Linux平台开发的早期容器技术之一。

如果你是普普通通的Linux用户，Docker/LXC可以带来什么样的好处呢？嗯，容器实际上是一种在诸发行版之间几乎即时切换的一种好方法。假设你当前的桌面系统是Debian，你需要Debian的稳定性。与此同时，你又想玩最新的Ubuntu游戏。然后，用不着很麻烦地通过双启动进入到Ubuntu分区，或者启动占用大量资源的Ubuntu虚拟机，只要立即启用一个Ubuntu容器即可，一切都搞定了。

即便没有Docker的种种好处，我喜欢LXC容器的地方在于，LXC可以由libvirt接口来管理，Docker却不是这样。如果你之前一直使用基于libvirt的管理工具（比如virt-manager或virsh），就可以使用同样那些工具来管理LXC容器。

我在本教程中着重介绍标准LXC容器工具的命令行用法，并且演示如何在Ubuntu上从命令行创建及管理LXC容器。

将LXC安装到Ubuntu上

想在Ubuntu上使用LXC，就要安装LXC用户空间工具，如下所示。

$ sudo apt-get install lxc
安装完毕之后，运行lxc-checkconifg工具，检查当前Linux内核支持LXC的情况。要是一切都已被启用，内核对LXC的支持已准备就绪。

$ lxc-checkconfig


安装LXC工具后，你会发现，LXC的默认网桥接口（lxcbr0）已自动创建（已在/etc/lxc/default.conf中加以配置）。



创建LXC容器后，窗口的接口就会自动连接到该网桥，那样容器就能与外界进行联系了。

创建LXC容器

为了能够创建某个特定目标环境（比如Debian Wheezy 64位）的LXC容器，你就需要一个相应的LXC模板。幸运的是，Ubuntu上的LXC用户空间工具随带一系列预先准备好的LXC模板。你可以在/usr/share/lxc/templates目录下找到可用的LXC模板。

$ ls /usr/share/lxc/templates


LXC模板其实就是一段脚本而已，用来为某个特定的Linux环境创建容器。你在创建LXC容器时，需要用到这其中一个模板。

比如说，为了创建Ubuntu容器，使用下面这个命令行：

$ sudo lxc-create -n -t ubuntu


默认情况下，它会创建与本地主机同一版本号和同一架构的最小Ubuntu安装系统，这种情况下是Saucy Salamander（13.10）64位。

如果你希望，可以创建任何一种版本的Ubuntu容器，只要传递release参数。比如说，想创建Ubuntu 14.10容器：

$ sudo lxc-create -n -t ubuntu -- --release utopic
它会下载并验证目标容器环境需要的所有程序包。整个过程可能需要几分钟或更长时间，具体取决于容器类型。所以请耐心点。



经过一系列的程序包下载和验证后，LXC容器映像最终创建完毕，你会看到默认的登录资料可供使用。容器存储在/var/lib/lxc/<container-name>，根文件系统则位于/var/lib/lxc/<container-name>/rootfs。

LXC创建过程中下载的所有程序包则缓存在/var/cache/lxc里面，那样使用同样的LXC模板创建额外的容器就不用花时间了。

现在不妨看一下主机上的LXC容器列表：

$ sudo lxc-ls –fancy
NAME STATE IPV4 IPV6 AUTOSTART

------------------------------------

test-lxc STOPPED - - NO

想启动一个容器，请使用下面这个命令。“-d”选项将容器作为守护程序来启动。要是没有这个选项，你在启动容器后，会直接被连接到控制台。

$ sudo lxc-start -n -d
启动容器后，不妨再次检查容器状态：

$ sudo lxc-ls –fancy
NAME STATE IPV4 IPV6 AUTOSTART

-----------------------------------------

lxc RUNNING 10.0.3.55 - NO

你会看到，容器处于“RUNNING”（运行）状态，已被赋予了一个IP地址。

你还可以证实，容器的接口（比如vethJ06SFL）自动连接到LXC的内部网桥（lxcbr0），如下所示。

$ brctl show lxcbr0


管理LXC容器

既然我们已知道了如何创建及启动LXC容器，现在不妨看看我们可以如何处理运行中的容器。

首先，我们想要访问容器的控制台。为此，键入这个命令：

$ sudo lxc-console -n 


键入<Ctrl+a q>组合键，退出控制台。

想停止和销毁容器：

$ sudo lxc-stop -n 
$ sudo lxc-destroy -n 
想把现有容器克隆成另一个容器，使用这些命令：

$ sudo lxc-stop -n 
$ sudo lxc-clone -o -n 
故障排查

如果你遇到了LXC方面的错误，下面是故障排查方面的几个要点。

1. 你无法创建LXC容器，出现下列错误。

$ sudo lxc-create -n test-lxc -t ubuntu
lxc-create: symbol lookup error: /usr/lib/x86_64-linux-gnu/liblxc.so.1: undefined symbol: cgmanager_get_pid_cgroup_abs_sync
这意味着你运行最新的LXC，却使用较旧的libcgmanager（libcg管理器）。想解决这个问题，你就需要更新libcg管理器。

$ sudo apt-get install libcgmanager0
英文：http://xmodulo.com/lxc-containers-ubuntu.html

【编辑推荐】

在Ubuntu上安装nginx,MySQL,PHP,phpmyadmin和WordPress
Ubuntu助力奔驰汽车自驾系统
从Windows双启动中卸载Ubuntu Linux
Ubuntu 14.10正式发布：这次真的很无聊
Mir和融合桌面在阻碍Ubuntu的发展吗？
