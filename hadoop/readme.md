#INSTALL

##Java ssh rsync bashrc

```
sudo apt-get install default-jdk
sudo apt-get install ssh rsync
ssh-keygen -t dsa -P ' ' -f ~/.ssh/id_dsa
cat ~/.ssh/id_dsa.pub >> ~/.ssh/authorized_keys  #or scp  master >> to slaves
sudo mv hadoop /usr/local/hadoop

sudo nano ~/.bashrc
          #Hadoop Variables
          export JAVA_HOME=/usr/lib/jvm/java-8-oracle
          export HADOOP_HOME=/data/apps/hadoop
          export PATH=$PATH:$HADOOP_HOME/bin
          export PATH=$PATH:$HADOOP_HOME/sbin
          export HADOOP_MAPRED_HOME=$HADOOP_HOME
          export HADOOP_COMMON_HOME=$HADOOP_HOME
          export HADOOP_HDFS_HOME=$HADOOP_HOME
          export YARN_HOME=$HADOOP_HOME
          export HADOOP_COMMON_LIB_NATIVE_DIR=$HADOOP_HOME/lib/native
          export HADOOP_OPTS="-Djava.library.path=$HADOOP_HOME/lib"
          
          
nano hadoop-env.sh

          #The java implementation to use.
          export JAVA_HOME="/usr/lib/jvm/java-7-openjdk-amd64"
          
```


#create new user

sudo adduser hduser1
sudo adduser hduser1 sudo


# remove 127.0.1.1 xxxx in hosts

#format data
```
hdfs namenode -format
```
#to verify
```

jps
hdfs dfsadmin -report
```

```
#Create a new directoy 

hdfs dfs -mkdir /input

#put data in hadoop

#word count 

hadoop jar ./share/hadoop/mapreduce/hadoop-mapreduce-examples-*.jar wordcount /input -

```


#Hadoop example

An example is provide for hadoop 2.7.3, each folder represents a node, the master node is ubuntu-1080. In each user home directoy there is a hidden file name .bashrc .

