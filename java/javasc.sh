sudo update-alternatives --install "/usr/bin/java" "java" "/usr/lib/jvm/jdk1.7.0/jre/bin/java" 1
#      // --install symlink name path priority
sudo update-alternatives --install "/usr/bin/javac" "javac" "/usr/lib/jvm/jdk1.7.0/bin/javac" 1
sudo update-alternatives --install "/usr/bin/javaws" "javaws" "/usr/lib/jvm/jdk1.7.0/jre/bin/javaws" 1
 
#// Use this Oracle JDK/JRE as the default
sudo update-alternatives --set java /usr/lib/jvm/jdk1.7.0/jre/bin/java
#      // --set name path
sudo update-alternatives --set javac /usr/lib/jvm/jdk1.7.0/bin/javac
sudo update-alternatives --set javaws /usr/lib/jvm/jdk1.7.0/jre/bin/javaws
