
sudo apt-get install gridengine-client gridengine-common gridengine-master gridengine-qmon 
sudo apt-get install gridengine-exec

sudo nano /var/lib/gridengine/wslg/common/act_qmaster 




sudo /etc/init.d/gridengine-exec start
sudo /etc/init.d/gridengine-master  start
sudo qmod
qsub hello_wolrd.sh  &
qstat -f

cd /usr/share/fonts/X11/100dpi/
cd /usr/share/fonts/X11/75dpi/
sudo mkfontdir
ls
xset fp+ /usr/share/fonts/X11/75dpi
qmon
sudo qmon
which qstat
ls /etc/default/

after install restart





安装 Sun Grid Engine 需要 Java 的支持（最好是 SUN 的 Java 版本），所以所有结点都需要安装 Java 包。Ubuntu 从 11.10 版本不在提供 SUN 的官方 Java 版本，需要另外的源：

$ sudo apt-get install python-software-properties
$ sudo add-apt-repository ppa:ferramroberto/java
$ sudo apt-get update
$ sudo apt-get install sun-java6-jre
首先在所有结点上修改 /etc/hosts，grid00 是主控结点，grid01 和 grid02 是执行结点（我们一般把主控结点叫做 master 或者 head，执行结点叫做 client）：

$sudo vi /etc/hosts
127.0.0.1 localhost.localdomain localhost
192.168.2.10 grid00.linuxidc.com grid00
192.168.2.11 grid01.linuxidc.com grid01
192.168.2.12 grid02.linuxidc.com grid02
在主控结点（grid00）上安装 gridengine-master 以及一些管理工具 qhost/qconf/qstat 等（包含在 gridengine-client 中）：

$ sudo apt-get install gridengine-client gridengine-common gridengine-master
如果不习惯命令行界面，可以在主控结点上装个 qmon 图形管理工具，界面如下：

$ sudo apt-get install xserver-xorg gridengine-qmon

在所有执行结点（grid01 和 grid02）上安装 gridengine-exec：

$ sudo apt-get install gridengine-exec
在主控结点上执行以下操作把所有的执行结点加到集群里：

$ sudo -i

# qconf -ah grid01
# qconf -ah grid02
在所有执行结点执行以下操作，通过 act_qmaster 这个文件识别主控结点，然后启动服务：

$ sudo echo "grid00.linuxidc.com" > /var/lib/gridengine/default/common/act_qmaster

$ sudo /etc/init.d/gridengine-exec start
最后在主控结点上查看一下是否成功：

# qhost
HOSTNAME                ARCH         NCPU  LOAD  MEMTOT  MEMUSE  SWAPTO  SWAPUS
-------------------------------------------------------------------------------
global                  -               -     -       -       -       -       -
grid00.linuxidc.com        -               -     -       -       -       -       -
grid01.linuxidc.com        lx26-amd64     24 24.00   63.0G    1.7G    3.7G     0.0
grid02.linuxidc.com        lx26-amd64      8  0.01   55.2G  642.2M    1.0G     0.0


