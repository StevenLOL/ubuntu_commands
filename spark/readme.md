
#Download pre-build spark

http://spark.apache.org/downloads.html

#Python API

PySpark is the Python API for Spark

#Install Pyspark

[PySpark: How to install and Integrate with the Jupyter Notebook](https://www.dataquest.io/blog/pyspark-installation-guide/)

Since you have pre-build spark, no building is needed.

Use nano or vim to open ~/.bash_profile and add the following lines at the end:
```
export SPARK_HOME="$HOME/spark-1.5.1"
export PYSPARK_SUBMIT_ARGS="--master local[2]"
#and IP:
export SPARK_LOCAL_IP="192.168.0.183"
```


#Enter interative mode
/data/apps/spark-2.0.1-bin-hadoop2.7$ ./bin/pyspark

