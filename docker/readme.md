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
# Download public contaienr to local
```
sudo docker pull container_name
```
# Run a local container with port forwarding and folder sharing:

```
sudo docker run -p 30011:30011 -p 30012:30012 -it -v /localfolder/:/container_folder/ contaienr_image_name_or_id
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
#only support 14.04 and 16.04 (TBC)
sudo add-apt-repository \
       "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
       $(lsb_release -cs) \
       stable"
sudo apt-get install docker-ce
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
