# Android Studio

## 1. What is it?

Android Studio is the official IDE for Android app development, built on IntelliJ IDEA. It bundles the Android SDK, an emulator, and build tooling (Gradle).

## 2. What is it for?

- Writing, building, and debugging Android applications.
- Downloading/updating the Android SDK platform images and build-tools via the **SDK Manager**.

## 3. How to download / install

Download from the official site: https://developer.android.com/studio

On Ubuntu/Debian:

```bash
# Download the latest Linux bundle, then unzip
unzip android-studio-*.zip -d ~/

# Launch the setup wizard
~/android-studio/bin/studio.sh
```

To install the IDE system-wide, the first-run wizard offers "Create a desktop entry".

## 4. How to use

Start a project, then use **SDK Manager** (`Tools → SDK Manager`) to fetch platforms/build-tools. Build and run with `Run ▸ Run 'app'` or the emulator.

## 5. Pitfalls

If you are behind a restricted network (e.g. Google blocked), the SDK download will fail. Configure a mirror in the SDK Manager before downloading:

```
http://mirrors.opencas.cn            port:80
http://mirrors.opencas.org           port:80
http://mirrors.opencas.ac.cn         port:80
http://android-mirror.bugly.qq.com   port:8080
```

Steps:

1. Open **Android SDK Manager**, go to `Tools → Options...` (`Android SDK Manager - Settings`).
2. Fill **HTTP Proxy Server** / **HTTP Proxy Port** with one of the mirrors above, and tick **"Force https://... sources to be fetched using http://..."**.
3. Click **Close**, then **Packages → Reload**.

[REF](http://ask.android-studio.org/?/article/34)
