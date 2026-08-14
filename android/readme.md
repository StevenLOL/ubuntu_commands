# Android Studio

## 1. What is it / What is it for?

Android Studio is the official IDE for Android app development, built on IntelliJ IDEA. It bundles the Android SDK, an emulator, and build tooling (Gradle).


- Writing, building, and debugging Android applications.
- Downloading/updating the Android SDK platform images and build-tools via the **SDK Manager**.

## 2. How to download / install

Download from the official site: https://developer.android.com/studio

On Ubuntu/Debian:

```bash
# Download the latest Linux bundle, then unzip
unzip android-studio-*.zip -d ~/

# Launch the setup wizard
~/android-studio/bin/studio.sh
```

To install the IDE system-wide, the first-run wizard offers "Create a desktop entry".

## 3. How to use

Start a project, then use **SDK Manager** (`Tools → SDK Manager`) to fetch platforms/build-tools. Build and run with `Run ▸ Run 'app'` or the emulator.

