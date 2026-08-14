# Build OpenCV from source (Ubuntu 12.04 reference)

## 1. What is it?

A reference recipe for compiling **OpenCV 2.4.x** from source on Ubuntu 12.04 (also works, with path tweaks, on later releases). Mirrors the official Ubuntu community OpenCV guide.

Ref: https://help.ubuntu.com/community/OpenCV

## 2. What is it for?

- Getting a custom OpenCV build with the exact modules/flags you need (GPU, Qt, TBB, Python support).
- When the pip/apt package is too old or missing a feature.

## 3. How to download / install

Install build deps:
```bash
sudo apt-get -y install build-essential cmake pkg-config
# Image I/O
sudo apt-get -y install libjpeg62-dev libtiff4-dev libjasper-dev
# GUI
sudo apt-get -y install libgtk2.0-dev
# Video I/O
sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
# Optional
sudo apt-get -y install libdc1394-22-dev
sudo apt-get -y install libxine-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
sudo apt-get -y install python-dev python-numpy
sudo apt-get -y install libtbb-dev
sudo apt-get -y install libqt4-dev
```

Download source:
```bash
mkdir xxx && cd xxx
wget http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.3/OpenCV-2.4.3.tar.bz2
tar -xvf OpenCV-2.4.*.tar.bz2
```

## 4. How to use

```bash
cd OpenCV-2.4.*
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local \
  -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON \
  -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON \
  -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..
make
sudo make install
```

> Don't forget the `..` at the end of the `cmake` command.

## 5. Pitfalls

- **Ubuntu 12.04-era package names** (`libjpeg62-dev`, `libtiff4-dev`, `libqt4-dev`) are gone on modern Ubuntu — adjust to `libjpeg-dev`, `libtiff-dev`, `libqt5-dev` etc.
- **Don't omit `..`** in cmake or it configures in the wrong directory.
- **`make` without `-j` is slow**; use `make -j$(nproc)` on multicore.
- **Python bindings**: ensure `python-dev`/`python-numpy` are present before cmake so `BUILD_NEW_PYTHON_SUPPORT` actually builds `cv2`.
- For current OpenCV, follow the official docs — this recipe targets 2.4.x.
