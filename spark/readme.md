
#Download pre-build spark

http://spark.apache.org/downloads.html

#Python API

PySpark is the Python API for Spark

##Install Pyspark

[PySpark: How to install and Integrate with the Jupyter Notebook](https://www.dataquest.io/blog/pyspark-installation-guide/)

Since you have pre-build spark, no building is needed.

Use nano or vim to open ~/.bash_profile and add the following lines at the end:
```
export SPARK_HOME="$HOME/spark-1.5.1"
export PYSPARK_SUBMIT_ARGS="--master local[2]"
#and IP:
export SPARK_LOCAL_IP="192.168.0.183"
```

And finally find the spark
```
sudo pip install findspark
```

now in any notebook:
```
import findspark
findspark.init()
import pyspark
```
#Enter interative mode
/data/apps/spark-2.0.1-bin-hadoop2.7$ ./bin/pyspark

#blog on spark

www.mccarroll.net/blog/pyspark/

#Spark cluster
##Active a master node
REF: http://spark.apache.org/docs/latest/spark-standalone.html


```
./sbin/start-master.sh -i YOU_MUST_SET_THIS_VALUE
```
cat the log file, and it will says:
```
...
Starting Spark master at spark://ubuntu-1080:7077
16/10/21 14:28:37 INFO Master: Running Spark version 2.0.1
...
#and a web UI
16/10/21 14:28:38 INFO MasterWebUI: Bound MasterWebUI to 192.168.0.183, and started at http://192.168.0.183:8080
...

#on the clinet side

./sbin/start-slave.sh spark://192.168.0.183:7077

```



#To verify

In cluster open pyspark

```
MASTER=spark://192.168.0.183:7077 ./bin/pyspark 
```

