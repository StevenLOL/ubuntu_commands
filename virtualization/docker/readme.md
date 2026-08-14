# Docker (containers)

> Refs: http://www.tuicool.com/articles/JBnQja · https://developer.basesdk.illumina.com/docs/content/documentation/native-apps/manage-docker-image

## 1. What is it / What is it for?

Docker packages applications and their dependencies into lightweight, portable **containers** that run identically on any host. This note covers everyday Docker usage, GPU-enabled containers (nvidia-docker), registry push/pull, and moving Docker's data root.


- Running reproducible environments (e.g. a CUDA + Jupyter ML stack) without polluting the host.
- Sharing images via Docker Hub / a private registry.
- Isolating services with port forwarding and volume mounts.

## 2. How to download / install

```bash
# Prerequisites
sudo apt-get -y install apt-transport-https ca-certificates curl

curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository \
  "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"

sudo apt-get update
sudo apt-get -y install docker-ce
sudo systemctl enable --now docker
```

For GPU support install **nvidia-docker** (legacy v1 example; prefer the current `nvidia-container-toolkit`):
```bash
wget -P /tmp https://github.com/NVIDIA/nvidia-docker/releases/download/v1.0.1/nvidia-docker_1.0.1-1_amd64.deb
sudo dpkg -i /tmp/nvidia-docker_*.deb
```

> **China mirror** — create/edit `/etc/docker/daemon.json`:
> ```json
> { "registry-mirrors": [
>     "https://registry.docker-cn.com",
>     "http://hub-mirror.c.163.com",
>     "https://docker.mirrors.ustc.edu.cn" ] }
> ```
> then `sudo systemctl daemon-reload && sudo systemctl restart docker`.

## 3. How to use

### Images & containers
```bash
sudo docker images                 # list local images (note: it's `images`, not `iamges`)
sudo docker ps                     # running containers
sudo docker ps -a                  # all containers
sudo docker search cuda            # search Docker Hub (or use https://hub.docker.com/)
sudo docker pull <image>           # download an image
```

### Run
```bash
# port forwarding + folder share
sudo docker run -p 30011:30011 -p 30012:30012 -it -v /localfolder/:/container_folder/ <image>

# with GPU + memory limit
sudo docker run --gpus device=1 -m 64g -p 30012:8080 \
  -it -v /mnt/steven/:/steven/ ubuntu20.04-jupyter

# if `-it` hangs, launch bash explicitly:
sudo docker run -it <image> /bin/bash

# legacy GPU device passthrough (no nvidia-docker):
sudo docker run -ti --device /dev/nvidia0:/dev/nvidia0 \
  --device /dev/nvidiactl:/dev/nvidiactl \
  --device /dev/nvidia-uvm:/dev/nvidia-uvm <image> ./deviceQuery
```

### Attach / inspect
```bash
sudo docker attach <id>            # attach to a running container
docker inspect <id>                # shows IP, mounts, config (incl. container IP)
```

### Save / load / export / import
```bash
# image <-> tar (keeps history/layers)
docker save <image> > ~/save.tar
docker load < ~/save.tar

# container <-> tar (flattened filesystem)
sudo docker commit <container> something
sudo docker export $CONTAINER_ID > image.tar
cat image.tar | sudo docker import - image_flat.tar
```

### Lifecycle
```bash
sudo docker stop <id>
sudo docker kill $(sudo docker ps -q)      # kill all running
sudo docker rm <id|name>
sudo docker rmi -f <id|name>               # force-remove an image
sudo docker logs <id|name>
```

### Publish to a registry
```bash
# 1. create the repo at https://hub.docker.com
# 2. login with USERNAME (not email)
sudo docker login
# 3. tag and push
sudo docker tag local-image:tagname new-repo:tagname
sudo docker push new-repo:tagname
```

### Set a Jupyter password in-container
Add `-e PASSWORD=password` to the `run` command; that becomes the Jupyter login password.
Ref: https://stackoverflow.com/questions/41202983/

### Start a container at boot
```bash
# option A: rc.local
sudo nano /etc/rc.local
# option B: init.d
cp runserver.sh /etc/init.d/
sudo update-rc.d runserver.sh defaults 90
# remove: sudo update-rc.d -f runserver.sh remove
```
Example `runserver.sh`:
```bash
#!/bin/bash
nvidia-docker run -p 30011:30011 -p 30012:30012 -tdi \
  -v <HOME>/face_release/:/data/ <DOCKERHUB>/caffe_tensorflow_keras /data/runme.indocker.sh
```

### Move Docker's data root
```bash
docker ps -q | xargs docker kill
sudo service docker stop
cd /var/lib/docker/devicemapper/mnt && umount ./*
mv /var/lib/docker $dest
ln -s $dest /var/lib/docker
sudo service docker start
```

