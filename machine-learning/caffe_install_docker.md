REF http://www.open-open.com/lib/view/open1455241083495.html

# Build a Caffe environment with Docker

Published 2016-02-12. Source: http://blog.csdn.net//chenriwei2/article/details/50250685

## Why Docker

Docker has been around for a while. When setting up a Caffe environment, it is the right time to use it. The goal is an isolated container with all Caffe dependencies installed, which you can run directly whenever you need to train or test.

Advantage: it resolves dependency conflicts — e.g. one component needs gcc 4.7 while another needs gcc 4.8, which cannot coexist in a single host environment.

## Install Docker

See the install / usage notes for Docker basics.

## Build the image

Two ways to build an image:
1. Write a Dockerfile — good for sharing.
2. `commit` from a running container — simpler, but not easy to share.

Because the Caffe environment needs many dependency packages, and due to network issues they often cannot all be installed in one pass, we start a base container and install dependencies interactively for convenience.

Use `ubuntu:14.04` as the base:

```bash
sudo docker run --dns 8.8.8.8 --dns 8.8.4.4 --name ubuntu_caffe -i -t ubuntu:14.04 /bin/bash
```

The DNS change is needed so the container can reach the network.

## Install dependencies

Once inside the container, operations are the same as a normal host. First install basic tools:

```bash
apt-get install wget
apt-get install unzip
apt-get install python-pip
```

Create a working path; all Caffe code goes here:

```bash
mkdir <YOUR_HOME>/Caffe
cd <YOUR_HOME>/Caffe
```

### OpenCV install

There is a one-click installer on GitHub, but it does not work inside the container, so the steps below are run one by one.

```bash
# run OpenCV install step by step
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
```

### Caffe and Python dependencies

```bash
sudo apt-get install libprotobuf-dev libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get install --no-install-recommends libboost-all-dev
sudo apt-get install libgflags-dev libgoogle-glog-dev liblmdb-dev
sudo apt-get install libatlas-base-dev
```

### Install CUDA

To import data files from the host into the container, copy them (run on the host):

```bash
sudo cp cuda_7.5.18_linux.run /var/lib/docker/aufs/mnt/<CONTAINER_ID><CONTAINER_HOME>/Caffe
sudo cp cudnn-7.0-linux-x64-v3.0-prod.tgz /var/lib/docker/aufs/mnt/<CONTAINER_ID><CONTAINER_HOME>/Caffe
sudo cp caffe-master.zip /var/lib/docker/aufs/mnt/<CONTAINER_ID><CONTAINER_HOME>/Caffe
```

Get the full container id with:

```bash
docker inspect -f '{{.Id}}' ubuntu_caffe   # ubuntu_caffe is the container name
```

```bash
./cuda_*_linux.run -extract=`pwd`
./NVIDIA-Linux-x86_64-*.run -s --no-kernel-module
./cuda-linux64-rel-*.run -noprompt
```

### Install cuDNN

```bash
tar -xvf cudnn-7.0-linux-x64-v3.0-prod.tgz
cp cuda/include/cudnn.h /usr/local/cuda/include/
cp cuda/lib64/* /usr/local/cuda/lib64/
```

### Install Caffe

```bash
cd caffe-15.12.07/
cp Makefile.config.example Makefile.config
vi Makefile.config
make all
make test
```

### Install Caffe Python binding

```bash
cd python/
apt-get install python-pip
for req in $(cat requirements.txt); do pip install $req; done
sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose
```

Some packages need manual install (the loop above may fail for them):

```bash
apt-get install python-scipy
for req in $(cat requirements.txt); do pip install $req; done
pip install -U scikit-learn
for req in $(cat requirements.txt); do pip install $req; done
pip install scikit-image
for req in $(cat requirements.txt); do pip install $req; done
make pycaffe
```

## Commit the image

After everything is set up, commit the container:

```bash
docker commit <CONTAINER_ID> my-ubuntu-caffe
```

This creates a ready-to-use Caffe GPU container in the local environment.

## Run the Caffe container

```bash
sudo docker run -ti \
    --device /dev/nvidia0:/dev/nvidia0 \
    --device /dev/nvidiactl:/dev/nvidiactl \
    --device /dev/nvidia-uvm:/dev/nvidia-uvm \
    -v /media/crw/MyBook:/media/crw/MyBook \
    my-ubuntu-caffe /bin/bash
```

Benefits: (1) direct interactive use; (2) file mapping — mount a host disk into the container path, set identical paths to reduce friction.

## Train a Caffe model

If CUDA is not found, set the environment variables:

```bash
export CUDA_HOME=/usr/local/cuda
export LD_LIBRARY_PATH=${CUDA_HOME}/lib64
PATH=${CUDA_HOME}/bin:${PATH}
export PATH
```

```bash
cd /media/crw/MyBook/Experience/FaceRecognition/Softmax/try3_3
./train.sh
```

Done.
