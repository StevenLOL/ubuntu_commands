# SQLite (embedded SQL database)

## 1. What is it / What is it for?

SQLite is a self-contained, serverless SQL database engine stored in a single file. It's the most-deployed database in the world (phones, browsers, apps).


- Embedding a relational DB in an application without running a server.
- Quick local data storage, prototyping, and small datasets.

## 2. How to download / install

```bash
sudo apt-get install sqlite3
# Python has sqlite3 built in (no install needed)
```

## 3. How to use

```bash
sqlite3 mydb.db
sqlite> CREATE TABLE t(id INTEGER, name TEXT);
sqlite> INSERT INTO t VALUES (1, 'alice');
sqlite> .mode column
sqlite> SELECT * FROM t;
sqlite> .quit
```

From Python:
```python
import sqlite3
con = sqlite3.connect('mydb.db')
con.execute('CREATE TABLE IF NOT EXISTS t(id INTEGER, name TEXT)')
con.commit()
```

