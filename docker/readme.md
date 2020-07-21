ref: http://www.tuicool.com/articles/JBnQja

Best source  https://developer.basespace.illumina.com/docs/content/documentation/native-apps/manage-docker-image

# list installed images

```
sudo docker iamges
```
# list running containers
```
sudo docker ps
```
# search public containers repository
```
#eg:
sudo docker search cuda
```
# Download public container to local
```
sudo docker pull container_name
```
# Run a local container with port forwarding and folder sharing:

```
sudo docker run -p 30011:30011 -p 30012:30012 -it -v /localfolder/:/container_folder/ contaienr_image_name_or_id
```
# if program hangs after run -it, try launch /bin/bash :

```
sudo docker run -it contaienr_image_name_or_id /bin/bash
```

# attach to a running container
```
sudo docer attach ######
```

# export and import local iamges
``` #export/import a saved container
    sudo docker commit whatever
    sudo docker export $CONTAINER_ID > image.tar
    cat image.tar > sudo docker import - image_flat.tar
```
``` #save/load an images
    docker save image_name > ~/save.tar
    docker load < /home/save.tar
```

# change default location of docker
```
docker ps -q | xargs docker kill
#stop docker
sudo service docker stop
cd /var/lib/docker/devicemapper/mnt
umount ./*
mv /var/lib/docker $dest
ln -s $dest /var/lib/docker
start docker
```
# Can docker work with cuda ?
Yes, check the [nvidia-docker](https://github.com/NVIDIA/nvidia-docker).

In short:
1) install the latest docker

```
Prerequisites

To install Docker CE, you need the 64-bit version of one of these Ubuntu versions:

    Yakkety 16.10
    Xenial 16.04
    Trusty 14.04

1. Set up the repository

Set up the Docker CE repository on Ubuntu. The lsb_release -cs sub-command prints the name of your Ubuntu version, like xenial or trusty.

sudo apt-get -y install \
  apt-transport-https \
  ca-certificates \
  curl

curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -

sudo add-apt-repository \
       "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
       $(lsb_release -cs) \
       stable"

sudo apt-get update

2. Get Docker CE

Install the latest version of Docker CE on Ubuntu:

sudo apt-get -y install docker-ce

```
2) install nvidia-docker
```
wget -P /tmp https://github.com/NVIDIA/nvidia-docker/releases/download/v1.0.1/nvidia-docker_1.0.1-1_amd64.deb
```
3) install driver on you host machine
4) pull a cuda enabled images 








# Others:

## start/restart docker
```
sudo docker stop ID
```
## kill all docker ps
```
sudo docker kill $(sudo docker ps -q)   
```
## delete a docker iamge
```
sudo docker rm ID/NAME
```

## show docker logs
```
sudo docker logs ID/NAME  
```


## Run docker with cuda 

```
#better use nvidia-docker
sudo docker run -ti --device /dev/nvidia0:/dev/nvidia0 --device /dev/nvidiactl:/dev/nvidiactl --device /dev/nvidia-uvm:/dev/nvidia-uvm <built-image> ./deviceQuery
```

## find ip address of docker from host
```
docker inspect <container id>
```

## Publish images to docker clould
1. create an account in https://cloud.docker.com
2. create an enmpty repositorie(new-repo) online
3. sudo docker login with **user_name** (not the email account)
4. tag a lcoal iamge with X
```
sudo docker tag local-image:tagname new-repo:tagname
```
5. docker push to the cloud
```
sudo docker push new-repo:tagname
```
# Change password a auto launched jupyter notebook
```
add option "-e PASSWORD=password" to set the environment variable. The set password is then the password for the jupyter login. 
```
REF: http://stackoverflow.com/questions/41202983/login-password-required-to-access-jupyter-notebook-running-in-nvidia-docker-cont

# run task during system startup
1
```
sudo nano /etc/rc.local
REF [How can I make “rc.local” run on startup?](http://askubuntu.com/questions/9853/how-can-i-make-rc-local-run-on-startup)
```
Or 2
```
cp runserver.sh /etc/init.d/
sudo update-rc.d runserver.sh defaults 90
#to remove
sudo update-rc.d -f runserver.sh remove
```
## example: start a docker image during startup

sudo cat /etc/init.d/runserver.sh 
```
#!/bin/bash
nvidia-docker run -p 30011:30011 -p 30012:30012 -tdi -v /home/aicyber/face_release/:/data/ stevendu/nvida_docker_caffe_tensorflow_keras_scikit_learn /data/runme.indocker.sh
```
sudo cat /home/aicyber/face_release/runme.indocker.sh
```
#!/bin/bash
export PATH=/usr/bin:/usr/local/bin:/bin:/sbin:/usr/sbin/:/usr/local/sbin:/usr/local/cuda/bin:$PATH
export PYTHONIOENCODING=utf-8 
export C_INCLUDE_PATH=/usr/local/cuda-8.0/include/      
export CPLUS_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export CUDA_HOME=/usr/local/cuda 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/cuda/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/downloads/cuda-8.0/lib64
export PYTHONPATH=/root/downloads/caffe/build/install/python:$PYTHONPATH
export CUDA_CUDA_LIBRARY=/usr/local/cuda/lib

cd /data/development_face_cut_and_embedding_v1_facenet
nohup python face_cut_server.py &
cd /data/facenet/src/
python face_v_server_extract_features_tf.py
```


# Docker 中国镜像

```
创建或修改 /etc/docker/daemon.json 文件，修改为如下形式
{
  "registry-mirrors": [
    "https://registry.docker-cn.com",
    "http://hub-mirror.c.163.com",
    "https://docker.mirrors.ustc.edu.cn"
  ]
}



$ sudo systemctl daemon-reload
$ sudo systemctl restart docker
$ sudo docker info
```
