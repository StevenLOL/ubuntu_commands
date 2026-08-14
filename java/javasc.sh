# Register Oracle JDK with update-alternatives

## 1. What is it?

A shell snippet that registers a manually-installed Oracle JDK (`java`, `javac`, `javaws`) with Debian's `update-alternatives` so the `java` command points at it.

## 2. What is it for?

- Making a manually extracted JDK the system default `java`/`javac` without symlink hacks.
- Supporting multiple JDKs and switching between them via `update-alternatives --config java`.

## 3. How to download / install

No download — this is the registration step after you've placed the JDK under `/usr/lib/jvm/`.

## 4. How to use

```bash
sudo update-alternatives --install "/usr/bin/java"  "java"  "/usr/lib/jvm/jdk1.7.0/jre/bin/java"   1
sudo update-alternatives --install "/usr/bin/javac" "javac" "/usr/lib/jvm/jdk1.7.0/bin/javac"     1
sudo update-alternatives --install "/usr/bin/javaws" "javaws" "/usr/lib/jvm/jdk1.7.0/jre/bin/javaws" 1

# make this JDK the default
sudo update-alternatives --set java  /usr/lib/jvm/jdk1.7.0/jre/bin/java
sudo update-alternatives --set javac /usr/lib/jvm/jdk1.7.0/bin/javac
sudo update-alternatives --set javaws /usr/lib/jvm/jdk1.7.0/jre/bin/javaws
```

## 5. Pitfalls

- **Edit the path** to your real JDK location (`jdk1.7.0` is just the example).
- **Priority `1`** means it won't auto-win over higher-priority entries; use `--set` (or a higher number) to make it default.
- Only relevant for manually extracted JDKs; `apt`-installed OpenJDK registers itself automatically.
