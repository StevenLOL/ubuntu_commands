ref: http://www.tuicool.com/articles/JBnQja

Best source  https://developer.basespace.illumina.com/docs/content/documentation/native-apps/manage-docker-image

#list installed images

sudo docker iamges

# export and import local iamges

    sudo docker export $CONTAINER_ID > image.tar
    cat image.tar > sudo docker import - image_flat.tar

# shared folder in host and vm

docker run -d -v /Users/kishore/main_folder:/test_container k3_s3:latest


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



$ sudo docker pull ubuntu

#如果想下载14.04版本的，可以这样指定

$ sudo docker pull ubuntu:14.04

6、在镜像里打印hello

$ sudo docker run ubuntu /bin/echo hello

打印出hello说明你已经成功安装docker并启动一个容器！

7、熟悉下常用命令

#查看docker环境信息

$ sudo docker version

# sudo docker -D info

#在仓库中搜索相关的镜像

$ sudo docker search centos

#查看已下载的镜像

$ sudo docker images

#启动镜像并进入容器

$ sudo docker run -itd --name=ubuntu ubuntu

run命令选项：

-t 模拟一个终端，-i进入这个容器，一般结合使用创建一个交互式容器。

-d 后台运行容器并打印ID，如果不加-i后面就不能附加进入容器

--name 为容器指定名称

run命令选项很多，先不过多介绍，在使用中你会慢慢了解到！

此时你已经创建并进入容器，怎么能退出呢？

# 按ctrl+d退出容器，会暂停容器，可以再启动（sudo docker start ID/NAME），里面数据不会丢失。

# 退出暂停容器太不符合常理啊！那么按ctrl+p+q退出docker，将不会暂停

#紧接查看运行的容器

$ sudo docker ps

#这时想进入某个容器可通过ps查看的ID进入运行的容器

$ sudo docker attach ID/NAME

#关闭运行的容器（start/restart）

$ sudo docker stop ID

#关闭所有运行的容器

$ sudo docker kill $(sudo docker ps -q)   #-q只显示ID

#删除一个容器

$ sudo docker rm ID/NAME

#删除所有容器

$ sudo docker rm $(sudo docker ps -q)

#删除一个镜像

$ sudo docker rmi ID

#查看Docker操作日志

$ sudo docker ps -a

#查看容器日志

$ sudo docker logs ID/NAME  

#查看容器配置详细信息

$ sudo docker inspect ID

此时，会返回一个json格式信息，我们可以获取指定的信息，比如获取IP地址：

$ sudo docker inspect -f '{{.NetworkSettings.IPAddress}}' ID

#列出容器内容文件状态变化情况

$ sudo docker diff ID

注：A - Add，D - Delet , C - Change

#查看容器资源使用情况

$ sudo docker stats

#免交互执行命令

$ sudo docker exec ID/NAME command


More at 

https://docs.docker.com/engine/installation/linux/ubuntulinux/

