# Mono / MonoDevelop (.NET on Linux)

## 1. What is it / What is it for?

**Mono** is an open-source implementation of Microsoft's .NET Framework, letting you run/compile C# and other .NET apps on Linux. **MonoDevelop** is the cross-platform IDE for it (now succeeded by Visual Studio for Mac). This folder also holds build scripts.


- Running .NET applications on Ubuntu.
- Developing C# apps in MonoDevelop.
- Hosting ASP.NET apps under Apache (`mod_mono`).

## 2. How to download / install

**Runtime + IDE (apt):**
```bash
sudo apt-get install mono-runtime mono-complete monodevelop \
  build-essential automake checkinstall intltool \
  mono-addins-utils gtk-sharp2 gnome-sharp2
```
(See `runtime-install.sh` in this folder.)

**MonoDevelop 7.1 via Flatpak** (see `readme.md` snippet):
```bash
sudo add-apt-repository ppa:alexlarsson/flatpak
sudo apt update && sudo apt install flatpak
flatpak install --user --from https://download.mono-project.com/repo/monodevelop.flatpakref
flatpak run com.xamarin.MonoDevelop
```
Ref: https://www.monodevelop.com/download/linux/

**Build MonoDevelop from source** (see `mono-developer-git-clone.sh`):
```bash
sudo git clone github.com/mono/monodevelop.git
cd monodevelop
sudo git submodule init && sudo git submodule update
sudo apt-get install mono-gmcs monodevelop
```

## 3. How to use

```bash
mcs hello.cs            # compile C#
mono hello.exe          # run it
```

ASP.NET on Apache uses `mod_mono` (see `mono_on_apache.md`).

