# SQLite (embedded SQL database)

## 1. What is it?

SQLite is a self-contained, serverless SQL database engine stored in a single file. It's the most-deployed database in the world (phones, browsers, apps).

## 2. What is it for?

- Embedding a relational DB in an application without running a server.
- Quick local data storage, prototyping, and small datasets.

## 3. How to download / install

```bash
sudo apt-get install sqlite3
# Python has sqlite3 built in (no install needed)
```

## 4. How to use

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

## 5. Pitfalls

- **Single-writer only**: SQLite locks the whole DB on writes; it's not for high-concurrency servers (use Postgres/MySQL instead).
- **No type enforcement** beyond storage class (dynamic typing) — declare columns carefully.
- **File permissions**: the process needs read/write on the `.db` file *and* its directory (journals).
- The original `readme.md` only linked to an external article (now dead); this note replaces it with working commands.
