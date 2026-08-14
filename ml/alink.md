# PyAlink 使用介绍

REF: https://github.com/alibaba/Alink

```
使用前准备：
包名和版本说明：

PyAlink 根据 Alink 所支持的 Flink 版本提供不同的 Python 包： 其中，pyalink 包对应为 Alink 所支持的最新 Flink 版本，而 pyalink-flink-*** 为旧版本的 Flink 版本，当前提供 pyalink-flink-1.9。
Python 包的版本号与 Alink 的版本号一致，例如1.1.0。
安装步骤：

确保使用环境中有Python3，版本限于 3.6 和 3.7。
确保使用环境中安装有 Java 8。
使用 pip 命令进行安装： pip install pyalink 或者 pip install pyalink-flink-1.9。
安装注意事项：

pyalink 和 pyalink-flink-*** 不能同时安装，也不能与旧版本同时安装。 如果之前安装过 pyalink 或者 pyalink-flink-***，请使用pip uninstall pyalink 或者 pip uninstall pyalink-flink-*** 卸载之前的版本。
出现pip安装缓慢或不成功的情况，可以参考这篇文章修改pip源，或者直接使用下面的链接下载 whl 包，然后使用 pip 安装：
Flink 1.10：链接 (MD5: 6bf3a50a4437116793149ead57d9793c)
Flink 1.9: 链接 (MD5: e6d2a0ba3549662d77b51a4a37483479)
如果有多个版本的 Python，可能需要使用特定版本的 pip，比如 pip3；如果使用 Anaconda，则需要在 Anaconda 命令行中进行安装。
```
