# Register Oracle JDK with update-alternatives

## 1. What is it / What is it for?

A shell snippet that registers a manually-installed Oracle JDK (`java`, `javac`, `javaws`) with Debian's `update-alternatives` so the `java` command points at it.


- Making a manually extracted JDK the system default `java`/`javac` without symlink hacks.
- Supporting multiple JDKs and switching between them via `update-alternatives --config java`.

## 2. How to download / install

No download — this is the registration step after you've placed the JDK under `/usr/lib/jvm/`.

## 3. How to use

```bash
sudo update-alternatives --install "/usr/bin/java"  "java"  "/usr/lib/jvm/jdk1.7.0/jre/bin/java"   1
sudo update-alternatives --install "/usr/bin/javac" "javac" "/usr/lib/jvm/jdk1.7.0/bin/javac"     1
sudo update-alternatives --install "/usr/bin/javaws" "javaws" "/usr/lib/jvm/jdk1.7.0/jre/bin/javaws" 1

# make this JDK the default
sudo update-alternatives --set java  /usr/lib/jvm/jdk1.7.0/jre/bin/java
sudo update-alternatives --set javac /usr/lib/jvm/jdk1.7.0/bin/javac
sudo update-alternatives --set javaws /usr/lib/jvm/jdk1.7.0/jre/bin/javaws
```

