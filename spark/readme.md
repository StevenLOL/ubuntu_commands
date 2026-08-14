# Apache Spark (incl. PySpark)

## 1. What is it?

Apache Spark is a fast, distributed data-processing engine. **PySpark** is its Python API. This note covers installing prebuilt Spark, using it from Jupyter, and running a standalone cluster.

## 2. What is it for?

- Large-scale batch/stream processing (ETL, ML) on a cluster or a single machine.
- Interactive analysis from Python/Jupyter.

## 3. How to download / install

Download a prebuilt tarball (no build needed):
https://spark.apache.org/downloads.html

Set env in `~/.bash_profile`:
```bash
export SPARK_HOME="$HOME/spark-1.5.1"          # adjust to your version
export PYSPARK_SUBMIT_ARGS="--master local[2]"
export SPARK_LOCAL_IP="<PRIVATE_IP>"
```
Then:
```bash
pip install findspark
```
Ref: https://www.dataquest.io/blog/pyspark-installation-guide/

## 4. How to use

**From a notebook:**
```python
import findspark
findspark.init()
import pyspark
```

**Interactive shell:**
```bash
$SPARK_HOME/bin/pyspark
```

**Standalone cluster:**
```bash
# master
$SPARK_HOME/sbin/start-master.sh -i YOU_MUST_SET_THIS_VALUE
# worker (on each node)
$SPARK_HOME/sbin/start-slave.sh spark://<PRIVATE_IP>:7077
# verify
MASTER=spark://<PRIVATE_IP>:7077 $SPARK_HOME/bin/pyspark
```

**With HDFS:** set `HADOOP_CONF_DIR` in `$SPARK_HOME/conf/spark-env.sh` to a dir containing `hdfs-site.xml` and `core-site.xml` (commonly `/etc/hadoop/conf`).

## 5. Pitfalls

- **Version pinning**: `spark-1.5.1` / `spark-2.0.1` in the note are old; download a current release.
- **`SPARK_LOCAL_IP` / master `-i`** must be a real reachable IP; `localhost` won't work across nodes.
- **HDFS not included** — Spark uses Hadoop for HDFS; install Hadoop separately if you need it.
- **`findspark`** is only needed to point PySpark at a non-default `SPARK_HOME`; with a proper env var it's optional.
- Refs: spark.apache.org/docs/latest/spark-standalone.html · http://www.mccarroll.net/blog/pyspark/
