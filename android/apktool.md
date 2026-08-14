# APK Reverse-Engineering Tools (apktool + uber-apk-signer)

## 1. What is it?

- **apktool**: a tool to decode (disassemble) an Android APK into (near) original form — `smali` code, resources, `AndroidManifest.xml` — and rebuild (assemble) it back into an APK.
- **uber-apk-signer**: a helper that signs and zip-aligns one or many APKs in one command (replaces the deprecated manual `jarsigner` + `zipalign` dance).

Together they let you inspect, modify, and re-package an APK.

## 2. What is it for?

- Decompile an APK to understand its structure/resources.
- Patch resources or `smali`, then rebuild into a working (re-signed) APK.
- Capture an app's crash logs via `adb logcat` during testing.

## 3. How to download / install

Download the standalone jars:

- apktool: https://ibotpeaches.github.io/Apktool/  (get `apktool_3.0.2.jar`, or latest)
- uber-apk-signer: https://github.com/patrickfav/uber-apk-signer/releases

Requires a JRE:

```bash
sudo apt install default-jre     # or default-jdk
java -version                    # confirm
```

> The examples below assume `apktool_3.0.2.jar` and `uber-apk-signer.jar` are in the current directory.

## 4. How to use

### Decompile a full APK
```bash
java -jar apktool_3.0.2.jar d "a-latest.apk" -o decompiled_mod_full
```

### Rebuild the modified project into an UNSIGNED apk
```bash
java -jar apktool_3.0.2.jar b decompiled_mod_full -o a_final.apk
```

### Sign & zip-align automatically
```bash
java -jar uber-apk-signer.jar --apks a_final.apk
```

### Capture app crash logs and export to a file
```bash
.\adb.exe logcat -s AndroidRuntime *:E > log.txt
```

## 5. Pitfalls

- **Rebuilt APK won't install ("not signed")**: APKs must be signed. Use uber-apk-signer (or `apksigner`) — an unsigned APK from `apktool b` is not installable.
- **`apktool` version mismatch**: newer AAPT/aapt2 or obfuscated resources may fail to decode; try a newer apktool build.
- **Smali edits**: editing `smali` is error-prone; a single bad instruction breaks the build. Prefer resource-only patches when possible.
- **`adb.exe` vs `adb`**: `.exe` only on Windows hosts.
- **Manifest conflicts after rebuild**: if you add activities, also register them in `AndroidManifest.xml`.
