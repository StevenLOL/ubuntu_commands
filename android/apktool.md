# APK Reverse Tools Commands


## Decompile full apk

```
 java -jar apktool_3.0.2.jar d "a-latest.apk" -o decompiled_mod_full
```

## Rebuild modified project into unsigned apk 
```
java -jar apktool_3.0.2.jar b decompiled_mod_full -o a_final.apk

```

## Sign & align built apk automatically
```
java -jar uber-apk-signer.jar --apks a_final.apk
```

# Capture app crash error logs and export to log file
```
.\adb.exe logcat -s AndroidRuntime *:E > log.txt
```
