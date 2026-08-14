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

