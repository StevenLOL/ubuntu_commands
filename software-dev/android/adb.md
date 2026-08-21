# ADB (Android Debug Bridge)

## 1. What is it / What is it for?

`adb` is a command-line tool that lets a PC talk to an Android device (phone, car head-unit, TV box, emulator) over USB or TCP. It is part of the Android SDK platform-tools.


- Install/uninstall APKs on a device.
- Push/pull files between PC and device.
- Get a shell on the device (`adb shell`).
- Capture logs (`logcat`), reboot, and grant system permissions.

> The commands below use `./adb.exe` (Windows host). On Linux/macOS drop the `.exe` and use `adb`.

## 2. How to download / install

```bash
# Debian/Ubuntu (standalone platform-tools)
sudo apt install android-tools-adb android-tools-fastboot

# Or download platform-tools from Google and add to PATH
# https://developer.android.com/stools/releases/platform-tools
```

Enable **USB debugging** on the device: `Settings → About phone → tap Build number 7×`, then `Settings → Developer options → USB debugging`.

## 3. How to use

### Enter engineering / diagnostic mode
```bash
./adb.exe devices          # list connected devices
```

### Install an APK (Windows host → device)
```bash
./adb.exe install -r local_app.apk     # -r = reinstall, keep data
```

### Push files (PC → device)
```bash
./adb.exe push ./Camera /sdcard/DCIM/Camera
```

### Reboot the device
```bash
./adb.exe reboot
```

### Gain system partition access
```bash
./adb.exe root
./adb.exe remount
./adb.exe shell          # now you are in the device's system shell
```

### Launch an app by activity
```bash
./adb.exe shell am start -n com.xxx.xwallpager/.view.xActivity
```

### Capture logs
```bash
./adb.exe logcat > my.log    # large file; compress with 7zip afterwards
```
### Capture Logs by PID and save to text file
First, clear the old logs, then capture the logs for the specific app and save them to a `.txt` file:
```bash
adb logcat -c && adb logcat --pid=$(adb shell pidof -s com.example.app) > app_log.txt
```
*(Note: In Windows PowerShell, if the above command throws an error, you can execute it in two steps: first run `adb logcat -c`, then run `adb logcat --pid=(adb shell pidof -s com.example.app) > app_log.txt`)*

### Query Installed Packages
List all third-party (user-installed) packages on the device:
```bash
adb shell pm list packages -3
```

### Query the APK Installation Path
Get the absolute path of a specific package (using `com.example.app` as an example):
```bash
adb shell pm path com.example.app
```
*(Note: The path after `package:` is the APK path)*

### Pull the APK to Local Machine
Copy the APK file from the device to your local computer (e.g., saving to the D: drive):
```bash
adb pull /data/app/com.example.app/base.apk D:\app.apk
```

### Launch the Application
Use the `monkey` command to launch the app (this only requires the package name, without needing the specific Activity name):
```bash
adb shell monkey -p com.example.app -c android.intent.category.LAUNCHER 1
```

