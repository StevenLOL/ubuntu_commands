# Install Java (JDK/JRE) on Ubuntu

## 1. What is it?

Instructions for installing a Java runtime/compiler — either **OpenJDK** (free, default) or **Oracle JDK** (legacy, license-restricted) — and registering it with `update-alternatives`.

## 2. What is it for?

- Running Java applications (e.g. Weka, ELAN, Android tooling) or building Java code.
- Setting `JAVA_HOME` / `CLASSPATH` and making `java`/`javac` available system-wide.

## 3. How to download / install

### OpenJDK (recommended)
```bash
sudo apt-get install openjdk-17-jdk     # or openjdk-11-jdk / openjdk-8-jdk
java -version
```

### Oracle Java (older approach — PPA may be dead)
```bash
sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer
```

### Manual Oracle JDK (tarball)
```bash
sudo mkdir -p /usr/lib/jvm/jdk1.7.0
sudo mv jdk1.7.0_xx/* /usr/lib/jvm/jdk1.7.0/
```

## 4. How to use

### Register with update-alternatives
```bash
sudo update-alternatives --install /usr/bin/java  java  /usr/lib/jvm/jdk1.7.0/jre/bin/java  1
sudo update-alternatives --install /usr/bin/javac javac /usr/lib/jvm/jdk1.7.0/bin/javac    1
sudo update-alternatives --install /usr/bin/javaws javaws /usr/lib/jvm/jdk1.7.0/jre/bin/javaws 1
# make it the default:
sudo update-alternatives --set java  /usr/lib/jvm/jdk1.7.0/jre/bin/java
sudo update-alternatives --config java     # or pick interactively
java -version
```

### Environment variables (add to ~/.bashrc)
```bash
export JAVA_HOME=/home/abc/jdk1.7.0_37
export PATH=$PATH:$JAVA_HOME/bin:$JAVA_HOME/jre/bin
export CLASSPATH=.:$JAVA_HOME/lib:$JAVA_HOME/jre/lib
```
Then `source ~/.bashrc`.

### Browser plugin (legacy, 32/64-bit)
```bash
mkdir -p ~/.mozilla/plugins
ln -s /usr/lib/jvm/jdk1.7.0/lib/amd64/libnpjp2.so ~/.mozilla/plugins/    # 64-bit
# ln -s /usr/lib/jvm/jdk1.7.0/lib/i386/libnpjp2.so ~/.mozilla/plugins/   # 32-bit
```

## 5. Pitfalls

- **`webupd8team/java` PPA is dead** — Oracle Java 8 installer there no longer works; use OpenJDK or Oracle's official tarball.
- **Version mismatch**: pin the version you need (some apps require Java 8 specifically).
- **`javasc.sh`** in this folder is just the `update-alternatives` snippet — adjust the JDK path to your actual install.
- **Browser plugin is obsolete**: modern browsers dropped NPAPI; the `libnpjp2.so` trick won't work in current Firefox/Chrome.
- **`JAVA_HOME` must point at the JDK root**, not `bin/`.
