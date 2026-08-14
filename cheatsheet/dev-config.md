# Dev environment config files (examples)

A collection of dotfile snippets for a DL/speech dev box: PATH setup, Java/CUDA/Spark/Hadoop env, Theano/Makefile/pip/tmux configs. Copy what you need into your own dotfiles. Replace `<YOUR_HOME>` with your real home path.

## ~/.bashrc (PATH + env)
```
PATH=<YOUR_HOME>/apps/getmfcc/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/bin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/fgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/gmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/kwsbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/nnet2bin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/onlinebin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/featbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/fstbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/ivectorbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/latbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/nnetbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmm2bin:$PATH
xinput set-button-map 8 1 2 3 0 0
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 60
update-alternatives --config g++
sudo update-initramfs -c -k all -u
export HADOOP_HOME=/usr/local/hadoop
export PATH=$PATH:$HADOOP_HOME/bin
export JAVA_HOME=/usr/lib/jvm/java-8-oracl
export CUDA_HOME=/usr/local/cuda
export C_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export CPLUS_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export SPARK_HOME="/data/apps/spark-2.0.1-bin-hadoop2.7"
# allow piping python output to a file in utf-8 (http://stackoverflow.com/questions/13481582)
export PYTHONIOENCODING=utf-8
```

## /etc/profile
```
export GREP_OPTIONS=--color=auto
export LD_LIBRARY_PATH=/usr/local/cuda-8.0/lib64:/usr/lib64/:$LD_LIBRARY_PATH
export JAVA_HOME=/usr/lib/jvm/jdk1.8.0_101
export JRE_HOME=${JAVA_HOME}/re
export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib
export PATH=${JAVA_HOME}/bin:$PATH
PS1="`whoami`@`hostname`:"'$PWD]'
history
USER_IP=`who -u am i 2>/dev/null| awk '{print $NF}'|sed -e 's/[()]//g'`
if [ "$USER_IP" = "" ]; then USER_IP=`hostname`; fi
if [ ! -d /tmp/dbasky ]; then mkdir /tmp/dbasky; chmod 777 /tmp/dbasky; fi
if [ ! -d /tmp/dbasky/${LOGNAME} ]; then mkdir /tmp/dbasky/${LOGNAME}; chmod 300 /tmp/dbasky/${LOGNAME}; fi
export HISTSIZE=4096
DT=`date "+%Y-%m-%d_%H:%M:%S"`
export HISTFILE="/tmp/dbasky/${LOGNAME}/${USER_IP} dbasky.$DT"
chmod 600 /tmp/dbasky/${LOGNAME}/*dbasky* 2>/dev/null
```

## ~/.theano.rc
```
[global]
floatX = float32
device = gpu0
mode= FAST_RUN

[nvcc]
fastmath = True

[dnn.conv]
algo_fwd = time_once
algo_bwd_data = time_once
algo_bwd_filter = time_once
```

## ~/.pip/pip.conf
```
[global]
download_cache = /data/.cache/pip
index-url = https://pypi.douban.com/simple
```
> Note: Douban mirror is retired; use `https://pypi.tuna.tsinghua.edu.cn/simple` instead.

## ~/.tmux.conf
```
setw -g mouse on
```

## ~/.tmat.conf
```
# more in folder tmux_tmate
set -g tmate-server-host "x.x.x.x"
set -g tmate-server-port your port
set -g tmate-server-rsa-fingerprint   "x...xxx.x.x.x"
set -g tmate-server-ecdsa-fingerprint "xxxx.x.x.x.x.x"
#set -g tmate-identity ""              # Can be specified to use a different SSH key.
```
