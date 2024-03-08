### ref [https://blog.csdn.net/SongyaoLee/article/details/132873034](https://blog.csdn.net/lx_ros/article/details/123597208)https://blog.csdn.net/lx_ros/article/details/123597208

# Conda使用指南
1.Conda是什么？
2.管理Conda
3.使用conda实现环境管理
4.包管理
5.配置管理

# 1.Conda是什么？

Conda是Anaconda中的一个开源的包和环境管理工具，可以在终端窗口通过命令行使用，也可以在Anaconda Navigator中通过图形化界面使用,对编程创建独立的环境和包管理，最初是为Python语言开发，现在已不限制语言，支持Python, R, Ruby, Lua, Scala, Java, JavaScript, C/ C++, FORTRAN等。

# 2.管理Conda
查看版本
···
conda --version
···

更新至最新版本
conda update conda

更新anaconda
conda update anaconda

# 3.使用conda实现环境管理
默认创建base环境

创建环境
conda create --name your-env

创建环境并同时安装指定包
conda create --name your-env your-pkg
conda create --name snakes python=3.5


激活环境
conda activate your-env

取消激活环境
conda deactivate

查看已经创建的环境
conda info --envs

完整的删除一个环境
conda remove --name ENVNAME --all

复制1个环境
conda create --clone ENVNAME --name NEWENV

将环境导出到yaml文件，用于创建新的环境
conda env export --name ENVNAME > envname.yml
conda env create -f=/path/to/environment.yml -n your-env-name


查看某个环境的修订版
conda list --revisions

将一个环境恢复到指定版本
conda list --name ENVNAME --revisions
conda install --name ENVNAME --revision
REV_NUMBER


# 4.包管理
查看一个未安装的包在Anaconda库中是否存在
conda search pkg-name

安装一个包
conda install pkg-name

查看刚安装的包是否存在
conda list

查看某个环境下的包
conda list --name ENVNAME

将当前环境下包的列表导出指定文件，用于创建新的环境
conda create --name NEWENV --file pkgs.txt

更新某个环境下的所有包
conda update --all --name ENVNAME

删除某个环境下的包
conda uninstall PKGNAME --name ENVNAME

一次安装多个包
conda install --yes PKG1 PKG2

安装指定版本的包
# 在当前通道查找大于3.1.0小于3.2的包
conda search PKGNAME=3.1 "PKGNAME
[version='>=3.1.0,<3.2']"
# 使用ananconda 客户端，在所有通道下模糊查找某个包
anaconda search FUZZYNAME
# 从指定通道中安装某个包
conda install conda-forge::PKGNAME
# 安装指定版本的包
conda install PKGNAME==3.1.4
# 限定包的版本范围
conda install "PKGNAME[version='3.1.2|3.1.4']"
conda install "PKGNAME>2.5,<3.2"

# 5.配置管理
conda使用的源管理，查看
conda config --show channels

增加源，解决下载慢的问题
conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/

移除源
conda config --remove channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/

清除索引缓存
conda clean -i

常用源
默认源：
https://repo.anaconda.com/

清华源：
channels:
  - defaults
show_channel_urls: true
default_channels:
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/r
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/msys2
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/bioconda/
  - https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/menpo/
custom_channels:
  conda-forge: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  msys2: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  bioconda: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  menpo: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  pytorch: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud
  simpleitk: https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud



使用yaml形式的配置文件.condarc


.condarc文件通常在这些目录下：
支持的配置见手册
if on_win:
 SEARCH_PATH = (
     'C:/ProgramData/conda/.condarc',
     'C:/ProgramData/conda/condarc',
     'C:/ProgramData/conda/condarc.d',
 )
 else:
 SEARCH_PATH = (
     '/etc/conda/.condarc',
     '/etc/conda/condarc',
     '/etc/conda/condarc.d/',
     '/var/lib/conda/.condarc',
     '/var/lib/conda/condarc',
     '/var/lib/conda/condarc.d/',
  )

 SEARCH_PATH += (
     '$CONDA_ROOT/.condarc',
     '$CONDA_ROOT/condarc',
     '$CONDA_ROOT/condarc.d/',
     '~/.conda/.condarc',
     '~/.conda/condarc',
     '~/.conda/condarc.d/',
     '~/.condarc',
     '$CONDA_PREFIX/.condarc',
     '$CONDA_PREFIX/condarc',
     '$CONDA_PREFIX/condarc.d/',
     '$CONDARC',
 )

