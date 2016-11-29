REF http://www.open-open.com/lib/view/open1455241083495.html

基于docker的caffe环境搭建
  2016-02-12 09:39:09 发布
您的评价: 	
     
	4.0 	
     1收藏

来自： http://blog.csdn.net//chenriwei2/article/details/50250685

引言

为什么要用docker， 熟悉docker 其实已经很久了，自从实习的时候觉得它是一个利器以来一直没有机会使用它，这几天在折腾caffe环境中，觉得是时候用上Docker了。

需求：构建一个独立的容器，里面安装了caffe 的所有依赖，在需要跑代码的时候，直接运行它。

优点：可以解决各种依赖问题，比如这个软件需要安装gcc 4.7 ，而另外一个需要安装gcc 4.8 等等互斥的环境需求。
Docker 安装

docker 的安装和基本使用，可以参照我上面的两篇博客：安装篇 和 使用篇。
构建镜像

构建镜像有两种方法：

1，写Dockerfile,优点是便于共享；

2，从容器中commit，优点是简单方便，但是不方便共享。

由于安装caffe环境需要众多的依赖包，而且由于网络方面的原因，这些依赖包往往不能一次够装好，为了方便起见，这里直接启动一个基本的容器，然后从里面安装依赖包。

这边选择基本的ubuntu:14.04 来开始构建环境，
1，启动容器：

sudo docker run --dns 8.8.8.8 --dns 8.8.4.4 --name ubuntu_caffe -i -t ubuntu:14.04 /bin/bash

需要更改dns，是因为如果没有的话，容器上不了网。
2，依赖包安装

进入容器之后，所有的操作就跟普通的一个样了。
首先安装一些基本的工具：

apt-get install wget
apt-get install unzip
apt-get install python-pip

新建路径，后面所有的caffe代码都放在这里

mkdir /home/crw/Caffe
cd /home/crw/Caffe

opencv 的安装：
github上面有一个一键安装的，但是在容器里面一键安装不了，后面我是把它里面的脚本一句一句的搞出来执行。

#opencv一句一句的运行
arch=$(uname -m)
if [ "$arch" == "i686" -o "$arch" == "i386" -o "$arch" == "i486" -o "$arch" == "i586" ]; then flag=1; else flag=0; fi
echo "Installing OpenCV 2.4.9"
mkdir OpenCV
cd OpenCV
sudo apt-get -y install libopencv-dev
sudo apt-get -y install build-essential checkinstall cmake pkg-config yasm
sudo apt-get -y install libtiff4-dev libjpeg-dev libjasper-dev
sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev libxine-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev
sudo apt-get -y install python-dev python-numpy
sudo apt-get -y install libtbb-dev
sudo apt-get -y install libqt4-dev libgtk2.0-dev
sudo apt-get -y install libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev
wget http://archive.ubuntu.com/ubuntu/pool/multiverse/f/faac/faac_1.28-6.debian.tar.gz
vi /etc/hosts
ifconfig
sudo apt-get -y install x264 v4l-utils ffmpeg
wget -O OpenCV-2.4.9.zip http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip/download
unzip OpenCV-2.4.9.zip
cd opencv-2.4.9
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..
make -j4
sudo make install
sudo sh -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
cd ..

caffe 和python依赖包：

sudo apt-get install libprotobuf-dev libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get install --no-install-recommends libboost-all-dev
sudo apt-get install libgflags-dev libgoogle-glog-dev liblmdb-dev
sudo apt-get install libatlas-base-dev

安装cuda：
这里面个小技巧就是，如何从host 主机导入数据到容器中，可以使用如下命令（在host上运行）

sudo cp cuda_7.5.18_linux.run /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2/home/crw/Caffe

sudo cp cudnn-7.0-linux-x64-v3.0-prod.tgz /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2/home/crw/Caffe

sudo cp caffe-master.zip /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2/home/crw/Caffe

其中，那一长串数字，是你的容器的完全id，可以用命令

docker inspect -f   '{{.Id}}' ubuntu_caffe #ubuntu_caffe 是容器的名字

./cuda_*_linux.run -extract=`pwd`
./NVIDIA-Linux-x86_64-*.run -s --no-kernel-module
./cuda-linux64-rel-*.run -noprompt

安装cudnn：

tar -xvf cudnn-7.0-linux-x64-v3.0-prod.tgz 
cp cuda/include/cudnn.h /usr/local/cuda/include/
cp cuda/lib64/* /usr/local/cuda/lib64/

安装caffe：

cd caffe-15.12.07/
cp Makefile.config.example Makefile.config
vi Makefile.config
make all
make test

安装caffe 的python绑定

cd python/

apt-get install python-pip
for req in $(cat requirements.txt); do pip install $req; done

sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose

下面这几个包需要手动自己去装，用上面的命令一直装不好。

apt-get install python-scipy
for req in $(cat requirements.txt); do pip install $req; done
pip install -U scikit-learn
for req in $(cat requirements.txt); do pip install $req; done
pip install scikit-image
for req in $(cat requirements.txt); do pip install $req; done

然后

make pycaffe

最后，
新建一个路径，以便于磁盘映射

mkdir /media/crw/MyBook

所有的环境都做好了之后，就可以commit 了

docker commit 92562f161e519 my-ubuntu-caffe

这样，就在本地环境中，制作好了一个转好caffe gpu环境的容器了。
启动caffe启动容器：

sudo docker run -ti \
    --device /dev/nvidia0:/dev/nvidia0 \     --device /dev/nvidiactl:/dev/nvidiactl \     --device /dev/nvidia-uvm:/dev/nvidia-uvm \     -v /media/crw/MyBook:/media/crw/MyBook \     my-ubuntu-caffe  /bin/bash

1，显卡直通使用，
2，文件映射，挂载一个host主机的磁盘到容器路径下，这里设置为相同，可以减少一些不必要的麻烦。
运行caffe模型训练

会有提示说找不到cuda啥的，设置一下环境变量即可。

$ export CUDA_HOME=/usr/local/cuda
$ export LD_LIBRARY_PATH=${CUDA_HOME}/lib64
$ PATH=${CUDA_HOME}/bin:${PATH}
$ export PATH

cd /media/crw/MyBook/Experience/FaceRecognition/Softmax/try3_3
./train.sh

Done~
