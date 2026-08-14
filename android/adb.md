
## 打开Android 工程模式
## 安装包 (windows->车机)
	a) 安装包windows 侧
	b) ./adb.exe  install -r  本地apk文件
## 文件移动
	a) 文件在windows 侧
	b) ./adb.exe push ./Camera(本地目录)   /sdcard/DCIM/Camera(车机目录)
## 车机重启
	a) ./adb.exe reboot
## 获得操作系统目录权限
```
./adb.exe root
./adb.exe remount
./adb.exe shell #进入系统目录
```
## 启动某个应用
adb shell am start -n com.xxx.xwallpager/.view.xActivity
## Log
./adb.exe logcat ->my.log   #文件较大可用 7zip 压缩
