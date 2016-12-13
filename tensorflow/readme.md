
#install bazel for ubuntu 15.. 14..

##install java 1.8 orcal

```
$ sudo add-apt-repository ppa:webupd8team/java
$ sudo apt-get update
$ sudo apt-get install oracle-java8-installer
```
Note: You might need to sudo apt-get install software-properties-common if you don't have the add-apt-repository command

##Add Bazel distribution URI as a package source (one time setup)

```
$ echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
$ curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -

#Update and install Bazel

$ sudo apt-get update && sudo apt-get install bazel
```